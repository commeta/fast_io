/*
 * Fast_IO (pre-release beta) Extension for PHP 8
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_streams.h" // Для работы с потоками PHP
#include <errno.h>

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <sys/types.h>
#include <pcre2.h>
#include <sys/file.h>
#include "fast_io.h"




ZEND_DECLARE_MODULE_GLOBALS(fast_io)

// Функция для обновления значения параметра
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("fast_io.buffer_size", "4096", PHP_INI_ALL, OnUpdateLong, buffer_size, zend_fast_io_globals, fast_io_globals)
PHP_INI_END()

PHP_MINIT_FUNCTION(fast_io)
{
    REGISTER_INI_ENTRIES();
    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(fast_io)
{
    UNREGISTER_INI_ENTRIES();
    return SUCCESS;
}


/* Декларация функций */
PHP_FUNCTION(find_array_by_key);
PHP_FUNCTION(find_value_by_key);
PHP_FUNCTION(indexed_find_value_by_key);
PHP_FUNCTION(write_key_value_pair);
PHP_FUNCTION(indexed_write_key_value_pair);
PHP_FUNCTION(delete_key_value_pair);
PHP_FUNCTION(rebuild_data_file);
PHP_FUNCTION(pop_key_value_pair);
PHP_FUNCTION(hide_key_value_pair);
PHP_FUNCTION(get_index_keys);
PHP_FUNCTION(update_key_value_pair);
PHP_FUNCTION(insert_key_value);
PHP_FUNCTION(select_key_value);
PHP_FUNCTION(update_key_value);
PHP_FUNCTION(detect_align_size);
PHP_FUNCTION(find_matches_pcre2);


/* Запись аргументов функций */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_find_array_by_key, 1, 2, IS_ARRAY, 1)
    ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, index_key, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, search_state, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, search_start, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, search_limit, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_find_value_by_key, 1, 2, IS_STRING, 1)
    ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, index_key, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, search_state, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_indexed_find_value_by_key, 0, 2, IS_STRING, 1)
    ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, index_key, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_write_key_value_pair, 0, 3, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, index_key, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, index_val, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_indexed_write_key_value_pair, 0, 3, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, index_key, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, index_val, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_delete_key_value_pair, 1, 1, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, index_key, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_rebuild_data_file, 1, 1, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, index_key, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_pop_key_value_pair, 1, 1, IS_STRING, 1)
    ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, index_align, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_hide_key_value_pair, 0, 2, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, index_key, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_get_index_keys, 1, 1, IS_ARRAY, 1)
    ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, mode, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_update_key_value_pair, 0, 3, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, index_key, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, index_val, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_insert_key_value, 0, 3, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, index_key, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, index_align, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_select_key_value, 1, 3, IS_STRING, 1)
    ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, index_row, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, index_align, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, mode, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_update_key_value, 0, 3, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, index_key, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, index_align, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_detect_align_size, 0, 1, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_find_matches_pcre2, 0, 2, IS_ARRAY, 0)
    ZEND_ARG_TYPE_INFO(0, pattern, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, subject, IS_STRING, 0)
ZEND_END_ARG_INFO()


/* Регистрация функций */
const zend_function_entry fast_io_functions[] = {
    PHP_FE(find_array_by_key, arginfo_find_array_by_key)
    PHP_FE(find_value_by_key, arginfo_find_value_by_key)
    PHP_FE(indexed_find_value_by_key, arginfo_indexed_find_value_by_key)
    PHP_FE(write_key_value_pair, arginfo_write_key_value_pair)
    PHP_FE(indexed_write_key_value_pair, arginfo_indexed_write_key_value_pair)
    PHP_FE(delete_key_value_pair, arginfo_delete_key_value_pair)
    PHP_FE(rebuild_data_file, arginfo_rebuild_data_file)
    PHP_FE(pop_key_value_pair, arginfo_pop_key_value_pair)
    PHP_FE(hide_key_value_pair, arginfo_hide_key_value_pair)
    PHP_FE(get_index_keys, arginfo_get_index_keys)
    PHP_FE(update_key_value_pair, arginfo_update_key_value_pair)
    PHP_FE(insert_key_value, arginfo_insert_key_value)
    PHP_FE(select_key_value, arginfo_select_key_value)
    PHP_FE(update_key_value, arginfo_update_key_value)
    PHP_FE(detect_align_size, arginfo_detect_align_size)
    PHP_FE(find_matches_pcre2, arginfo_find_matches_pcre2)
    PHP_FE_END
};

/* Определение модуля */
zend_module_entry fast_io_module_entry = {
    STANDARD_MODULE_HEADER,
    "fast_io",
    fast_io_functions,
    PHP_MINIT(fast_io),
    PHP_MSHUTDOWN(fast_io),
    NULL,
    NULL,
    NULL,
    "0.1",
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_FAST_IO
ZEND_GET_MODULE(fast_io)
#endif




PHP_FUNCTION(find_array_by_key) {
    char *filename, *index_key;
    size_t filename_len, index_key_len;
    zend_long search_state = 0;
    zend_long search_start = 0;
    zend_long search_limit = 1;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "ss|lll", &filename, &filename_len, &index_key, &index_key_len, &search_state, &search_start, &search_limit) == FAILURE) {
        return;
    }

    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        php_error_docref(NULL, E_WARNING, "Failed to open file: %s", filename);
        RETURN_FALSE;
    }

    // Попытка установить блокирующую блокировку на запись
    if (flock(fileno(fp), LOCK_EX) < 0) {
        php_error_docref(NULL, E_WARNING, "Failed to lock file: %s", filename);
        fclose(fp);
        RETURN_FALSE;
    }

    zend_long ini_buffer_size = FAST_IO_G(buffer_size);
    zend_long dynamic_buffer_size = ini_buffer_size;
    char *dynamic_buffer = (char *)emalloc(dynamic_buffer_size + 1);

    ssize_t bytesRead;
    size_t current_size = 0; // Текущий размер данных в динамическом буфере
    off_t search_offset = 0; // Смещение строки поиска

    zend_long found_count = 0;
    zend_long add_count = 0;
    zend_long line_count = 0;

    bool found_match = false;

    pcre2_code *re;
    pcre2_match_data *match_data; 

    array_init(return_value);

    KeyArray keys = {0};
    KeyValueArray keys_values = {0};

    int value[2];
    bool isEOF;

    if(search_state > 9){
        PCRE2_SIZE erroffset;
        int errorcode;
        re = pcre2_compile((PCRE2_SPTR)index_key, PCRE2_ZERO_TERMINATED, 0, &errorcode, &erroffset, NULL);

        if (re == NULL) {
            PCRE2_UCHAR message[256];
            pcre2_get_error_message(errorcode, message, sizeof(message));
            php_error_docref(NULL, E_WARNING, "PCRE2 compilation failed at offset %d: %s", (int)erroffset, message);
            fclose(fp);
            efree(dynamic_buffer);
            RETURN_FALSE;
        }

        match_data = pcre2_match_data_create_from_pattern(re, NULL);
    }


    while ((bytesRead = fread(dynamic_buffer + current_size, 1, ini_buffer_size, fp)) > 0) {
        current_size += bytesRead;
        // Проверяем, достигли ли мы конца файла (EOF)
        isEOF = feof(fp);
        
        if (isEOF && dynamic_buffer[current_size - 1] != '\n') {
            // Если это EOF и последний символ не является переводом строки,
            // добавляем перевод строки для упрощения обработки
            dynamic_buffer[current_size] = '\n';
            current_size++;
        }
        
        dynamic_buffer[current_size] = '\0';

        char *lineStart = dynamic_buffer;
        char *lineEnd;
        while ((lineEnd = strchr(lineStart, '\n')) != NULL) {
            ssize_t lineLength = lineEnd - lineStart + 1;
            *lineEnd = '\0';
            line_count++;

            if(search_state == 0 && strstr(lineStart, index_key) != NULL){
                found_count++;

                if(search_start < found_count){
                    add_count++;
                    
                    for (int i = lineLength - 2; i >= 0; --i) {
                        if(lineStart[i] == ' ') lineStart[i] = '\0';
                        else break;
                    }

                    if(add_key(&keys, lineStart) == false){
                        php_error_docref(NULL, E_WARNING, "Out of memory");
                        found_match = true;
                        break;
                    }
                }
            }

            if(search_state == 1 && strstr(lineStart, index_key) != NULL){
                found_count++;

                if(search_start < found_count){
                    add_count++;

                    value[0] = line_count;
                    value[1] = lineLength - 1;
                    if(add_key_value(&keys_values, value) == false){
                        php_error_docref(NULL, E_WARNING, "Out of memory");
                        found_match = true;
                        break;
                    }
                }
            }

            if(search_state == 2 && strstr(lineStart, index_key) != NULL){
                found_count++;

                if(search_start < found_count){
                    add_count++;

                    value[0] = search_offset;
                    value[1] = lineLength - 1;
                    if(add_key_value(&keys_values, value) == false){
                        php_error_docref(NULL, E_WARNING, "Out of memory");
                        found_match = true;
                        break;
                    }
                }
            }

            if(search_state == 3 && strstr(lineStart, index_key) != NULL){
                found_count++;
            }

            if(search_state == 10 && pcre2_match(re, lineStart, lineLength - 1, 0, 0, match_data, NULL) > 0){
                found_count++;

                if(search_start < found_count){
                    add_count++;

                    for (int i = lineLength - 2; i >= 0; --i) {
                        if(lineStart[i] == ' ') lineStart[i] = '\0';
                        else break;
                    }

                    if(add_key(&keys, lineStart) == false){
                        php_error_docref(NULL, E_WARNING, "Out of memory");
                        found_match = true;
                        break;
                    }
                }
            }

            if(search_state == 11 && pcre2_match(re, lineStart, lineLength, 0, 0, match_data, NULL) > 0){
                found_count++;

                if(search_start < found_count){
                    add_count++;
                    
                    value[0] = line_count;
                    value[1] = lineLength - 1;
                    if(add_key_value(&keys_values, value) == false){
                        php_error_docref(NULL, E_WARNING, "Out of memory");
                        found_match = true;
                        break;
                    }
                }
            }

            if(search_state == 12 && pcre2_match(re, lineStart, lineLength, 0, 0, match_data, NULL) > 0){
                found_count++;

                if(search_start < found_count){
                    add_count++;

                    value[0] = search_offset;
                    value[1] = lineLength - 1;
                    if(add_key_value(&keys_values, value) == false){
                        php_error_docref(NULL, E_WARNING, "Out of memory");
                        found_match = true;
                        break;
                    }
                }
            }

            if(search_state == 13 && pcre2_match(re, lineStart, lineLength, 0, 0, match_data, NULL) > 0){
                found_count++;
            }

            search_offset += lineLength; // Обновляем смещение
            lineStart = lineEnd + 1;

            if(add_count >= search_limit){
                found_match = true;
                break;
            }
        }

        if (found_match) break;

        // Подготавливаем буфер к следующему чтению, если это не конец файла
        if (!isEOF) {
            current_size -= (lineStart - dynamic_buffer);
            memmove(dynamic_buffer, lineStart, current_size);

            if (current_size + ini_buffer_size > dynamic_buffer_size) {
                dynamic_buffer_size += ini_buffer_size;
                dynamic_buffer = (char *)erealloc(dynamic_buffer, dynamic_buffer_size + 1);
                if (!dynamic_buffer) {
                    php_error_docref(NULL, E_WARNING, "Out of memory");
                    break;
                }
            }
        }
    }

    efree(dynamic_buffer);

    if (search_state > 9 && re != NULL) pcre2_code_free(re);
    if (search_state > 9 && match_data != NULL) pcre2_match_data_free(match_data);

    if(search_state == 3 || search_state == 13){      
        value[0] = found_count;
        value[1] = line_count;

        if(add_key_value(&keys_values, value) == false){
            php_error_docref(NULL, E_WARNING, "Out of memory");
        }
    }

    fclose(fp);
    
    if(search_state > 19){
        free_key_array(&keys);
        free_key_value_array(&keys_values);
        return;
    }

    if(
        search_state == 0 || 
        search_state == 10
    ){
        for (size_t i = 0; i < keys.count; i++) {
            add_next_index_string(return_value, keys.keys[i]);
        }
    } else {
        for (size_t i = 0; i < keys_values.count; i++) {
            zval key_value_arr;
            array_init(&key_value_arr);

            add_index_long(&key_value_arr, 0, keys_values.values[i][0]);
            add_index_long(&key_value_arr, 1, keys_values.values[i][1]);       
            add_next_index_zval(return_value, &key_value_arr);
        }
    }

    free_key_array(&keys);
    free_key_value_array(&keys_values);
}



PHP_FUNCTION(find_value_by_key) {
    char *filename, *index_key;
    size_t filename_len, index_key_len;
    zend_long search_state = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "ss|l", &filename, &filename_len, &index_key, &index_key_len, &search_state) == FAILURE) {
        RETURN_FALSE; // Неправильные параметры вызова функции
    }

    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        php_error_docref(NULL, E_WARNING, "Failed to open file: %s", filename);
        RETURN_FALSE;
    }

    // Попытка установить блокирующую блокировку на запись
    if (flock(fileno(fp), LOCK_EX) < 0) {
        php_error_docref(NULL, E_WARNING, "Failed to lock file: %s", filename);
        fclose(fp);
        RETURN_FALSE;
    }

    zend_long ini_buffer_size = FAST_IO_G(buffer_size);
    zend_long dynamic_buffer_size = ini_buffer_size;
    char *dynamic_buffer = (char *)emalloc(dynamic_buffer_size + 1);

    ssize_t bytesRead;
    size_t current_size = 0; // Текущий размер данных в динамическом буфере
    bool found_match = false;

    pcre2_code *re;
    pcre2_match_data *match_data;

    char *found_value = NULL;
    bool isEOF;

    if(search_state > 9){
        PCRE2_SIZE erroffset;
        int errorcode;
        re = pcre2_compile((PCRE2_SPTR)index_key, PCRE2_ZERO_TERMINATED, 0, &errorcode, &erroffset, NULL);

        if (re == NULL) {
            PCRE2_UCHAR message[256];
            pcre2_get_error_message(errorcode, message, sizeof(message));
            php_error_docref(NULL, E_WARNING, "PCRE2 compilation failed at offset %d: %s", (int)erroffset, message);
            fclose(fp);
            efree(dynamic_buffer);
            RETURN_FALSE;
        }

        match_data = pcre2_match_data_create_from_pattern(re, NULL);
    }


    while ((bytesRead = fread(dynamic_buffer + current_size, 1, ini_buffer_size, fp)) > 0) {
        current_size += bytesRead;
        // Проверяем, достигли ли мы конца файла (EOF)
        isEOF = feof(fp);
        
        if (isEOF && dynamic_buffer[current_size - 1] != '\n') {
            // Если это EOF и последний символ не является переводом строки,
            // добавляем перевод строки для упрощения обработки
            dynamic_buffer[current_size ] = '\n';
            current_size++;
        }
        
        dynamic_buffer[current_size] = '\0';

        char *lineStart = dynamic_buffer;
        char *lineEnd;
        while ((lineEnd = strchr(lineStart, '\n')) != NULL) {
            ssize_t lineLength = lineEnd - lineStart + 1;
            *lineEnd = '\0';

            if(search_state == 0 && strstr(lineStart, index_key) != NULL){
                found_value = estrndup(lineStart, lineLength - 2);
                found_match = true;
                break;
            }

            if(search_state == 10 && pcre2_match(re, lineStart, lineLength - 1, 0, 0, match_data, NULL) > 0){
                found_value = estrndup(lineStart, lineLength - 2);
                found_match = true;
                break;
            }

            lineStart = lineEnd + 1;
        }

        if (found_match) break;

        if (!isEOF) {
            current_size -= (lineStart - dynamic_buffer);
            memmove(dynamic_buffer, lineStart, current_size);

            if (current_size + ini_buffer_size > dynamic_buffer_size) {
                dynamic_buffer_size += ini_buffer_size;
                dynamic_buffer = (char *)erealloc(dynamic_buffer, dynamic_buffer_size + 1);
                if (!dynamic_buffer) {
                    php_error_docref(NULL, E_WARNING, "Out of memory");
                    break;
                }
            }
        }
    }

    efree(dynamic_buffer);

    if (search_state > 9 && re != NULL) pcre2_code_free(re);
    if (search_state > 9 && match_data != NULL) pcre2_match_data_free(match_data);

    fclose(fp);

    if (found_value == NULL) {
        RETURN_FALSE;
    } else {
        // Обрезка пробелов справа и символа перевода строки
        size_t len = strlen(found_value);
        for (int i = len - 1; i >= 0; --i) {
            if(found_value[i] == ' ' || found_value[i] == '\n') found_value[i] = '\0';
            else break;
        }

        RETVAL_STRING(found_value);
        efree(found_value);
    }
}




PHP_FUNCTION(indexed_find_value_by_key) {
    char *filename, *index_key;
    size_t filename_len, index_key_len;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "ss", &filename, &filename_len, &index_key, &index_key_len) == FAILURE) {
        RETURN_FALSE;
    }

    char index_filename[filename_len + 7];
    snprintf(index_filename, sizeof(index_filename), "%s.index", filename);

    FILE *data_fp = fopen(filename, "r");
    if (!data_fp) {
        php_error_docref(NULL, E_WARNING, "Failed to open file: %s", filename);
        RETURN_FALSE;
    }

    FILE *index_fp = fopen(index_filename, "r");
    if (!index_fp) {
        php_error_docref(NULL, E_WARNING, "Failed to open file: %s", index_filename);
        fclose(data_fp);
        RETURN_FALSE;
    }

    if (flock(fileno(data_fp), LOCK_EX) == -1 || flock(fileno(index_fp), LOCK_EX) == -1) {
        php_error_docref(NULL, E_WARNING, "Failed to lock the file: %s", filename);
        fclose(data_fp);
        fclose(index_fp);
        RETURN_FALSE;
    }

    zend_long ini_buffer_size = FAST_IO_G(buffer_size);
    zend_long dynamic_buffer_size = ini_buffer_size;
    char *dynamic_buffer = (char *)emalloc(dynamic_buffer_size + 1);
    if (!dynamic_buffer) {
        php_error_docref(NULL, E_WARNING, "Out of memory");
        fclose(data_fp);
        fclose(index_fp);
        RETURN_FALSE;
    }

    ssize_t bytesRead;
    size_t current_size = 0;
    bool found_match = false;
    char *found_value = NULL;

    while ((bytesRead = fread(dynamic_buffer + current_size, 1, ini_buffer_size - current_size, index_fp)) > 0) {
        current_size += bytesRead;
        dynamic_buffer[current_size] = '\0';

        char *lineStart = dynamic_buffer;
        char *lineEnd;

        while ((lineEnd = strchr(lineStart, '\n')) != NULL) {
            *lineEnd = '\0';
            if (strstr(lineStart, index_key) != NULL) {
                found_value = estrdup(lineStart + index_key_len + 1);
                found_match = true;
                break;
            }
            lineStart = lineEnd + 1;
        }

        if (found_match) break;

        current_size -= (lineStart - dynamic_buffer);
        memmove(dynamic_buffer, lineStart, current_size);

        if (current_size + ini_buffer_size > dynamic_buffer_size) {
            dynamic_buffer_size += ini_buffer_size;
            char *temp_buffer = (char *)erealloc(dynamic_buffer, dynamic_buffer_size + 1);
            if (!temp_buffer) {
                php_error_docref(NULL, E_WARNING, "Out of memory");
                efree(dynamic_buffer);
                fclose(data_fp);
                fclose(index_fp);
                RETURN_FALSE;
            }
            dynamic_buffer = temp_buffer;
        }
    }

    efree(dynamic_buffer);
    fclose(index_fp);

    if (!found_match) {
        fclose(data_fp);
        RETURN_FALSE;
    }

    char *colon_ptr = strchr(found_value, ':');
    if (!colon_ptr) {
        efree(found_value);
        fclose(data_fp);
        RETURN_FALSE;
    }

    *colon_ptr = '\0';
    long offset = atol(found_value);
    size_t size = (size_t)strtoul(colon_ptr + 1, NULL, 10);
    efree(found_value);

    if (size > 0) {
        if (fseek(data_fp, offset, SEEK_SET) < 0) {
            php_error_docref(NULL, E_WARNING, "Failed to seek file: %s", filename);
            fclose(data_fp);
            RETURN_FALSE;
        }

        char *dataBuffer = emalloc(size + 1);
        if (!dataBuffer) {
            php_error_docref(NULL, E_WARNING, "Outof memory");
            fclose(data_fp);
            RETURN_FALSE;
        }

        bytesRead = fread(dataBuffer, 1, size, data_fp);
        if (bytesRead < size) {
            efree(dataBuffer);
            fclose(data_fp);
            php_error_docref(NULL, E_WARNING, "Failed to read data block.");
            RETURN_FALSE;
        }

        dataBuffer[size] = '\0';
        RETVAL_STRING(dataBuffer);
        efree(dataBuffer);
    } else {
        RETURN_FALSE;
    }

    fclose(data_fp);
}



/* Реализация функции */
PHP_FUNCTION(write_key_value_pair) {
    char *filename, *index_key, *index_val;
    size_t filename_len, index_key_len, index_val_len;

    // Парсинг аргументов, переданных в функцию
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "sss", &filename, &filename_len, &index_key, &index_key_len, &index_val, &index_val_len) == FAILURE) {
        RETURN_FALSE;
    }

    // Открытие файла для добавления; текстовый режим
    FILE *fp = fopen(filename, "a");
    
    if (!fp) {
        php_error_docref(NULL, E_WARNING, "Failed to open file: %s", filename);
        RETURN_LONG(-1);
    }

    // Блокировка файла для записи
    if (flock(fileno(fp), LOCK_EX) == -1) {
        php_error_docref(NULL, E_WARNING, "Failed to lock the file: %s", filename);
        fclose(fp); // Это также разблокирует файл
        RETURN_LONG(-2);
    }

    // Запись пары ключ-значение в файл
    if (fprintf(fp, "%s %s\n", index_key, index_val) < 0) {
        php_error_docref(NULL, E_WARNING, "Failed to write to the file: %s", filename);
        fclose(fp); // Это также разблокирует файл
        RETURN_LONG(-3);
    }

    fclose(fp); // Закрытие файла также разблокирует его
    RETURN_LONG(1);
}



/* Реализация функции */
PHP_FUNCTION(indexed_write_key_value_pair) {
    char *filename, *index_key, *index_val;
    size_t filename_len, index_key_len, index_val_len;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "sss", &filename, &filename_len, &index_key, &index_key_len, &index_val, &index_val_len) == FAILURE) {
        RETURN_FALSE;
    }

    char index_filename[filename_len + 7];
    snprintf(index_filename, filename_len + 7, "%s.index", filename);

    FILE *data_fp = fopen(filename, "ab+");
    FILE *index_fp = fopen(index_filename, "ab+");

    if (!data_fp) {
        php_error_docref(NULL, E_WARNING, "Failed to open file: %s", filename);
        if (index_fp) fclose(index_fp);
        RETURN_LONG(-1);
    }

    if (!index_fp) {
        php_error_docref(NULL, E_WARNING, "Failed to open file: %s", index_filename);
        fclose(data_fp);
        RETURN_LONG(-1);
    }

    // Блокировка файла для записи
    if (flock(fileno(data_fp), LOCK_EX) == -1) {
        php_error_docref(NULL, E_WARNING, "Failed to lock the file: %s", filename);
        fclose(data_fp); // Это также разблокирует файл
        fclose(index_fp); 
        RETURN_LONG(-2);
    }
    // Блокировка файла для записи
    if (flock(fileno(index_fp), LOCK_EX) == -1) {
        php_error_docref(NULL, E_WARNING, "Failed to lock the file: %s", filename);
        fclose(index_fp); // Это также разблокирует файл
        fclose(data_fp);
        RETURN_LONG(-2);
    }

    // Запись значения в файл данных
    fseek(data_fp, 0, SEEK_END); // Перемещаем указатель в конец файла
    // Получаем текущее смещение в файле данных
    long offset = ftell(data_fp);
    size_t size = strlen(index_val);

    if (fwrite(index_val, 1, size, data_fp) != size) {
        php_error_docref(NULL, E_WARNING, "Failed to write to the file: %s", filename);
        fclose(data_fp);
        fclose(index_fp);
        RETURN_LONG(-3);
    }

    // Запись индекса в индексный файл
    fprintf(index_fp, "%s %ld:%zu\n", index_key, offset, size);

    // Закрытие файлов
    fclose(data_fp);
    fclose(index_fp);
    RETURN_LONG(1);
}


/* Реализация функции */
PHP_FUNCTION(delete_key_value_pair) { // рефакторинг
    char *filename, *index_key = NULL;
    size_t filename_len, index_key_len = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "s|s", &filename, &filename_len, &index_key, &index_key_len) == FAILURE) {
        RETURN_FALSE;
    }

    char *temp_filename = emalloc(filename_len + 5); // Дополнительные символы для ".tmp" и терминирующего нуля
    snprintf(temp_filename, filename_len + 5, "%s.tmp", filename);
    
    int fd = open(filename, O_RDWR);
    if (fd == -1) {
        efree(temp_filename);
        php_error_docref(NULL, E_WARNING, "Failed to open file: %s", filename);
        RETURN_LONG(-1);
    }

    int temp_fd = open(temp_filename, O_RDWR | O_CREAT | O_APPEND, 0644);
    if (temp_fd == -1) {
        close(fd);
        unlink(temp_filename);
        php_error_docref(NULL, E_WARNING, "Failed to open file: %s", temp_filename);
        efree(temp_filename);
        RETURN_LONG(-2);
    }

    // Блокировка файлов
    if (lock_file(fd, LOCK_EX) == -1) {
        php_error_docref(NULL, E_WARNING, "Failed to lock the file: %s", filename);
        close(fd);
        close(temp_fd);
        unlink(temp_filename);
        efree(temp_filename);
        RETURN_LONG(-3);
    }

    FILE *file = fdopen(fd, "r");
    FILE *temp_file = fdopen(temp_fd, "w");
    
    if (!file || !temp_file) {
        php_error_docref(NULL, E_WARNING, "Failed to associate a stream with the file descriptor.");
        if (file) fclose(file);
        if (temp_file) fclose(temp_file);
        close(fd);
        close(temp_fd);
        unlink(temp_filename);
        efree(temp_filename);
        RETURN_LONG(-4);
    }

    zend_long ini_buffer_size = FAST_IO_G(buffer_size);

    char *buffer = (char *)emalloc(ini_buffer_size + 1);
    size_t bytesRead;
    char specialChar = 127;

    // Инициализация динамического буфера
    char *dynamicBuffer = NULL;
    size_t dynamicBufferSize = 0;
    
    while ((bytesRead = fread(buffer, 1, ini_buffer_size, file)) > 0) {
        buffer[bytesRead] = '\0';

        // Если в динамическом буфере уже есть данные, добавляем новые данные к ним
        if (dynamicBufferSize > 0) {
            dynamicBuffer = (char *)erealloc(dynamicBuffer, dynamicBufferSize + bytesRead + 1);
            memcpy(dynamicBuffer + dynamicBufferSize, buffer, bytesRead + 1);
            dynamicBufferSize += bytesRead;
        } else {
            dynamicBuffer = (char *)emalloc(bytesRead + 1);
            memcpy(dynamicBuffer, buffer, bytesRead + 1);
            dynamicBufferSize = bytesRead;
        }

        char *lineStart = dynamicBuffer;
        char *lineEnd;

        while ((lineEnd = strchr(lineStart, '\n')) != NULL) {
            *lineEnd = '\0';

            if (index_key != NULL) {
                if (
                    strncmp(lineStart, &specialChar, 1) != 0 && 
                    (
                        strstr(lineStart, index_key) != NULL || 
                        lineStart[strlen(index_key)] != ' '
                    )
                ) {
                    fprintf(temp_file, "%s\n", lineStart);
                }
            } else {
                if (strncmp(lineStart, &specialChar, 1) != 0) {
                    fprintf(temp_file, "%s\n", lineStart);
                }
            }

            lineStart = lineEnd + 1;
        }

        // Переносим непрочитанный остаток в начало динамического буфера
        dynamicBufferSize -= (lineStart - dynamicBuffer);
        memmove(dynamicBuffer, lineStart, dynamicBufferSize + 1); // +1 для '\\0'
    }

    fclose(file);
    fclose(temp_file);
    close(fd); 
    close(temp_fd);
    efree(buffer);
    if (dynamicBuffer) efree(dynamicBuffer);

    // Заменяем оригинальный файл временным файлом
    if (rename(temp_filename, filename) == -1) {
        unlink(temp_filename);
        efree(temp_filename);
        php_error_docref(NULL, E_WARNING, "Failed to replace the original file with the temporary file.");
        RETURN_LONG(-5);
    }
    efree(temp_filename);

    RETURN_TRUE;
}




PHP_FUNCTION(rebuild_data_file) { // рефакторинг
    char *filename, *index_key = NULL;
    size_t filename_len, index_key_len = 0;
    long ret_code = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "s|s", &filename, &filename_len, &index_key, &index_key_len) == FAILURE) {
        RETURN_FALSE;
    }
    
    char *index_filename = emalloc(filename_len + 7); // Дополнительные символы для ".index" и терминирующего нуля
    snprintf(index_filename, filename_len + 7, "%s.index", filename);
    char *temp_filename = emalloc(filename_len + 5); // Дополнительные символы для ".tmp" и терминирующего нуля
    snprintf(temp_filename, filename_len + 5, "%s.tmp", filename);
    char *temp_index_filename = emalloc(filename_len + 11); // Дополнительные символы для ".index.tmp" и терминирующего нуля
    snprintf(temp_index_filename, filename_len + 11, "%s.index.tmp", filename);


    // Открытие файлов
    int index_fd = open(index_filename, O_RDONLY);
    int data_fd = open(filename, O_RDONLY);
    int temp_data_fd = open(temp_filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    int temp_index_fd = open(temp_index_filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if (index_fd == -1) {
        php_error_docref(NULL, E_WARNING, "Failed to open file: %s", index_filename);
        ret_code = -1;
        goto check;
    }
    if (data_fd == -1) {
        php_error_docref(NULL, E_WARNING, "Failed to open file: %s", index_filename);
        ret_code = -1;
        goto check;
    }
    if (temp_data_fd == -1) {
        php_error_docref(NULL, E_WARNING, "Failed to open file: %s", temp_filename);
        ret_code = -1;
        goto check;
    }
    if (temp_index_fd == -1) {
        php_error_docref(NULL, E_WARNING, "Failed to open file: %s", temp_index_filename);
        ret_code = -1;
        goto check;
    }


    // Блокировка файлов
    if (lock_file(data_fd, LOCK_EX) == -1) {
        php_error_docref(NULL, E_WARNING, "Failed to lock the file: %s", filename);
        ret_code = -2;
        goto check;
    }
    if (lock_file(index_fd, LOCK_EX) == -1) {
        php_error_docref(NULL, E_WARNING, "Failed to lock the file: %s", index_filename);
        ret_code = -2;
        goto check;
    }

    check:
    if(ret_code != 0){
        if (index_fd != -1) close(index_fd);
        if (data_fd != -1) close(data_fd);
        if (temp_data_fd != -1) {
            close(temp_data_fd);
            unlink(temp_filename);
        }
        if (temp_index_fd != -1) {
            close(temp_index_fd);
            unlink(temp_index_filename);
        }
        
        efree(index_filename);
        efree(temp_filename);
        efree(temp_index_filename);
        RETURN_LONG(ret_code);
    }


    zend_long ini_buffer_size = FAST_IO_G(buffer_size);

    char *buffer = emalloc(ini_buffer_size);
    char specialChar = 127;
    ssize_t bytesRead;

    // Чтение индексного файла порциями
    while ((bytesRead = read(index_fd, buffer, ini_buffer_size)) > 0) {
        int bufferPos = 0;
        while (bufferPos < bytesRead) {
            char *lineStart = buffer + bufferPos;
            char *lineEnd = memchr(lineStart, '\n', bytesRead - bufferPos);
            if (!lineEnd) break; // Если конец строки не найден

            size_t lineLength = lineEnd - lineStart;
            char *line = emalloc(ini_buffer_size);

            strncpy(line, lineStart, lineLength);
            line[lineLength] = '\0';

            bufferPos += lineLength + 1;

            // Парсинг строки индексного файла
            char *keyEnd = strchr(line, ' ');
            if (!keyEnd) continue; // Если формат строки неверен

            *keyEnd = '\0';

            if (index_key != NULL && strcmp(line, index_key) == 0) continue; // Пропускаем строку с исключаемым ключом
            if (strncmp(lineStart, &specialChar, 1) == 0) continue; // Пропускаем строку с исключаемыми ключами

            long offset = atol(keyEnd + 1);
            char *sizePtr = strchr(keyEnd + 1, ':');
            if (!sizePtr) continue;

            size_t size = atol(sizePtr + 1);

            // Чтение и запись блока данных
            lseek(data_fd, offset, SEEK_SET);
            char *dataBuffer = emalloc(size);
            
            if(read(data_fd, dataBuffer, size) == -1) {
                php_error_docref(NULL, E_WARNING, "Failed to read data block.");
                ret_code = -3;
                goto check_error;
            }
            if(write(temp_data_fd, dataBuffer, size) == -1) {
                php_error_docref(NULL, E_WARNING, "Failed to write data block.");
                ret_code = -4;
                goto check_error;
            }

            check_error:
            if(ret_code != 0){
                efree(dataBuffer);
                efree(line);
                efree(buffer);
                goto check;
            }

            // Запись во временный индексный файл
            dprintf(temp_index_fd, "%s %ld:%zu\n", line, offset, size);
            efree(dataBuffer);
            efree(line);
        }
    }

    efree(buffer);

    // Закрытие файлов
    close(index_fd);
    close(data_fd);
    close(temp_data_fd);
    close(temp_index_fd);

    ret_code = 1;

    // Заменяем оригинальный файл временным файлом
    if (rename(temp_filename, filename) == -1 || rename(temp_index_filename, index_filename)) {
        php_error_docref(NULL, E_WARNING, "Failed to replace the original file with the temporary file.");
        unlink(temp_filename);
        unlink(temp_index_filename);
        ret_code = -5;
    }

    efree(index_filename);
    efree(temp_filename);
    efree(temp_index_filename);
    RETURN_LONG(ret_code);
}



/* Функция для извлечения и удаления последней строки из файла */
PHP_FUNCTION(pop_key_value_pair) {
    char *filename;
    size_t filename_len;
    zend_long index_align = -1; // Значение по умолчанию для необязательного аргумента

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "s|l", &filename, &filename_len, &index_align) == FAILURE) {
        RETURN_FALSE;
    }

    FILE *fp = fopen(filename, "r+b"); 
    if (!fp) {
        php_error_docref(NULL, E_WARNING, "Failed to open file: %s", filename);
        RETURN_LONG(-1);
    }

    // Блокировка файла для записи
    if (flock(fileno(fp), LOCK_EX) == -1) {
        php_error_docref(NULL, E_WARNING, "Failed to lock the file: %s", filename);
        fclose(fp);
        RETURN_LONG(-2);
    }

    // Перемещение указателя в конец файла для получения его размера
    fseek(fp, 0, SEEK_END);

    // Получаем текущее смещение в файле данных
    off_t fileSize = ftell(fp);
    if (fileSize <= 0) {
        fclose(fp);
        RETURN_FALSE;
    }

    off_t pos = fileSize;
    ssize_t bytesRead;

    if (index_align != -1) {
        pos -= index_align + 1;
        fseek(fp, pos , SEEK_SET);

        // Увеличиваем размер буфера на 1 для возможного символа перевода строки
        char *buffer = (char *)emalloc(index_align + 1); // +1 для '\0'
        bytesRead = fread(buffer, 1, index_align, fp);

        if (bytesRead < 0) {
            php_error_docref(NULL, E_WARNING, "Error reading file: %s", filename);
            efree(buffer);
            fclose(fp);
            RETURN_FALSE;
        }

        // Убедимся, что строка нуль-терминирована
        buffer[bytesRead] = '\0';

        // Обрезка пробелов справа и символа перевода строки
        for (int i = bytesRead - 1; i >= 0; --i) {
            if(buffer[i] == ' ' || buffer[i] == '\n') buffer[i] = '\0';
            else break;
        }

        // Усекаем файл
        if(ftruncate(fileno(fp), pos)) {
            efree(buffer);
            fclose(fp);
            php_error_docref(NULL, E_WARNING, "Failed to truncate file: %s", filename);
            RETURN_FALSE;
        }
        
        fclose(fp);

        // Возврат строки в PHP
        RETVAL_STRING(buffer);
        efree(buffer);
        return;
    }


    // Авто поиск последней строки
    zend_long ini_buffer_size = FAST_IO_G(buffer_size);
    char *buffer = (char *)emalloc(ini_buffer_size + 1);
    zend_string *result_str = NULL;
    int found_line_start = 0;

    while (pos > 0 && !found_line_start) {
        pos -= ini_buffer_size;
        pos = pos < 0 ? 0 : pos;

        fseek(fp, pos , SEEK_SET);
        bytesRead = fread(buffer, 1, ini_buffer_size, fp);

        if (bytesRead < 0) {
            php_error_docref(NULL, E_WARNING, "Error reading file: %s", filename);
            fclose(fp);
            efree(buffer);
            if (result_str) zend_string_release(result_str);
            RETURN_FALSE;
        }

        for (ssize_t i = bytesRead - 1; i >= 0; --i) {
            if (
                buffer[i] == '\n'
            ) {
                if (!result_str) { // Найден первый перенос строки с конца
                    result_str = zend_string_alloc(bytesRead - i - 1, 0);
                    memcpy(ZSTR_VAL(result_str), buffer + i + 1, bytesRead - i - 1);
                } else if (i != bytesRead - 1 || pos + bytesRead < fileSize) { // Найдено начало строки
                    size_t new_len = ZSTR_LEN(result_str) + bytesRead - i - 1;
                    result_str = zend_string_extend(result_str, new_len, 0);
                    memmove(ZSTR_VAL(result_str) + bytesRead - i - 1, ZSTR_VAL(result_str), ZSTR_LEN(result_str) - (bytesRead - i - 1));
                    memcpy(ZSTR_VAL(result_str), buffer + i + 1, bytesRead - i - 1);

                    found_line_start = 1;
                    break;
                }
            }
        }


        if (!found_line_start && bytesRead > 0 && result_str != NULL && pos == 0) {
            size_t result_str_len = ZSTR_LEN(result_str);
            zend_string *new_result_str = NULL;

            if(result_str_len + 1 > ini_buffer_size && bytesRead == ini_buffer_size ){
                result_str = zend_string_alloc(result_str_len + 1, 0);
                memcpy(ZSTR_VAL(result_str), buffer, bytesRead);
            } else {
                result_str = zend_string_alloc(bytesRead, 0);
                memcpy(ZSTR_VAL(result_str), buffer, bytesRead);
            }

            found_line_start = 1;
            break;
        }

        if (!found_line_start && bytesRead == ini_buffer_size && result_str != NULL) { // Если строка начинается до текущего буфера
            result_str = zend_string_extend(result_str, ZSTR_LEN(result_str) + ini_buffer_size, 0);
            memmove(ZSTR_VAL(result_str) + ini_buffer_size, ZSTR_VAL(result_str), ZSTR_LEN(result_str) - ini_buffer_size);
            memcpy(ZSTR_VAL(result_str), buffer, ini_buffer_size);
        }
    }

    efree(buffer);

    if (
        found_line_start && result_str != NULL ||
        (
            result_str != NULL
        )
    ) {
        ZSTR_VAL(result_str)[ZSTR_LEN(result_str)] = '\0'; // Установить конечный нулевой символ

        // Обрезка пробелов справа и символа перевода строки
        size_t len = ZSTR_LEN(result_str);
        char *str = ZSTR_VAL(result_str);

        // Усечь файл
        off_t new_file_size = fileSize - len;
        if(new_file_size < 0) new_file_size = 0;

        if(ftruncate(fileno(fp), new_file_size)) {
            fclose(fp);
            php_error_docref(NULL, E_WARNING, "Failed to truncate file: %s", filename);
            RETURN_FALSE;
        }

        // Находим позицию, до которой нужно обрезать строку
        ssize_t i;
        for (i = len - 1; i >= 0; --i) {
            if(str[i] != ' ' && str[i] != '\n') break;
        }

        // Обрезаем строку, если найдены пробелы или символы перевода строки
        if (i < (ssize_t)(len - 1)) {
            // Устанавливаем новый конец строки
            str[i + 1] = '\0';
            // Обновляем длину zend_string
            ZSTR_LEN(result_str) = i + 1;
        }

        RETVAL_STR(result_str);
    } else {
        RETVAL_FALSE;
    }

    fclose(fp);
}





/* Реализация функции */
PHP_FUNCTION(hide_key_value_pair) {
    char *filename, *index_key;
    size_t filename_len, index_key_len;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "ss", &filename, &filename_len, &index_key, &index_key_len) == FAILURE) {
        RETURN_FALSE;
    }

    FILE *fp = fopen(filename, "r+");
    if (fp == NULL) {
        php_error_docref(NULL, E_WARNING, "Failed to open file: %s", filename);
        RETURN_LONG(-1);
    }

    // Попытка установить блокирующую блокировку на запись
    if (flock(fileno(fp), LOCK_EX) < 0) {
        php_error_docref(NULL, E_WARNING, "Failed to lock file: %s", filename);
        fclose(fp);
        RETURN_LONG(-2);
    }

    zend_long ini_buffer_size = FAST_IO_G(buffer_size);
    zend_long dynamic_buffer_size = ini_buffer_size;
    char *dynamic_buffer = (char *)emalloc(dynamic_buffer_size + 1);

    ssize_t bytesRead;
    size_t current_size = 0; // Текущий размер данных в динамическом буфере
    bool found_match = false;

    bool isEOF;
    off_t writeOffset = 0; // Смещение для записи обновленных данных

    while ((bytesRead = fread(dynamic_buffer + current_size, 1, ini_buffer_size, fp)) > 0) {
        current_size += bytesRead;
        // Проверяем, достигли ли мы конца файла (EOF)
        isEOF = feof(fp);
        
        if (isEOF && dynamic_buffer[current_size - 1] != '\n') {
            // Если это EOF и последний символ не является переводом строки,
            // добавляем перевод строки для упрощения обработки
            dynamic_buffer[current_size ] = '\n';
            current_size++;
        }
        
        dynamic_buffer[current_size] = '\0';

        char *lineStart = dynamic_buffer;
        char *lineEnd;
        while ((lineEnd = strchr(lineStart, '\n')) != NULL) {
            ssize_t lineLength = lineEnd - lineStart + 1;
            *lineEnd = '\0';

            if (strstr(lineStart, index_key) != NULL) {
                // Найдено совпадение ключа, подготавливаем замену
                char *replacement = emalloc(index_key_len + 1);
                memset(replacement, 127, index_key_len); // Заполнение символами DEL
                replacement[index_key_len] = ' '; // Сохраняем пробел после ключа
                
                // Перемещаемся к началу найденной строки и записываем замену
                fseek(fp, writeOffset , SEEK_SET);

                if (fwrite(replacement, 1, index_key_len + 1, fp) < 0) {
                    efree(replacement);
                    efree(dynamic_buffer);
                    php_error_docref(NULL, E_WARNING, "Failed to write to the file: %s", filename);
                    fclose(fp);
                    RETURN_LONG(-3);
                }

                efree(replacement);
                found_match = true;
                break;
            }

            writeOffset += lineLength; // Обновляем смещение
            lineStart = lineEnd + 1;
        }

        if (found_match) break;

        if (!isEOF) {
            current_size -= (lineStart - dynamic_buffer);
            memmove(dynamic_buffer, lineStart, current_size);

            if (current_size + ini_buffer_size > dynamic_buffer_size) {
                dynamic_buffer_size += ini_buffer_size;
                dynamic_buffer = (char *)erealloc(dynamic_buffer, dynamic_buffer_size + 1);
                if (!dynamic_buffer) {
                    php_error_docref(NULL, E_WARNING, "Out of memory");
                    break;
                }
            }
        }
    }

    efree(dynamic_buffer);
    fclose(fp);

    if(found_match) RETURN_LONG(1);
    RETURN_LONG(0);
}




/* Реализация функции */
PHP_FUNCTION(get_index_keys) {
    char *filename;
    size_t filename_len;
    zend_long mode = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "s|l", &filename, &filename_len, &mode) == FAILURE) {
        RETURN_FALSE;
    }

    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        php_error_docref(NULL, E_WARNING, "Failed to open file: %s", filename);
        RETURN_FALSE;
    }

    // Попытка установить блокирующую блокировку на запись
    if (flock(fileno(fp), LOCK_EX) < 0) {
        php_error_docref(NULL, E_WARNING, "Failed to lock file: %s", filename);
        fclose(fp);
        RETURN_FALSE;
    }

    zend_long ini_buffer_size = FAST_IO_G(buffer_size);
    zend_long dynamic_buffer_size = ini_buffer_size;
    char *dynamic_buffer = (char *)emalloc(dynamic_buffer_size + 1);

    ssize_t bytesRead;
    size_t current_size = 0; // Текущий размер данных в динамическом буфере
    bool found_match = false;

    bool isEOF;

    KeyValueArray keys_values = {0};
    KeyArray keys = {0};

    off_t writeOffset = 0; // Смещение для записи обновленных данных


    while ((bytesRead = fread(dynamic_buffer + current_size, 1, ini_buffer_size, fp)) > 0) {
        current_size += bytesRead;
        // Проверяем, достигли ли мы конца файла (EOF)
        isEOF = feof(fp);
        
        if (isEOF && dynamic_buffer[current_size - 1] != '\n') {
            // Если это EOF и последний символ не является переводом строки,
            // добавляем перевод строки для упрощения обработки
            dynamic_buffer[current_size ] = '\n';
            current_size++;
        }
        
        dynamic_buffer[current_size] = '\0';

        char *lineStart = dynamic_buffer;
        char *lineEnd;
        while ((lineEnd = strchr(lineStart, '\n')) != NULL) {
            ssize_t lineLength = lineEnd - lineStart + 1;
            *lineEnd = '\0';

            if(mode == 0){
                char *spacePos = strchr(lineStart, ' ');
                if (spacePos) *spacePos = '\0';
                if(add_key(&keys, lineStart) == false){
                    php_error_docref(NULL, E_WARNING, "Out of memory");
                    found_match = true;
                    break;
                }
            }

            if(mode == 1){
                int value[2];
                value[0] = writeOffset;
                value[1] = lineLength;
                if(add_key_value(&keys_values, value) == false){
                    php_error_docref(NULL, E_WARNING, "Out of memory");
                    found_match = true;
                    break;
                }
            }

            writeOffset += lineLength; // Обновляем смещение
            lineStart = lineEnd + 1;
        }

        if (found_match) break;

        if (!isEOF) {
            current_size -= (lineStart - dynamic_buffer);
            memmove(dynamic_buffer, lineStart, current_size);

            if (current_size + ini_buffer_size > dynamic_buffer_size) {
                dynamic_buffer_size += ini_buffer_size;
                dynamic_buffer = (char *)erealloc(dynamic_buffer, dynamic_buffer_size + 1);
                if (!dynamic_buffer) {
                    php_error_docref(NULL, E_WARNING, "Out of memory");
                    break;
                }
            }
        }
    }

    efree(dynamic_buffer);
    fclose(fp);

    if(mode == 0){
        array_init(return_value);
        for (size_t i = 0; i < keys.count; i++) {
            add_next_index_string(return_value, keys.keys[i]);
        }
    }

    if(mode == 1){
        array_init(return_value);
        for (size_t i = 0; i < keys_values.count; i++) {
            zval key_value_arr;
            array_init(&key_value_arr);

            add_index_long(&key_value_arr, 0, keys_values.values[i][0]);
            add_index_long(&key_value_arr, 1, keys_values.values[i][1]);       
            add_next_index_zval(return_value, &key_value_arr);
        }
    } 

    free_key_value_array(&keys_values);
    free_key_array(&keys);
    if(mode == -1) RETURN_FALSE;
}



// Функция обновления пары ключ-значение
PHP_FUNCTION(update_key_value_pair) {// рефакторинг
    char *filename;
    size_t filename_len;
    char *index_key;
    size_t index_key_len;
    char *index_value;
    size_t index_value_len;

    // Парсинг аргументов
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "sss", &filename, &filename_len, &index_key, &index_key_len, &index_value, &index_value_len) == FAILURE) {
        RETURN_FALSE;
    }

    char *temp_filename = emalloc(filename_len + 5); // Дополнительные символы для ".tmp" и нуль-терминатора
    snprintf(temp_filename, filename_len + 5, "%s.tmp", filename);

    int fd = open(filename, O_RDWR);
    if (fd == -1) {
        php_error_docref(NULL, E_WARNING, "Failed to open file: %s", filename);
        efree(temp_filename);
        RETURN_LONG(-1);
    }

    int temp_fd = open(temp_filename, O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (temp_fd == -1) {
        php_error_docref(NULL, E_WARNING, "Failed to open file: %s", temp_filename);
        close(fd);
        efree(temp_filename);
        RETURN_LONG(-2);
    }

    // Блокировка файлов
    if (lock_file(fd, LOCK_EX) == -1) {
        php_error_docref(NULL, E_WARNING, "Failed to lock the file: %s", filename);
        close(fd);
        close(temp_fd);
        unlink(temp_filename);
        efree(temp_filename);
        RETURN_LONG(-3);
    }

    FILE *file = fdopen(fd, "r");
    FILE *temp_file = fdopen(temp_fd, "w");

    if (!file || !temp_file) {
        php_error_docref(NULL, E_WARNING, "Failed to associate a stream with the file descriptor.");
        if (file) fclose(file);
        if (temp_file) fclose(temp_file);
        close(fd);
        close(temp_fd);
        unlink(temp_filename);
        efree(temp_filename);
        RETURN_LONG(-4);
    }


    zend_long ini_buffer_size = FAST_IO_G(buffer_size);
    zend_long dynamic_buffer_size = ini_buffer_size;
    char *dynamic_buffer = (char *)emalloc(dynamic_buffer_size + 1);
    ssize_t bytesRead;
    bool found_value = false;
    size_t current_size = 0; // Текущий размер данных в динамическом буфере

    while ((bytesRead = read(fd, dynamic_buffer + current_size, ini_buffer_size)) > 0) {
        current_size += bytesRead;
        dynamic_buffer[current_size] = '\0'; // Завершаем прочитанный буфер нуль-терминатором

        char *lineStart = dynamic_buffer;
        char *lineEnd;

        while ((lineEnd = strchr(lineStart, '\n')) != NULL) {
            *lineEnd = '\0'; // Завершаем строку нуль-терминатором
            ssize_t lineLength = lineEnd - lineStart + 1;

            if (strstr(lineStart, index_key) != NULL && lineStart[index_key_len] == ' ') {
                fprintf(temp_file, "%s %s\n", index_key, index_value);
            } else {
                fprintf(temp_file, "%s\n", lineStart);
            }

            lineStart = lineEnd + 1; // Переходим к следующей строке
        }

        if (found_value) break;

        // Перемещаем непрочитанную часть в начало буфера и обновляем current_size
        current_size -= (lineStart - dynamic_buffer);
        memmove(dynamic_buffer, lineStart, current_size);

        // Проверяем, нужно ли расширить буфер
        if (current_size == dynamic_buffer_size) {
            dynamic_buffer_size *= 2; // Удваиваем размер буфера
            dynamic_buffer = (char *)erealloc(dynamic_buffer, dynamic_buffer_size + 1);
        }
    }

    fclose(file);
    fclose(temp_file);
    close(fd); 
    close(temp_fd);

    // Заменяем оригинальный файл временным файлом
    if (rename(temp_filename, filename) == -1) {
        unlink(temp_filename);
        efree(dynamic_buffer);
        efree(temp_filename);
        php_error_docref(NULL, E_WARNING, "Failed to replace the original file with the temporary file.");
        RETURN_LONG(-5);
    }

    efree(dynamic_buffer);
    efree(temp_filename);

    RETURN_LONG(1); // Успешное завершение операции
}



PHP_FUNCTION(insert_key_value) {
    char *filename;
    size_t filename_len;
    char *index_key;
    size_t index_key_len;
    zend_long index_align;

    // Парсинг аргументов, переданных в функцию
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "ssl", &filename, &filename_len, &index_key, &index_key_len, &index_align) == FAILURE) {
        RETURN_FALSE;
    }

    FILE *fp = fopen(filename, "ab+"); // Открытие файла для добавления и чтения; бинарный режим
    if (!fp) {
        php_error_docref(NULL, E_WARNING, "Failed to open file: %s", filename);
        RETURN_LONG(-1);
    }

    // Блокировка файла для записи
    if (flock(fileno(fp), LOCK_EX) == -1) {
        php_error_docref(NULL, E_WARNING, "Failed to lock the file: %s", filename);
        fclose(fp);
        RETURN_LONG(-2);
    }

    // Перемещение указателя в конец файла для получения его размера
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    long line_number = file_size / (index_align + 1); // Учет символа перевода строки

    // Подготовка строки к записи с учетом выравнивания
    char *buffer = (char *)emalloc(index_align + 2); // +1 для '\n' и +1 для '\0'
    memset(buffer, ' ', index_align); // Заполнение пробелами
    buffer[index_align] = '\n'; // Добавление перевода строки
    buffer[index_align + 1] = '\0';

    // Копирование index_key в буфер с учетом выравнивания
    strncpy(buffer, index_key, index_align < index_key_len ? index_align : index_key_len);

    // Запись в файл
    size_t written = fwrite(buffer, sizeof(char), index_align + 1, fp); // +1 для записи '\n'
    efree(buffer);
    
    if (written != index_align + 1) {
        php_error_docref(NULL, E_WARNING, "Failed to write to the file: %s", filename);
        fclose(fp);
        RETURN_LONG(-4);
    }
    
    fclose(fp); // Это также разблокирует файл

    // Возврат номера добавленной строки
    RETURN_LONG(line_number + 1); // Нумерация строк начинается с 1
}




PHP_FUNCTION(select_key_value) {
    char *filename;
    size_t filename_len;
    zend_long index_row;
    zend_long index_align;
    zend_long mode = 0;

    // Парсинг переданных аргументов
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "sll|l", &filename, &filename_len, &index_row, &index_align, &mode) == FAILURE) {
        RETURN_FALSE;
    }

    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        php_error_docref(NULL, E_WARNING, "Failed to open file: %s", filename);
        RETURN_FALSE;
    }

    // Попытка установить блокирующую блокировку на запись
    if (flock(fileno(fp), LOCK_EX) < 0) {
        php_error_docref(NULL, E_WARNING, "Failed to lock file: %s", filename);
        fclose(fp);
        RETURN_FALSE;
    }

    ssize_t bytesRead;
    off_t offset = (mode == 0) ? index_row * (index_align + 1) : index_row;

    if(fseek(fp, offset , SEEK_SET) < 0){
        php_error_docref(NULL, E_WARNING, "Failed to seek file: %s", filename);
        fclose(fp);
        RETURN_FALSE;
    }

    // Увеличиваем размер буфера на 1 для возможного символа перевода строки
    char *buffer = (char *)emalloc(index_align + 2); // +1 для '\0' и +1 для '\n'

    bytesRead = fread(buffer, 1, index_align + 1, fp);
    if(bytesRead < 0){
        php_error_docref(NULL, E_WARNING, "Error reading file: %s", filename);
        efree(buffer);
        fclose(fp);
        RETURN_FALSE;
    }

    // Убедимся, что строка нуль-терминирована
    buffer[bytesRead] = '\0';

    if (mode == 1) {
        char *lineEnd = strchr(buffer, '\n');
        if (lineEnd != NULL) {
            *lineEnd = '\0'; // Заменяем перевод строки на нуль-терминатор
        }
    }

    // Обрезка пробелов справа и символа перевода строки
    size_t len = strlen(buffer);
    for (int i = len - 1; i >= 0; --i) {
        if(buffer[i] == ' ' || buffer[i] == '\n') buffer[i] = '\0';
        else break;
    }

    fclose(fp);

    // Возврат строки в PHP
    RETVAL_STRING(buffer);
    efree(buffer);
}





PHP_FUNCTION(update_key_value) {
    char *filename;
    size_t filename_len;
    char *index_key;
    size_t index_key_len;
    zend_long index_row, index_align;

    // Парсинг аргументов, переданных в функцию
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "ssll", &filename, &filename_len, &index_key, &index_key_len, &index_row, &index_align) == FAILURE) {
        RETURN_FALSE;
    }

    FILE *fp = fopen(filename, "r+b"); // Открытие файла для чтения и записи в двоичном режиме
    if (!fp) {
        php_error_docref(NULL, E_WARNING, "Failed to open file: %s", filename);
        RETURN_LONG(-1);
    }

    // Попытка установить блокирующую блокировку на запись
    if (flock(fileno(fp), LOCK_EX) < 0) {
        php_error_docref(NULL, E_WARNING, "Failed to lock file: %s", filename);
        fclose(fp);
        RETURN_LONG(-2);
    }

    // Рассчитываем смещение для записи в файл, учитывая перевод строки
    long offset = (index_row - 1) * (index_align + 1); // +1 для '\n'
    if (fseek(fp, offset, SEEK_SET) != 0) {
        php_error_docref(NULL, E_WARNING, "Failed to seek in the file: %s", filename);
        fclose(fp);
        RETURN_LONG(-3);
    }

    // Подготовка строки к записи с учетом выравнивания и перевода строки
    char *buffer = (char *)emalloc(index_align + 2); // +1 для '\n', +1 для '\0'
    memset(buffer, ' ', index_align); // Заполнение пробелами
    buffer[index_align] = '\n'; // Добавление перевода строки
    buffer[index_align + 1] = '\0'; // Нуль-терминатор

    // Копирование index_key в буфер с учетом выравнивания
    strncpy(buffer, index_key, index_key_len < index_align ? index_key_len : index_align);

    // Запись в файл
    size_t written = fwrite(buffer, 1, index_align + 1, fp); // +1 для '\n'
    efree(buffer);
    fclose(fp); // Это также разблокирует файл

    if (written != index_align + 1) {
        php_error_docref(NULL, E_WARNING, "Failed to write to the file: %s", filename);
        RETURN_LONG(-4);
    }

    RETURN_TRUE;
}




PHP_FUNCTION(detect_align_size) {
    char *filename;
    size_t filename_len;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &filename, &filename_len) == FAILURE) {
        RETURN_FALSE;
    }

    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        php_error_docref(NULL, E_WARNING, "Failed to open file: %s", filename);
        RETURN_LONG(-1);
    }

    // Попытка установить блокирующую блокировку на запись
    if (flock(fileno(fp), LOCK_EX) < 0) {
        php_error_docref(NULL, E_WARNING, "Failed to lock file: %s", filename);
        fclose(fp);
        RETURN_LONG(-2);
    }

    zend_long ini_buffer_size = FAST_IO_G(buffer_size);
    char *buffer = (char *)emalloc(ini_buffer_size);

    ssize_t bytes_read;
    long max_length = 0, current_length = 0;

    while ((bytes_read = fread(buffer, 1, ini_buffer_size, fp)) > 0) {
        for (ssize_t i = 0; i < bytes_read; ++i) {
            if (buffer[i] == '\n') { // Конец текущей строки
                if (current_length > max_length) {
                    max_length = current_length; // Обновляем максимальную длину
                }
                current_length = 0; // Сброс длины для следующей строки
            } else {
                ++current_length; // Увеличиваем длину текущей строки
            }
        }
    }

    efree(buffer);
    fclose(fp);

    // Возвращаем максимальную длину строки. Если строка оканчивается в конце файла без '\n', её длина уже учтена.
    RETURN_LONG(max_length);
}



PHP_FUNCTION(find_matches_pcre2) {
    char *pattern;
    size_t pattern_len;
    char *subject;
    size_t subject_len;

    /* Парсинг аргументов, переданных из PHP */
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "ss", &pattern, &pattern_len, &subject, &subject_len) == FAILURE) {
        return;
    }

    pcre2_code *re;
    pcre2_match_data *match_data; 

    PCRE2_SIZE erroffset;
    int errorcode;
    re = pcre2_compile((PCRE2_SPTR)pattern, PCRE2_ZERO_TERMINATED, 0, &errorcode, &erroffset, NULL);

    if (re == NULL) {
        PCRE2_UCHAR message[256];
        pcre2_get_error_message(errorcode, message, sizeof(message));
        php_error_docref(NULL, E_WARNING, "PCRE2 compilation failed at offset %d: %s", (int)erroffset, message);
        RETURN_FALSE;
    }

    match_data = pcre2_match_data_create_from_pattern(re, NULL);

    int rc;
    PCRE2_SIZE *ovector;
    size_t start_offset = 0;

    zval return_matched;
    array_init(&return_matched);

    while ((rc = pcre2_match(re, (PCRE2_SPTR)subject, subject_len, start_offset, 0, match_data, NULL)) > 0) {
        ovector = pcre2_get_ovector_pointer(match_data);

        for (int i = 0; i < rc; i++) {
            PCRE2_SIZE start = ovector[2*i];
            PCRE2_SIZE end = ovector[2*i+1];
            add_next_index_stringl(&return_matched, subject + start, end - start);
        }

        // Изменение для предотвращения потенциального бесконечного цикла
        if (ovector[1] > start_offset) {
            start_offset = ovector[1];
        } else {
            start_offset++; // Для продолжения поиска следующего совпадения
            if (start_offset >= subject_len) break; // Выходим из цикла, если достигнут конец строки
        }
    }

    if (rc == PCRE2_ERROR_NOMATCH) {
        /* Если совпадений нет, возвращаем пустой массив. */
    } else if (rc < 0) {
        /* Обработка других ошибок. */
        php_error_docref(NULL, E_WARNING, "Matching error %d", rc);
        RETURN_FALSE;
    }

    /* Освобождаем выделенную память */
    pcre2_match_data_free(match_data);
    pcre2_code_free(re);

    /* Возвращаем подготовленный массив */
    RETURN_ZVAL(&return_matched, 0, 1);
}

