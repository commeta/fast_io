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

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_file_update_line, 0, 3, IS_LONG, 0)
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

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_find_matches_pcre2, 0, 2, IS_ARRAY, 0)
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
        return;
    }


    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        php_error_docref(NULL, E_WARNING, "Failed to open file: %s", filename);
        RETURN_FALSE;
    }

    // Попытка установить блокирующую блокировку на запись
    if(mode < 100){
        if (flock(fileno(fp), LOCK_EX) < 0) {
            php_error_docref(NULL, E_WARNING, "Failed to lock file: %s", filename);
            fclose(fp);
            RETURN_FALSE;
        }
    }

    if(mode > 99) mode -= 100;

    // Перемещение указателя в конец файла для получения его размера
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);

    off_t search_offset = 0; // Смещение строки поиска

    if(position > 0){
        if(position >= file_size){
            php_error_docref(NULL, E_WARNING, "Failed to seek file: %s", filename);
            fclose(fp);
            RETURN_FALSE;
        }

        fseek(fp, position, SEEK_SET);
        search_offset = position;
    } else {
        fseek(fp, 0, SEEK_SET);
    }

    zend_long ini_buffer_size = FAST_IO_G(buffer_size);

    if(file_size < ini_buffer_size) ini_buffer_size = file_size;
    if(ini_buffer_size < 16) ini_buffer_size = 16;

    zend_long dynamic_buffer_size = ini_buffer_size;
    char *dynamic_buffer = (char *)emalloc(dynamic_buffer_size + 1);
    if (!dynamic_buffer) {
        php_error_docref(NULL, E_WARNING, "Out of memory");
        fclose(fp);
        RETURN_FALSE;
    }


    ssize_t bytesRead;
    size_t current_size = 0; // Текущий размер данных в динамическом буфере

    zend_long found_count = 0;
    zend_long add_count = 0;
    zend_long line_count = 0;

    bool found_match = false;

    pcre2_code *re;
    pcre2_match_data *match_data; 

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
    }


    while ((bytesRead = fread(dynamic_buffer + current_size, 1, ini_buffer_size, fp)) > 0) {
        current_size += bytesRead;
       
        dynamic_buffer[current_size] = '\0';

        char *lineStart = dynamic_buffer;
        char *lineEnd;
        while ((lineEnd = strchr(lineStart, '\n')) != NULL) {
            ssize_t lineLength = lineEnd - lineStart + 1;
            *lineEnd = '\0';
            line_count++;

            if(mode < 3 && strstr(lineStart, line_key) != NULL){
                found_count++;

                if(search_start < found_count){
                    add_count++;

                    zval line_arr;
                    array_init(&line_arr);

                    if(mode == 0 || mode == 2){
                        for (int i = lineLength - 2; i >= 0; --i) {
                            if(lineStart[i] == ' ') lineStart[i] = '\0';
                            else break;
                        }

                        add_assoc_string(&line_arr, "trim_line", lineStart);
                    }


                    if(mode == 0){
                        add_assoc_long(&line_arr, "trim_length", strlen(lineStart));
                    }

                    if(mode == 1){
                        add_assoc_string(&line_arr, "line", lineStart);
                    }

                    if(mode != 2){
                        add_assoc_long(&line_arr, "line_offset", search_offset);
                        add_assoc_long(&line_arr, "line_length", lineLength);
                        add_assoc_long(&line_arr, "line_count", line_count);
                    }

                    add_next_index_zval(return_value, &line_arr);
                }
            }


            if(mode == 3 && strstr(lineStart, line_key) != NULL){
                found_count++;
            }

            if(mode > 9 && mode < 13 && pcre2_match(re, lineStart, lineLength - 1, 0, 0, match_data, NULL) > 0){
                found_count++;

                if(search_start < found_count){
                    add_count++;

                    zval line_arr;
                    array_init(&line_arr);

                    if(mode == 10 || mode == 12){
                        for (int i = lineLength - 2; i >= 0; --i) {
                            if(lineStart[i] == ' ') lineStart[i] = '\0';
                            else break;
                        }
                    }
                    
                    if(mode == 10){
                        add_assoc_string(&line_arr, "trim_line", lineStart);
                        add_assoc_long(&line_arr, "trim_length", strlen(lineStart));
                    } else {
                        add_assoc_string(&line_arr, "line", lineStart);
                    }

                    if(mode != 12){
                        add_assoc_long(&line_arr, "line_offset", search_offset);
                        add_assoc_long(&line_arr, "line_length", lineLength);
                        add_assoc_long(&line_arr, "line_count", line_count);
                    }

                    if(mode == 12){
                        add_next_index_string(return_value, lineStart);
                    } else {
                        add_next_index_zval(return_value, &line_arr);
                    }
                }
            }

            if(mode == 13 && pcre2_match(re, lineStart, lineLength, 0, 0, match_data, NULL) > 0){
                found_count++;
            }


            if(mode > 19 && mode < 25){
                if(search_start < found_count + 1){
                    zval return_matched;
                    array_init(&return_matched);

                    int rc;
                    PCRE2_SIZE *ovector;
                    size_t start_offset = 0;

                    bool is_matched = false;

                    while ((rc = pcre2_match(re, (PCRE2_SPTR)lineStart, lineLength, start_offset, 0, match_data, NULL)) > 0) {
                        ovector = pcre2_get_ovector_pointer(match_data);

                        for (int i = 0; i < rc; i++) {
                            PCRE2_SIZE start = ovector[2*i];
                            PCRE2_SIZE end = ovector[2*i+1];

                            if(mode > 21){
                                zval match_arr;
                                array_init(&match_arr);

                                if(mode == 23){
                                    add_next_index_stringl(&return_matched, lineStart + start, end - start);
                                } else {
                                    add_assoc_stringl(&match_arr, "line_match", lineStart + start, end - start);
                                }

                                if(mode != 23){
                                    add_next_index_zval(&return_matched, &match_arr);
                                    add_assoc_long(&match_arr, "match_offset", start_offset);
                                    add_assoc_long(&match_arr, "match_length", end - start);
                                }
                                
                            } else {
                                add_next_index_stringl(&return_matched, lineStart + start, end - start);
                            }
                        }

                        // Изменение для предотвращения потенциального бесконечного цикла
                        if (ovector[1] > start_offset) {
                            start_offset = ovector[1];
                        } else {
                            start_offset++; // Для продолжения поиска следующего совпадения
                            if (start_offset >= lineLength) break; // Выходим из цикла, если достигнут конец строки
                        }

                        is_matched = true;
                    }


                    if (rc == PCRE2_ERROR_NOMATCH) {
                        /* Если совпадений нет, возвращаем пустой массив. */
                    } else if (rc < 0) {
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
                            for (int i = lineLength - 2; i >= 0; --i) {
                                if(lineStart[i] == ' ') lineStart[i] = '\0';
                                else break;
                            }

                            add_assoc_string(&line_arr, "trim_line", lineStart);
                            add_assoc_long(&line_arr, "trim_length", strlen(lineStart));
                        }

                        if(mode == 21) {
                            add_assoc_string(&line_arr, "line", lineStart);
                        }

                        if(mode != 23){
                            add_assoc_zval(&line_arr, "line_matches", &return_matched);

                            add_assoc_long(&line_arr, "line_offset", search_offset);
                            add_assoc_long(&line_arr, "line_length", lineLength);
                            add_assoc_long(&line_arr, "line_count", line_count);
                        }

                        if(mode == 23){
                            add_next_index_zval(return_value, &return_matched);
                        } else {
                            add_next_index_zval(return_value, &line_arr);
                        }
                    }

                    found_count++;
                }
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
        current_size -= (lineStart - dynamic_buffer);
        memmove(dynamic_buffer, lineStart, current_size);

        if (current_size + ini_buffer_size > dynamic_buffer_size) {
            dynamic_buffer_size += ini_buffer_size;
            char *temp_buffer = (char *)erealloc(dynamic_buffer, dynamic_buffer_size + 1);
            if (!temp_buffer) {
                php_error_docref(NULL, E_WARNING, "Out of memory");
                fclose(fp);
                if (dynamic_buffer) efree(dynamic_buffer);
                if (re != NULL) pcre2_code_free(re);
                if (mode > 9 && match_data != NULL) pcre2_match_data_free(match_data);
                RETURN_FALSE;
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

    if (mode > 9 && re != NULL) pcre2_code_free(re);
    if (mode > 9 && match_data != NULL) pcre2_match_data_free(match_data);
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
        if (flock(fileno(fp), LOCK_EX) < 0) {
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

    zend_long ini_buffer_size = FAST_IO_G(buffer_size);

    if(file_size < ini_buffer_size) ini_buffer_size = file_size;
    if(ini_buffer_size < 16) ini_buffer_size = 16;

    zend_long dynamic_buffer_size = ini_buffer_size;
    char *dynamic_buffer = (char *)emalloc(dynamic_buffer_size + 1);
    if (!dynamic_buffer) {
        php_error_docref(NULL, E_WARNING, "Out of memory");
        fclose(fp);
        RETURN_FALSE;
    }

    ssize_t bytesRead;
    size_t current_size = 0; // Текущий размер данных в динамическом буфере
    bool found_match = false;

    pcre2_code *re;
    pcre2_match_data *match_data;

    char *found_value = NULL;

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


    while ((bytesRead = fread(dynamic_buffer + current_size, 1, ini_buffer_size, fp)) > 0) {
        current_size += bytesRead;
       
        dynamic_buffer[current_size] = '\0';

        char *lineStart = dynamic_buffer;
        char *lineEnd;
        while ((lineEnd = strchr(lineStart, '\n')) != NULL) {
            ssize_t lineLength = lineEnd - lineStart + 1;
            *lineEnd = '\0';

            if(mode == 0 && strstr(lineStart, line_key) != NULL){
                found_value = estrndup(lineStart, lineLength - 1);
                if(found_value == NULL){
                    php_error_docref(NULL, E_WARNING, "Out of memory");
                    fclose(fp);
                    efree(dynamic_buffer);
                    RETURN_FALSE;
                }

                found_match = true;
                break;
            }

            if(mode == 10 && pcre2_match(re, lineStart, lineLength - 1, 0, 0, match_data, NULL) > 0){
                found_value = estrndup(lineStart, lineLength - 1);
                if(found_value == NULL){
                    php_error_docref(NULL, E_WARNING, "Out of memory");
                    fclose(fp);
                    efree(dynamic_buffer);
                    if (re != NULL) pcre2_code_free(re);
                    if (match_data != NULL) pcre2_match_data_free(match_data);
                    RETURN_FALSE;
                }
                
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
                if (dynamic_buffer) efree(dynamic_buffer);
                if (re != NULL) pcre2_code_free(re);
                if (mode > 9 && match_data != NULL) pcre2_match_data_free(match_data);
                php_error_docref(NULL, E_WARNING, "Out of memory");
                fclose(fp);
                RETURN_FALSE;
            }
            dynamic_buffer = temp_buffer;
        }
        
    }

    fclose(fp);
    efree(dynamic_buffer);

    if (mode > 9 && re != NULL) pcre2_code_free(re);
    if (mode > 9 && match_data != NULL) pcre2_match_data_free(match_data);

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

    if(position > 0){
        if(position >= file_size){
            php_error_docref(NULL, E_WARNING, "Failed to seek file: %s", filename);
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

    if(mode < 100){
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

    if(mode > 99) mode -= 100;

    zend_long ini_buffer_size = FAST_IO_G(buffer_size);

    if(file_size < ini_buffer_size) ini_buffer_size = file_size;
    if(ini_buffer_size < 16) ini_buffer_size = 16;


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

    while ((bytesRead = fread(dynamic_buffer + current_size, 1, ini_buffer_size, index_fp)) > 0) {
        current_size += bytesRead;
        dynamic_buffer[current_size] = '\0';

        char *lineStart = dynamic_buffer;
        char *lineEnd;

        while ((lineEnd = strchr(lineStart, '\n')) != NULL) {
            *lineEnd = '\0';
            if (strstr(lineStart, line_key) != NULL) {
                found_value = estrdup(lineStart + line_key_len + 1);
                if(found_value == NULL){
                    php_error_docref(NULL, E_WARNING, "Out of memory");
                    fclose(data_fp);
                    fclose(index_fp);
                    efree(dynamic_buffer);
                    RETURN_FALSE;
                }
                
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
                if (dynamic_buffer) efree(dynamic_buffer);
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
    position = atol(found_value);
    size_t size = (size_t)strtoul(colon_ptr + 1, NULL, 10);
    efree(found_value);

    if (size > 0) {
        if (fseek(data_fp, position, SEEK_SET) < 0) {
            php_error_docref(NULL, E_WARNING, "Failed to seek file: %s", filename);
            fclose(data_fp);
            RETURN_FALSE;
        }

        char *dataBuffer = emalloc(size + 1);
        if (!dataBuffer) {
            php_error_docref(NULL, E_WARNING, "Out of memory");
            fclose(data_fp);
            RETURN_FALSE;
        }

        bytesRead = fread(dataBuffer, 1, size, data_fp);
        if(bytesRead != size){
            php_error_docref(NULL, E_WARNING, "Failed to read to the file: %s", filename);
            fclose(data_fp);
            RETURN_FALSE;
        }

        dataBuffer[bytesRead] = '\0';
        RETVAL_STRING(dataBuffer);
        efree(dataBuffer);
    } else {
        RETURN_FALSE;
    }

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
    snprintf(index_filename, filename_len + 7, "%s.index", filename);

    FILE *data_fp = fopen(filename, "a+");
    FILE *index_fp = fopen(index_filename, "a+");

    if (!data_fp) {
        php_error_docref(NULL, E_WARNING, "Failed to open file: %s", filename);
        fclose(index_fp);
        RETURN_LONG(-1);
    }

    if (!index_fp) {
        php_error_docref(NULL, E_WARNING, "Failed to open file: %s", index_filename);
        fclose(data_fp);
        RETURN_LONG(-1);
    }


    if(mode < 100){
        // Блокировка файла для записи
        if (flock(fileno(data_fp), LOCK_EX) == -1) {
            php_error_docref(NULL, E_WARNING, "Failed to lock the file: %s", filename);
            fclose(data_fp); // Это также разблокирует файл
            fclose(index_fp); 
            RETURN_LONG(-2);
        }
        // Блокировка файла для записи
        if (flock(fileno(index_fp), LOCK_EX) == -1) {
            php_error_docref(NULL, E_WARNING, "Failed to lock the file: %s", index_filename);
            fclose(index_fp); // Это также разблокирует файл
            fclose(data_fp);
            RETURN_LONG(-2);
        }
    }

    if(mode > 99) mode -= 100;

    // Запись значения в файл данных
    fseek(data_fp, 0, SEEK_END); // Перемещаем указатель в конец файла
    // Получаем текущее смещение в файле данных
    long position = ftell(data_fp);
    size_t size = strlen(line_value);

    if (fwrite(line_value, 1, size, data_fp) != size) {
        php_error_docref(NULL, E_WARNING, "Failed to write to the file: %s", filename);
        if(ftruncate(fileno(data_fp), position) < 0) {
            zend_error(E_ERROR, "Failed to truncate to the file: %s", filename);
            fclose(data_fp);
            fclose(index_fp);
            exit(EXIT_FAILURE);
        }

        fclose(data_fp);
        fclose(index_fp);
        RETURN_LONG(-3);
    }


    fseek(index_fp, 0, SEEK_END);
    long file_size = ftell(index_fp);

    // Запись индекса в индексный файл
    if(fprintf(index_fp, "%s %ld:%zu\n", line_key, position, size) < line_key_len + 5){
        php_error_docref(NULL, E_WARNING, "Failed to write to the file: %s", index_filename);

        if(ftruncate(fileno(index_fp), file_size) < 0) {
            zend_error(E_ERROR, "Failed to truncate to the file: %s", index_filename);
            fclose(data_fp);
            fclose(index_fp);
            exit(EXIT_FAILURE);
        }
        if(ftruncate(fileno(data_fp), position) < 0) {
            zend_error(E_ERROR, "Failed to truncate to the file: %s", index_filename);
            fclose(data_fp);
            fclose(index_fp);
            exit(EXIT_FAILURE);
        }

        fclose(data_fp);
        fclose(index_fp);
        RETURN_LONG(-3);
    }

    // Закрытие файлов
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
        if (flock(fileno(data_fp), LOCK_EX) < 0) {
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

    zend_long ini_buffer_size = FAST_IO_G(buffer_size);

    if(file_size < ini_buffer_size) ini_buffer_size = file_size;
    if(ini_buffer_size < 16) ini_buffer_size = 16;
    
    zend_long dynamic_buffer_size = ini_buffer_size;
    char *dynamic_buffer = (char *)emalloc(dynamic_buffer_size + 1);
    if (!dynamic_buffer) {
        php_error_docref(NULL, E_WARNING, "Out of memory");
        fclose(data_fp);
        fclose(temp_fp);
        unlink(temp_filename);
        RETURN_LONG(-8);
    }

    zend_long found_count = 0;
    ssize_t bytesRead;
    ssize_t bytesWrite;
    size_t current_size = 0;

    bool found_match;

    while ((bytesRead = fread(dynamic_buffer + current_size, 1, ini_buffer_size, data_fp)) > 0) {
        current_size += bytesRead;
               
        dynamic_buffer[current_size] = '\0';

        char *lineStart = dynamic_buffer;
        char *lineEnd;
        while ((lineEnd = strchr(lineStart, '\n')) != NULL) {
            ssize_t lineLength = lineEnd - lineStart + 1;
            *lineEnd = '\0';

            found_match = false;

            if (line_key != NULL) {
                if (*lineStart == SPECIAL_CHAR || strstr(lineStart, line_key) != NULL) {
                    found_match = true;
                }
            } else {
                if (*lineStart == SPECIAL_CHAR) {
                    found_match = true;
                }
            }

            if(!found_match){
                *lineEnd = '\n';

                bytesWrite = fwrite(lineStart, 1, lineLength, temp_fp);

                if (bytesWrite != lineLength) {
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

            lineStart = lineEnd + 1;
        }


        current_size -= (lineStart - dynamic_buffer);
        memmove(dynamic_buffer, lineStart, current_size);

        if (current_size + ini_buffer_size > dynamic_buffer_size) {
            dynamic_buffer_size += ini_buffer_size;

            char *temp_buffer = (char *)erealloc(dynamic_buffer, dynamic_buffer_size + 1);
            if (!temp_buffer) {
                php_error_docref(NULL, E_WARNING, "Out of memory");
                fclose(data_fp);
                fclose(temp_fp);
                unlink(temp_filename);
                if (dynamic_buffer) efree(dynamic_buffer);
                RETURN_LONG(-8);
            }
            dynamic_buffer = temp_buffer;
        }
    }


    if(mode == 0){
        fseek(data_fp, 0 , SEEK_SET);
        fseek(temp_fp, 0 , SEEK_SET);
        current_size = 0;

        while ((bytesRead = fread(dynamic_buffer, 1, sizeof(dynamic_buffer), temp_fp)) > 0) {
            bytesWrite = fwrite(dynamic_buffer, 1, bytesRead, data_fp); 
            if(bytesRead != bytesWrite){
                php_error_docref(NULL, E_WARNING, "Failed to write to the file: %s", temp_filename);
                fclose(data_fp);
                fclose(temp_fp);
                rename(temp_filename, filename);
                efree(dynamic_buffer);
                RETURN_LONG(-4);
            }
            current_size += bytesWrite;
        }

        // Усекаем файл
        if (ftruncate(fileno(data_fp), current_size) < 0) {
            php_error_docref(NULL, E_WARNING, "Failed to truncate file: %s", filename);
            fclose(data_fp);
            efree(dynamic_buffer);
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




PHP_FUNCTION(file_defrag_data) {
    char *filename, *line_key = NULL;
    size_t filename_len, line_key_len = 0;
    zend_long mode = 0;
    zend_long found_count = 0;

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

    zend_long ini_buffer_size = FAST_IO_G(buffer_size);

    if(file_size < ini_buffer_size) ini_buffer_size = file_size;
    if(ini_buffer_size < 16) ini_buffer_size = 16;
    
    zend_long dynamic_buffer_size = ini_buffer_size;
    char *dynamic_buffer = (char *)emalloc(dynamic_buffer_size + 1);
    if (!dynamic_buffer) {
        php_error_docref(NULL, E_WARNING, "Out of memory");
        fclose(index_fp);
        fclose(data_fp);
        fclose(temp_fp);
        fclose(temp_index_fp);
        unlink(temp_filename);
        unlink(temp_index_filename);
        RETURN_LONG(-8);
    }

    ssize_t bytesRead;
    ssize_t bytesWrite;

    ssize_t bytesReadData;
    ssize_t bytesWriteData;

    size_t current_size = 0;

    bool found_match;

    off_t position;
    size_t size;

    while ((bytesRead = fread(dynamic_buffer + current_size, 1, ini_buffer_size, index_fp)) > 0) {
        current_size += bytesRead;
               
        dynamic_buffer[current_size] = '\0';

        char *lineStart = dynamic_buffer;
        char *lineEnd;
        while ((lineEnd = strchr(lineStart, '\n')) != NULL) {
            ssize_t lineLength = lineEnd - lineStart + 1;
            *lineEnd = '\0';

            found_match = false;

            if (line_key != NULL) {
                if (*lineStart == SPECIAL_CHAR || strstr(lineStart, line_key) != NULL) {
                    found_match = true;
                }
            } else {
                if (*lineStart == SPECIAL_CHAR) {
                    found_match = true;
                }
            }

            if(!found_match){
                *lineEnd = '\n';
                bytesWrite = fwrite(lineStart, 1, lineLength, temp_index_fp);

                if (bytesWrite != lineLength) {
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

                *lineEnd = '\0';

                char *offsetPtr = NULL;
                char *sizePtr = NULL;
                bool parsed_err = false;
                
                char *token = strtok(lineStart, " ");  // Разделяем строку по пробелам
                
                while (token != NULL) {
                    if (strchr(token, ':') != NULL) {  // Проверяем наличие двоеточия
                        offsetPtr = strtok(token, ":");
                        sizePtr = strtok(NULL, ":");
                        break;
                    }
                    token = strtok(NULL, " ");
                }
                
                if (offsetPtr && sizePtr) {
                    position = strtoul(offsetPtr, NULL, 10);
                    size = strtoul(sizePtr, NULL, 10);

                    if (
                        (position == 0 && *offsetPtr != '0') ||
                        (size == 0 && *sizePtr != '0')
                    ) {
                        parsed_err = true;
                    }
                    
                } else {
                    parsed_err = true;
                }

                if(parsed_err == false){
                    fseek(data_fp, position, SEEK_SET); 
                    char *dataBuffer = emalloc(size + 1);

                    if(dataBuffer == NULL){
                        php_error_docref(NULL, E_WARNING, "Out of memory");
                        fclose(index_fp);
                        fclose(data_fp);
                        fclose(temp_fp);
                        fclose(temp_index_fp);
                        unlink(temp_filename);
                        unlink(temp_index_filename);
                        efree(dynamic_buffer);
                        RETURN_LONG(-8);
                    }

                    bytesReadData = fread(dataBuffer, 1, size, data_fp);
                    if(bytesReadData != size){
                        php_error_docref(NULL, E_WARNING, "Failed to read to the file: %s", filename);
                        fclose(index_fp);
                        fclose(data_fp);
                        fclose(temp_fp);
                        fclose(temp_index_fp);
                        unlink(temp_filename);
                        unlink(temp_index_filename);
                        efree(dynamic_buffer);
                        RETURN_LONG(-6);
                    }

                    bytesWriteData = fwrite(dataBuffer, 1, bytesReadData, temp_fp);

                    if(bytesReadData != bytesWriteData){ // err
                        php_error_docref(NULL, E_WARNING, "Failed to write to the file: %s", temp_index_filename);
                        fclose(index_fp);
                        fclose(data_fp);
                        fclose(temp_fp);
                        fclose(temp_index_fp);
                        unlink(temp_filename);
                        unlink(temp_index_filename);
                        efree(dynamic_buffer);
                        efree(dataBuffer);
                        RETURN_LONG(-4);
                    }

                    efree(dataBuffer);
                } else {
                    php_error_docref(NULL, E_WARNING, "Failed to find offset:size");
                    fclose(index_fp);
                    fclose(data_fp);
                    fclose(temp_fp);
                    fclose(temp_index_fp);
                    unlink(temp_filename);
                    unlink(temp_index_filename);
                    efree(dynamic_buffer);
                    RETURN_LONG(-7);
                }

            } else {
                found_count++;
            }

            lineStart = lineEnd + 1;
        }


        current_size -= (lineStart - dynamic_buffer);
        memmove(dynamic_buffer, lineStart, current_size);

        if (current_size + ini_buffer_size > dynamic_buffer_size) {
            dynamic_buffer_size += ini_buffer_size;

            char *temp_buffer = (char *)erealloc(dynamic_buffer, dynamic_buffer_size + 1);
            if (!temp_buffer) {
                php_error_docref(NULL, E_WARNING, "Out of memory");
                fclose(index_fp);
                fclose(data_fp);
                fclose(temp_fp);
                fclose(temp_index_fp);
                unlink(temp_filename);
                unlink(temp_index_filename);
                if (dynamic_buffer) efree(dynamic_buffer);
                RETURN_LONG(-8);
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

        while ((bytesRead = fread(dynamic_buffer, 1, sizeof(dynamic_buffer), temp_fp)) > 0) {
            bytesWrite = fwrite(dynamic_buffer, 1, bytesRead, data_fp); 
            if(bytesWrite != bytesRead){
                php_error_docref(NULL, E_WARNING, "Failed to write to the file: %s", filename);
                fclose(index_fp);
                fclose(data_fp);
                fclose(temp_fp);
                fclose(temp_index_fp);
                rename(temp_filename, filename);
                rename(temp_index_filename, index_filename);
                efree(dynamic_buffer);
                RETURN_LONG(-4);
            }

            current_size += bytesWrite;
        }

        // Усекаем файл
        if (ftruncate(fileno(data_fp), current_size) < 0) {
            php_error_docref(NULL, E_WARNING, "Failed to truncate file: %s", filename);
            fclose(index_fp);
            fclose(data_fp);
            fclose(temp_fp);
            fclose(temp_index_fp);
            rename(temp_filename, filename);
            rename(temp_index_filename, index_filename);
            efree(dynamic_buffer);
            RETURN_LONG(-5);
        }

        current_size = 0;

        while ((bytesRead = fread(dynamic_buffer, 1, sizeof(dynamic_buffer), temp_index_fp)) > 0) {
            bytesWrite = fwrite(dynamic_buffer, 1, bytesRead, index_fp); 
            if(bytesWrite != bytesRead){
                php_error_docref(NULL, E_WARNING, "Failed to write to the file: %s", index_filename);
                fclose(index_fp);
                fclose(data_fp);
                fclose(temp_fp);
                fclose(temp_index_fp);                
                rename(temp_filename, filename);
                rename(temp_index_filename, index_filename);
                efree(dynamic_buffer);
                RETURN_LONG(-4);
            }

            current_size += bytesWrite;
        }
        
        if (ftruncate(fileno(index_fp), current_size) < 0) {
            php_error_docref(NULL, E_WARNING, "Failed to truncate file: %s", index_filename);
            fclose(index_fp);
            fclose(data_fp);
            fclose(temp_fp);
            fclose(temp_index_fp);
            rename(temp_filename, filename);
            rename(temp_index_filename, index_filename);
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

        // Заменяем оригинальный файл временным файлом
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
    zend_long offset = -1; // Значение по умолчанию для необязательного аргумента
    zend_long mode = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "s|ll", &filename, &filename_len, &offset, &mode) == FAILURE) {
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
    off_t file_size = ftell(fp);
    if (file_size <= 0 || (offset != -1 && file_size < offset)) {
        php_error_docref(NULL, E_WARNING, "Failed to pop line from file: %s", filename);
        fclose(fp);
        RETURN_FALSE;
    }

    off_t pos = file_size;
    ssize_t bytesRead;

    if (offset != -1) {
        pos -= offset;
        fseek(fp, pos , SEEK_SET);

        // Увеличиваем размер буфера на 1 для возможного символа перевода строки
        char *buffer = (char *)emalloc(offset + 1); // +1 для '\0'
        if (!buffer) {
            php_error_docref(NULL, E_WARNING, "Out of memory");
            fclose(fp);
            RETURN_FALSE;
        }

        bytesRead = fread(buffer, 1, offset, fp);
        if(bytesRead != offset){
            efree(buffer);
            fclose(fp);
            php_error_docref(NULL, E_WARNING, "Failed to read to the file: %s", filename);
            RETURN_FALSE;
        }

        // Убедимся, что строка нуль-терминирована
        buffer[bytesRead] = '\0';

        if(mode < 1 || mode == 2){
            // Обрезка пробелов справа и символа перевода строки
            for (int i = bytesRead - 1; i >= 0; --i) {
                if(buffer[i] == ' ' || buffer[i] == '\n') buffer[i] = '\0';
                else break;
            }
        }

        if(mode < 2){
            // Усекаем файл
            if(ftruncate(fileno(fp), pos) < 0) {
                efree(buffer);
                fclose(fp);
                php_error_docref(NULL, E_WARNING, "Failed to truncate file: %s", filename);
                RETURN_FALSE;
            }
        }
        
        fclose(fp);

        // Возврат строки в PHP
        RETVAL_STRING(buffer);
        efree(buffer);
        return;
    }


    zend_long ini_buffer_size = FAST_IO_G(buffer_size);
    if(file_size < ini_buffer_size) ini_buffer_size = file_size;
    if(ini_buffer_size < 16) ini_buffer_size = 16;

    // Авто поиск последней строки
    char *buffer = (char *)emalloc(ini_buffer_size + 1);
    if (!buffer) {
        php_error_docref(NULL, E_WARNING, "Out of memory");
        fclose(fp);
        RETURN_FALSE;
    }

    zend_string *result_str = NULL;
    int found_line_start = 0;

    while (pos > 0 && !found_line_start) {
        pos -= ini_buffer_size;
        pos = pos < 0 ? 0 : pos;

        fseek(fp, pos, SEEK_SET);
        bytesRead = fread(buffer, 1, ini_buffer_size, fp);
        
        for (ssize_t i = bytesRead - 1; i >= 0; --i) {
            if (buffer[i] == '\n') {
                if (!result_str) { // Найден первый перенос строки с конца
                    result_str = zend_string_alloc(bytesRead - i - 1, 0);
                    if(result_str == NULL){
                        php_error_docref(NULL, E_WARNING, "Out of memory");
                        fclose(fp);
                        efree(buffer);
                        RETURN_FALSE;
                    }

                    memcpy(ZSTR_VAL(result_str), buffer + i + 1, bytesRead - i - 1);
                } else if (i != bytesRead - 1 || pos + bytesRead < file_size) { // Найдено начало строки
                    size_t new_len = ZSTR_LEN(result_str) + bytesRead - i - 1;
                    result_str = zend_string_extend(result_str, new_len, 0);
                    if(result_str == NULL){
                        php_error_docref(NULL, E_WARNING, "Out of memory");
                        fclose(fp);
                        efree(buffer);
                        RETURN_FALSE;
                    }
                    
                    memmove(ZSTR_VAL(result_str) + bytesRead - i - 1, ZSTR_VAL(result_str), ZSTR_LEN(result_str) - (bytesRead - i - 1));
                    memcpy(ZSTR_VAL(result_str), buffer + i + 1, bytesRead - i - 1);

                    found_line_start = 1;
                    break;
                }
            }
        }

        if (!found_line_start && bytesRead > 0 && result_str != NULL && pos == 0) {
            size_t result_str_len = ZSTR_LEN(result_str);

            if(result_str_len + 1 > ini_buffer_size && bytesRead == ini_buffer_size ){
                result_str = zend_string_extend(result_str, ZSTR_LEN(result_str) + ini_buffer_size, 0);
                if(result_str == NULL){
                    php_error_docref(NULL, E_WARNING, "Out of memory");
                    fclose(fp);
                    efree(buffer);
                    RETURN_FALSE;
                }

                memcpy(ZSTR_VAL(result_str), buffer, ini_buffer_size);

                ZSTR_LEN(result_str) = result_str_len - 1;                
            } else {
                result_str = zend_string_alloc(bytesRead, 0);
                if(result_str == NULL){
                    php_error_docref(NULL, E_WARNING, "Out of memory");
                    fclose(fp);
                    efree(buffer);
                    RETURN_FALSE;
                }
                
                memcpy(ZSTR_VAL(result_str), buffer, bytesRead);
            }

            found_line_start = 1;
            break;
        }

        if (!found_line_start && bytesRead == ini_buffer_size && result_str != NULL) { // Если строка начинается до текущего буфера
            result_str = zend_string_extend(result_str, ZSTR_LEN(result_str) + ini_buffer_size, 0);
            if(result_str == NULL){
                php_error_docref(NULL, E_WARNING, "Out of memory");
                fclose(fp);
                efree(buffer);
                RETURN_FALSE;
            }
            
            memmove(ZSTR_VAL(result_str) + ini_buffer_size, ZSTR_VAL(result_str), ZSTR_LEN(result_str) - ini_buffer_size);
            memcpy(ZSTR_VAL(result_str), buffer, ini_buffer_size);
        }
    }

    efree(buffer);

    if (found_line_start && result_str != NULL) {
        ZSTR_VAL(result_str)[ZSTR_LEN(result_str)] = '\0'; // Установить конечный нулевой символ

        // Обрезка пробелов справа и символа перевода строки
        size_t len = ZSTR_LEN(result_str);
        char *str = ZSTR_VAL(result_str);
        
        if(mode < 2){
            // Усечь файл
            off_t new_file_size = file_size - len;
            if(new_file_size < 0) new_file_size = 0;

            if(ftruncate(fileno(fp), new_file_size) < 0) {
                fclose(fp);
                php_error_docref(NULL, E_WARNING, "Failed to truncate file: %s", filename);
                RETURN_FALSE;
            }
        }

        fclose(fp);

        if(mode < 1 || mode == 2){
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
        }

        RETVAL_STR(result_str);
    } else {
        fclose(fp);
        RETVAL_FALSE;
    }
}




/* Реализация функции */
PHP_FUNCTION(file_erase_line) {
    char *filename, *line_key;
    size_t filename_len, line_key_len;
    zend_long position = 0;
    zend_long mode = 0;

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
        if (flock(fileno(fp), LOCK_EX) < 0) {
            php_error_docref(NULL, E_WARNING, "Failed to lock file: %s", filename);
            fclose(fp);
            RETURN_LONG(-2);
        }
    }

    if(mode > 99) mode -= 100;


    // Перемещение указателя в конец файла для получения его размера
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);

    off_t writeOffset = 0; // Смещение для записи обновленных данных

    if(position > 0){
        if(position > file_size){
            php_error_docref(NULL, E_WARNING, "Failed to seek file: %s", filename);
            fclose(fp);
            RETURN_LONG(-5);
        }

        fseek(fp, position, SEEK_SET);
        writeOffset = position;
    } else {
        fseek(fp, 0, SEEK_SET);
    }

    zend_long ini_buffer_size = FAST_IO_G(buffer_size);

    if(file_size < ini_buffer_size) ini_buffer_size = file_size;
    if(ini_buffer_size < 16) ini_buffer_size = 16;

    zend_long dynamic_buffer_size = ini_buffer_size;
    char *dynamic_buffer = (char *)emalloc(dynamic_buffer_size + 1);
    if (!dynamic_buffer) {
        php_error_docref(NULL, E_WARNING, "Out of memory");
        fclose(fp);
        RETURN_LONG(-8);
    }

    ssize_t bytesRead;
    ssize_t bytesWrite;

    size_t current_size = 0; // Текущий размер данных в динамическом буфере
    bool found_match = false;

    while ((bytesRead = fread(dynamic_buffer + current_size, 1, ini_buffer_size, fp)) > 0) {
        current_size += bytesRead;
               
        dynamic_buffer[current_size] = '\0';

        char *lineStart = dynamic_buffer;
        char *lineEnd;
        while ((lineEnd = strchr(lineStart, '\n')) != NULL) {
            ssize_t lineLength = lineEnd - lineStart + 1;
            *lineEnd = '\0';

            if (strstr(lineStart, line_key) != NULL) {
                // Найдено совпадение ключа, подготавливаем замену
                char *replacement = emalloc(lineLength);
                if (!replacement) {
                    php_error_docref(NULL, E_WARNING, "Out of memory");
                    efree(dynamic_buffer);
                    fclose(fp);
                    RETURN_LONG(-8);
                }

                memset(replacement, ' ', lineLength - 1); // Заполнение пробелами
                replacement[0] = SPECIAL_CHAR; // символ DEL
                
                // Перемещаемся к началу найденной строки и записываем замену
                fseek(fp, writeOffset , SEEK_SET);

                bytesWrite = fwrite(replacement, 1, lineLength - 1, fp);

                if (bytesWrite != lineLength - 1) {
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

        current_size -= (lineStart - dynamic_buffer);
        memmove(dynamic_buffer, lineStart, current_size);

        if (current_size + ini_buffer_size > dynamic_buffer_size) {
            dynamic_buffer_size += ini_buffer_size;

            char *temp_buffer = (char *)erealloc(dynamic_buffer, dynamic_buffer_size + 1);
            if (!temp_buffer) {
                php_error_docref(NULL, E_WARNING, "Out of memory");
                fclose(fp);
                if (dynamic_buffer) efree(dynamic_buffer);
                RETURN_LONG(-8);
            }
            dynamic_buffer = temp_buffer;
        }
        
    }

    efree(dynamic_buffer);
    fclose(fp);

    if(found_match) RETURN_LONG(writeOffset);
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
        if (flock(fileno(fp), LOCK_EX) < 0) {
            php_error_docref(NULL, E_WARNING, "Failed to lock file: %s", filename);
            fclose(fp);
            RETURN_FALSE;
        }
    }

    if(mode > 99) mode -= 100;

    // Перемещение указателя в конец файла для получения его размера
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);

    off_t searchOffset = 0; // Смещение для записи обновленных данных

    if(position > 0){
        if(position > file_size){
            php_error_docref(NULL, E_WARNING, "Failed to seek file: %s", filename);

            fclose(fp);
            RETURN_FALSE;
        }

        fseek(fp, position, SEEK_SET);
        searchOffset = position;
    } else {
        fseek(fp, 0, SEEK_SET);
    }

    zend_long ini_buffer_size = FAST_IO_G(buffer_size);

    if(file_size < ini_buffer_size) ini_buffer_size = file_size;
    if(ini_buffer_size < 16) ini_buffer_size = 16;

    zend_long dynamic_buffer_size = ini_buffer_size;
    char *dynamic_buffer = (char *)emalloc(dynamic_buffer_size + 1);
    if (!dynamic_buffer) {
        php_error_docref(NULL, E_WARNING, "Out of memory");
        fclose(fp);
        RETURN_LONG(-8);
    }

    ssize_t bytesRead;
    size_t current_size = 0; // Текущий размер данных в динамическом буфере
    bool found_match = false;

    zend_long add_count = 0;
    zend_long line_count = 0;
    array_init(return_value);


    while ((bytesRead = fread(dynamic_buffer + current_size, 1, ini_buffer_size, fp)) > 0) {
        current_size += bytesRead;
        
        dynamic_buffer[current_size] = '\0';

        char *lineStart = dynamic_buffer;
        char *lineEnd;
        while ((lineEnd = strchr(lineStart, '\n')) != NULL) {
            ssize_t lineLength = lineEnd - lineStart + 1;
            *lineEnd = '\0';

            line_count++;

            if(search_start < line_count){
                add_count++;
                zval line_arr;
                array_init(&line_arr);

                if(mode == 1) add_assoc_string(&line_arr, "line", lineStart);

                if(mode == 2 || mode == 5) {
                    // Обрезка пробелов справа и символа перевода строки
                    for (int i = lineLength - 2; i >= 0; --i) {
                        if(lineStart[i] == ' ' || lineStart[i] == '\n') lineStart[i] = '\0';
                        else break;
                    }

                    add_assoc_string(&line_arr, "trim_line", lineStart);
                    if(mode != 5) add_assoc_long(&line_arr, "trim_length", strlen(lineStart));

                    if(mode == 5){
                        add_next_index_string(return_value, lineStart);
                    }
                }

                char *spacePos = strchr(lineStart, ' ');
                if (spacePos) *spacePos = '\0';

                if(mode == 0) add_assoc_string(&line_arr, "key", lineStart);

                if(mode < 4){
                    add_assoc_long(&line_arr, "line_offset", searchOffset);
                    add_assoc_long(&line_arr, "line_length", lineLength);
                    add_assoc_long(&line_arr, "line_count", line_count);
                }

                if(mode < 5) {
                    if(mode == 4) {
                        add_next_index_string(return_value, lineStart);
                    } else {
                        add_next_index_zval(return_value, &line_arr);
                    }
                }
            }
            
            searchOffset += lineLength; // Обновляем смещение
            lineStart = lineEnd + 1;

            if(add_count >= search_limit){
                found_match = true;
                break;
            }
        }

        if (found_match) break;

        current_size -= (lineStart - dynamic_buffer);
        memmove(dynamic_buffer, lineStart, current_size);

        if (current_size + ini_buffer_size > dynamic_buffer_size) {
            dynamic_buffer_size += ini_buffer_size;

            char *temp_buffer = (char *)erealloc(dynamic_buffer, dynamic_buffer_size + 1);
            if (!temp_buffer) {
                php_error_docref(NULL, E_WARNING, "Out of memory");
                fclose(fp);
                if (dynamic_buffer) efree(dynamic_buffer);
                RETURN_LONG(-8);
            }
            dynamic_buffer = temp_buffer;
        }
    }

    efree(dynamic_buffer);
    fclose(fp);
    RETURN_ZVAL(return_value, 0, 1);
}



// Функция обновления пары ключ-значение
PHP_FUNCTION(file_replace_line) {
    char *filename;
    size_t filename_len;
    char *line_key;
    char *line;
    size_t line_len;
    size_t line_key_len;
    zend_long mode = 0;

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
    long file_size = ftell(data_fp);
    fseek(data_fp, 0, SEEK_SET);

    zend_long ini_buffer_size = FAST_IO_G(buffer_size);

    if(file_size < ini_buffer_size) ini_buffer_size = file_size;
    if(ini_buffer_size < 16) ini_buffer_size = 16;

    zend_long dynamic_buffer_size = ini_buffer_size;
    char *dynamic_buffer = (char *)emalloc(dynamic_buffer_size + 1);
    if (!dynamic_buffer) {
        php_error_docref(NULL, E_WARNING, "Out of memory");
        fclose(data_fp);
        fclose(temp_fp);
        unlink(temp_filename);
        RETURN_LONG(-8);
    }

    zend_long found_count = 0;
    ssize_t bytesRead;
    ssize_t bytesWrite;
    size_t current_size = 0;


    while ((bytesRead = fread(dynamic_buffer + current_size, 1, ini_buffer_size, data_fp)) > 0) {
        current_size += bytesRead;

        dynamic_buffer[current_size] = '\0';

        char *lineStart = dynamic_buffer;
        char *lineEnd;
        while ((lineEnd = strchr(lineStart, '\n')) != NULL) {
            ssize_t lineLength = lineEnd - lineStart + 1;
            *lineEnd = '\0';

            if (strstr(lineStart, line_key) != NULL) { 
                char *replacement = estrndup(line, line_len + 1);
                if(replacement == NULL){
                    php_error_docref(NULL, E_WARNING, "Out of memory");
                    fclose(data_fp);
                    fclose(temp_fp);
                    unlink(temp_filename);
                    efree(dynamic_buffer);
                    RETURN_LONG(-8);
                }

                replacement[line_len] = '\n';
                replacement[line_len + 1] = '\0';

                bytesWrite = fwrite(replacement, 1, line_len + 1, temp_fp);
                
                if (bytesWrite != line_len + 1) {
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
                *lineEnd = '\n';

                bytesWrite = fwrite(lineStart, 1, lineLength, temp_fp);

                if (bytesWrite != lineLength) {
                    php_error_docref(NULL, E_WARNING, "Failed to write to the file: %s", temp_filename);
                    fclose(data_fp);
                    fclose(temp_fp);
                    unlink(temp_filename);
                    efree(dynamic_buffer);
                    RETURN_LONG(-4);
                }
            }

            found_count++;
            lineStart = lineEnd + 1;
        }


        current_size -= (lineStart - dynamic_buffer);
        memmove(dynamic_buffer, lineStart, current_size);

        if (current_size + ini_buffer_size > dynamic_buffer_size) {
            dynamic_buffer_size += ini_buffer_size;

            char *temp_buffer = (char *)erealloc(dynamic_buffer, dynamic_buffer_size + 1);
            if (!temp_buffer) {
                php_error_docref(NULL, E_WARNING, "Out of memory");
                fclose(data_fp);
                fclose(temp_fp);
                unlink(temp_filename);
                if (dynamic_buffer) efree(dynamic_buffer);
                RETURN_LONG(-8);
            }
            dynamic_buffer = temp_buffer;
        }
        
    }


    if(mode == 0){
        fseek(data_fp, 0 , SEEK_SET);
        fseek(temp_fp, 0 , SEEK_SET);
        current_size = 0;

        while ((bytesRead = fread(dynamic_buffer, 1, sizeof(dynamic_buffer), temp_fp)) > 0) {
            bytesWrite = fwrite(dynamic_buffer, 1, bytesRead, data_fp); 
            if(bytesWrite != bytesRead){
                php_error_docref(NULL, E_WARNING, "Failed to write to the file: %s", filename);
                efree(dynamic_buffer);
                fclose(data_fp);
                fclose(temp_fp);

                rename(temp_filename, filename);
                RETURN_LONG(-4);
            }

            current_size += bytesWrite;
        }

        // Усекаем файл
        if (ftruncate(fileno(data_fp), current_size) < 0) {
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



PHP_FUNCTION(file_insert_line) {
    char *filename;
    size_t filename_len;
    char *line;
    size_t line_len;
    size_t line_length = 0;
    zend_long mode = 0;

    // Парсинг аргументов, переданных в функцию
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "ss|ll", &filename, &filename_len, &line, &line_len, &mode, &line_length) == FAILURE) {
        RETURN_FALSE;
    }

    FILE *fp = fopen(filename, "a"); // Открытие файла для добавления и чтения;
    if (!fp) {
        php_error_docref(NULL, E_WARNING, "Failed to open file: %s", filename);
        RETURN_LONG(-1);
    }

    if(mode < 100){
        // Блокировка файла для записи
        if (flock(fileno(fp), LOCK_EX) == -1) {
            php_error_docref(NULL, E_WARNING, "Failed to lock the file: %s", filename);
            fclose(fp);
            RETURN_LONG(-2);
        }
    }

    if(mode > 99) mode -= 100;

    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);

    if(line_length == 0) line_length = line_len + 1; 

    // Подготовка строки к записи с учетом выравнивания и перевода строки
    char *buffer = (char *)emalloc(line_length + 1); // +1 для '\0'
    if (!buffer) {
        php_error_docref(NULL, E_WARNING, "Out of memory");
        fclose(fp);
        RETURN_LONG(-8);
    }

    memset(buffer, ' ', line_length); // Заполнение пробелами
    // Копирование line в буфер с учетом выравнивания
    strncpy(buffer, line, line_length < line_len ? line_length : line_len);
    if(mode == 0 || mode == 2) buffer[line_length - 1] = '\n';
    buffer[line_length] = '\0'; // Нуль-терминатор

    // Запись в файл
    size_t written = fwrite(buffer, sizeof(char), line_length, fp);
    if (written != line_length) {
        php_error_docref(NULL, E_WARNING, "Failed to write to the file: %s", filename);

        if(ftruncate(fileno(fp), file_size) < 0) {
            zend_error(E_ERROR, "Failed to truncate to the file: %s", filename);
            efree(buffer);
            fclose(fp);
            exit(EXIT_FAILURE);
        }

        fclose(fp);
        efree(buffer);
        RETURN_LONG(-3);
    }

    fclose(fp); // Это также разблокирует файл
    efree(buffer);
    
    if(mode > 1) RETURN_LONG(file_size);
    if(mode < 2) RETURN_LONG(file_size / line_length);
}




PHP_FUNCTION(file_select_line) {
    char *filename;
    size_t filename_len;
    zend_long row;
    zend_long align;
    zend_long mode = 0;

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
        if (flock(fileno(fp), LOCK_EX) < 0) {
            php_error_docref(NULL, E_WARNING, "Failed to lock file: %s", filename);
            fclose(fp);
            RETURN_FALSE;
        }
    }

    if(mode > 99) mode -= 100;

    ssize_t bytesRead;
    off_t position;

    if(mode == 0 || mode == 2) position = row * align;
    if(mode == 1 || mode == 3) position = row;


    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);

    if(position > file_size || fseek(fp, position , SEEK_SET) < 0){
        php_error_docref(NULL, E_WARNING, "Failed to seek file: %s", filename);
        fclose(fp);
        RETURN_FALSE;
    }

    // Увеличиваем размер буфера на 1 для возможного символа перевода строки
    char *buffer = (char *)emalloc(align + 1); // +1 для '\0' и +1 для '\n'
    if (!buffer) {
        php_error_docref(NULL, E_WARNING, "Out of memory");
        fclose(fp);
        RETURN_FALSE;
    }

    bytesRead = fread(buffer, 1, align, fp);
    if(bytesRead != align){
        php_error_docref(NULL, E_WARNING, "Failed to read to the file: %s", filename);
        fclose(fp);
        RETURN_FALSE;
    }
    fclose(fp);

    // Убедимся, что строка нуль-терминирована
    buffer[bytesRead] = '\0';

    if(mode == 0 || mode == 1){
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
    }

    // Возврат строки в PHP
    RETVAL_STRING(buffer);
    efree(buffer);
}



PHP_FUNCTION(file_update_line) { 
    char *filename;
    size_t filename_len;
    char *line;
    size_t line_len;
    zend_long position, line_length;
    zend_long mode = 0;

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
        if (flock(fileno(fp), LOCK_EX) < 0) {
            php_error_docref(NULL, E_WARNING, "Failed to lock file: %s", filename);
            fclose(fp);
            RETURN_LONG(-2);
        }
    }

    if(mode > 99) mode -= 100;

    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);

    if (fseek(fp, position, SEEK_SET) != 0 || position > file_size) {
        php_error_docref(NULL, E_WARNING, "Failed to seek in the file: %s", filename);
        fclose(fp);
        RETURN_LONG(-3);
    }

    // Подготовка строки к записи с учетом выравнивания и перевода строки
    char *buffer = (char *)emalloc(line_length + 1); // +1 для '\0'
    if (!buffer) {
        php_error_docref(NULL, E_WARNING, "Out of memory");
        fclose(fp);
        RETURN_LONG(-8);
    }


    memset(buffer, ' ', line_length); // Заполнение пробелами

    // Копирование line в буфер с учетом выравнивания
    strncpy(buffer, line, line_len < line_length ? line_len : line_length);

    if(mode == 0) buffer[line_length - 1] = '\n';
    buffer[line_length] = '\0'; // Нуль-терминатор

    // Запись в файл
    size_t written = fwrite(buffer, 1, line_length, fp);
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
    zend_long mode = 0;

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
        if (flock(fileno(fp), LOCK_EX) < 0) {
            php_error_docref(NULL, E_WARNING, "Failed to lock file: %s", filename);
            fclose(fp);
            RETURN_LONG(-2);
        }
    }

    if(mode > 99) mode -= 100;

    // Перемещение указателя в конец файла для получения его размера
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);

    char last_symbol;
    fseek(fp, file_size - 1, SEEK_SET);
    last_symbol = fgetc(fp);

    fseek(fp, 0, SEEK_SET);

    zend_long ini_buffer_size = FAST_IO_G(buffer_size);

    if(file_size < ini_buffer_size) ini_buffer_size = file_size;
    if(ini_buffer_size < 16) ini_buffer_size = 16;

    char *buffer = (char *)emalloc(ini_buffer_size);
    if (!buffer) {
        php_error_docref(NULL, E_WARNING, "Out of memory");
        fclose(fp);
        RETURN_LONG(-8);
    }

    ssize_t bytes_read;
    size_t max_length = 0, current_length = 0;
    double avg_length = 0.0;

    size_t min_length = SIZE_MAX;
    size_t line_count = 0;
    size_t total_characters = 0;
    zend_long flow_interruption = 0;

    array_init(return_value);

    while ((bytes_read = fread(buffer, 1, ini_buffer_size, fp)) > 0) {
        for (ssize_t i = 0; i < bytes_read; ++i) {
            if (buffer[i] == '\n') { // Конец текущей строки
                line_count++;

                if (current_length > max_length) {
                    max_length = current_length + 1; // Обновляем максимальную длину
                }

                if(current_length < min_length){
                    min_length = current_length + 1;
                }

                total_characters += current_length + 1;
                avg_length = total_characters / line_count; // Вычисляем среднюю длину

                if(mode == 1) { // Возвращаем длину первой строки.
                    efree(buffer);
                    fclose(fp);

                    add_assoc_long(return_value, "min_length", min_length);
                    add_assoc_long(return_value, "max_length", max_length);
                    add_assoc_double(return_value, "avg_length", avg_length);
                    add_assoc_long(return_value, "line_count", line_count);
                    add_assoc_long(return_value, "total_characters", total_characters);
                    add_assoc_long(return_value, "last_symbol", last_symbol);
                    add_assoc_long(return_value, "file_size", file_size);
                    return;
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
    add_assoc_long(return_value, "max_length", max_length);
    add_assoc_double(return_value, "avg_length", avg_length);
    add_assoc_long(return_value, "line_count", line_count);
    add_assoc_long(return_value, "total_characters", total_characters);

    if(file_size > total_characters) flow_interruption = file_size - total_characters;
    add_assoc_long(return_value, "flow_interruption", flow_interruption);
    add_assoc_long(return_value, "last_symbol", last_symbol);
    add_assoc_long(return_value, "file_size", file_size);
}



PHP_FUNCTION(find_matches_pcre2) {
    char *pattern;
    size_t pattern_len;
    char *subject;
    size_t subject_len;
    zend_long mode = 0;

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
                add_assoc_long(&match_arr, "match_offset", start_offset);
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
    } else if (rc < 0) {
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
    zend_long mode = 0;

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
    long file_size = ftell(source_fp);
    fseek(source_fp, 0, SEEK_SET);

    zend_long ini_buffer_size = FAST_IO_G(buffer_size);

    if(file_size < ini_buffer_size) ini_buffer_size = file_size;
    if(ini_buffer_size < 16) ini_buffer_size = 16;

    zend_long dynamic_buffer_size = ini_buffer_size;
    char *dynamic_buffer = (char *)emalloc(dynamic_buffer_size + 1);
    if (!dynamic_buffer) {
        php_error_docref(NULL, E_WARNING, "Out of memory");
        fclose(index_source_fp);
        fclose(index_destination_fp);
        fclose(source_fp);
        fclose(destination_fp);
        unlink(destination);
        if(mode == 1) unlink(index_destination);
        RETURN_LONG(-8);
    }


    size_t current_size = 0;
    ssize_t bytesRead;
    ssize_t bytesWrite;

    while ((bytesRead = fread(dynamic_buffer, 1, sizeof(dynamic_buffer), source_fp)) > 0) {
        bytesWrite = fwrite(dynamic_buffer, 1, bytesRead, destination_fp);

        if(bytesRead != bytesWrite) { 
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

        current_size += bytesWrite;
    }

    if(mode == 1){
        while ((bytesRead = fread(dynamic_buffer, 1, sizeof(dynamic_buffer), index_source_fp)) > 0) {
            bytesWrite = fwrite(dynamic_buffer, 1, bytesRead, index_destination_fp);

            if(bytesRead != bytesWrite) { 
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

            current_size += bytesWrite;
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
    char *pattern;
    size_t pattern_len;
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
        if (flock(fileno(fp), LOCK_EX) < 0) {
            php_error_docref(NULL, E_WARNING, "Failed to lock file: %s", filename);
            fclose(fp);
            RETURN_FALSE;
        }
    }

    if(mode > 99) mode -= 100;

    // Перемещение указателя в конец файла для получения его размера
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);

    array_init(return_value);
    zval *elem, *value;
    ssize_t bytesRead;
    bool found_match = false;

    zend_long found_count = 0;
    zend_long line_count = 0;

    pcre2_code *re;
    pcre2_match_data *match_data; 

    array_init(return_value);


    if(mode > 9){
        PCRE2_SIZE erroffset;
        int errorcode;
        re = pcre2_compile((PCRE2_SPTR)pattern, PCRE2_ZERO_TERMINATED, 0, &errorcode, &erroffset, NULL);

        if (re == NULL) {
            PCRE2_UCHAR message[256];
            pcre2_get_error_message(errorcode, message, sizeof(message));
            php_error_docref(NULL, E_WARNING, "PCRE2 compilation failed at offset %d: %s", (int)erroffset, message);
            fclose(fp);
            RETURN_FALSE;
        }

        match_data = pcre2_match_data_create_from_pattern(re, NULL);
    }


    // Если массив был передан, обходим его
    if (array) {
        ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(array), elem) {
            if (Z_TYPE_P(elem) == IS_ARRAY) {
                int num_elem = 0;
                zend_long select_pos = -1; // Позиция
                zend_long select_size = -1;

                ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(elem), value) {
                    if (Z_TYPE_P(value) == IS_LONG) {
                        if(num_elem == 0) select_pos = Z_LVAL_P(value);
                        if(num_elem == 1) select_size = Z_LVAL_P(value);
                        num_elem++;
                    }
                } ZEND_HASH_FOREACH_END();

                if(select_pos != -1 && select_size != -1 && file_size > select_pos + select_size){
                    line_count++;
                    found_match = false;

                    char *buffer = (char *)emalloc(select_size + 1);
                    if (!buffer) {
                        php_error_docref(NULL, E_WARNING, "Out of memory");
                        fclose(fp);
                        if (re != NULL) pcre2_code_free(re);
                        if (mode > 19 && match_data != NULL) pcre2_match_data_free(match_data);
                        RETURN_FALSE;
                    }

                    fseek(fp, select_pos, SEEK_SET);

                    bytesRead = fread(buffer, 1, select_size, fp);
                    if(bytesRead != select_size){
                        php_error_docref(NULL, E_WARNING, "Failed to read to the file: %s", filename);
                        fclose(fp);
                        efree(buffer);
                        if (re != NULL) pcre2_code_free(re);
                        if (mode > 19 && match_data != NULL) pcre2_match_data_free(match_data);
                        RETURN_FALSE;
                    }

                    buffer[bytesRead] = '\0';

                    zval line_arr;
                    array_init(&line_arr);


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
                        for (int i = bytesRead - 1; i >= 0; --i) {
                            if(buffer[i] == ' ' || buffer[i] == '\n') buffer[i] = '\0';
                            else break;
                        }
                    }

                    if(mode == 0) {
                        add_assoc_string(&line_arr, "trim_line", buffer);
                        add_assoc_long(&line_arr, "trim_length", strlen(buffer));
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

                    if(mode == 3 && strstr(buffer, pattern) != NULL) {
                        found_count++;
                        found_match = false;
                    }

                    if(
                        mode == 5 &&
                        strstr(buffer, pattern) != NULL
                    ){
                        add_assoc_string(&line_arr, "trim_line", buffer);
                        add_assoc_long(&line_arr, "trim_length", strlen(buffer));
                        found_match = true;
                    }

                    if(mode == 6) {
                        add_assoc_string(&line_arr, "line", buffer);
                        found_match = true;
                    }

                    if(
                        mode == 7 &&
                        strstr(buffer, pattern) != NULL
                    ){
                        add_next_index_string(return_value, buffer);
                        found_match = false;
                    }

                    if(mode == 0 || mode == 5){
                        add_assoc_long(&line_arr, "line_offset", select_pos);
                        add_assoc_long(&line_arr, "line_length", select_size);
                    }
                    
                    if(mode > 9 && mode < 14){
                        int rc;
                        size_t start_offset = 0;
                    
                        if(rc = pcre2_match(re, (PCRE2_SPTR)buffer, select_size, start_offset, 0, match_data, NULL) > 0){
                            if(mode < 13){
                                if(mode == 10){
                                    add_assoc_string(&line_arr, "trim_line", buffer);
                                    add_assoc_long(&line_arr, "trim_length", strlen(buffer));
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

                    if(mode > 19 && mode < 24) {
                        zval return_matched;
                        array_init(&return_matched);

                        int rc;
                        PCRE2_SIZE *ovector;
                        size_t start_offset = 0;
                        found_match = false;

                        while ((rc = pcre2_match(re, (PCRE2_SPTR)buffer, select_size, start_offset, 0, match_data, NULL)) > 0) {
                            ovector = pcre2_get_ovector_pointer(match_data);

                            for (int i = 0; i < rc; i++) {
                                PCRE2_SIZE start = ovector[2*i];
                                PCRE2_SIZE end = ovector[2*i+1];

                                if(mode == 23 || mode == 20){
                                    add_next_index_stringl(&return_matched, buffer + start, end - start);
                                } else {
                                    zval match_arr;
                                    array_init(&match_arr);

                                    add_assoc_stringl(&match_arr, "line_match", buffer + start, end - start);
                                    add_next_index_zval(&return_matched, &match_arr);

                                    
                                    add_assoc_long(&match_arr, "match_offset", start_offset);
                                    add_assoc_long(&match_arr, "match_length", end - start);
                                }
                            }

                            // Изменение для предотвращения потенциального бесконечного цикла
                            if (ovector[1] > start_offset) {
                                start_offset = ovector[1];
                            } else {
                                start_offset++; // Для продолжения поиска следующего совпадения
                                if (start_offset >= bytesRead) break; // Выходим из цикла, если достигнут конец строки
                            }

                            found_match = true;
                        }

                        if (rc == PCRE2_ERROR_NOMATCH) {
                            /* Если совпадений нет, возвращаем пустой массив. */
                            //
                        } else if (rc < 0) {
                            /* Обработка других ошибок. */
                            php_error_docref(NULL, E_WARNING, "Matching error %d", rc);
                            fclose(fp);
                            efree(buffer);
                            if (re != NULL) pcre2_code_free(re);
                            if (match_data != NULL) pcre2_match_data_free(match_data);
                            RETURN_FALSE;
                        }

                        if(found_match){
                            if(mode == 20) {
                                add_assoc_string(&line_arr, "trim_line", buffer);
                                add_assoc_long(&line_arr, "trim_length", strlen(buffer));
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
                        }
                    }

                    if(found_match){
                        add_next_index_zval(return_value, &line_arr);
                    }

                    efree(buffer);
                }
            }
        } ZEND_HASH_FOREACH_END();
    }

    fclose(fp);
    if (mode > 9 && re != NULL) pcre2_code_free(re);
    if (mode > 9 && match_data != NULL) pcre2_match_data_free(match_data);


    if(mode == 3 || mode == 13){      
        add_assoc_long(return_value, "line_count", line_count);
        add_assoc_long(return_value, "found_count", found_count);
    }

    // Если массив пуст, возвращаем FALSE
    if (Z_TYPE_P(return_value) == IS_ARRAY && zend_hash_num_elements(Z_ARRVAL_P(return_value)) == 0) {
        zval_ptr_dtor(return_value);
        RETURN_FALSE;
    }
}



PHP_FUNCTION(file_update_array) {
    char *filename;
    size_t filename_len;
    
    zval  *array = NULL;
    zend_long mode = 0;

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
        if (flock(fileno(fp), LOCK_EX) < 0) {
            php_error_docref(NULL, E_WARNING, "Failed to lock file: %s", filename);
            fclose(fp);
            RETURN_LONG(-2);
        }
    }

    if(mode > 99) mode -= 100;

    // Перемещение указателя в конец файла для получения его размера
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);

    ssize_t bytesWrite;
    char *buffer = NULL;
    
    size_t written = 0;
    size_t len = 0;

    zval *elem, *value;


    // Если массив был передан, обходим его
    if (array) {
        ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(array), elem) {
            if (Z_TYPE_P(elem) == IS_ARRAY) {
                int num_elem = 0;
                len = 0;
                zend_long update_pos = -1; // Позиция
                zend_long update_size = -1;
                char *found_value = NULL;

                ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(elem), value) {
                    if(Z_TYPE_P(value) == IS_STRING && num_elem == 0) found_value = Z_STRVAL_P(value);
                    if(Z_TYPE_P(value) == IS_LONG && num_elem == 1) update_pos = Z_LVAL_P(value);
                    if(Z_TYPE_P(value) == IS_LONG && num_elem == 2) update_size = Z_LVAL_P(value);
                    num_elem++;
                } ZEND_HASH_FOREACH_END();

                if(update_pos != -1 && update_size != -1 && file_size > update_pos && update_size > 0 && found_value != NULL){
                    len = strlen(found_value);
                    buffer = (char *)emalloc(update_size + 1); // +1 для '\0'

                    if (!buffer) {
                        php_error_docref(NULL, E_WARNING, "Out of memory");
                        fclose(fp);
                        RETURN_LONG(-8);
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
                    bytesWrite = fwrite(buffer, 1, update_size, fp);
                    
                    if (bytesWrite != update_size) {
                        php_error_docref(NULL, E_WARNING, "Failed to write to the file: %s", filename);
                        efree(buffer);
                        RETURN_LONG(-4);
                    }

                    written += bytesWrite;
                    efree(buffer);
                }
            }
        } ZEND_HASH_FOREACH_END();
    }

    fclose(fp);
    RETURN_LONG(written);
}


