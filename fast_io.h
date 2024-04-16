/*
 * Fast_IO (pre-release) Extension for PHP 8
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


extern zend_module_entry fast_io_module_entry;
#define phpext_fast_io_ptr &fast_io_module_entry

#ifdef ZTS
#include "TSRM.h"
#endif

ZEND_BEGIN_MODULE_GLOBALS(fast_io)
    zend_long buffer_size;
ZEND_END_MODULE_GLOBALS(fast_io)

#ifdef ZTS
#define FAST_IO_G(v) TSRMG(fast_io_globals_id, zend_fast_io_globals *, v)
#else
#define FAST_IO_G(v) (fast_io_globals.v)
#endif

// Вспомогательная функция для блокировки файла
int lock_file(int fd, int lock_type) {
    struct flock fl;
    fl.l_type = lock_type;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0; // Блокировка всего файла
    return fcntl(fd, F_SETLKW, &fl);
}


// Структура для хранения динамического массива ключей и их значений
typedef struct {
    int (*values)[2];
    size_t count;
} KeyValueArray;

void add_key_value(KeyValueArray *array, int value[2]) {
    array->count++;
    array->values = erealloc(array->values, array->count * sizeof(int[2]));
    
    array->values[array->count - 1][0] = value[0];
    array->values[array->count - 1][1] = value[1];
}

void free_key_value_array(KeyValueArray *array) {
    efree(array->values);
}


// Структура для хранения динамического массива ключей
typedef struct {
    char **keys;
    size_t count;
} KeyArray;


// Функция для добавления ключа в массив ключей
void add_key(KeyArray *array, const char *key) {
    array->count++;
    array->keys = erealloc(array->keys, array->count * sizeof(char *));
    array->keys[array->count - 1] = estrdup(key);
}

// Функция для освобождения памяти, выделенной под массив ключей
void free_key_array(KeyArray *array) {
    for (size_t i = 0; i < array->count; i++) {
        efree(array->keys[i]);
    }
    efree(array->keys);
}

