/*
 * Fast_IO (beta) Extension for PHP 8
 * https://github.com/commeta/fast_io
 * 
 * Copyright 2024 commeta <dcs-spb@ya.ru>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>


// Вспомогательная функция для блокировки файла
int lock_file(int fd, int lock_type) {
    struct flock fl;
    fl.l_type = lock_type;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0; // Блокировка всего файла
    return fcntl(fd, F_SETLKW, &fl);
}


char *find_value_by_key(const char *filename, const char *index_key) {
    size_t index_size = 256;

    int fd = open(filename, O_RDONLY);
    if (fd == -1) return NULL;

    if (lock_file(fd, F_RDLCK) == -1) {
        close(fd);
        return NULL;
    }

    char *buffer = malloc(index_size + 1); // Выделяем память под одну запись + нуль-терминатор
    if (!buffer) {
        close(fd);
        return NULL;
    }
    
    ssize_t bytesRead;
    char *found_value = NULL;
    size_t keyLength = strlen(index_key);

    while ((bytesRead = read(fd, buffer, index_size)) == index_size) {
        buffer[bytesRead] = '\0'; // Завершаем прочитанный буфер нуль-терминатором

        // Проверяем наличие ключа в текущем сегменте
        if (strncmp(buffer, index_key, keyLength) == 0 && buffer[keyLength] == ' ') {
            // Найден ключ, извлекаем значение
            char *newlinePos = strchr(buffer, '\n');
            if (newlinePos != NULL) {
                *newlinePos = '\0'; // Заменяем символ новой строки на нуль-терминатор
                found_value = strdup(buffer + keyLength + 1);
                break;
            }
        }
    }

    free(buffer);
    close(fd); 
    return found_value;
}


// Запись пары ключ-значение с выравниванием по index_size
void write_key_value_pair(const char *filename, const char *index_key, const char *index_val) {
    size_t index_size = 256;

    int fd = open(filename, O_WRONLY | O_APPEND | O_CREAT, 0666);
    if (fd == -1) return;

    if (lock_file(fd, F_WRLCK) == -1) {
        close(fd);
        return;
    }

    // Рассчитываем длину строки с учетом ключа, значения и пробела между ними
    size_t key_val_len = strlen(index_key) + strlen(index_val) + 1; // +1 для пробела
    size_t total_len = key_val_len + 1; // +1 для символа новой строки

    // Вычисляем сколько пробелов нужно добавить для выравнивания
    size_t padding = (index_size - total_len % index_size) % index_size;

    // Формируем строку с учетом выравнивания
    char *formatted_str = (char *)malloc(index_size * sizeof(char));
    if (!formatted_str) {
        close(fd);
        return;
    }
    int written = snprintf(formatted_str, index_size, "%s %s", index_key, index_val);
    memset(formatted_str + written, ' ', padding); // Заполняем пробелами
    formatted_str[written + padding] = '\n'; // Добавляем символ новой строки

    write(fd, formatted_str, written + padding + 1);

    free(formatted_str);
    close(fd); // Это также разблокирует файл
}



void delete_key_value_pair(const char *filename, const char *index_key) {
    size_t index_size = 256;

    char temp_filename[256];
    snprintf(temp_filename, sizeof(temp_filename), "%s.tmp", filename);

    int fd = open(filename, O_RDWR);
    if (fd == -1) return;

    if (lock_file(fd, F_WRLCK) == -1) {
        close(fd);
        return;
    }

    int temp_fd = open(temp_filename, O_RDWR | O_CREAT | O_APPEND, 0644);
    if (temp_fd == -1) {
        close(fd); // Закрываем fd, так как temp_fd не удалось открыть
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

    char *buffer = malloc(index_size + 1); // +1 для нуль-терминатора
    if (!buffer) {
        fclose(file);
        fclose(temp_file);
        close(fd);
        close(temp_fd);
        unlink(temp_filename);
        return;
    }

    size_t bytesRead;
    while ((bytesRead = fread(buffer, 1, index_size, file)) == index_size) {
        buffer[bytesRead] = '\0'; // Добавляем нуль-терминатор для безопасной работы со строками
        
        // Проверяем, начинается ли запись с ключа и следующий символ - пробел
        if (!(strncmp(buffer, index_key, strlen(index_key)) == 0 && buffer[strlen(index_key)] == ' ')) {
            // Если запись не начинается с ключа или следующий символ не пробел, копируем её во временный файл
            fprintf(temp_file, "%s", buffer); // Не добавляем \n, так как он уже включен в buffer
        }
    }
    
    free(buffer);
    fclose(file);
    fclose(temp_file);
    close(fd); 
    close(temp_fd);

    // Заменяем оригинальный файл временным файлом
    rename(temp_filename, filename);

    return;
}



// Структура для хранения информации о записи в индексном файле
typedef struct {
    off_t offset; // Смещение данных в файле данных
    size_t size;  // Размер блока данных
} IndexRecord;

char *indexed_find_value_by_key(const char *filename, const char *index_key) {
    size_t index_size = 32;

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

    char *buffer = malloc(index_size); // Выделение памяти под буфер размером с index_size
    if (!buffer) {
        close(index_fd);
        close(data_fd);
        return NULL;
    }

    ssize_t bytesRead;
    char *value = NULL;

    while ((bytesRead = read(index_fd, buffer, index_size)) == index_size) { 
        if (strncmp(buffer, index_key, strlen(index_key)) == 0) {
            // Найдено совпадение ключа, читаем информацию о смещении и размере
            IndexRecord record;
            memcpy(&record, buffer + strlen(index_key) + 1, sizeof(IndexRecord)); // +1 для пропуска двоеточия

            // Чтение данных из файла данных
            value = malloc(record.size + 1);
            if (value) {
                pread(data_fd, value, record.size, record.offset);
                value[record.size] = '\0'; // Добавляем нуль-терминатор
                break; // Выходим из цикла после нахождения значения
            }
        }
    }

    free(buffer); // Освобождаем выделенную память

    // Разблокировка и закрытие файлов
    lock_file(index_fd, F_UNLCK);
    lock_file(data_fd, F_UNLCK);
    close(index_fd);
    close(data_fd);

    return value;
}



void indexed_write_key_value_pair(const char *filename, const char *index_key, const char *index_val) {
    size_t index_size = 32;

    char index_filename[256];
    snprintf(index_filename, sizeof(index_filename), "%s.index", filename);

    int data_fd = open(filename, O_RDWR | O_CREAT | O_APPEND, 0644);
    int index_fd = open(index_filename, O_RDWR | O_CREAT | O_APPEND, 0644);
    if (data_fd == -1 || index_fd == -1) {
        if (data_fd != -1) close(data_fd);
        if (index_fd != -1) close(index_fd);
        return;
    }

    // Блокировка обоих файлов
    if (lock_file(data_fd, F_WRLCK) == -1 || lock_file(index_fd, F_WRLCK) == -1) {
        close(data_fd);
        close(index_fd);
        return;
    }

    // Запись значения в файл данных
    off_t offset = lseek(data_fd, 0, SEEK_END); // Получаем текущее смещение в файле данных
    size_t size = strlen(index_val);
    if (write(data_fd, index_val, size) != size) {
        close(data_fd);
        close(index_fd);
        return;
    }

    // Формирование и запись индекса в индексный файл
    IndexRecord record = {offset, size};
    char *buffer = malloc(index_size); // Выделение памяти под буфер
    if (!buffer) {
        close(data_fd);
        close(index_fd);
        return;
    }
    int written = snprintf(buffer, index_size, "%s:", index_key); // Форматирование ключа
    memcpy(buffer + written, &record, sizeof(IndexRecord)); // Копирование структуры IndexRecord
    memset(buffer + written + sizeof(IndexRecord), ' ', index_size - written - sizeof(IndexRecord) - 1); // Заполнение оставшейся части пробелами
    buffer[index_size - 1] = '\n'; // Добавление символа новой строки в конец

    if (write(index_fd, buffer, index_size) != index_size) {
        free(buffer);
        close(data_fd);
        close(index_fd);
        return;
    }

    free(buffer);

    // Разблокировка и закрытие файлов
    lock_file(data_fd, F_UNLCK);
    lock_file(index_fd, F_UNLCK);
    close(data_fd);
    close(index_fd);
}



void indexed_delete_key(const char *filename, const char *index_key) {
    size_t index_size = 32;

    char index_filename[256];
    snprintf(index_filename, sizeof(index_filename), "%s.index", filename);

    int index_fd = open(index_filename, O_RDWR);
    if (index_fd == -1) {
        //perror("Ошибка при открытии индексного файла");
        return;
    }

    if (lock_file(index_fd, F_WRLCK) == -1) {
        //perror("Ошибка при блокировке индексного файла");
        close(index_fd);
        return;
    }

    char *buffer = malloc(index_size);
    if (!buffer) {
        //perror("Ошибка выделения памяти");
        close(index_fd);
        return;
    }

    ssize_t bytesRead;
    while ((bytesRead = read(index_fd, buffer, index_size)) == index_size) {
        if (strncmp(buffer, index_key, strlen(index_key)) == 0) {
            // Найден ключ, затираем его пробелами (или нулями, если требуется)
            memset(buffer, ' ', index_size); // Замените ' ' на '\0', если нужно затирать нулями
            off_t current_pos = lseek(index_fd, -index_size, SEEK_CUR);
            if (current_pos == -1 || write(index_fd, buffer, index_size) != index_size) {
                //perror("Ошибка при записи в индексный файл");
                break;
            }
            // Выход после первого найденного совпадения
            break;
        }
    }

    free(buffer); // Освобождаем выделенную память

    lock_file(index_fd, F_UNLCK); // Разблокировка файла
    close(index_fd); // Закрытие файла
}
