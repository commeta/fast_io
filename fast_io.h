#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define BUFFER_SIZE 4096




// Вспомогательная функция для блокировки файла
int lock_file(int fd, int lock_type) {
    struct flock fl;
    fl.l_type = lock_type;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0; // Блокировка всего файла
    return fcntl(fd, F_SETLKW, &fl);
}



// Функция поиска значения по ключу с чтением файла порциями
char *find_value_by_key(const char *filename, const char *index_key) {
    int fd = open(filename, O_RDONLY);
    if (fd == -1) return NULL;

    if (lock_file(fd, F_RDLCK) == -1) {
        close(fd);
        return NULL;
    }

    char buffer[BUFFER_SIZE + 1]; // Дополнительный байт для нуль-терминатора
    ssize_t bytesRead;
    char *found_value = NULL;
    char *lineStart = buffer;
    int keyLength = strlen(index_key);

    while ((bytesRead = read(fd, buffer, BUFFER_SIZE)) > 0) {
        buffer[bytesRead] = '\0'; // Завершаем прочитанный буфер нуль-терминатором

        // Обработка данных в буфере
        char *lineEnd;
        while ((lineEnd = strchr(lineStart, '\n')) != NULL) {
            *lineEnd = '\0'; // Завершаем строку нуль-терминатором

            // Проверяем, начинается ли строка с ключа
            if (strncmp(lineStart, index_key, keyLength) == 0 && lineStart[keyLength] == ' ') {
                found_value = strdup(lineStart + keyLength + 1); // Пропускаем ключ и пробел
                break;
            }

            lineStart = lineEnd + 1; // Переходим к следующей строке
        }
        if (found_value != NULL) break;

        // Если не нашли в этом буфере, подготавливаемся к чтению следующего
        size_t remaining = bytesRead - (lineStart - buffer);
        memmove(buffer, lineStart, remaining); // Перемещаем оставшуюся часть в начало буфера
        lineStart = buffer;
    }

    close(fd); 
    return found_value;
}



// Структура для хранения информации о записи в индексном файле
typedef struct {
    off_t offset; // Смещение данных в файле данных
    size_t size;  // Размер блока данных
} IndexRecord;

char *indexed_find_value_by_key(const char *filename, const char *index_key) {
    char index_filename[256];
    snprintf(index_filename, sizeof(index_filename), "%s.index", filename);

    int index_fd = open(index_filename, O_RDONLY);
    int data_fd = open(filename, O_RDONLY);
    if (index_fd == -1 || data_fd == -1) {
        if (index_fd != -1) close(index_fd);
        if (data_fd != -1) close(data_fd);
        return NULL;
    }

    // Блокировка обоих файлов
    if (lock_file(index_fd, F_RDLCK) == -1 || lock_file(data_fd, F_RDLCK) == -1) {
        close(index_fd);
        close(data_fd);
        return NULL;
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytesRead;
    char *value = NULL;

    while ((bytesRead = read(index_fd, buffer, BUFFER_SIZE)) > 0) {
        for (size_t i = 0; i < bytesRead; i += sizeof(IndexRecord) + strlen(index_key)) {
            if (strncmp(buffer + i, index_key, strlen(index_key)) == 0) {
                // Найдено совпадение ключа, читаем информацию о смещении и размере
                IndexRecord record;
                memcpy(&record, buffer + i + strlen(index_key), sizeof(IndexRecord));

                // Чтение данных из файла данных
                value = malloc(record.size + 1);
                if (value) {
                    pread(data_fd, value, record.size, record.offset);
                    value[record.size] = '\0'; // Добавляем нуль-терминатор
                }
                break;
            }
        }
        if (value != NULL) break;
    }

    // Разблокировка и закрытие файлов
    lock_file(index_fd, F_UNLCK);
    lock_file(data_fd, F_UNLCK);
    close(index_fd);
    close(data_fd);

    return value;
}



// Запись пары ключ-значение
void write_key_value_pair(const char *filename, const char *index_key, const char *index_val) {
    int fd = open(filename, O_WRONLY | O_APPEND | O_CREAT, 0666);
    if (fd == -1) return;

    if (lock_file(fd, F_WRLCK) == -1) {
        close(fd);
        return;
    }

    dprintf(fd, "%s %s\n", index_key, index_val);

    close(fd); // Это также разблокирует файл
}



void indexed_write_key_value_pair(const char *filename, const char *index_key, const char *index_val) {
    char index_filename[256];
    snprintf(index_filename, sizeof(index_filename), "%s.index", filename);

    int data_fd = open(filename, O_RDWR | O_CREAT | O_APPEND, 0644);
    int index_fd = open(index_filename, O_RDWR | O_CREAT | O_APPEND, 0644);
    if (data_fd == -1 || index_fd == -1) {
        //perror("Error opening file");
        if (data_fd != -1) close(data_fd);
        if (index_fd != -1) close(index_fd);
        return;
    }

    // Блокировка обоих файлов
    if (lock_file(data_fd, F_WRLCK) == -1 || lock_file(index_fd, F_WRLCK) == -1) {
        //perror("Error locking file");
        close(data_fd);
        close(index_fd);
        return;
    }

    // Запись значения в файл данных
    off_t offset = lseek(data_fd, 0, SEEK_END); // Получаем текущее смещение в файле данных
    size_t size = strlen(index_val);
    if (write(data_fd, index_val, size) != size) {
        //perror("Error writing to data file");
        close(data_fd);
        close(index_fd);
        return;
    }

    // Запись индекса в индексный файл
    IndexRecord record = {offset, size};
    if (write(index_fd, index_key, strlen(index_key)) != strlen(index_key) ||
        write(index_fd, &record, sizeof(IndexRecord)) != sizeof(IndexRecord)
    ) {
        //perror("Error writing to index file");
        close(data_fd);
        close(index_fd);
        return;
    }

    // Разблокировка и закрытие файлов
    lock_file(data_fd, F_UNLCK);
    lock_file(index_fd, F_UNLCK);
    close(data_fd);
    close(index_fd);
}



// Удаление пары ключ-значение с чтением файла порциями
void delete_key_value_pair(const char *filename, const char *index_key) {
    int fd = open(filename, O_RDWR);
    if (fd == -1) return;

    if (lock_file(fd, F_WRLCK) == -1) {
        close(fd);
        return;
    }


    char temp_filename[256];
    snprintf(temp_filename, sizeof(temp_filename), "%s.tmp", filename);


    int temp_fd = mkstemp(temp_filename);
    if (temp_fd == -1) {
        close(fd);
        return;
    }

    FILE *file = fdopen(fd, "r");
    FILE *temp_file = fdopen(temp_fd, "w");
    
    if (!file || !temp_file) {
        if (file) fclose(file);
        if (temp_file) fclose(temp_file);
        close(fd);
        close(temp_fd);
        unlink(temp_filename);
        return;
    }

    char buffer[BUFFER_SIZE + 1];
    size_t bytesRead;
    while ((bytesRead = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
        buffer[bytesRead] = '\0'; // Добавляем нуль-терминатор для безопасной работы со строками
        
        char *lineStart = buffer;
        char *lineEnd;
        while ((lineEnd = strchr(lineStart, '\n')) != NULL) {
            *lineEnd = '\0'; // Заменяем символ новой строки на нуль-терминатор
            
            // Проверяем, начинается ли строка с ключа
            if (strncmp(lineStart, index_key, strlen(index_key)) != 0 || lineStart[strlen(index_key)] != ' ') {
                // Если строка не начинается с ключа, копируем её во временный файл
                fprintf(temp_file, "%s\n", lineStart);
            }
            
            lineStart = lineEnd + 1; // Переходим к следующей строке
        }
        
        // Обработка случая, когда последняя строка в буфере не завершена
        if (lineStart != buffer + bytesRead) {
            fseek(file, -(bytesRead - (lineStart - buffer)), SEEK_CUR); // Возвращаемся назад в исходном файле
        }
    }
    
    fclose(file);
    fclose(temp_file);

    // Заменяем оригинальный файл временным файлом
    rename(temp_filename, filename);

    close(fd); // Это также разблокирует файл
}

