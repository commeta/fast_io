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

    char buffer[BUFFER_SIZE + 1]; // Дополнительный байт для нуль-терминатора
    ssize_t bytesRead;
    char *found_value = NULL;
    char *lineStart = buffer;
    int keyLength = strlen(index_key);
    char *value = NULL;

    while ((bytesRead = read(index_fd, buffer, BUFFER_SIZE)) > 0) {
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


    if (found_value != NULL){
        // Парсим ключ и IndexRecord
        char *token = strtok(found_value, ":");
        if (!token) return found_value; // Некорректная строка

        off_t offset = atoll(token);
        token = strtok(NULL, ":"); 
        size_t size = atoll(token);

        if(offset == 0 || size == 0){
            close(index_fd);
            close(data_fd);

            return value;
        }
        
        value = malloc(size);

        if (value) {
            if(pread(data_fd, value, size, offset) == -1) return NULL;
            value[size] = '\0'; // Добавляем нуль-терминатор
        }
    }

    // Разблокировка и закрытие файлов
    close(index_fd);
    close(data_fd);

    return value;
}



// Запись пары ключ-значение
long write_key_value_pair(const char *filename, const char *index_key, const char *index_val) {
    int fd = open(filename, O_WRONLY | O_APPEND | O_CREAT, 0666);

    if (fd == -1) return -1;

    if (lock_file(fd, F_WRLCK) == -1) {
        close(fd);
        return -1;
    }

    dprintf(fd, "%s %s\n", index_key, index_val);
    close(fd); // Это также разблокирует файл
    return 1;
}



long indexed_write_key_value_pair(const char *filename, const char *index_key, const char *index_val) {
    char index_filename[256];

    snprintf(index_filename, sizeof(index_filename), "%s.index", filename);

    int data_fd = open(filename, O_RDWR | O_CREAT | O_APPEND, 0644);
    int index_fd = open(index_filename, O_RDWR | O_CREAT | O_APPEND, 0644);
    if (data_fd == -1 || index_fd == -1) {
        //perror("Error opening file");
        if (data_fd != -1) close(data_fd);
        if (index_fd != -1) close(index_fd);
        return -1;
    }

    // Блокировка обоих файлов
    if (lock_file(data_fd, F_WRLCK) == -1 || lock_file(index_fd, F_WRLCK) == -1) {
        //perror("Error locking file");
        close(data_fd);
        close(index_fd);
        return -1;
    }

    // Запись значения в файл данных
    off_t offset = lseek(data_fd, 0, SEEK_END); // Получаем текущее смещение в файле данных
    size_t size = strlen(index_val);
    if (write(data_fd, index_val, size) != size) {
        //perror("Error writing to data file");
        close(data_fd);
        close(index_fd);
        return -1;
    }

    // Запись индекса в индексный файл
    dprintf(index_fd, "%s %ld:%zu\n", index_key, offset, size);

    // Разблокировка и закрытие файлов
    close(data_fd);
    close(index_fd);
    return 1;
}



// Удаление пары ключ-значение с чтением файла порциями
long delete_key_value_pair(const char *filename, const char *index_key) {
    char temp_filename[256];
    snprintf(temp_filename, sizeof(temp_filename), "%s.tmp", filename);

    int fd = open(filename, O_RDWR);
    if (fd == -1) return -1;

    int temp_fd = open(temp_filename, O_RDWR | O_CREAT | O_APPEND, 0644);
    if (temp_fd == -1) {
        close(temp_fd);
        //perror("Error opening file");
        return -1;
    }

    // Блокировка  файлов
    if (
        lock_file(fd, F_WRLCK) == -1 || 
        lock_file(temp_fd, F_WRLCK) == -1
    ) {
        //perror("Error locking file");
        close(fd);
        close(temp_fd);
        return -1;
    }


    FILE *file = fdopen(fd, "r");
    FILE *temp_file = fdopen(temp_fd, "w");
    
    if (!file || !temp_file) {
        if (file) fclose(file);
        if (temp_file) fclose(temp_file);
        close(fd);
        close(temp_fd);
        unlink(temp_filename);
        return -1;
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
    close(fd); 
    close(temp_fd);

    // Заменяем оригинальный файл временным файлом
    rename(temp_filename, filename);

    return 1;
}


// Функция для пересоздания файла данных на основе индексного файла
long rebuild_data_file(const char *filename) {
    char index_filename[256];
    snprintf(index_filename, sizeof(index_filename), "%s.index", filename);

    int index_fd = open(index_filename, O_RDONLY);
    if (index_fd == -1) {
        //perror("Ошибка открытия индексного файла");
        return -1;
    }

    char temp_filename[256];
    snprintf(temp_filename, sizeof(temp_filename), "%s.tmp", filename);

    int temp_fd = open(temp_filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (temp_fd == -1) {
        //perror("Ошибка создания временного файла");
        close(index_fd);
        return -1;
    }

    char temp_index_filename[256];
    snprintf(temp_index_filename, sizeof(temp_filename), "%s.index.tmp", filename);

    int temp_index_fd = open(temp_index_filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (temp_index_fd == -1) {
        //perror("Ошибка создания временного файла");
        close(index_fd);
        return -1;
    }


    // Блокировка  файлов
    if (
        lock_file(index_fd, F_RDLCK) == -1 || 
        lock_file(temp_fd, F_WRLCK) == -1 ||
        lock_file(temp_index_fd, F_WRLCK) == -1
    ) {
        //perror("Error locking file");
        close(index_fd);
        close(temp_fd);
        close(temp_index_fd);
        return -1;
    }
    

    char buffer[BUFFER_SIZE + 1]; // +1 для нуль-терминатора
    ssize_t bytesRead;

    // Чтение индексного файла посекторно
    while ((bytesRead = read(index_fd, buffer, BUFFER_SIZE)) > 0) {
        buffer[bytesRead] = '\0'; // Добавляем нуль-терминатор для безопасной работы со строками

        char *ptr = buffer;
        while (*ptr) {
            // Находим конец строки или конец буфера
            char *end_line = strchr(ptr, '\n');
            if (!end_line) break;
            *end_line = '\0'; // Заменяем символ конца строки на нуль-терминатор для изоляции строки

            // Парсинг строки индексного файла
            char *space_pos = strchr(ptr, ' ');
            if (space_pos) {
                *space_pos = '\0'; // Разделяем ключ и пару offset:size
                char *colon_pos = strchr(space_pos + 1, ':');
                if (colon_pos) {
                    *colon_pos = '\0'; // Разделяем offset и size

                    off_t offset = atol(space_pos + 1); // Преобразуем offset в число
                    size_t size = atol(colon_pos + 1); // Преобразуем size в число

                    // Чтение и запись блока данных из оригинального файла данных во временный файл
                    int data_fd = open(filename, O_RDONLY);
                    if (data_fd == -1) {
                        //perror("Ошибка открытия файла данных");
                        close(temp_fd);
                        close(index_fd);
                        return -1;
                    }

                    lseek(data_fd, offset, SEEK_SET); // Перемещаемся к началу блока данных

                    char data_buffer[size];
                    
                    // Читаем блок данных
                    if(read(data_fd, data_buffer, size) == -1) return -1;

                    off_t new_offset = lseek(temp_fd, 0, SEEK_END); // Получаем текущее смещение в файле данных

                    // Записываем блок данных во временный файл
                    if(write(temp_fd, data_buffer, size)) return -1;

                    close(data_fd);

                    dprintf(temp_index_fd, "%s %ld:%zu\n", ptr, new_offset, size);
                }
            }

            ptr = end_line + 1; // Перемещаемся к началу следующей строки
        }
    }

    close(index_fd);
    close(temp_fd);
    close(temp_index_fd);

    // Замена оригинального файла данных новой версией
    rename(temp_filename, filename);
    rename(temp_index_filename, index_filename);
    return 1;
}


// Функция для извлечения и удаления последней строки из файла
char *pop_key_value_pair(const char *filename) {
    int fd = open(filename, O_RDWR);
    if (fd == -1) return NULL;

    if (lock_file(fd, F_WRLCK) == -1) {
        close(fd);
        return NULL;
    }

    off_t fileSize = lseek(fd, 0, SEEK_END);
    if (fileSize == -1) {
        close(fd);
        return NULL;
    }

    char buffer[BUFFER_SIZE];
    off_t pos = fileSize;
    ssize_t bytesRead;
    char *line = NULL;
    int state = 0; // Состояние конечного автомата

    // Читаем файл с конца, пока не найдем начало строки
    while (pos > 0 && state != 2) {
        pos -= BUFFER_SIZE;
        if (pos < 0) {
            pos = 0; // Адаптируем позицию, если вышли за начало файла
        }

        lseek(fd, pos, SEEK_SET);
        bytesRead = read(fd, buffer, BUFFER_SIZE);
        if (bytesRead <= 0) break; // Ошибка чтения или достигнут конец файла

        // Обрабатываем буфер с конца к началу
        for (ssize_t i = bytesRead - 1; i >= 0 && state != 2; --i) {
            switch (state) {
                case 0: // Ищем перенос строки
                    if (buffer[i] == '\n') {
                        state = 1; // Нашли перенос строки, переходим к следующему состоянию
                    }
                    break;
                case 1: // Ищем начало строки
                    if (buffer[i] == '\n' || pos + i == 0) { // Второе условие для случая, когда строка - первая в файле
                        // Выделяем память и копируем строку
                        size_t len = bytesRead - i - 1;
                        line = malloc(len + 1);
                        memcpy(line, buffer + i + 1, len);
                        line[len] = '\0';
                        state = 2; // Переходим к финальному состоянию

                        // Усекаем файл
                        if(ftruncate(fd, pos + i + (pos + i == 0 ? 0 : 1))) return NULL;
                    }
                    break;
            }
        }
    }

    close(fd);
    return line; // Возвращаем последнюю строку или NULL, если строка не найдена
}

