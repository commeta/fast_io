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
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "ext/standard/info.h"


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

// Структура для хранения динамического массива ключей
typedef struct {
    char **keys;
    size_t count;
} KeyArray;

// Функция для добавления ключа в массив ключей
void add_key(KeyArray *array, const char *key) {
    array->count++;
    array->keys = realloc(array->keys, array->count * sizeof(char *));
    array->keys[array->count - 1] = strdup(key);
}

// Функция для освобождения памяти, выделенной под массив ключей
void free_key_array(KeyArray *array) {
    for (size_t i = 0; i < array->count; i++) {
        free(array->keys[i]);
    }
    free(array->keys);
}


long rebuild_data_file(const char *filename, const char *index_key) {
    char index_filename[256], temp_filename[260], temp_index_filename[260];
    snprintf(index_filename, sizeof(index_filename), "%s.index", filename);
    snprintf(temp_filename, sizeof(temp_filename), "%s.tmp", filename);
    snprintf(temp_index_filename, sizeof(temp_index_filename), "%s.index.tmp", filename);

    // Открытие файлов
    int index_fd = open(index_filename, O_RDONLY);
    int data_fd = open(filename, O_RDONLY);
    int temp_data_fd = open(temp_filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    int temp_index_fd = open(temp_index_filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if (index_fd == -1 || data_fd == -1 || temp_data_fd == -1 || temp_index_fd == -1) {
        //perror("Ошибка при открытии файлов");
        return -1;
    }

    // Блокировка файлов
    lock_file(index_fd, F_RDLCK);
    lock_file(data_fd, F_RDLCK);

    char buffer[BUFFER_SIZE];
    ssize_t bytesRead;

    // Чтение индексного файла порциями
    while ((bytesRead = read(index_fd, buffer, BUFFER_SIZE)) > 0) {
        int bufferPos = 0;
        while (bufferPos < bytesRead) {
            char *lineStart = buffer + bufferPos;
            char *lineEnd = memchr(lineStart, '\n', bytesRead - bufferPos);
            if (!lineEnd) break; // Если конец строки не найден

            size_t lineLength = lineEnd - lineStart;
            char line[BUFFER_SIZE];
            strncpy(line, lineStart, lineLength);
            line[lineLength] = '\0';

            bufferPos += lineLength + 1;

            // Парсинг строки индексного файла
            char *keyEnd = strchr(line, ' ');
            if (!keyEnd) continue; // Если формат строки неверен

            *keyEnd = '\0';
            char specialChar = 127;

            if (index_key != NULL && strcmp(line, index_key) == 0) continue; // Пропускаем строку с исключаемым ключом
            if (strncmp(lineStart, &specialChar, 1) == 0) continue; // Пропускаем строку с исключаемыми ключами

            long offset = atol(keyEnd + 1);
            char *sizePtr = strchr(keyEnd + 1, ':');
            if (!sizePtr) continue;

            size_t size = atol(sizePtr + 1);

            // Чтение и запись блока данных
            lseek(data_fd, offset, SEEK_SET);
            char dataBuffer[size];
            if(read(data_fd, dataBuffer, size) == -1) return -2;
            if(write(temp_data_fd, dataBuffer, size) == -1) return -3;

            // Запись во временный индексный файл
            dprintf(temp_index_fd, "%s %ld:%zu\n", line, offset, size);
        }
    }

    // Закрытие файлов
    close(index_fd);
    close(data_fd);
    close(temp_data_fd);
    close(temp_index_fd);

    // Переименование временных файлов
    rename(temp_filename, filename);
    rename(temp_index_filename, index_filename);

    return 1;
}

