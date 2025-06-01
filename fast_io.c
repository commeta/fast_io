/*
 * Fast_IO (pre-release beta) Extension for PHP 8
 * https://github.com/commeta/fast_io
 * 
 * Copyright 2025 commeta <dcs-spb@ya.ru>
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
PHP_FUNCTION(file_search_array);
PHP_FUNCTION(file_search_line);
PHP_FUNCTION(file_search_data);
PHP_FUNCTION(file_push_data);
PHP_FUNCTION(file_defrag_lines);
PHP_FUNCTION(file_defrag_data);
PHP_FUNCTION(file_pop_line);
PHP_FUNCTION(file_erase_line);
PHP_FUNCTION(file_get_keys);
PHP_FUNCTION(file_replace_line);
PHP_FUNCTION(file_insert_line);
PHP_FUNCTION(file_select_line);
PHP_FUNCTION(file_update_line);
PHP_FUNCTION(file_analize);
PHP_FUNCTION(find_matches_pcre2);
PHP_FUNCTION(replicate_file);
PHP_FUNCTION(file_select_array);
PHP_FUNCTION(file_update_array);
PHP_FUNCTION(file_callback_line);



/* Запись аргументов функций */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_file_search_array, 1, 2, IS_ARRAY, 1)
    ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, line_key, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, search_start, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, search_limit, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, position, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, mode, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_file_search_line, 1, 2, IS_STRING, 1)
    ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, line_key, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, position, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, mode, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_file_search_data, 1, 2, IS_STRING, 1)
    ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, line_key, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, position, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, mode, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_file_push_data, 1, 3, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, line_key, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, line_value, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, mode, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_file_defrag_lines, 1, 1, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, line_key, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, mode, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_file_defrag_data, 1, 1, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, line_key, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, mode, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_file_pop_line, 1, 1, IS_STRING, 1)
    ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, offset, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, mode, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, end, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_file_erase_line, 1, 2, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, line_key, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, position, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, mode, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_file_get_keys, 1, 1, IS_ARRAY, 1)
    ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, search_start, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, search_limit, IS_LONG, 0)  
    ZEND_ARG_TYPE_INFO(0, position, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, mode, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_file_replace_line, 1, 3, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, line_key, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, line_value, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, mode, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_file_insert_line, 1, 2, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, line, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, mode, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, line_length, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_file_select_line, 1, 3, IS_STRING, 1)
    ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, row, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, align, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, mode, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_file_update_line, 1, 3, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, line, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, row, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, align, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, mode, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_file_analize, 1, 1, IS_ARRAY, 0)
    ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, mode, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_find_matches_pcre2, 1, 2, IS_ARRAY, 0)
    ZEND_ARG_TYPE_INFO(0, pattern, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, subject, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, mode, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_replicate_file, 1, 2, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, source, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, destination, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, mode, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_file_select_array, 1, 2, IS_ARRAY, 0)
    ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, array, IS_ARRAY, 0)
    ZEND_ARG_TYPE_INFO(0, pattern, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, mode, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_file_update_array, 1, 2, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, array, IS_ARRAY, 0)
    ZEND_ARG_TYPE_INFO(0, mode, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_file_callback_line, 1, 2, IS_STRING, 1)
    ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 0)
    ZEND_ARG_TYPE_INFO(0, position, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, mode, IS_LONG, 0)
ZEND_END_ARG_INFO()



/* Регистрация функций */
const zend_function_entry fast_io_functions[] = {
    PHP_FE(file_search_array, arginfo_file_search_array)
    PHP_FE(file_search_line, arginfo_file_search_line)
    PHP_FE(file_search_data, arginfo_file_search_data)
    PHP_FE(file_push_data, arginfo_file_push_data)
    PHP_FE(file_defrag_lines, arginfo_file_defrag_lines)
    PHP_FE(file_defrag_data, arginfo_file_defrag_data)
    PHP_FE(file_pop_line, arginfo_file_pop_line)
    PHP_FE(file_erase_line, arginfo_file_erase_line)
    PHP_FE(file_get_keys, arginfo_file_get_keys)
    PHP_FE(file_replace_line, arginfo_file_replace_line)
    PHP_FE(file_insert_line, arginfo_file_insert_line)
    PHP_FE(file_select_line, arginfo_file_select_line)
    PHP_FE(file_update_line, arginfo_file_update_line)
    PHP_FE(file_analize, arginfo_file_analize)
    PHP_FE(find_matches_pcre2, arginfo_find_matches_pcre2)
    PHP_FE(replicate_file, arginfo_replicate_file)
    PHP_FE(file_select_array, arginfo_file_select_array)
    PHP_FE(file_update_array, arginfo_file_update_array)
    PHP_FE(file_callback_line, arginfo_file_callback_line)
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




PHP_FUNCTION(file_search_array) {
    char *filename, *line_key;
    size_t filename_len, line_key_len;
    zend_long search_start = 0;
    zend_long search_limit = 1;
    zend_long position = 0;
    zend_long mode = 0;
    
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "ss|llll", &filename, &filename_len, &line_key, &line_key_len, &search_start, &search_limit, &position, &mode) == FAILURE) {
        RETURN_FALSE;
    }

    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        php_error_docref(NULL, E_WARNING, "Failed to open file: %s", filename);
        RETURN_FALSE;
    }

    // Попытка установить блокирующую блокировку на запись
    if(mode < 100){
        if (flock(fileno(fp), LOCK_EX) == -1) {
            php_error_docref(NULL, E_WARNING, "Failed to lock file: %s", filename);
            fclose(fp);
            RETURN_FALSE;
        }
    }

    if(mode > 99) mode -= 100;

    // Перемещение указателя в конец файла для получения его размера
    if (fseek(fp, 0, SEEK_END) != 0) {
        php_error_docref(NULL, E_WARNING, "Failed to seek to end of file: %s", filename);
        fclose(fp);
        RETURN_FALSE;
    }
    
    long file_size = ftell(fp);
    if (file_size == -1) {
        php_error_docref(NULL, E_WARNING, "Failed to get file size: %s", filename);
        fclose(fp);
        RETURN_FALSE;
    }

    long search_offset = 0; // Смещение строки поиска

    if(position > 0){
        if(position >= file_size){
            php_error_docref(NULL, E_WARNING, "Position exceeds file size: %s", filename);
            fclose(fp);
            RETURN_FALSE;
        }

        if (fseek(fp, position, SEEK_SET) != 0) {
            php_error_docref(NULL, E_WARNING, "Failed to seek file: %s", filename);
            fclose(fp);
            RETURN_FALSE;
        }
        search_offset = position;
    } else {
        if (fseek(fp, 0, SEEK_SET) != 0) {
            php_error_docref(NULL, E_WARNING, "Failed to seek to start of file: %s", filename);
            fclose(fp);
            RETURN_FALSE;
        }
    }

    long ini_buffer_size = FAST_IO_G(buffer_size);

    if(file_size < ini_buffer_size) ini_buffer_size = file_size;
    if(ini_buffer_size < 16) ini_buffer_size = 16;

    long dynamic_buffer_size = ini_buffer_size;
    char *dynamic_buffer = (char *)emalloc(dynamic_buffer_size + 1);
    if (!dynamic_buffer) {
        fclose(fp);
        zend_error(E_ERROR, "Out of memory to allocate %ld bytes", dynamic_buffer_size + 1);
        return;
    }

    long bytes_read;
    long current_size = 0; // Текущий размер данных в динамическом буфере

    size_t found_count = 0;
    size_t add_count = 0;
    size_t line_count = 0;

    bool found_match = false;

    pcre2_code *re = NULL;
    pcre2_match_data *match_data = NULL; 

    array_init(return_value);

    if(mode > 9){
        PCRE2_SIZE erroffset;
        int errorcode;
        re = pcre2_compile((PCRE2_SPTR)line_key, PCRE2_ZERO_TERMINATED, 0, &errorcode, &erroffset, NULL);

        if (re == NULL) {
            PCRE2_UCHAR message[256];
            pcre2_get_error_message(errorcode, message, sizeof(message));
            php_error_docref(NULL, E_WARNING, "PCRE2 compilation failed at offset %d: %s", (int)erroffset, message);
            fclose(fp);
            efree(dynamic_buffer);
            RETURN_FALSE;
        }

        match_data = pcre2_match_data_create_from_pattern(re, NULL);
        if (!match_data) {
            php_error_docref(NULL, E_WARNING, "Failed to create PCRE2 match data");
            fclose(fp);
            efree(dynamic_buffer);
            pcre2_code_free(re);
            RETURN_FALSE;
        }
    }

    while ((bytes_read = fread(dynamic_buffer + current_size, 1, ini_buffer_size, fp)) > 0) {
        current_size += bytes_read;
       
        dynamic_buffer[current_size] = '\0';

        char *line_start = dynamic_buffer;
        char *line_end;
        while ((line_end = strchr(line_start, '\n')) != NULL) {
            long line_length = line_end - line_start + 1;
            *line_end = '\0';

            if(mode < 3 && strstr(line_start, line_key) != NULL){
                found_count++;

                if(search_start < found_count){
                    add_count++;

                    zval line_arr;
                    array_init(&line_arr);

                    long i;

                    if(mode == 0 || mode == 2){
                        for (i = line_length - 2; i >= 0; --i) {
                            if(line_start[i] == ' ') line_start[i] = '\0';
                            else break;
                        }

                        add_assoc_string(&line_arr, "trim_line", line_start);
                    }

                    if(mode == 0){
                        add_assoc_long(&line_arr, "trim_length", i + 1);
                    }

                    if(mode == 1){
                        add_assoc_string(&line_arr, "line", line_start);
                    }

                    if(mode != 2){
                        add_assoc_long(&line_arr, "line_offset", search_offset);
                        add_assoc_long(&line_arr, "line_length", line_length);
                        add_assoc_long(&line_arr, "line_count", line_count);
                    }

                    add_next_index_zval(return_value, &line_arr);
                }
            }

            if(mode == 3 && strstr(line_start, line_key) != NULL){
                found_count++;
            }

            if(mode > 9 && mode < 13 && re != NULL && match_data != NULL && pcre2_match(re, (PCRE2_SPTR)line_start, line_length - 1, 0, 0, match_data, NULL) > 0){
                found_count++;

                if(search_start < found_count){
                    add_count++;

                    zval line_arr;
                    array_init(&line_arr);

                    long i;

                    if(mode == 10 || mode == 12){
                        for (i = line_length - 2; i >= 0; --i) {
                            if(line_start[i] == ' ') line_start[i] = '\0';
                            else break;
                        }
                    }
                    
                    if(mode == 10){
                        add_assoc_string(&line_arr, "trim_line", line_start);
                        add_assoc_long(&line_arr, "trim_length", i + 1);
                    } else {
                        add_assoc_string(&line_arr, "line", line_start);
                    }

                    if(mode != 12){
                        add_assoc_long(&line_arr, "line_offset", search_offset);
                        add_assoc_long(&line_arr, "line_length", line_length);
                        add_assoc_long(&line_arr, "line_count", line_count);
                    }

                    if(mode == 12){
                        add_next_index_string(return_value, line_start);
                    } else {
                        add_next_index_zval(return_value, &line_arr);
                    }
                }
            }

            if(mode == 13 && re != NULL && match_data != NULL && pcre2_match(re, (PCRE2_SPTR)line_start, line_length - 1, 0, 0, match_data, NULL) > 0){
                found_count++;
            }

            if(mode > 19 && mode < 25 && re != NULL && match_data != NULL){
                if(search_start < found_count + 1){
                    zval return_matched;
                    array_init(&return_matched);

                    int rc;
                    PCRE2_SIZE *ovector;
                    size_t start_offset = 0;

                    bool is_matched = false;

                    while ((rc = pcre2_match(re, (PCRE2_SPTR)line_start, line_length - 1, start_offset, 0, match_data, NULL)) > 0) {
                        ovector = pcre2_get_ovector_pointer(match_data);

                        for (int i = 0; i < rc; i++) {
                            PCRE2_SIZE start = ovector[2*i];
                            PCRE2_SIZE end = ovector[2*i+1];

                            if(mode > 21){
                                zval match_arr;
                                array_init(&match_arr);

                                if(mode == 23){
                                    add_next_index_stringl(&return_matched, line_start + start, end - start);
                                } else {
                                    add_assoc_stringl(&match_arr, "line_match", line_start + start, end - start);
                                }

                                if(mode != 23){
                                    add_assoc_long(&match_arr, "match_offset", start);
                                    add_assoc_long(&match_arr, "match_length", end - start);
                                    add_next_index_zval(&return_matched, &match_arr);
                                }
                                
                            } else {
                                add_next_index_stringl(&return_matched, line_start + start, end - start);
                            }
                        }

                        // Изменение для предотвращения потенциального бесконечного цикла
                        if (ovector[1] > start_offset) {
                            start_offset = ovector[1];
                        } else {
                            start_offset++; // Для продолжения поиска следующего совпадения
                            if (start_offset >= (size_t)(line_length - 1)) break; // Выходим из цикла, если достигнут конец строки
                        }

                        is_matched = true;
                    }

                    if (rc < -1) {
                        /* Обработка других ошибок. */
                        php_error_docref(NULL, E_WARNING, "Matching error %d", rc);
                        fclose(fp);
                        if (dynamic_buffer) efree(dynamic_buffer);
                        if (re != NULL) pcre2_code_free(re);
                        if (match_data != NULL) pcre2_match_data_free(match_data);
                        RETURN_FALSE;
                    }

                    if(is_matched){
                        add_count++;

                        zval line_arr;
                        array_init(&line_arr);

                        if(mode == 20) {
                            long i;

                            for (i = line_length - 2; i >= 0; --i) {
                                if(line_start[i] == ' ') line_start[i] = '\0';
                                else break;
                            }

                            add_assoc_string(&line_arr, "trim_line", line_start);
                            add_assoc_long(&line_arr, "trim_length", i + 1);
                        }

                        if(mode == 21) {
                            add_assoc_string(&line_arr, "line", line_start);
                        }

                        if(mode != 23){
                            add_assoc_zval(&line_arr, "line_matches", &return_matched);

                            add_assoc_long(&line_arr, "line_offset", search_offset);
                            add_assoc_long(&line_arr, "line_length", line_length);
                            add_assoc_long(&line_arr, "line_count", line_count);
                        }

                        if(mode == 23){
                            add_next_index_zval(return_value, &return_matched);
                        } else {
                            add_next_index_zval(return_value, &line_arr);
                        }
                    } else {
                        zval_dtor(&return_matched); // Освобождаем память, если совпадений не было
                    }

                    found_count++;
                }
            }
            
            search_offset += line_length; // Обновляем смещение
            line_start = line_end + 1;
            line_count++;

            if(add_count >= search_limit){
                found_match = true;
                break;
            }
        }

        if (found_match) break;

        // Подготавливаем буфер к следующему чтению, если это не конец файла
        long remaining_size = current_size - (line_start - dynamic_buffer);
        if (remaining_size > 0) {
            memmove(dynamic_buffer, line_start, remaining_size);
        }
        current_size = remaining_size;

        if (current_size + ini_buffer_size > dynamic_buffer_size) {
            dynamic_buffer_size += ini_buffer_size;
            char *temp_buffer = (char *)erealloc(dynamic_buffer, dynamic_buffer_size + 1);
            if (!temp_buffer) {
                fclose(fp);
                if (dynamic_buffer) efree(dynamic_buffer);
                if (re != NULL) pcre2_code_free(re);
                if (match_data != NULL) pcre2_match_data_free(match_data);
                zend_error(E_ERROR, "Out of memory to allocate %ld bytes", dynamic_buffer_size + 1);
                return;
            }
            dynamic_buffer = temp_buffer;
        }
    }

    fclose(fp);
    efree(dynamic_buffer);

    if(mode == 3 || mode == 13){      
        add_assoc_long(return_value, "line_count", line_count);
        add_assoc_long(return_value, "found_count", found_count);
    }

    if (re != NULL) pcre2_code_free(re);
    if (match_data != NULL) pcre2_match_data_free(match_data);
}



PHP_FUNCTION(file_search_line) {
    char *filename, *line_key;
    size_t filename_len, line_key_len;
    zend_long mode = 0;
    zend_long position = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "ss|ll", &filename, &filename_len, &line_key, &line_key_len, &position, &mode) == FAILURE) {
        RETURN_FALSE; // Неправильные параметры вызова функции
    }

    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        php_error_docref(NULL, E_WARNING, "Failed to open file: %s", filename);
        RETURN_FALSE;
    }

    // Попытка установить блокирующую блокировку на запись
    if(mode < 100){
        if (flock(fileno(fp), LOCK_EX) == -1) {
            php_error_docref(NULL, E_WARNING, "Failed to lock file: %s", filename);
            fclose(fp);
            RETURN_FALSE;
        }
    }

    if(mode > 99) mode -= 100;

    // Перемещение указателя в конец файла для получения его размера
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);

    if(position > 0){
        if(position >= file_size){
            php_error_docref(NULL, E_WARNING, "Failed to seek file: %s", filename);
            fclose(fp);
            RETURN_FALSE;
        }

        fseek(fp, position, SEEK_SET);
    } else {
        fseek(fp, 0, SEEK_SET);
    }

    long ini_buffer_size = FAST_IO_G(buffer_size);

    if(file_size < ini_buffer_size) ini_buffer_size = file_size;
    if(ini_buffer_size < 16) ini_buffer_size = 16;

    long dynamic_buffer_size = ini_buffer_size;
    char *dynamic_buffer = (char *)emalloc(dynamic_buffer_size + 1);
    if (!dynamic_buffer) {
        fclose(fp);
        zend_error(E_ERROR, "Out of memory to allocate %ld bytes", dynamic_buffer_size + 1);
    }

    size_t bytes_read;
    long current_size = 0; // Текущий размер данных в динамическом буфере
    bool found_match = false;

    pcre2_code *re = NULL;
    pcre2_match_data *match_data = NULL;

    char *found_value = NULL;

    size_t line_length;

    if(mode > 9){
        PCRE2_SIZE erroffset;
        int errorcode;
        re = pcre2_compile((PCRE2_SPTR)line_key, PCRE2_ZERO_TERMINATED, 0, &errorcode, &erroffset, NULL);

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

    while ((bytes_read = fread(dynamic_buffer + current_size, 1, ini_buffer_size, fp)) > 0) {
        current_size += bytes_read;
       
        dynamic_buffer[current_size] = '\0';

        char *line_start = dynamic_buffer;
        char *line_end;
        while ((line_end = strchr(line_start, '\n')) != NULL) {
            line_length = line_end - line_start;
            *line_end = '\0';

            if(mode == 0 && strstr(line_start, line_key) != NULL){
                found_value = estrndup(line_start, line_length);
                if(found_value == NULL){
                    fclose(fp);
                    efree(dynamic_buffer);
                    if (re != NULL) pcre2_code_free(re);
                    if (match_data != NULL) pcre2_match_data_free(match_data);
                    zend_error(E_ERROR, "Out of memory to allocate %ld bytes", line_length);
                }

                found_match = true;
                break;
            }

            if(mode == 10 && pcre2_match(re, (PCRE2_SPTR)line_start, line_length, 0, 0, match_data, NULL) > 0){
                found_value = estrndup(line_start, line_length);
                if(found_value == NULL){
                    fclose(fp);
                    efree(dynamic_buffer);
                    if (re != NULL) pcre2_code_free(re);
                    if (match_data != NULL) pcre2_match_data_free(match_data);
                    zend_error(E_ERROR, "Out of memory to allocate %ld bytes", line_length);
                }
                
                found_match = true;
                break;
            }

            line_start = line_end + 1;
        }

        if (found_match) break;

        current_size -= (line_start - dynamic_buffer);
        memmove(dynamic_buffer, line_start, current_size);

        if (current_size + ini_buffer_size > dynamic_buffer_size) {
            dynamic_buffer_size += ini_buffer_size;
            char *temp_buffer = (char *)erealloc(dynamic_buffer, dynamic_buffer_size + 1);
            if (!temp_buffer) {
                if (dynamic_buffer) efree(dynamic_buffer);
                if (re != NULL) pcre2_code_free(re);
                if (match_data != NULL) pcre2_match_data_free(match_data);
                fclose(fp);
                zend_error(E_ERROR, "Out of memory to allocate %ld bytes", dynamic_buffer_size + 1);
            }
            dynamic_buffer = temp_buffer;
        }
    }

    fclose(fp);
    efree(dynamic_buffer);

    if (re != NULL) pcre2_code_free(re);
    if (match_data != NULL) pcre2_match_data_free(match_data);

    if (found_value == NULL) {
        RETURN_FALSE;
    } else {
        if(mode == 1 || mode == 11){
            // Обрезка пробелов справа и символа перевода строки
            size_t len = strlen(found_value);
            for (size_t i = len; i > 0; --i) {
                if(found_value[i-1] == ' ' || found_value[i-1] == '\n') {
                    found_value[i-1] = '\0';
                } else {
                    break;
                }
            }
        }

        RETVAL_STRING(found_value);
        efree(found_value);
        return;
    }
}




PHP_FUNCTION(file_search_data) {
    char *filename, *line_key;
    size_t filename_len, line_key_len;
    zend_long position = 0;
    zend_long mode = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "ss|ll", &filename, &filename_len, &line_key, &line_key_len, &position, &mode) == FAILURE) {
        RETURN_FALSE;
    }

    char index_filename[filename_len + 7];
    snprintf(index_filename, sizeof(index_filename), "%s.index", filename);

    FILE *index_fp = fopen(index_filename, "r");
    if (!index_fp) {
        php_error_docref(NULL, E_WARNING, "Failed to open file: %s", index_filename);
        RETURN_FALSE;
    }

    // Перемещение указателя в конец файла для получения его размера
    fseek(index_fp, 0, SEEK_END);
    long file_size = ftell(index_fp);
    
    if (file_size < 0) {
        php_error_docref(NULL, E_WARNING, "Failed to get file size: %s", index_filename);
        fclose(index_fp);
        RETURN_FALSE;
    }

    if (position > 0) {
        if (position >= file_size) {
            php_error_docref(NULL, E_WARNING, "Position exceeds file size: %s", index_filename);
            fclose(index_fp);
            RETURN_FALSE;
        }
        fseek(index_fp, position, SEEK_SET);
    } else {
        fseek(index_fp, 0, SEEK_SET);
    }

    FILE *data_fp = fopen(filename, "r");
    if (!data_fp) {
        php_error_docref(NULL, E_WARNING, "Failed to open file: %s", filename);
        fclose(index_fp);
        RETURN_FALSE;
    }

    if (mode < 100) {
        if (flock(fileno(data_fp), LOCK_EX) == -1) {
            php_error_docref(NULL, E_WARNING, "Failed to lock the file: %s", filename);
            fclose(data_fp);
            fclose(index_fp);
            RETURN_FALSE;
        }
        if (flock(fileno(index_fp), LOCK_EX) == -1) {
            php_error_docref(NULL, E_WARNING, "Failed to lock the file: %s", index_filename);
            fclose(data_fp);
            fclose(index_fp);
            RETURN_FALSE;
        }
    }

    if (mode > 99) mode -= 100;

    long ini_buffer_size = FAST_IO_G(buffer_size);

    if (file_size < ini_buffer_size) ini_buffer_size = file_size;
    if (ini_buffer_size < 16) ini_buffer_size = 16;

    long dynamic_buffer_size = ini_buffer_size;
    char *dynamic_buffer = (char *)emalloc(dynamic_buffer_size + 1);
    if (!dynamic_buffer) {
        fclose(data_fp);
        fclose(index_fp);
        zend_error(E_ERROR, "Out of memory to allocate %ld bytes", dynamic_buffer_size + 1);
    }

    size_t bytes_read;
    long current_size = 0;
    bool found_match = false;
    char *found_value = NULL;

    while ((bytes_read = fread(dynamic_buffer + current_size, 1, ini_buffer_size, index_fp)) > 0) {
        current_size += bytes_read;
        dynamic_buffer[current_size] = '\0';

        char *line_start = dynamic_buffer;
        char *line_end;

        while ((line_end = strchr(line_start, '\n')) != NULL) {
            *line_end = '\0';
            
            // Проверяем, что строка начинается с искомого ключа и за ним следует пробел
            if (strncmp(line_start, line_key, line_key_len) == 0 && 
                line_start[line_key_len] == ' ') {
                
                // Копируем значение после ключа и пробела
                found_value = estrdup(line_start + line_key_len + 1);
                if (found_value == NULL) {
                    fclose(data_fp);
                    fclose(index_fp);
                    efree(dynamic_buffer);
                    zend_error(E_ERROR, "Out of memory");
                }
                
                found_match = true;
                break;
            }
            line_start = line_end + 1;
        }

        if (found_match) break;

        // Перемещаем незавершенную строку в начало буфера
        long remaining_size = current_size - (line_start - dynamic_buffer);
        if (remaining_size > 0) {
            memmove(dynamic_buffer, line_start, remaining_size);
        }
        current_size = remaining_size;

        // Увеличиваем буфер при необходимости
        if (current_size + ini_buffer_size > dynamic_buffer_size) {
            dynamic_buffer_size += ini_buffer_size;
            char *temp_buffer = (char *)erealloc(dynamic_buffer, dynamic_buffer_size + 1);
            if (!temp_buffer) {
                efree(dynamic_buffer);
                fclose(data_fp);
                fclose(index_fp);
                zend_error(E_ERROR, "Out of memory to allocate %ld bytes", dynamic_buffer_size + 1);
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

    // Парсим значение формата "position:size"
    char *colon_ptr = strchr(found_value, ':');
    if (!colon_ptr) {
        efree(found_value);
        fclose(data_fp);
        RETURN_FALSE;
    }

    *colon_ptr = '\0';
    long data_position = atol(found_value);
    long data_size = atol(colon_ptr + 1);
    efree(found_value);

    if (data_size <= 0) {
        fclose(data_fp);
        RETURN_FALSE;
    }

    if (fseek(data_fp, data_position, SEEK_SET) == -1) {
        php_error_docref(NULL, E_WARNING, "Failed to seek file: %s", filename);
        fclose(data_fp);
        RETURN_FALSE;
    }

    char *data_buffer = emalloc(data_size + 1);
    if (!data_buffer) {
        fclose(data_fp);
        zend_error(E_ERROR, "Out of memory to allocate %ld bytes", data_size + 1);
    }

    bytes_read = fread(data_buffer, 1, data_size, data_fp);
    if (bytes_read != data_size) {
        php_error_docref(NULL, E_WARNING, "Failed to read from file: %s", filename);
        fclose(data_fp);
        efree(data_buffer);
        RETURN_FALSE;
    }

    data_buffer[bytes_read] = '\0';
    RETVAL_STRING(data_buffer);
    efree(data_buffer);

    fclose(data_fp);
}




/* Реализация функции */
PHP_FUNCTION(file_push_data) {
    char *filename, *line_key, *line_value;
    size_t filename_len, line_key_len, line_value_len;
    zend_long mode = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "sss|l", &filename, &filename_len, &line_key, &line_key_len, &line_value, &line_value_len, &mode) == FAILURE) {
        RETURN_FALSE;
    }

    char index_filename[filename_len + 7];
    snprintf(index_filename, sizeof(index_filename), "%s.index", filename);

    FILE *data_fp = fopen(filename, "a+");
    FILE *index_fp = fopen(index_filename, "a+");

    if (!data_fp) {
        php_error_docref(NULL, E_WARNING, "Failed to open file: %s", filename);
        if (index_fp) {
            fclose(index_fp);
        }
        RETURN_LONG(-1);
    }

    if (!index_fp) {
        php_error_docref(NULL, E_WARNING, "Failed to open file: %s", index_filename);
        fclose(data_fp);
        RETURN_LONG(-1);
    }

    if (mode < 100) {
        // Блокировка файла данных
        if (flock(fileno(data_fp), LOCK_EX) == -1) {
            php_error_docref(NULL, E_WARNING, "Failed to lock the file: %s", filename);
            fclose(data_fp);
            fclose(index_fp);
            RETURN_LONG(-2);
        }
        // Блокировка индексного файла
        if (flock(fileno(index_fp), LOCK_EX) == -1) {
            php_error_docref(NULL, E_WARNING, "Failed to lock the file: %s", index_filename);
            fclose(data_fp); // Это также разблокирует data_fp
            fclose(index_fp);
            RETURN_LONG(-2);
        }
    }

    if (mode > 99) mode -= 100;

    // Запись значения в файл данных
    fseek(data_fp, 0, SEEK_END);
    long position = ftell(data_fp);
    
    if (position == -1) {
        php_error_docref(NULL, E_WARNING, "Failed to get file position: %s", filename);
        fclose(data_fp);
        fclose(index_fp);
        RETURN_LONG(-3);
    }

    if (fwrite(line_value, 1, line_value_len, data_fp) != line_value_len) {
        php_error_docref(NULL, E_WARNING, "Failed to write to the file: %s", filename);
        if (ftruncate(fileno(data_fp), position) == -1) {
            php_error_docref(NULL, E_ERROR, "Failed to truncate the file: %s", filename);
        }
        fclose(data_fp);
        fclose(index_fp);
        RETURN_LONG(-3);
    }

    // Принудительная запись буферов на диск
    if (fflush(data_fp) != 0) {
        php_error_docref(NULL, E_WARNING, "Failed to flush data file: %s", filename);
        if (ftruncate(fileno(data_fp), position) == -1) {
            php_error_docref(NULL, E_ERROR, "Failed to truncate the file: %s", filename);
        }
        fclose(data_fp);
        fclose(index_fp);
        RETURN_LONG(-3);
    }

    fseek(index_fp, 0, SEEK_END);
    long index_file_size = ftell(index_fp);
    
    if (index_file_size == -1) {
        php_error_docref(NULL, E_WARNING, "Failed to get index file position: %s", index_filename);
        if (ftruncate(fileno(data_fp), position) == -1) {
            php_error_docref(NULL, E_ERROR, "Failed to truncate the file: %s", filename);
        }
        fclose(data_fp);
        fclose(index_fp);
        RETURN_LONG(-3);
    }

    // Запись индекса в индексный файл
    int written = fprintf(index_fp, "%s %ld:%zu\n", line_key, position, line_value_len);
    if (written < 0) {
        php_error_docref(NULL, E_WARNING, "Failed to write to the file: %s", index_filename);
        
        if (ftruncate(fileno(index_fp), index_file_size) == -1) {
            php_error_docref(NULL, E_ERROR, "Failed to truncate the file: %s", index_filename);
        }
        if (ftruncate(fileno(data_fp), position) == -1) {
            php_error_docref(NULL, E_ERROR, "Failed to truncate the file: %s", filename);
        }
        fclose(data_fp);
        fclose(index_fp);
        RETURN_LONG(-3);
    }

    // Принудительная запись буферов индексного файла на диск
    if (fflush(index_fp) != 0) {
        php_error_docref(NULL, E_WARNING, "Failed to flush index file: %s", index_filename);
        
        if (ftruncate(fileno(index_fp), index_file_size) == -1) {
            php_error_docref(NULL, E_ERROR, "Failed to truncate the file: %s", index_filename);
        }
        if (ftruncate(fileno(data_fp), position) == -1) {
            php_error_docref(NULL, E_ERROR, "Failed to truncate the file: %s", filename);
        }
        fclose(data_fp);
        fclose(index_fp);
        RETURN_LONG(-3);
    }

    // Закрытие файлов (блокировки снимаются автоматически)
    fclose(data_fp);
    fclose(index_fp);
    RETURN_LONG(position);
}



/* Реализация функции */
PHP_FUNCTION(file_defrag_lines) {
    char *filename, *line_key = NULL;
    size_t filename_len, line_key_len = 0;
    zend_long mode = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "s|sl", &filename, &filename_len, &line_key, &line_key_len, &mode) == FAILURE) {
        RETURN_FALSE;
    }

    char temp_filename[filename_len + 5];
    snprintf(temp_filename, sizeof(temp_filename), "%s.tmp", filename);

    FILE *data_fp = fopen(filename, "r+");
    if (!data_fp) {
        php_error_docref(NULL, E_WARNING, "Failed to open file: %s", filename);
        RETURN_LONG(-1);
    }

    FILE *temp_fp = fopen(temp_filename, "w+");
    if (!temp_fp) {
        php_error_docref(NULL, E_WARNING, "Failed to open file: %s", temp_filename);
        fclose(data_fp);
        RETURN_LONG(-2);
    }

    if(mode < 100){
        if (flock(fileno(data_fp), LOCK_EX) == -1) {
            php_error_docref(NULL, E_WARNING, "Failed to lock the file: %s", filename);
            fclose(data_fp);
            fclose(temp_fp);
            unlink(temp_filename);
            RETURN_LONG(-3);
        }
    }
 
    if(mode > 99) mode -= 100;

    // Перемещение указателя в конец файла для получения его размера
    fseek(data_fp, 0, SEEK_END);
    long file_size = ftell(data_fp);
    fseek(data_fp, 0, SEEK_SET);

    long ini_buffer_size = FAST_IO_G(buffer_size);

    if(file_size < ini_buffer_size) ini_buffer_size = file_size;
    if(ini_buffer_size < 16) ini_buffer_size = 16;
    
    long dynamic_buffer_size = ini_buffer_size;
    char *dynamic_buffer = (char *)emalloc(dynamic_buffer_size + 1);
    if (!dynamic_buffer) {
        fclose(data_fp);
        fclose(temp_fp);
        unlink(temp_filename);
        zend_error(E_ERROR, "Out of memory to allocate %ld bytes", dynamic_buffer_size + 1);
    }

    size_t found_count = 0;
    size_t bytes_read;
    size_t bytes_write;
    long current_size = 0;

    bool found_match;

    while ((bytes_read = fread(dynamic_buffer + current_size, 1, ini_buffer_size, data_fp)) > 0) {
        current_size += bytes_read;
               
        dynamic_buffer[current_size] = '\0';

        char *line_start = dynamic_buffer;
        char *line_end;
        while ((line_end = strchr(line_start, '\n')) != NULL) {
            long line_length = line_end - line_start + 1;
            *line_end = '\0';

            found_match = false;

            if (line_key != NULL) {
                if (*line_start == SPECIAL_CHAR || strstr(line_start, line_key) != NULL) {
                    found_match = true;
                }
            } else {
                if (*line_start == SPECIAL_CHAR) {
                    found_match = true;
                }
            }

            if(!found_match){
                *line_end = '\n';

                bytes_write = fwrite(line_start, 1, line_length, temp_fp);

                if (bytes_write != line_length) {
                    php_error_docref(NULL, E_WARNING, "Failed to write to the file: %s", temp_filename);
                    fclose(data_fp);
                    fclose(temp_fp);
                    unlink(temp_filename);
                    efree(dynamic_buffer);
                    RETURN_LONG(-4);
                }
            } else {
                found_count++;
            }

            line_start = line_end + 1;
        }

        current_size -= (line_start - dynamic_buffer);
        memmove(dynamic_buffer, line_start, current_size);

        if (current_size + ini_buffer_size > dynamic_buffer_size) {
            dynamic_buffer_size += ini_buffer_size;

            char *temp_buffer = (char *)erealloc(dynamic_buffer, dynamic_buffer_size + 1);
            if (!temp_buffer) {
                fclose(data_fp);
                fclose(temp_fp);
                unlink(temp_filename);
                efree(dynamic_buffer);
                zend_error(E_ERROR, "Out of memory to allocate %ld bytes", dynamic_buffer_size + 1);
            }
            dynamic_buffer = temp_buffer;
        }
    }

    // Обрабатываем остаток буфера, если он не заканчивается на \n
    if (current_size > 0) {
        found_match = false;

        if (line_key != NULL) {
            if (*dynamic_buffer == SPECIAL_CHAR || strstr(dynamic_buffer, line_key) != NULL) {
                found_match = true;
            }
        } else {
            if (*dynamic_buffer == SPECIAL_CHAR) {
                found_match = true;
            }
        }

        if (!found_match) {
            bytes_write = fwrite(dynamic_buffer, 1, current_size, temp_fp);
            if (bytes_write != current_size) {
                php_error_docref(NULL, E_WARNING, "Failed to write to the file: %s", temp_filename);
                fclose(data_fp);
                fclose(temp_fp);
                unlink(temp_filename);
                efree(dynamic_buffer);
                RETURN_LONG(-4);
            }
        } else {
            found_count++;
        }
    }

    if(mode == 0){
        fseek(data_fp, 0 , SEEK_SET);
        fseek(temp_fp, 0 , SEEK_SET);
        current_size = 0;

        while ((bytes_read = fread(dynamic_buffer, 1, dynamic_buffer_size, temp_fp)) > 0) {
            bytes_write = fwrite(dynamic_buffer, 1, bytes_read, data_fp); 
            if(bytes_read != bytes_write){
                php_error_docref(NULL, E_WARNING, "Failed to write to the file: %s", filename);
                fclose(data_fp);
                fclose(temp_fp);
                unlink(temp_filename);
                efree(dynamic_buffer);
                RETURN_LONG(-4);
            }
            current_size += bytes_write;
        }

        // Усекаем файл
        if (ftruncate(fileno(data_fp), current_size) == -1) {
            php_error_docref(NULL, E_WARNING, "Failed to truncate file: %s", filename);
            fclose(data_fp);
            fclose(temp_fp);
            unlink(temp_filename);
            efree(dynamic_buffer);
            RETURN_LONG(-5);
        }

        fclose(temp_fp);
        unlink(temp_filename);
    }

    efree(dynamic_buffer);
    fclose(data_fp);

    if(mode == 1){
        fclose(temp_fp);
        // Заменяем оригинальный файл временным файлом
        if (rename(temp_filename, filename) != 0) {
            php_error_docref(NULL, E_WARNING, "Failed to rename temp file to original: %s", filename);
            unlink(temp_filename);
            RETURN_LONG(-6);
        }
    }

    RETURN_LONG(found_count);
}




PHP_FUNCTION(file_defrag_data) {
    char *filename, *line_key = NULL;
    size_t filename_len, line_key_len = 0;
    zend_long mode = 0;
    size_t found_count = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "s|sl", &filename, &filename_len, &line_key, &line_key_len, &mode) == FAILURE) {
        RETURN_FALSE;
    }

    char temp_filename[filename_len + 5];
    snprintf(temp_filename, sizeof(temp_filename), "%s.tmp", filename);
    char index_filename[filename_len + 7];
    snprintf(index_filename, sizeof(index_filename), "%s.index", filename);
    char temp_index_filename[filename_len + 11];
    snprintf(temp_index_filename, sizeof(temp_index_filename), "%s.index.tmp", filename);

    FILE *data_fp = fopen(filename, "r+");
    if (!data_fp) {
        php_error_docref(NULL, E_WARNING, "Failed to open file: %s", filename);
        RETURN_LONG(-1);
    }

    FILE *temp_fp = fopen(temp_filename, "w+");
    if (!temp_fp) {
        php_error_docref(NULL, E_WARNING, "Failed to open file: %s", temp_filename);
        fclose(data_fp);
        RETURN_LONG(-1);
    }

    FILE *index_fp = fopen(index_filename, "r+");
    if (!index_fp) {
        php_error_docref(NULL, E_WARNING, "Failed to open file: %s", index_filename);
        fclose(data_fp);
        fclose(temp_fp);
        unlink(temp_filename);
        RETURN_LONG(-1);
    }

    FILE *temp_index_fp = fopen(temp_index_filename, "w+");
    if (!temp_index_fp) {
        php_error_docref(NULL, E_WARNING, "Failed to open file: %s", temp_index_filename);
        fclose(data_fp);
        fclose(temp_fp);
        fclose(index_fp);
        unlink(temp_filename);
        RETURN_LONG(-1);
    }

    if(mode < 100){
        if (flock(fileno(data_fp), LOCK_EX) == -1) {
            php_error_docref(NULL, E_WARNING, "Failed to lock the file: %s", filename);
            fclose(index_fp);
            fclose(data_fp);
            fclose(temp_fp);
            fclose(temp_index_fp);
            unlink(temp_filename);
            unlink(temp_index_filename);
            RETURN_LONG(-2);
        }
        if (flock(fileno(index_fp), LOCK_EX) == -1) {
            php_error_docref(NULL, E_WARNING, "Failed to lock the file: %s", index_filename);
            fclose(index_fp);
            fclose(data_fp);
            fclose(temp_fp);
            fclose(temp_index_fp);
            unlink(temp_filename);
            unlink(temp_index_filename);
            RETURN_LONG(-2);
        }
    }
 
    if(mode > 99) mode -= 100;

    // Перемещение указателя в конец файла для получения его размера
    fseek(index_fp, 0, SEEK_END);
    long file_size = ftell(index_fp);
    fseek(index_fp, 0, SEEK_SET);

    long ini_buffer_size = FAST_IO_G(buffer_size);

    if(file_size < ini_buffer_size) ini_buffer_size = file_size;
    if(ini_buffer_size < 16) ini_buffer_size = 16;
    
    long dynamic_buffer_size = ini_buffer_size;
    char *dynamic_buffer = (char *)emalloc(dynamic_buffer_size + 1);
    if (!dynamic_buffer) {
        fclose(index_fp);
        fclose(data_fp);
        fclose(temp_fp);
        fclose(temp_index_fp);
        unlink(temp_filename);
        unlink(temp_index_filename);
        zend_error(E_ERROR, "Out of memory to allocate %ld bytes", dynamic_buffer_size + 1);
    }

    long bytes_read;
    long bytes_write;
    long bytes_read_data;
    long bytes_write_data;
    long current_size = 0;

    bool found_match;

    off_t position;
    size_t size;

    while ((bytes_read = fread(dynamic_buffer + current_size, 1, ini_buffer_size, index_fp)) > 0) {
        current_size += bytes_read;
               
        dynamic_buffer[current_size] = '\0';

        char *line_start = dynamic_buffer;
        char *line_end;
        while ((line_end = strchr(line_start, '\n')) != NULL) {
            long line_length = line_end - line_start + 1;
            *line_end = '\0';

            found_match = false;

            if (line_key != NULL) {
                if (*line_start == SPECIAL_CHAR || strstr(line_start, line_key) != NULL) {
                    found_match = true;
                }
            } else {
                if (*line_start == SPECIAL_CHAR) {
                    found_match = true;
                }
            }

            if(!found_match){
                *line_end = '\n';
                bytes_write = fwrite(line_start, 1, line_length, temp_index_fp);

                if (bytes_write != line_length) {
                    php_error_docref(NULL, E_WARNING, "Failed to write to the file: %s", temp_index_filename);
                    fclose(index_fp);
                    fclose(data_fp);
                    fclose(temp_fp);
                    fclose(temp_index_fp);
                    unlink(temp_filename);
                    unlink(temp_index_filename);
                    efree(dynamic_buffer);
                    RETURN_LONG(-4);
                }

                *line_end = '\0';

                char *line_copy = estrdup(line_start);
                if (!line_copy) {
                    php_error_docref(NULL, E_WARNING, "Failed to allocate memory for line copy");
                    fclose(index_fp);
                    fclose(data_fp);
                    fclose(temp_fp);
                    fclose(temp_index_fp);
                    unlink(temp_filename);
                    unlink(temp_index_filename);
                    efree(dynamic_buffer);
                    RETURN_LONG(-8);
                }

                char *offset_ptr = NULL;
                char *size_ptr = NULL;
                bool parsed_err = false;
                
                char *token = strtok(line_copy, " ");
                
                while (token != NULL) {
                    if (strchr(token, ':') != NULL) {
                        offset_ptr = strtok(token, ":");
                        size_ptr = strtok(NULL, ":");
                        break;
                    }
                    token = strtok(NULL, " ");
                }
                
                if (offset_ptr && size_ptr) {
                    char *endptr;
                    position = strtoul(offset_ptr, &endptr, 10);
                    if (*endptr != '\0') {
                        parsed_err = true;
                    } else {
                        size = strtoul(size_ptr, &endptr, 10);
                        if (*endptr != '\0') {
                            parsed_err = true;
                        }
                    }
                } else {
                    parsed_err = true;
                }

                if(parsed_err == false){
                    if (fseek(data_fp, position, SEEK_SET) != 0) {
                        php_error_docref(NULL, E_WARNING, "Failed to seek in file: %s", filename);
                        fclose(index_fp);
                        fclose(data_fp);
                        fclose(temp_fp);
                        fclose(temp_index_fp);
                        unlink(temp_filename);
                        unlink(temp_index_filename);
                        efree(dynamic_buffer);
                        efree(line_copy);
                        RETURN_LONG(-6);
                    }

                    char *data_buffer = emalloc(size + 1);
                    if(data_buffer == NULL){
                        fclose(index_fp);
                        fclose(data_fp);
                        fclose(temp_fp);
                        fclose(temp_index_fp);
                        unlink(temp_filename);
                        unlink(temp_index_filename);
                        efree(dynamic_buffer);
                        efree(line_copy);
                        zend_error(E_ERROR, "Out of memory to allocate %ld bytes", size + 1);
                    }

                    bytes_read_data = fread(data_buffer, 1, size, data_fp);
                    if(bytes_read_data != (long) size){
                        php_error_docref(NULL, E_WARNING, "Failed to read from the file: %s", filename);
                        fclose(index_fp);
                        fclose(data_fp);
                        fclose(temp_fp);
                        fclose(temp_index_fp);
                        unlink(temp_filename);
                        unlink(temp_index_filename);
                        efree(dynamic_buffer);
                        efree(data_buffer);
                        efree(line_copy);
                        RETURN_LONG(-6);
                    }

                    bytes_write_data = fwrite(data_buffer, 1, bytes_read_data, temp_fp);

                    if(bytes_read_data != bytes_write_data){
                        php_error_docref(NULL, E_WARNING, "Failed to write to the file: %s", temp_filename);
                        fclose(index_fp);
                        fclose(data_fp);
                        fclose(temp_fp);
                        fclose(temp_index_fp);
                        unlink(temp_filename);
                        unlink(temp_index_filename);
                        efree(dynamic_buffer);
                        efree(data_buffer);
                        efree(line_copy);
                        RETURN_LONG(-4);
                    }

                    efree(data_buffer);
                } else {
                    php_error_docref(NULL, E_WARNING, "Failed to parse offset:size");
                    fclose(index_fp);
                    fclose(data_fp);
                    fclose(temp_fp);
                    fclose(temp_index_fp);
                    unlink(temp_filename);
                    unlink(temp_index_filename);
                    efree(dynamic_buffer);
                    efree(line_copy);
                    RETURN_LONG(-7);
                }

                efree(line_copy);

            } else {
                found_count++;
            }

            line_start = line_end + 1;
        }

        current_size -= (line_start - dynamic_buffer);
        memmove(dynamic_buffer, line_start, current_size);

        if (current_size + ini_buffer_size > dynamic_buffer_size) {
            dynamic_buffer_size += ini_buffer_size;

            char *temp_buffer = (char *)erealloc(dynamic_buffer, dynamic_buffer_size + 1);
            if (!temp_buffer) {
                fclose(index_fp);
                fclose(data_fp);
                fclose(temp_fp);
                fclose(temp_index_fp);
                unlink(temp_filename);
                unlink(temp_index_filename);
                efree(dynamic_buffer);
                zend_error(E_ERROR, "Out of memory to allocate %ld bytes", dynamic_buffer_size + 1);
            }

            dynamic_buffer = temp_buffer;
        }
    }

    if(mode == 0){
        fseek(data_fp, 0 , SEEK_SET);
        fseek(temp_fp, 0 , SEEK_SET);
        fseek(index_fp, 0 , SEEK_SET);
        fseek(temp_index_fp, 0 , SEEK_SET);
        current_size = 0;

        while ((bytes_read = fread(dynamic_buffer, 1, dynamic_buffer_size, temp_fp)) > 0) {
            bytes_write = fwrite(dynamic_buffer, 1, bytes_read, data_fp); 
            if(bytes_write != bytes_read){
                php_error_docref(NULL, E_WARNING, "Failed to write to the file: %s", filename);
                fclose(index_fp);
                fclose(data_fp);
                fclose(temp_fp);
                fclose(temp_index_fp);
                efree(dynamic_buffer);
                RETURN_LONG(-4);
            }

            current_size += bytes_write;
        }

        if (ftruncate(fileno(data_fp), current_size) == -1) {
            php_error_docref(NULL, E_WARNING, "Failed to truncate file: %s", filename);
            fclose(index_fp);
            fclose(data_fp);
            fclose(temp_fp);
            fclose(temp_index_fp);
            efree(dynamic_buffer);
            RETURN_LONG(-5);
        }

        current_size = 0;

        while ((bytes_read = fread(dynamic_buffer, 1, dynamic_buffer_size, temp_index_fp)) > 0) {
            bytes_write = fwrite(dynamic_buffer, 1, bytes_read, index_fp); 
            if(bytes_write != bytes_read){
                php_error_docref(NULL, E_WARNING, "Failed to write to the file: %s", index_filename);
                fclose(index_fp);
                fclose(data_fp);
                fclose(temp_fp);
                fclose(temp_index_fp);                
                efree(dynamic_buffer);
                RETURN_LONG(-4);
            }

            current_size += bytes_write;
        }
        
        if (ftruncate(fileno(index_fp), current_size) == -1) {
            php_error_docref(NULL, E_WARNING, "Failed to truncate file: %s", index_filename);
            fclose(index_fp);
            fclose(data_fp);
            fclose(temp_fp);
            fclose(temp_index_fp);
            efree(dynamic_buffer);
            RETURN_LONG(-5);
        }

        fclose(temp_fp);
        unlink(temp_filename);
        fclose(temp_index_fp);
        unlink(temp_index_filename);
    }

    efree(dynamic_buffer);
    fclose(data_fp);
    fclose(index_fp);

    if(mode == 1){
        fclose(temp_fp);
        rename(temp_filename, filename);

        fclose(temp_index_fp);
        rename(temp_index_filename, index_filename);
    }

    RETURN_LONG(found_count);
}





/* Функция для извлечения и удаления последней строки из файла */
PHP_FUNCTION(file_pop_line) {
    char *filename;
    size_t filename_len;
    ssize_t offset = -1; // Значение по умолчанию для необязательного аргумента
    size_t mode = 0;
    ssize_t end = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "s|lll", &filename, &filename_len, &offset, &mode, &end) == FAILURE) {
        RETURN_FALSE;
    }

    FILE *fp = fopen(filename, "r+"); 
    if (!fp) {
        php_error_docref(NULL, E_WARNING, "Failed to open file: %s", filename);
        RETURN_FALSE;
    }

    if(mode < 100){
        // Блокировка файла для записи
        if (flock(fileno(fp), LOCK_EX) == -1) {
            php_error_docref(NULL, E_WARNING, "Failed to lock the file: %s", filename);
            fclose(fp);
            RETURN_FALSE;
        }
    }

    if(mode > 99) mode -= 100;

    // Перемещение указателя в конец файла для получения его размера
    fseek(fp, 0, SEEK_END);

    // Получаем текущее смещение в файле данных
    ssize_t file_size = ftell(fp);
    ssize_t pos = file_size;
    ssize_t bytes_read;

    if (offset > 0) {
        pos -= offset;
        fseek(fp, pos , SEEK_SET);

        // Увеличиваем размер буфера на 1 для возможного символа перевода строки
        char *buffer = (char *)emalloc(offset + 1); // +1 для '\0'
        if (!buffer) {
            fclose(fp);
            zend_error(E_ERROR, "Out of memory to allocate %ld bytes", offset + 1);
        }

        bytes_read = fread(buffer, 1, offset, fp);
        if(bytes_read != offset){
            efree(buffer);
            fclose(fp);
            php_error_docref(NULL, E_WARNING, "Failed to read to the file: %s", filename);
            RETURN_FALSE;
        }

        // Убедимся, что строка нуль-терминирована
        buffer[bytes_read] = '\0';

        if(mode < 1 || mode == 2){
            // Обрезка пробелов справа и символа перевода строки
            for (ssize_t i = bytes_read - 1; i >= 0; --i) {
                if(buffer[i] == ' ' || buffer[i] == '\n') buffer[i] = '\0';
                else break;
            }
        }

        if(mode < 2){
            // Усекаем файл
            if(ftruncate(fileno(fp), pos) == -1) {
                efree(buffer);
                fclose(fp);
                php_error_docref(NULL, E_WARNING, "Failed to truncate file: %s", filename);
                RETURN_FALSE;
            }
        }
        
        fclose(fp);

        // Возврат строки в PHP
        RETURN_STRING(buffer);
    }


    if(offset < 0){
        ssize_t ini_buffer_size = FAST_IO_G(buffer_size);
        if(file_size < ini_buffer_size) ini_buffer_size = file_size;
        if(ini_buffer_size < 16) ini_buffer_size = 16;

        // Авто поиск последней строки
        ssize_t dynamic_buffer_size = ini_buffer_size;
        char *dynamic_buffer = (char *)emalloc(dynamic_buffer_size + 1);
        if (!dynamic_buffer) {
            fclose(fp);
            zend_error(E_ERROR, "Out of memory to allocate %ld bytes", dynamic_buffer_size + 1);
        }

        ssize_t current_size = 0; // Текущий размер данных в динамическом буфере
        ssize_t first_block_size = 0;
        
        char *line_start;
        ssize_t line_length;

        if(end > 0 && pos - end > 0) pos -= end;
        else offset--;


        if(file_size < ini_buffer_size) {
            first_block_size = file_size;
            dynamic_buffer_size = file_size;
        }

        while(pos > 0){
            if (first_block_size > 0) {
                fseek(fp, 0, SEEK_SET); // Перемещаем указатель на предыдущую порцию
                bytes_read = fread(dynamic_buffer, 1, first_block_size, fp);

                if(bytes_read != first_block_size){
                    efree(dynamic_buffer);
                    fclose(fp);
                    php_error_docref(NULL, E_WARNING, "Failed to read to the file: %s", filename);
                    RETURN_FALSE;
                }
            } else {
                fseek(fp, pos - ini_buffer_size, SEEK_SET); // Перемещаем указатель на предыдущую порцию
                bytes_read = fread(dynamic_buffer, 1, ini_buffer_size, fp);

                if(bytes_read != ini_buffer_size){
                    efree(dynamic_buffer);
                    fclose(fp);
                    php_error_docref(NULL, E_WARNING, "Failed to read to the file: %s", filename);
                    RETURN_FALSE;
                }
            }

            pos -= bytes_read;
            current_size += bytes_read;
            ssize_t i = bytes_read - 1;

            while(i >= 0){
                if (dynamic_buffer[i] == '\n') {
                    offset++;

                    if(offset == 0){ // Все строки найдены
                        line_start = dynamic_buffer + i + 1;
                        line_length = current_size - i - 1;

                        goto line_found;
                    }
                }

                i--;
            }


            if(pos == 0){ // Все строки найдены
                line_start = dynamic_buffer;
                line_length = current_size;

                goto line_found;
            }


            if (pos - ini_buffer_size < 0) {
                first_block_size = pos;
                dynamic_buffer_size += first_block_size;
            } else {
                dynamic_buffer_size += ini_buffer_size;
            }

            char *temp_buffer = (char *)erealloc(dynamic_buffer, dynamic_buffer_size + 1);
            if (!temp_buffer) {
                fclose(fp);
                if (dynamic_buffer) efree(dynamic_buffer);
                zend_error(E_ERROR, "Out of memory to allocate %ld bytes", dynamic_buffer_size + 1);
            }

            dynamic_buffer = temp_buffer;

            memmove(dynamic_buffer + dynamic_buffer_size - current_size, dynamic_buffer, current_size);
        }

        efree(dynamic_buffer);
        fclose(fp);
        RETURN_FALSE;


line_found:

        dynamic_buffer[current_size] = '\0';
        ssize_t new_file_size = file_size - line_length;
        if(end > 0) new_file_size -= end;
        if(new_file_size < 0) new_file_size = 0;

        if(mode < 1 || mode == 2){
            // Обрезка пробелов справа и символа перевода строки
            for (ssize_t i = dynamic_buffer_size - 1; i >= 0; --i) {
                if(dynamic_buffer[i] == ' ' || dynamic_buffer[i] == '\n') dynamic_buffer[i] = '\0';
                else break;
            }
        }

        if(mode < 2){
            // Усекаем файл
            if(ftruncate(fileno(fp), new_file_size) == -1) {
                efree(dynamic_buffer);
                fclose(fp);
                php_error_docref(NULL, E_WARNING, "Failed to truncate file: %s", filename);
                RETURN_FALSE;
            }
        }

        fclose(fp);
        RETVAL_STRING(line_start);
        efree(dynamic_buffer);
        return;
    }

    fclose(fp);
    RETURN_FALSE;
}





/* Реализация функции */
PHP_FUNCTION(file_erase_line) {
    char *filename, *line_key;
    size_t filename_len, line_key_len;
    ssize_t position = 0;
    size_t mode = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "ss|ll", &filename, &filename_len, &line_key, &line_key_len, &position, &mode) == FAILURE) {
        RETURN_FALSE;
    }

    FILE *fp = fopen(filename, "r+");
    if (fp == NULL) {
        php_error_docref(NULL, E_WARNING, "Failed to open file: %s", filename);
        RETURN_LONG(-1);
    }

    if(mode < 100){
        // Попытка установить блокирующую блокировку на запись
        if (flock(fileno(fp), LOCK_EX) == -1) {
            php_error_docref(NULL, E_WARNING, "Failed to lock file: %s", filename);
            fclose(fp);
            RETURN_LONG(-2);
        }
    }

    if(mode > 99) mode -= 100;


    // Перемещение указателя в конец файла для получения его размера
    fseek(fp, 0, SEEK_END);
    ssize_t file_size = ftell(fp);
    ssize_t write_offset = 0; // Смещение для записи обновленных данных

    if(position > 0){
        if(position > file_size){
            php_error_docref(NULL, E_WARNING, "Failed to seek file: %s", filename);
            fclose(fp);
            RETURN_LONG(-5);
        }

        fseek(fp, position, SEEK_SET);
        write_offset = position;
    } else {
        fseek(fp, 0, SEEK_SET);
    }

    ssize_t ini_buffer_size = FAST_IO_G(buffer_size);

    if(file_size < ini_buffer_size) ini_buffer_size = file_size;
    if(ini_buffer_size < 16) ini_buffer_size = 16;

    ssize_t dynamic_buffer_size = ini_buffer_size;
    char *dynamic_buffer = (char *)emalloc(dynamic_buffer_size + 1);
    if (!dynamic_buffer) {
        fclose(fp);
        zend_error(E_ERROR, "Out of memory to allocate %ld bytes", dynamic_buffer_size + 1);
    }

    ssize_t bytes_read;
    ssize_t bytes_write;

    ssize_t current_size = 0; // Текущий размер данных в динамическом буфере
    bool found_match = false;

    while ((bytes_read = fread(dynamic_buffer + current_size, 1, ini_buffer_size, fp)) > 0) {
        current_size += bytes_read;
               
        dynamic_buffer[current_size] = '\0';

        char *line_start = dynamic_buffer;
        char *line_end;
        while ((line_end = strchr(line_start, '\n')) != NULL) {
            ssize_t line_length = line_end - line_start + 1;
            *line_end = '\0';

            if (strstr(line_start, line_key) != NULL) {
                // Найдено совпадение ключа, подготавливаем замену
                char *replacement = emalloc(line_length);
                if (!replacement) {
                    efree(dynamic_buffer);
                    fclose(fp);
                    zend_error(E_ERROR, "Out of memory to allocate %ld bytes", line_length);
                }

                memset(replacement, ' ', line_length - 1); // Заполнение пробелами
                replacement[0] = SPECIAL_CHAR; // символ DEL
                
                // Перемещаемся к началу найденной строки и записываем замену
                fseek(fp, write_offset , SEEK_SET);

                bytes_write = fwrite(replacement, 1, line_length - 1, fp);

                if (bytes_write != line_length - 1) {
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

            write_offset += line_length; // Обновляем смещение
            line_start = line_end + 1;
        }

        if (found_match) break;

        current_size -= (line_start - dynamic_buffer);
        memmove(dynamic_buffer, line_start, current_size);

        if (current_size + ini_buffer_size > dynamic_buffer_size) {
            dynamic_buffer_size += ini_buffer_size;

            char *temp_buffer = (char *)erealloc(dynamic_buffer, dynamic_buffer_size + 1);
            if (!temp_buffer) {
                fclose(fp);
                if (dynamic_buffer) efree(dynamic_buffer);
                zend_error(E_ERROR, "Out of memory to allocate %ld bytes", dynamic_buffer_size + 1);
            }
            dynamic_buffer = temp_buffer;
        }
        
    }

    efree(dynamic_buffer);
    fclose(fp);

    if(found_match) RETURN_LONG(write_offset);
    RETURN_LONG(-4);
}



/* Реализация функции */
PHP_FUNCTION(file_get_keys) {
    char *filename;
    size_t filename_len;
    zend_long search_start = 0;
    zend_long search_limit = 1;
    zend_long position = 0;
    zend_long mode = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "s|llll", &filename, &filename_len, &search_start, &search_limit, &position, &mode) == FAILURE) {
        RETURN_FALSE;
    }

    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        php_error_docref(NULL, E_WARNING, "Failed to open file: %s", filename);
        RETURN_FALSE;
    }

    if(mode < 100){
        // Попытка установить блокирующую блокировку на запись
        if (flock(fileno(fp), LOCK_EX) == -1) {
            php_error_docref(NULL, E_WARNING, "Failed to lock file: %s", filename);
            fclose(fp);
            RETURN_FALSE;
        }
    }

    if(mode > 99) mode -= 100;

    // Перемещение указателя в конец файла для получения его размера
    if (fseek(fp, 0, SEEK_END) != 0) {
        php_error_docref(NULL, E_WARNING, "Failed to seek to end of file: %s", filename);
        fclose(fp);
        RETURN_FALSE;
    }
    
    long file_size = ftell(fp);
    if (file_size == -1) {
        php_error_docref(NULL, E_WARNING, "Failed to get file size: %s", filename);
        fclose(fp);
        RETURN_FALSE;
    }
    
    long line_offset = 0; // Смещение для записи обновленных данных

    if(position > 0){
        if(position > file_size){
            php_error_docref(NULL, E_WARNING, "Position beyond file size: %s", filename);
            fclose(fp);
            RETURN_FALSE;
        }

        if (fseek(fp, position, SEEK_SET) != 0) {
            php_error_docref(NULL, E_WARNING, "Failed to seek file: %s", filename);
            fclose(fp);
            RETURN_FALSE;
        }
        line_offset = position;
    } else {
        if (fseek(fp, 0, SEEK_SET) != 0) {
            php_error_docref(NULL, E_WARNING, "Failed to seek to start of file: %s", filename);
            fclose(fp);
            RETURN_FALSE;
        }
    }

    long ini_buffer_size = FAST_IO_G(buffer_size);

    if(file_size < ini_buffer_size) ini_buffer_size = file_size;
    if(ini_buffer_size < 16) ini_buffer_size = 16;

    long dynamic_buffer_size = ini_buffer_size;
    char *dynamic_buffer = (char *)emalloc(dynamic_buffer_size + 1);
    if (!dynamic_buffer) {
        fclose(fp);
        zend_error(E_ERROR, "Out of memory to allocate %ld bytes", dynamic_buffer_size + 1);
        RETURN_FALSE;
    }

    size_t bytes_read;
    long current_size = 0; // Текущий размер данных в динамическом буфере
    bool found_match = false;

    zend_long add_count = 0;
    zend_long line_count = 0;
    array_init(return_value);

    while ((bytes_read = fread(dynamic_buffer + current_size, 1, ini_buffer_size, fp)) > 0) {
        current_size += bytes_read;
        
        dynamic_buffer[current_size] = '\0';

        char *line_start = dynamic_buffer;
        char *line_end;
        while ((line_end = strchr(line_start, '\n')) != NULL) {
            long line_length = line_end - line_start + 1;
            *line_end = '\0';

            line_count++;

            if(search_start < line_count){
                add_count++;
                zval line_arr;
                array_init(&line_arr);

                if(mode == 1) add_assoc_string(&line_arr, "line", line_start);

                if(mode == 2 || mode == 5) {
                    // Обрезка пробелов справа и символа перевода строки
                    long i;
                    long str_len = strlen(line_start);

                    for (i = str_len - 1; i >= 0; --i) {
                        if(line_start[i] == ' ' || line_start[i] == '\n' || line_start[i] == '\0') {
                            line_start[i] = '\0';
                        } else {
                            break;
                        }
                    }

                    add_assoc_string(&line_arr, "trim_line", line_start);
                    if(mode != 5) add_assoc_long(&line_arr, "trim_length", i + 1);

                    if(mode == 5){
                        add_next_index_string(return_value, line_start);
                    }
                }

                // Создаем копию строки для поиска пробела, чтобы не модифицировать оригинал
                char *key_str = estrdup(line_start);
                char *space_pos = strchr(key_str, ' ');
                if (space_pos) *space_pos = '\0';

                if(mode == 0) add_assoc_string(&line_arr, "key", key_str);

                if(mode < 4){
                    add_assoc_long(&line_arr, "line_offset", line_offset);
                    add_assoc_long(&line_arr, "line_length", line_length);
                    add_assoc_long(&line_arr, "line_count", line_count - 1);
                }

                if(mode < 5) {
                    if(mode == 4) {
                        add_next_index_string(return_value, key_str);
                    } else {
                        add_next_index_zval(return_value, &line_arr);
                    }
                }
                
                efree(key_str);
            }
            
            line_offset += line_length; // Обновляем смещение
            line_start = line_end + 1;

            if(add_count >= search_limit && search_limit > 0){
                found_match = true;
                break;
            }
        }

        if (found_match) break;

        // Проверяем, что есть данные для перемещения
        long remaining_size = current_size - (line_start - dynamic_buffer);
        if (remaining_size > 0) {
            memmove(dynamic_buffer, line_start, remaining_size);
            current_size = remaining_size;
        } else {
            current_size = 0;
        }

        if (current_size + ini_buffer_size > dynamic_buffer_size) {
            dynamic_buffer_size += ini_buffer_size;

            char *temp_buffer = (char *)erealloc(dynamic_buffer, dynamic_buffer_size + 1);
            if (!temp_buffer) {
                efree(dynamic_buffer);
                fclose(fp);
                zend_error(E_ERROR, "Out of memory to allocate %ld bytes", dynamic_buffer_size + 1);
                RETURN_FALSE;
            }
            dynamic_buffer = temp_buffer;
        }
    }

    efree(dynamic_buffer);
    fclose(fp);
}



// Функция обновления пары ключ-значение
PHP_FUNCTION(file_replace_line) {
    char *filename;
    size_t filename_len;
    char *line_key;
    char *line;
    ssize_t line_len;
    size_t line_key_len;
    size_t mode = 0;

    // Парсинг аргументов
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "sss|l", &filename, &filename_len, &line_key, &line_key_len, &line, &line_len, &mode) == FAILURE) {
        RETURN_FALSE;
    }

    char temp_filename[filename_len + 5];
    snprintf(temp_filename, sizeof(temp_filename), "%s.tmp", filename);

    FILE *data_fp = fopen(filename, "r+");
    if (!data_fp) {
        php_error_docref(NULL, E_WARNING, "Failed to open file: %s", filename);
        RETURN_LONG(-1);
    }

    FILE *temp_fp = fopen(temp_filename, "w+");
    if (!temp_fp) {
        php_error_docref(NULL, E_WARNING, "Failed to open file: %s", temp_filename);
        fclose(data_fp);
        RETURN_LONG(-2);
    }

    if(mode < 100){
        if (flock(fileno(data_fp), LOCK_EX) == -1) {
            php_error_docref(NULL, E_WARNING, "Failed to lock the file: %s", filename);
            fclose(data_fp);
            fclose(temp_fp);
            unlink(temp_filename);
            RETURN_LONG(-3);
        }
        if (flock(fileno(temp_fp), LOCK_EX) == -1) {
            php_error_docref(NULL, E_WARNING, "Failed to lock the file: %s", temp_filename);
            fclose(data_fp);
            fclose(temp_fp);
            unlink(temp_filename);
            RETURN_LONG(-3);
        }
    }

    if(mode > 99) mode -= 100;

    // Перемещение указателя в конец файла для получения его размера
    fseek(data_fp, 0, SEEK_END);
    ssize_t file_size = ftell(data_fp);
    fseek(data_fp, 0, SEEK_SET);

    ssize_t ini_buffer_size = FAST_IO_G(buffer_size);

    if(file_size < ini_buffer_size) ini_buffer_size = file_size;
    if(ini_buffer_size < 16) ini_buffer_size = 16;

    ssize_t dynamic_buffer_size = ini_buffer_size;
    char *dynamic_buffer = (char *)emalloc(dynamic_buffer_size + 1);
    if (!dynamic_buffer) {
        fclose(data_fp);
        fclose(temp_fp);
        unlink(temp_filename);
        zend_error(E_ERROR, "Out of memory to allocate %ld bytes", dynamic_buffer_size + 1);
    }

    size_t found_count = 0;
    ssize_t bytes_read;
    ssize_t bytes_write;
    ssize_t current_size = 0;


    while ((bytes_read = fread(dynamic_buffer + current_size, 1, ini_buffer_size, data_fp)) > 0) {
        current_size += bytes_read;

        dynamic_buffer[current_size] = '\0';

        char *line_start = dynamic_buffer;
        char *line_end;
        while ((line_end = strchr(line_start, '\n')) != NULL) {
            ssize_t line_length = line_end - line_start + 1;
            *line_end = '\0';

            if (strstr(line_start, line_key) != NULL) { 
                char *replacement = estrndup(line, line_len + 1);
                if(replacement == NULL){
                    fclose(data_fp);
                    fclose(temp_fp);
                    unlink(temp_filename);
                    efree(dynamic_buffer);
                    zend_error(E_ERROR, "Out of memory to allocate %ld bytes", line_len + 1);
                }

                replacement[line_len] = '\n';
                replacement[line_len + 1] = '\0';

                bytes_write = fwrite(replacement, 1, line_len + 1, temp_fp);
                
                if (bytes_write != line_len + 1) {
                    php_error_docref(NULL, E_WARNING, "Failed to write to the file: %s", temp_filename);
                    fclose(data_fp);
                    fclose(temp_fp);
                    unlink(temp_filename);
                    efree(dynamic_buffer);
                    efree(replacement);
                    RETURN_LONG(-4);
                }

                efree(replacement);
            } else {
                *line_end = '\n';

                bytes_write = fwrite(line_start, 1, line_length, temp_fp);

                if (bytes_write != line_length) {
                    php_error_docref(NULL, E_WARNING, "Failed to write to the file: %s", temp_filename);
                    fclose(data_fp);
                    fclose(temp_fp);
                    unlink(temp_filename);
                    efree(dynamic_buffer);
                    RETURN_LONG(-4);
                }
            }

            found_count++;
            line_start = line_end + 1;
        }


        current_size -= (line_start - dynamic_buffer);
        memmove(dynamic_buffer, line_start, current_size);

        if (current_size + ini_buffer_size > dynamic_buffer_size) {
            dynamic_buffer_size += ini_buffer_size;

            char *temp_buffer = (char *)erealloc(dynamic_buffer, dynamic_buffer_size + 1);
            if (!temp_buffer) {
                fclose(data_fp);
                fclose(temp_fp);
                unlink(temp_filename);
                if (dynamic_buffer) efree(dynamic_buffer);
                zend_error(E_ERROR, "Out of memory to allocate %ld bytes", dynamic_buffer_size + 1);
            }
            dynamic_buffer = temp_buffer;
        }
        
    }


    if(mode == 0){
        fseek(data_fp, 0 , SEEK_SET);
        fseek(temp_fp, 0 , SEEK_SET);
        current_size = 0;

        while ((bytes_read = fread(dynamic_buffer, 1, sizeof(dynamic_buffer), temp_fp)) > 0) {
            bytes_write = fwrite(dynamic_buffer, 1, bytes_read, data_fp); 
            if(bytes_write != bytes_read){
                php_error_docref(NULL, E_WARNING, "Failed to write to the file: %s", filename);
                efree(dynamic_buffer);
                fclose(data_fp);
                fclose(temp_fp);
                rename(temp_filename, filename);
                RETURN_LONG(-4);
            }

            current_size += bytes_write;
        }

        // Усекаем файл
        if (ftruncate(fileno(data_fp), current_size) == -1) {
            php_error_docref(NULL, E_WARNING, "Failed to truncate file: %s", filename);
            efree(dynamic_buffer);
            fclose(data_fp);
            fclose(temp_fp);
            rename(temp_filename, filename);
            RETURN_LONG(-5);
        }

        fclose(temp_fp);
        unlink(temp_filename);
    }

    efree(dynamic_buffer);
    fclose(data_fp);

    if(mode == 1){
        fclose(temp_fp);

        // Заменяем оригинальный файл временным файлом
        rename(temp_filename, filename);
    }

    RETURN_LONG(found_count);
}



PHP_FUNCTION(file_insert_line)
{
    char *filename, *line;
    size_t filename_len, line_len;
    zend_long line_length = 0;
    zend_long mode = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "ss|ll", 
                               &filename, &filename_len, &line, &line_len, &mode, &line_length) == FAILURE) {
        RETURN_FALSE;
    }

    if(line_len == 0) {
        php_error_docref(NULL, E_WARNING, "Cannot insert an empty line");
        RETURN_LONG(-4);
    }

    /* Если длина строки не указана, используем длину + место для символа переноса */
    if (line_length <= 0) {
        line_length = (zend_long)line_len + 1;
    }

    FILE *fp = fopen(filename, "a+");
    if (!fp) {
        php_error_docref(NULL, E_WARNING, "Failed to open file: %s", filename);
        RETURN_LONG(-1);
    }

    /* Блокировка файла, если mode < 100 */
    if (mode < 100) {
        if (flock(fileno(fp), LOCK_EX) == -1) {
            php_error_docref(NULL, E_WARNING, "Failed to lock file: %s", filename);
            fclose(fp);
            RETURN_LONG(-2);
        }
    } else if(mode > 99) {
        mode -= 100;
    }

    /* Получаем текущее смещение (начальная позиция записи) */
    if(fseek(fp, 0, SEEK_END) != 0) {
        php_error_docref(NULL, E_WARNING, "fseek failed in file: %s", filename);
        fclose(fp);
        RETURN_LONG(-1);
    }
    long file_size = ftell(fp);
    if(file_size < 0) {
        php_error_docref(NULL, E_WARNING, "ftell failed in file: %s", filename);
        fclose(fp);
        RETURN_LONG(-1);
    }

    /* Выделяем буфер фиксированной длины */
    char *buffer = emalloc((size_t)line_length + 1);
    if (!buffer) {
        fclose(fp);
        zend_error(E_ERROR, "Out of memory to allocate %ld bytes", line_length + 1);
    }

    /* Заполняем буфер пробелами и копируем строку (до copy_len байт) */
    memset(buffer, ' ', (size_t)line_length);
    size_t copy_len = (line_len < (size_t)line_length ? line_len : (size_t)line_length);
    memcpy(buffer, line, copy_len);
    if(mode == 0 || mode == 2) {
        buffer[line_length - 1] = '\n';
    }
    buffer[line_length] = '\0';

    size_t written = fwrite(buffer, sizeof(char), (size_t)line_length, fp);
    if (written != (size_t)line_length) {
        php_error_docref(NULL, E_WARNING, "Failed to write to file: %s", filename);
        if(ftruncate(fileno(fp), file_size) == -1) {
            efree(buffer);
            fclose(fp);
            zend_error(E_ERROR, "Failed to truncate file: %s", filename);
        }
        efree(buffer);
        fclose(fp);
        RETURN_LONG(-3);
    }

    efree(buffer);
    fclose(fp);

    if(mode > 1) {
        RETURN_LONG(file_size);
    } else {
        RETURN_LONG(file_size / line_length);
    }
}


PHP_FUNCTION(file_select_line) {
    char *filename;
    size_t filename_len;
    ssize_t row;
    ssize_t align;
    size_t mode = 0;

    // Парсинг переданных аргументов
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "sll|l", &filename, &filename_len, &row, &align, &mode) == FAILURE) {
        RETURN_FALSE;
    }

    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        php_error_docref(NULL, E_WARNING, "Failed to open file: %s", filename);
        RETURN_FALSE;
    }

    // Попытка установить блокирующую блокировку на запись
    if(mode < 100){
        if (flock(fileno(fp), LOCK_EX) == -1) {
            php_error_docref(NULL, E_WARNING, "Failed to lock file: %s", filename);
            fclose(fp);
            RETURN_FALSE;
        }
    }

    if(mode > 99) mode -= 100;

    ssize_t bytes_read;
    ssize_t position;

    if(mode == 0 || mode == 2) position = row * align;
    if(mode == 1 || mode == 3) position = row;


    fseek(fp, 0, SEEK_END);
    ssize_t file_size = ftell(fp);

    if(position < 0 || position > file_size || fseek(fp, position , SEEK_SET) == -1){
        php_error_docref(NULL, E_WARNING, "Failed to seek file: %s", filename);
        fclose(fp);
        RETURN_FALSE;
    }

    // Увеличиваем размер буфера на 1 для возможного символа перевода строки
    char *buffer = (char *)emalloc(align + 1); // +1 для '\0' и +1 для '\n'
    if (!buffer) {
        fclose(fp);
        zend_error(E_ERROR, "Out of memory to allocate %ld bytes", align + 1);
    }

    bytes_read = fread(buffer, 1, align, fp);
    if(bytes_read != align){
        php_error_docref(NULL, E_WARNING, "Failed to read to the file: %s", filename);
        efree(buffer);
        fclose(fp);
        RETURN_FALSE;
    }
    fclose(fp);

    // Убедимся, что строка нуль-терминирована
    buffer[bytes_read] = '\0';

    if(mode == 0 || mode == 1){
        if (mode == 1) {
            char *line_end = strchr(buffer, '\n');
            if (line_end != NULL) {
                *line_end = '\0'; // Заменяем перевод строки на нуль-терминатор
            }
        }

        // Обрезка пробелов справа и символа перевода строки
        for (ssize_t i = bytes_read - 1; i >= 0; --i) {
            if(buffer[i] == ' ' || buffer[i] == '\n') buffer[i] = '\0';
            else break;
        }
    }

    // Возврат строки в PHP
    RETURN_STRING(buffer);
}



PHP_FUNCTION(file_update_line) { 
    char *filename;
    size_t filename_len;
    char *line;
    ssize_t line_len;
    ssize_t position, line_length;
    size_t mode = 0;

    // Парсинг аргументов, переданных в функцию
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "ssll|l", &filename, &filename_len, &line, &line_len, &position, &line_length, &mode) == FAILURE) {
        RETURN_FALSE;
    }

    FILE *fp = fopen(filename, "r+"); // Открытие файла для чтения и записи
    if (!fp) {
        php_error_docref(NULL, E_WARNING, "Failed to open file: %s", filename);
        RETURN_LONG(-1);
    }

    // Попытка установить блокирующую блокировку на запись
    if(mode < 100){
        if (flock(fileno(fp), LOCK_EX) == -1) {
            php_error_docref(NULL, E_WARNING, "Failed to lock file: %s", filename);
            fclose(fp);
            RETURN_LONG(-2);
        }
    }

    if(mode > 99) mode -= 100;

    fseek(fp, 0, SEEK_END);
    ssize_t file_size = ftell(fp);

    if (fseek(fp, position, SEEK_SET) != 0 || position > file_size) {
        php_error_docref(NULL, E_WARNING, "Failed to seek in the file: %s", filename);
        fclose(fp);
        RETURN_LONG(-3);
    }

    // Подготовка строки к записи с учетом выравнивания и перевода строки
    char *buffer = (char *)emalloc(line_length + 1); // +1 для '\0'
    if (!buffer) {
        fclose(fp);
        zend_error(E_ERROR, "Out of memory to allocate %ld bytes", line_length + 1);
    }


    memset(buffer, ' ', line_length); // Заполнение пробелами

    // Копирование line в буфер с учетом выравнивания
    strncpy(buffer, line, line_len < line_length ? line_len : line_length);

    if(mode == 0) buffer[line_length - 1] = '\n';
    buffer[line_length] = '\0'; // Нуль-терминатор

    // Запись в файл
    ssize_t written = fwrite(buffer, 1, line_length, fp);
    fclose(fp); // Это также разблокирует файл
    efree(buffer);

    if (written != line_length) {
        php_error_docref(NULL, E_WARNING, "Failed to write to the file: %s", filename);
        RETURN_LONG(-4);
    }

    RETURN_LONG(written);
}




PHP_FUNCTION(file_analize) { // Анализ таблицы
    char *filename;
    size_t filename_len;
    size_t mode = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "s|l", &filename, &filename_len, &mode) == FAILURE) {
        RETURN_FALSE;
    }

    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        php_error_docref(NULL, E_WARNING, "Failed to open file: %s", filename);
        RETURN_LONG(-1);
    }

    // Попытка установить блокирующую блокировку на запись
    if(mode < 100){
        if (flock(fileno(fp), LOCK_EX) == -1) {
            php_error_docref(NULL, E_WARNING, "Failed to lock file: %s", filename);
            fclose(fp);
            RETURN_LONG(-2);
        }
    }

    if(mode > 99) mode -= 100;

    // Перемещение указателя в конец файла для получения его размера
    fseek(fp, 0, SEEK_END);
    ssize_t file_size = ftell(fp);

    char last_symbol;
    fseek(fp, file_size - 1, SEEK_SET);
    last_symbol = fgetc(fp);

    fseek(fp, 0, SEEK_SET);

    ssize_t ini_buffer_size = FAST_IO_G(buffer_size);

    if(file_size < ini_buffer_size) ini_buffer_size = file_size;
    if(ini_buffer_size < 16) ini_buffer_size = 16;

    char *buffer = (char *)emalloc(ini_buffer_size + 1);
    if (!buffer) {
        fclose(fp);
        zend_error(E_ERROR, "Out of memory to allocate %ld bytes", ini_buffer_size + 1);
    }

    ssize_t bytes_read;
    size_t max_length = 0, max_length_offset = 0, min_length_offset = 0, current_length = 0;
    double avg_length = 0.0;
    size_t min_length = SIZE_MAX;
    size_t line_count = 0;
    ssize_t total_characters = 0;
    ssize_t flow_interruption = 0;

    array_init(return_value);

    while ((bytes_read = fread(buffer, 1, ini_buffer_size, fp)) > 0) {
        for (ssize_t i = 0; i < bytes_read; ++i) {
            if (buffer[i] == '\n') { // Конец текущей строки
                line_count++;

                if (current_length > max_length) {
                    max_length = current_length + 1; // Обновляем максимальную длину
                    max_length_offset = total_characters;
                }

                if(current_length < min_length){
                    min_length = current_length + 1;
                    min_length_offset = total_characters;
                }

                total_characters += current_length + 1;
                avg_length = total_characters / line_count; // Вычисляем среднюю длину

                if(mode == 1) { // Возвращаем длину первой строки.
                    efree(buffer);
                    fclose(fp);

                    add_assoc_long(return_value, "min_length", min_length);
                    add_assoc_long(return_value, "max_length", max_length);
                    add_assoc_double(return_value, "avg_length", avg_length);
                    add_assoc_long(return_value, "line_count", line_count - 1);
                    add_assoc_long(return_value, "total_characters", total_characters);
                    add_assoc_long(return_value, "last_symbol", last_symbol);
                    add_assoc_long(return_value, "file_size", file_size);
                    RETURN_ZVAL(return_value, 0, 1);
                }

                current_length = 0; // Сброс длины для следующей строки
            } else {
                ++current_length; // Увеличиваем длину текущей строки
            }
        }
    }

    efree(buffer);
    fclose(fp);

    add_assoc_long(return_value, "min_length", min_length);
    add_assoc_long(return_value, "min_length_offset", min_length_offset);
    add_assoc_long(return_value, "max_length", max_length);
    add_assoc_long(return_value, "max_length_offset", max_length_offset);
    add_assoc_double(return_value, "avg_length", avg_length);
    add_assoc_long(return_value, "line_count", line_count);
    add_assoc_long(return_value, "total_characters", total_characters);

    if(file_size > total_characters) flow_interruption = file_size - total_characters;
    add_assoc_long(return_value, "flow_interruption", flow_interruption);
    add_assoc_long(return_value, "last_symbol", last_symbol);
    add_assoc_long(return_value, "file_size", file_size);
    RETURN_ZVAL(return_value, 0, 1);
}



PHP_FUNCTION(find_matches_pcre2) {
    char *pattern;
    size_t pattern_len;
    char *subject;
    size_t subject_len;
    size_t mode = 0;

    /* Парсинг аргументов, переданных из PHP */
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "ss|l", &pattern, &pattern_len, &subject, &subject_len, &mode) == FAILURE) {
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

            if(mode == 1){
                zval match_arr;
                array_init(&match_arr);

                add_assoc_stringl(&match_arr, "line_match", subject + start, end - start);
                add_assoc_long(&match_arr, "match_offset", start);
                add_assoc_long(&match_arr, "match_length", end - start);
                add_next_index_zval(&return_matched, &match_arr);
            } else {
                add_next_index_stringl(&return_matched, subject + start, end - start);
            }
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
    } else if (rc == -1) {
        /* Обработка других ошибок. */
        php_error_docref(NULL, E_WARNING, "Matching error %d", rc);
        if (re != NULL) pcre2_code_free(re);
        if (match_data != NULL) pcre2_match_data_free(match_data);
        RETURN_FALSE;
    }

    /* Освобождаем выделенную память */
    pcre2_match_data_free(match_data);
    pcre2_code_free(re);

    /* Возвращаем подготовленный массив */
    RETURN_ZVAL(&return_matched, 0, 1);
}



PHP_FUNCTION(replicate_file) {
    char *source, *destination;
    size_t source_len, destination_len;
    size_t mode = 0;

    // Парсинг аргументов
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "ss|l", &source, &source_len, &destination, &destination_len, &mode) == FAILURE) {
        RETURN_FALSE;
    }


    FILE *source_fp = fopen(source, "r");
    if (!source_fp) {
        php_error_docref(NULL, E_WARNING, "Failed to open file: %s", source);
        RETURN_LONG(-1);
    }

    FILE *destination_fp = fopen(destination, "w");
    if (!destination_fp) {
        php_error_docref(NULL, E_WARNING, "Failed to open file: %s", destination);
        fclose(source_fp);
        RETURN_LONG(-2);
    }

    if (flock(fileno(source_fp), LOCK_EX) == -1) {
        php_error_docref(NULL, E_WARNING, "Failed to lock the file: %s", source);
        fclose(source_fp);
        fclose(destination_fp);
        unlink(destination);
        RETURN_LONG(-3);
    }
    

    FILE *index_source_fp;
    FILE *index_destination_fp;
    char index_source[source_len + 7];
    char index_destination[destination_len + 7];

    if(mode == 1){
        snprintf(index_source, sizeof(index_source), "%s.index", source);
        snprintf(index_destination, sizeof(index_destination), "%s.index", destination);

        index_source_fp = fopen(index_source, "r");
        if (!index_source_fp) {
            php_error_docref(NULL, E_WARNING, "Failed to open file: %s", index_source);
            fclose(source_fp);
            fclose(destination_fp);
            unlink(destination);
            unlink(index_destination);
            RETURN_LONG(-1);
        }

        index_destination_fp = fopen(index_destination, "w");
        if (!index_destination_fp) {
            php_error_docref(NULL, E_WARNING, "Failed to open file: %s", index_destination);
            fclose(index_source_fp);
            fclose(source_fp);
            unlink(destination);
            unlink(index_destination);
            RETURN_LONG(-2);
        }

        if (flock(fileno(index_source_fp), LOCK_EX) == -1) {
            php_error_docref(NULL, E_WARNING, "Failed to lock the file: %s", index_source);
            fclose(index_source_fp);
            fclose(index_destination_fp);
            fclose(source_fp);
            fclose(destination_fp);
            unlink(destination);
            unlink(index_destination);
            RETURN_LONG(-3);
        }
        
    }

    // Перемещение указателя в конец файла для получения его размера
    fseek(source_fp, 0, SEEK_END);
    ssize_t file_size = ftell(source_fp);
    fseek(source_fp, 0, SEEK_SET);

    ssize_t ini_buffer_size = FAST_IO_G(buffer_size);

    if(file_size < ini_buffer_size) ini_buffer_size = file_size;
    if(ini_buffer_size < 16) ini_buffer_size = 16;

    ssize_t dynamic_buffer_size = ini_buffer_size;
    char *dynamic_buffer = (char *)emalloc(dynamic_buffer_size + 1);
    if (!dynamic_buffer) {
        fclose(index_source_fp);
        fclose(index_destination_fp);
        fclose(source_fp);
        fclose(destination_fp);
        unlink(destination);
        if(mode == 1) unlink(index_destination);
        zend_error(E_ERROR, "Out of memory to allocate %ld bytes", dynamic_buffer_size + 1);
    }


    ssize_t current_size = 0;
    ssize_t bytes_read;
    ssize_t bytes_write;

    while ((bytes_read = fread(dynamic_buffer, 1, sizeof(dynamic_buffer), source_fp)) > 0) {
        bytes_write = fwrite(dynamic_buffer, 1, bytes_read, destination_fp);

        if(bytes_read != bytes_write) { 
            php_error_docref(NULL, E_WARNING, "Failed to write to the file: %s", destination);
            fclose(source_fp);
            fclose(destination_fp);
            if(mode == 1){
                fclose(index_source_fp);
                fclose(index_destination_fp);
                unlink(index_destination);
            }
            efree(dynamic_buffer);
            unlink(destination);
            RETURN_LONG(-4);
        }

        current_size += bytes_write;
    }

    if(mode == 1){
        while ((bytes_read = fread(dynamic_buffer, 1, sizeof(dynamic_buffer), index_source_fp)) > 0) {
            bytes_write = fwrite(dynamic_buffer, 1, bytes_read, index_destination_fp);

            if(bytes_read != bytes_write) { 
                php_error_docref(NULL, E_WARNING, "Failed to write to the file: %s", index_destination);
                fclose(source_fp);
                fclose(destination_fp);
                fclose(index_source_fp);
                fclose(index_destination_fp);
                efree(dynamic_buffer);
                unlink(index_destination);
                unlink(destination);
                RETURN_LONG(-4);
            }

            current_size += bytes_write;
        }

        fclose(index_source_fp);
        fclose(index_destination_fp);
    }

    fclose(source_fp);
    fclose(destination_fp);
    efree(dynamic_buffer);

    RETURN_LONG(current_size);
}





PHP_FUNCTION(file_select_array) {
    char  *filename;
    size_t filename_len;
    char *pattern = NULL;
    size_t pattern_len = 0;
    zval  *array = NULL;
    zend_long mode = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "sa|sl", &filename, &filename_len, &array, &pattern, &pattern_len, &mode) == FAILURE) {
        RETURN_FALSE;
    }

    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        php_error_docref(NULL, E_WARNING, "Failed to open file: %s", filename);
        RETURN_FALSE;
    }

    // Попытка установить блокирующую блокировку на запись
    if(mode < 100){
        if (flock(fileno(fp), LOCK_EX) == -1) {
            php_error_docref(NULL, E_WARNING, "Failed to lock file: %s", filename);
            fclose(fp);
            RETURN_FALSE;
        }
    }

    if(mode > 99) mode -= 100;

    // Перемещение указателя в конец файла для получения его размера
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    if (file_size == -1) {
        php_error_docref(NULL, E_WARNING, "Failed to get file size: %s", filename);
        fclose(fp);
        RETURN_FALSE;
    }

    array_init(return_value);
    zval *elem, *value;
    size_t bytes_read;
    bool found_match = false;

    size_t found_count = 0;
    size_t line_count = 0;

    pcre2_code *re = NULL;
    pcre2_match_data *match_data = NULL; 

    if(mode > 9){
        PCRE2_SIZE erroffset;
        int errorcode;
        
        if (pattern == NULL) {
            php_error_docref(NULL, E_WARNING, "Pattern is required for regex modes");
            fclose(fp);
            RETURN_FALSE;
        }
        
        re = pcre2_compile((PCRE2_SPTR)pattern, PCRE2_ZERO_TERMINATED, 0, &errorcode, &erroffset, NULL);

        if (re == NULL) {
            PCRE2_UCHAR message[256];
            pcre2_get_error_message(errorcode, message, sizeof(message));
            php_error_docref(NULL, E_WARNING, "PCRE2 compilation failed at offset %d: %s", (int)erroffset, message);
            fclose(fp);
            RETURN_FALSE;
        }

        match_data = pcre2_match_data_create_from_pattern(re, NULL);
        if (match_data == NULL) {
            php_error_docref(NULL, E_WARNING, "Failed to create PCRE2 match data");
            pcre2_code_free(re);
            fclose(fp);
            RETURN_FALSE;
        }
    }

    // Если массив был передан, обходим его
    if (array) {
        ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(array), elem) {
            if (Z_TYPE_P(elem) == IS_ARRAY) {
                int num_elem = 0;
                long select_pos = -1; // Позиция
                long select_size = -1;

                ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(elem), value) {
                    if (Z_TYPE_P(value) == IS_LONG) {
                        if(num_elem == 0) select_pos = Z_LVAL_P(value);
                        if(num_elem == 1) select_size = Z_LVAL_P(value);
                        num_elem++;
                    }
                } ZEND_HASH_FOREACH_END();

                if(select_pos >= 0 && select_size > 0 && 
                   select_pos < file_size && 
                   select_pos + select_size <= file_size){
                    
                    line_count++;
                    found_match = false;

                    char *buffer = (char *)emalloc(select_size + 1);
                    if (!buffer) {
                        if (re != NULL) pcre2_code_free(re);
                        if (match_data != NULL) pcre2_match_data_free(match_data);
                        fclose(fp);
                        zend_error(E_ERROR, "Out of memory to allocate %ld bytes", select_size + 1);
                        RETURN_FALSE;
                    }

                    if (fseek(fp, select_pos, SEEK_SET) != 0) {
                        php_error_docref(NULL, E_WARNING, "Failed to seek in file: %s", filename);
                        efree(buffer);
                        if (re != NULL) pcre2_code_free(re);
                        if (match_data != NULL) pcre2_match_data_free(match_data);
                        fclose(fp);
                        RETURN_FALSE;
                    }

                    bytes_read = fread(buffer, 1, select_size, fp);
                    if(bytes_read != select_size){
                        php_error_docref(NULL, E_WARNING, "Failed to read from file: %s", filename);
                        efree(buffer);
                        if (re != NULL) pcre2_code_free(re);
                        if (match_data != NULL) pcre2_match_data_free(match_data);
                        fclose(fp);
                        RETURN_FALSE;
                    }

                    buffer[bytes_read] = '\0';

                    zval line_arr;
                    array_init(&line_arr);

                    long i;

                    if(
                        mode == 0 ||
                        mode == 2 ||
                        mode == 5 ||
                        mode == 7 ||
                        mode == 10 ||
                        mode == 12 ||
                        mode == 20 ||                      
                        mode == 24                        
                    ) {
                        // Обрезка пробелов справа и символа перевода строки
                        for (i = bytes_read - 1; i >= 0; --i) {
                            if(buffer[i] == ' ' || buffer[i] == '\n') buffer[i] = '\0';
                            else break;
                        }
                    }

                    if(mode == 0) {
                        add_assoc_string(&line_arr, "trim_line", buffer);
                        add_assoc_long(&line_arr, "trim_length", i + 1);
                        found_match = true;
                    }

                    if(mode == 1) {
                        add_assoc_string(&line_arr, "line", buffer);
                        found_match = true;
                    }

                    if(mode == 2) {
                        add_next_index_string(return_value, buffer);
                        found_match = false;
                    }

                    if(mode == 3) {
                        if (pattern != NULL && strstr(buffer, pattern) != NULL) {
                            found_count++;
                        }
                        found_match = false;
                    }

                    if(mode == 5){
                        if (pattern != NULL && strstr(buffer, pattern) != NULL) {
                            add_assoc_string(&line_arr, "trim_line", buffer);
                            add_assoc_long(&line_arr, "trim_length", i + 1);
                            found_match = true;
                        }
                    }

                    if(mode == 6) {
                        add_assoc_string(&line_arr, "line", buffer);
                        found_match = true;
                    }

                    if(mode == 7){
                        if (pattern != NULL && strstr(buffer, pattern) != NULL) {
                            add_next_index_string(return_value, buffer);
                        }
                        found_match = false;
                    }

                    if(mode == 0 || mode == 5 || mode == 1 || mode == 6){
                        add_assoc_long(&line_arr, "line_offset", select_pos);
                        add_assoc_long(&line_arr, "line_length", select_size);
                    }
                    
                    if(mode > 9 && mode < 14){
                        int rc = pcre2_match(re, (PCRE2_SPTR)buffer, select_size, 0, 0, match_data, NULL);
                        
                        if(rc > 0){
                            if(mode < 13){
                                if(mode == 10){
                                    add_assoc_string(&line_arr, "trim_line", buffer);
                                    add_assoc_long(&line_arr, "trim_length", i + 1);
                                    found_match = true;
                                }

                                if(mode == 11) {
                                    add_assoc_string(&line_arr, "line", buffer);
                                    found_match = true;
                                }

                                if(mode < 12){
                                    add_assoc_long(&line_arr, "line_offset", select_pos);
                                    add_assoc_long(&line_arr, "line_length", select_size);
                                }

                                if(mode == 12) {
                                    add_next_index_string(return_value, buffer);
                                    found_match = false;
                                }
                            }

                            if(mode == 13){
                                found_count++;
                                found_match = false;
                            }
                        }
                    }

                    if(mode > 19 && mode < 25) {
                        zval return_matched;
                        array_init(&return_matched);

                        int rc;
                        PCRE2_SIZE *ovector;
                        size_t start_offset = 0;
                        found_match = false;

                        while ((rc = pcre2_match(re, (PCRE2_SPTR)buffer, select_size, start_offset, 0, match_data, NULL)) > 0) {
                            ovector = pcre2_get_ovector_pointer(match_data);

                            for (int j = 0; j < rc; j++) {
                                PCRE2_SIZE start = ovector[2*j];
                                PCRE2_SIZE end = ovector[2*j+1];

                                if(mode > 21){
                                    zval match_arr;
                                    array_init(&match_arr);

                                    if(mode == 23){
                                        add_next_index_stringl(&return_matched, buffer + start, end - start);
                                    } else {
                                        add_assoc_stringl(&match_arr, "line_match", buffer + start, end - start);
                                    }

                                    if(mode != 23){
                                        add_assoc_long(&match_arr, "match_offset", start);
                                        add_assoc_long(&match_arr, "match_length", end - start);
                                        add_next_index_zval(&return_matched, &match_arr);
                                    }
                                } else {
                                    add_next_index_stringl(&return_matched, buffer + start, end - start);
                                }
                            }

                            // Изменение для предотвращения потенциального бесконечного цикла
                            if (ovector[1] > start_offset) {
                                start_offset = ovector[1];
                            } else {
                                start_offset++; // Для продолжения поиска следующего совпадения
                                if (start_offset >= bytes_read) break; // Выходим из цикла, если достигнут конец строки
                            }

                            found_match = true;
                        }

                        if (rc == PCRE2_ERROR_NOMATCH) {
                            /* Если совпадений нет, возвращаем пустой массив. */
                        } else if (rc < 0) {
                            /* Обработка других ошибок. */
                            php_error_docref(NULL, E_WARNING, "Matching error %d", rc);
                            efree(buffer);
                            if (re != NULL) pcre2_code_free(re);
                            if (match_data != NULL) pcre2_match_data_free(match_data);
                            fclose(fp);
                            RETURN_FALSE;
                        }

                        if(found_match){
                            if(mode == 20) {
                                add_assoc_string(&line_arr, "trim_line", buffer);
                                add_assoc_long(&line_arr, "trim_length", i + 1);
                            }

                            if(mode == 21) {
                                add_assoc_string(&line_arr, "line", buffer);
                            }

                            if(mode != 23){
                                add_assoc_zval(&line_arr, "line_matches", &return_matched);
                                
                                add_assoc_long(&line_arr, "line_offset", select_pos);
                                add_assoc_long(&line_arr, "line_length", select_size);
                            }

                            if(mode == 23){
                                add_next_index_zval(return_value, &return_matched);
                                found_match = false;
                            }
                        } else {
                            zval_ptr_dtor(&return_matched);
                        }
                    }

                    if(found_match){
                        add_next_index_zval(return_value, &line_arr);
                    } else {
                        zval_ptr_dtor(&line_arr);
                    }

                    efree(buffer);
                }
            }
        } ZEND_HASH_FOREACH_END();
    }

    fclose(fp);
    if (re != NULL) pcre2_code_free(re);
    if (match_data != NULL) pcre2_match_data_free(match_data);

    if(mode == 3 || mode == 13){      
        add_assoc_long(return_value, "line_count", line_count);
        add_assoc_long(return_value, "found_count", found_count);
    }

    // Если массив пуст, возвращаем FALSE
    if (Z_TYPE_P(return_value) == IS_ARRAY && zend_hash_num_elements(Z_ARRVAL_P(return_value)) == 0) {
        zval_ptr_dtor(return_value);
        RETURN_FALSE;
    }

    return;
}



PHP_FUNCTION(file_update_array) {
    char *filename;
    size_t filename_len;
    
    zval  *array = NULL;
    size_t mode = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "sa|l", &filename, &filename_len, &array, &mode) == FAILURE) {
        RETURN_FALSE;
    }

    FILE *fp = fopen(filename, "r+");
    if (fp == NULL) {
        php_error_docref(NULL, E_WARNING, "Failed to open file: %s", filename);
        RETURN_LONG(-1);
    }

    // Попытка установить блокирующую блокировку на запись
    if(mode < 100){
        if (flock(fileno(fp), LOCK_EX) == -1) {
            php_error_docref(NULL, E_WARNING, "Failed to lock file: %s", filename);
            fclose(fp);
            RETURN_LONG(-2);
        }
    }

    if(mode > 99) mode -= 100;

    // Перемещение указателя в конец файла для получения его размера
    fseek(fp, 0, SEEK_END);
    ssize_t file_size = ftell(fp);

    ssize_t bytes_write;
    char *buffer = NULL;
    
    ssize_t written = 0;
    ssize_t len = 0;

    zval *elem, *value;


    // Если массив был передан, обходим его
    if (array) {
        ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(array), elem) {
            if (Z_TYPE_P(elem) == IS_ARRAY) {
                int num_elem = 0;
                len = 0;
                ssize_t update_pos = -1; // Позиция
                ssize_t update_size = -1;
                char *found_value = NULL;

                ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(elem), value) {
                    if(Z_TYPE_P(value) == IS_STRING && num_elem == 0) found_value = Z_STRVAL_P(value);
                    if(Z_TYPE_P(value) == IS_LONG && num_elem == 1) update_pos = Z_LVAL_P(value);
                    if(Z_TYPE_P(value) == IS_LONG && num_elem == 2) update_size = Z_LVAL_P(value);
                    num_elem++;
                } ZEND_HASH_FOREACH_END();

                if(update_pos != -1 && file_size > update_pos && update_size > 0 && found_value != NULL){
                    len = (ssize_t)strlen(found_value);
                    buffer = (char *)emalloc(update_size + 1); // +1 для '\0'

                    if (!buffer) {
                        fclose(fp);
                        zend_error(E_ERROR, "Out of memory to allocate %ld bytes", update_size + 1);
                    }

                    memset(buffer, ' ', update_size); // Заполнение пробелами
                    buffer[update_size] = '\0'; // Нуль-терминатор

                    // Копирование в буфер с учетом выравнивания
                    if(update_size < len) len = update_size;
                    strncpy(buffer, found_value, len);
                    if(mode == 0) buffer[update_size - 1] = '\n'; // Добавление перевода строки

                    if (fseek(fp, update_pos, SEEK_SET) != 0) {
                        php_error_docref(NULL, E_WARNING, "Failed to seek in the file: %s", filename);
                        fclose(fp);
                        efree(buffer);
                        RETURN_LONG(-3);
                    }

                    // Запись в файл
                    bytes_write = fwrite(buffer, 1, update_size, fp);
                    
                    if (bytes_write != update_size) {
                        php_error_docref(NULL, E_WARNING, "Failed to write to the file: %s", filename);
                        fclose(fp);
                        efree(buffer);
                        RETURN_LONG(-4);
                    }

                    written += bytes_write;
                    efree(buffer);
                }
            }
        } ZEND_HASH_FOREACH_END();
    }

    fclose(fp);
    RETURN_LONG(written);
}




PHP_FUNCTION(file_callback_line) {
    char *filename;
    size_t filename_len;
    ssize_t position = 0;
    size_t mode = 0;
    zval *callback;
    zval retval;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "sz|ll", &filename, &filename_len, &callback, &position, &mode) == FAILURE) {
        RETURN_FALSE;
    }

    // Проверяем, что переданный параметр действительно является callback-функцией
    if (!zend_is_callable(callback, 0, NULL)) {
        php_error_docref(NULL, E_WARNING, "The passed parameter is not a callback function");
        RETURN_FALSE;
    }

    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        php_error_docref(NULL, E_WARNING, "Failed to open file: %s", filename);
        RETURN_FALSE;
    }

    // Попытка установить блокирующую блокировку на запись
    if(mode < 100){
        if (flock(fileno(fp), LOCK_EX) == -1) {
            php_error_docref(NULL, E_WARNING, "Failed to lock file: %s", filename);
            fclose(fp);
            RETURN_FALSE;
        }
    }

    if(mode > 99) mode -= 100;

    // Перемещение указателя в конец файла для получения его размера
    fseek(fp, 0, SEEK_END);
    ssize_t file_size = ftell(fp);
    ssize_t line_offset = 0; // Смещение начала строки

    if(position > 0){
        if(position >= file_size){
            php_error_docref(NULL, E_WARNING, "Failed to seek file: %s", filename);
            fclose(fp);
            RETURN_FALSE;
        }

        fseek(fp, position, SEEK_SET);
        line_offset = position;
    } else {
        fseek(fp, 0, SEEK_SET);
    }

    ssize_t ini_buffer_size = FAST_IO_G(buffer_size);

    if(file_size < ini_buffer_size) ini_buffer_size = file_size;
    if(ini_buffer_size < 16) ini_buffer_size = 16;

    ssize_t dynamic_buffer_size = ini_buffer_size;
    char *dynamic_buffer = (char *)emalloc(dynamic_buffer_size + 1);
    if (!dynamic_buffer) {
        fclose(fp);
        zend_error(E_ERROR, "Out of memory to allocate %ld bytes", dynamic_buffer_size + 1);
    }

    char *found_value = (char *)emalloc(1);
    found_value[0] = '\0';

    ssize_t bytes_read;
    ssize_t current_size = 0; // Текущий размер данных в динамическом буфере
    size_t line_count = 0;
    bool found_match = false;
    bool jump = false;

    while ((bytes_read = fread(dynamic_buffer + current_size, 1, ini_buffer_size, fp)) > 0) {
        current_size += bytes_read;
        
        dynamic_buffer[current_size] = '\0';

        char *line_start = dynamic_buffer;
        char *line_end;
        while ((line_end = strchr(line_start, '\n')) != NULL) {
            ssize_t line_length = line_end - line_start + 1;

            zval retval; // Инициализация переменной для возвращаемого значения
            zval args[10];

            *line_end = '\0';
            ZVAL_STRING(&args[0], line_start);
            *line_end = '\n';
            
            if(mode > 0){
                // Подготовка параметров для callback-функции
                ZVAL_STRING(&args[1], filename);
                if(mode > 1) ZVAL_LONG(&args[2], line_offset); 
                if(mode > 2) ZVAL_LONG(&args[3], line_length);
                if(mode > 3) ZVAL_LONG(&args[4], line_count);
                if(mode > 4) ZVAL_LONG(&args[5], position);
                if(mode > 5) ZVAL_STRING(&args[6], found_value);
                if(mode > 6) ZVAL_LONG(&args[7], file_size);
                if(mode > 7) ZVAL_LONG(&args[8], dynamic_buffer_size);
                if(mode > 8) ZVAL_STRING(&args[9], dynamic_buffer);
            }

            // Вызываем callback-функцию с одним аргументом
            if (call_user_function(EG(function_table), NULL, callback, &retval, mode + 1, args) == SUCCESS) {
                if(Z_TYPE_P(&retval) == IS_STRING) {
                    char *temp_retval = (char *)erealloc(found_value, Z_STRLEN_P(&retval) + 1);
                    if (!temp_retval) {
                        fclose(fp);
                        if (dynamic_buffer) efree(dynamic_buffer);
                        if (found_value) efree(found_value);
                        for (size_t i = 0; i <= mode; i++) zval_dtor(&args[i]);
                        zval_dtor(&retval);
                        zend_error(E_ERROR, "Out of memory to allocate %ld bytes", Z_STRLEN_P(&retval) + 1);
                    }

                    found_value = temp_retval;

                    strncpy(found_value, Z_STRVAL_P(&retval), Z_STRLEN_P(&retval));
                    found_value[Z_STRLEN_P(&retval)] = '\0';
                }

                if(Z_TYPE_P(&retval) == IS_LONG) {
                    if((ssize_t) Z_LVAL_P(&retval) >= file_size || Z_LVAL_P(&retval) < 0){
                        php_error_docref(NULL, E_WARNING, "Failed to seek file: %s", filename);
                        fclose(fp);
                        if (dynamic_buffer) efree(dynamic_buffer);
                        if (found_value) efree(found_value);
                        for (size_t i = 0; i <= mode; i++) zval_dtor(&args[i]);
                        zval_dtor(&retval);
                        RETURN_FALSE;
                    }

                    position = Z_LVAL_P(&retval);

                    fseek(fp, position, SEEK_SET);
                    line_offset = position;
                    jump = true;
                }


                if(Z_TYPE_P(&retval) == IS_FALSE) {
                    found_match = true;
                }

            } else {
                // Если вызов функции не удался, возвращаем FALSE
                php_error_docref(NULL, E_WARNING, "Failed call callback function");
                fclose(fp);
                if (dynamic_buffer) efree(dynamic_buffer);
                if (found_value) efree(found_value);
                for (size_t i = 0; i <= mode; i++) zval_dtor(&args[i]);
                zval_dtor(&retval);
                RETURN_FALSE;
            }

            zval_dtor(&retval);

            // Освобождаем ресурсы
            for (size_t i = 0; i <= mode; i++) {
                zval_dtor(&args[i]);
            }

            line_count++; 
            line_start = line_end + 1;
  
            if (jump) {
                jump = false;
                break;
            } else {
                line_offset += line_length; // Обновляем смещение
            }


            if (found_match) break;
        }

        if (found_match) break;

        current_size -= (line_start - dynamic_buffer);
        memmove(dynamic_buffer, line_start, current_size);

        if (current_size + ini_buffer_size > dynamic_buffer_size) {
            dynamic_buffer_size += ini_buffer_size;

            char *temp_buffer = (char *)erealloc(dynamic_buffer, dynamic_buffer_size + 1);
            if (!temp_buffer) {
                fclose(fp);
                if (dynamic_buffer) efree(dynamic_buffer);
                efree(found_value);
                zend_error(E_ERROR, "Out of memory to allocate %ld bytes", dynamic_buffer_size + 1);
            }

            dynamic_buffer = temp_buffer;
        }
    }

    efree(dynamic_buffer);
    fclose(fp);

    RETURN_STRING(found_value);
}
