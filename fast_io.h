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




// Запись пары ключ-значение
void write_key_value_pair(const char *filename, const char *index_key, const char *index_val) {
    int fd = open(filename, O_WRONLY | O_APPEND | O_CREAT, 0666);
    if (fd == -1) return;

    if (lock_file(fd, F_WRLCK) == -1) {
        close(fd);
        return;
    }

    dprintf(fd, "%s %s\\n", index_key, index_val);

    close(fd); // Это также разблокирует файл
}

// Удаление пары ключ-значение
void delete_key_value_pair(const char *filename, const char *index_key) {
    int fd = open(filename, O_RDWR);
    if (fd == -1) return;

    if (lock_file(fd, F_WRLCK) == -1) {
        close(fd);
        return;
    }

    FILE *file = fdopen(fd, "r+");
    if (!file) {
        close(fd);
        return;
    }

    // Создаем временный файл для копирования данных
    char temp_filename[64];
    tmpnam(temp_filename);

    int temp_fd = mkstemp(temp_filename);
    if (temp_fd == -1) {
        fclose(file);
        return;
    }

    FILE *temp_file = fdopen(temp_fd, "w");
    if (!temp_file) {
        fclose(file);
        close(temp_fd);
        unlink(temp_filename);
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        char *key = strtok(line, " ");
        if (strcmp(index_key, key) != 0) {
            fputs(line, temp_file);
        }
    }

    fclose(file);
    fclose(temp_file);

    // Переименовываем временный файл
    rename(temp_filename, filename);
}
