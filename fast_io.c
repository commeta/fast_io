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



PHP_FUNCTION(file_search_array)
{
    char *filename = NULL, *line_key = NULL;
    size_t filename_len = 0, line_key_len = 0;
    size_t search_start = 0, search_limit = 1, mode = 0;
    ssize_t position = 0;

    FILE *fp = NULL;
    char *dynamic_buffer = NULL;
    ssize_t ini_buffer_size, dynamic_buffer_size;
    ssize_t bytes_read, current_size = 0, file_size, search_offset = 0;
    size_t found_count = 0, add_count = 0, line_count = 0;
    bool found_match = false;

    pcre2_code *re = NULL;
    pcre2_match_data *match_data = NULL;

    // Проверка длины имени файла для предотвращения path-injection
    if (filename_len > PATH_MAX - 1) {
        php_error_docref(NULL, E_WARNING, "Filename too long");
        RETURN_FALSE;
    }

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "ss|llll",
                              &filename, &filename_len,
                              &line_key, &line_key_len,
                              &search_start, &search_limit, &position, &mode) == FAILURE) {
        RETURN_FALSE;
    }

    fp = fopen(filename, "r");
    if (!fp) {
        php_error_docref(NULL, E_WARNING, "Failed to open file: %s", filename);
        RETURN_FALSE;
    }

    if (mode < 100) {
        if (flock(fileno(fp), LOCK_EX) == -1) {
            php_error_docref(NULL, E_WARNING, "Failed to lock file: %s", filename);
            fclose(fp);
            RETURN_FALSE;
        }
    }
    if (mode > 99)
        mode -= 100;

    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    if (position > 0) {
        if (position >= file_size) {
            php_error_docref(NULL, E_WARNING, "Invalid position in file: %s", filename);
            fclose(fp);
            RETURN_FALSE;
        }
        fseek(fp, position, SEEK_SET);
        search_offset = position;
    } else {
        fseek(fp, 0, SEEK_SET);
    }

    ini_buffer_size = FAST_IO_G(buffer_size);
    if (file_size < ini_buffer_size)
        ini_buffer_size = file_size;
    if (ini_buffer_size < 16)
        ini_buffer_size = 16;
    dynamic_buffer_size = ini_buffer_size;
    dynamic_buffer = (char *)emalloc(dynamic_buffer_size + 1);
    if (!dynamic_buffer) {
        fclose(fp);
        zend_error(E_ERROR, "Out of memory to allocate %ld bytes", dynamic_buffer_size + 1);
    }

    array_init(return_value);

    if (mode > 9) {
        PCRE2_SIZE erroffset;
        int errorcode;
        re = pcre2_compile((PCRE2_SPTR)line_key, PCRE2_ZERO_TERMINATED,
                           0, &errorcode, &erroffset, NULL);
        if (!re) {
            PCRE2_UCHAR message[256];
            pcre2_get_error_message(errorcode, message, sizeof(message));
            php_error_docref(NULL, E_WARNING, "PCRE2 compilation failed at offset %d: %s", (int)erroffset, message);
            fclose(fp);
            efree(dynamic_buffer);
            RETURN_FALSE;
        }
        match_data = pcre2_match_data_create_from_pattern(re, NULL);
        if (!match_data) {
            pcre2_code_free(re);
            fclose(fp);
            efree(dynamic_buffer);
            zend_error(E_ERROR, "Out of memory to allocate PCRE2 match data");
        }
    }

    while ((bytes_read = fread(dynamic_buffer + current_size, 1, ini_buffer_size, fp)) > 0) {
        current_size += bytes_read;
        dynamic_buffer[current_size] = '\0';

        char *line_start = dynamic_buffer;
        char *line_end;
        while ((line_end = strchr(line_start, '\n')) != NULL) {
            ssize_t line_length = line_end - line_start + 1;
            *line_end = '\0'; // Terminate the current line

            // Substring search for modes 0-3
            if (mode < 3 && strstr(line_start, line_key)) {
                found_count++;
                if (search_start < found_count) {
                    add_count++;
                    zval line_arr;
                    array_init(&line_arr);
                    ssize_t i = line_length - 2;
                    if (mode == 0 || mode == 2) {
                        for (; i >= 0; --i) {
                            if (line_start[i] == ' ' || line_start[i] == '\n')
                                line_start[i] = '\0';
                            else
                                break;
                        }
                        add_assoc_string(&line_arr, "trim_line", line_start);
                    }
                    if (mode == 0)
                        add_assoc_long(&line_arr, "trim_length", i + 1);
                    if (mode == 1)
                        add_assoc_string(&line_arr, "line", line_start);
                    if (mode != 2) {
                        add_assoc_long(&line_arr, "line_offset", search_offset);
                        add_assoc_long(&line_arr, "line_length", line_length);
                        add_assoc_long(&line_arr, "line_count", line_count);
                    }
                    if (mode == 2)
                        add_next_index_string(return_value, line_start);
                    else
                        add_next_index_zval(return_value, &line_arr);
                }
            }
            if (mode == 3 && strstr(line_start, line_key))
                found_count++;

            // Regex search for modes 10-12
            if (mode > 9 && mode < 13 &&
                pcre2_match(re, (PCRE2_SPTR)line_start, line_length - 1, 0, 0, match_data, NULL) > 0) {
                found_count++;
                if (search_start < found_count) {
                    add_count++;
                    zval line_arr;
                    array_init(&line_arr);
                    ssize_t i = line_length - 2;
                    if (mode == 10 || mode == 12) {
                        for (; i >= 0; --i) {
                            if (line_start[i] == ' ' || line_start[i] == '\n')
                                line_start[i] = '\0';
                            else
                                break;
                        }
                    }
                    if (mode == 10) {
                        add_assoc_string(&line_arr, "trim_line", line_start);
                        add_assoc_long(&line_arr, "trim_length", i + 1);
                    } else {
                        add_assoc_string(&line_arr, "line", line_start);
                    }
                    if (mode != 12) {
                        add_assoc_long(&line_arr, "line_offset", search_offset);
                        add_assoc_long(&line_arr, "line_length", line_length);
                        add_assoc_long(&line_arr, "line_count", line_count);
                    }
                    if (mode == 12)
                        add_next_index_string(return_value, line_start);
                    else
                        add_next_index_zval(return_value, &line_arr);
                }
            }
            if (mode == 13 &&
                pcre2_match(re, (PCRE2_SPTR)line_start, line_length, 0, 0, match_data, NULL) > 0)
                found_count++;

            // Regex extraction for modes 20-23
            if (mode > 19 && mode < 25 && re && match_data) {
                if (search_start < found_count + 1) {
                    zval return_matched;
                    array_init(&return_matched);
                    int rc;
                    PCRE2_SIZE *ovector;
                    size_t start_offset = 0;
                    bool is_matched = false;
                    while ((rc = pcre2_match(re, (PCRE2_SPTR)line_start, line_length, start_offset, 0, match_data, NULL)) > 0) {
                        ovector = pcre2_get_ovector_pointer(match_data);
                        for (int j = 0; j < rc; j++) {
                            PCRE2_SIZE start = ovector[2 * j];
                            PCRE2_SIZE end = ovector[2 * j + 1];
                            if (start > end || end > (PCRE2_SIZE)line_length)
                                continue;
                            if (mode > 21) {
                                zval match_arr;
                                array_init(&match_arr);
                                if (mode == 23)
                                    add_next_index_stringl(&return_matched, line_start + start, end - start);
                                else {
                                    add_assoc_stringl(&match_arr, "line_match", line_start + start, end - start);
                                    add_assoc_long(&match_arr, "match_offset", start);
                                    add_assoc_long(&match_arr, "match_length", end - start);
                                    add_next_index_zval(&return_matched, &match_arr);
                                }
                            } else {
                                add_next_index_stringl(&return_matched, line_start + start, end - start);
                            }
                        }
                        if (ovector[1] > start_offset)
                            start_offset = ovector[1];
                        else {
                            start_offset++;
                            if ((ssize_t)start_offset >= line_length)
                                break;
                        }
                        is_matched = true;
                    }
                    if (rc < 0 && rc != PCRE2_ERROR_NOMATCH) {
                        php_error_docref(NULL, E_WARNING, "Matching error %d", rc);
                        fclose(fp);
                        efree(dynamic_buffer);
                        if (re) pcre2_code_free(re);
                        if (match_data) pcre2_match_data_free(match_data);
                        RETURN_FALSE;
                    }
                    if (is_matched) {
                        add_count++;
                        zval line_arr;
                        array_init(&line_arr);
                        if (mode == 20) {
                            ssize_t i = line_length - 2;
                            for (; i >= 0; --i) {
                                if (line_start[i] == ' ' || line_start[i] == '\n')
                                    line_start[i] = '\0';
                                else
                                    break;
                            }
                            add_assoc_string(&line_arr, "trim_line", line_start);
                            add_assoc_long(&line_arr, "trim_length", i + 1);
                        }
                        if (mode == 21)
                            add_assoc_string(&line_arr, "line", line_start);
                        if (mode != 23) {
                            add_assoc_zval(&line_arr, "line_matches", &return_matched);
                            add_assoc_long(&line_arr, "line_offset", search_offset);
                            add_assoc_long(&line_arr, "line_length", line_length);
                            add_assoc_long(&line_arr, "line_count", line_count);
                        }
                        if (mode == 23)
                            add_next_index_zval(return_value, &return_matched);
                        else
                            add_next_index_zval(return_value, &line_arr);
                    } else {
                        zval_ptr_dtor(&return_matched);
                    }
                    found_count++;
                }
            }

            search_offset += line_length;
            line_start = line_end + 1;
            line_count++;
            if (add_count >= search_limit) {
                found_match = true;
                break;
            }
        }
        if (found_match)
            break;

        current_size -= (line_start - dynamic_buffer);
        memmove(dynamic_buffer, line_start, current_size);

        if (current_size + ini_buffer_size > dynamic_buffer_size) {
            dynamic_buffer_size += ini_buffer_size;
            char *temp_buffer = (char *)erealloc(dynamic_buffer, dynamic_buffer_size + 1);
            if (!temp_buffer) {
                fclose(fp);
                efree(dynamic_buffer);
                if (re) pcre2_code_free(re);
                if (mode > 9 && match_data) pcre2_match_data_free(match_data);
                zend_error(E_ERROR, "Out of memory to allocate %ld bytes", dynamic_buffer_size + 1);
            }
            dynamic_buffer = temp_buffer;
        }
    }

    fclose(fp);
    efree(dynamic_buffer);

    if (mode == 3 || mode == 13) {
        add_assoc_long(return_value, "line_count", line_count);
        add_assoc_long(return_value, "found_count", found_count);
    }

    if (mode > 9 && re)
        pcre2_code_free(re);
    if (mode > 9 && match_data)
        pcre2_match_data_free(match_data);

    if (Z_TYPE_P(return_value) == IS_ARRAY && zend_hash_num_elements(Z_ARRVAL_P(return_value)) == 0) {
        zval_ptr_dtor(return_value);
        RETURN_FALSE;
    }

    RETURN_ZVAL(return_value, 0, 1);
}
// path-injection Unvalidated input in path value creation risks unintended file/directory access



PHP_FUNCTION(file_search_line)
{
    char *filename = NULL, *line_key = NULL;
    size_t filename_len = 0, line_key_len = 0;
    size_t mode = 0; /* 0: substring search; 1,11: trim result; 10: regex search */
    ssize_t position = 0;

    FILE *fp = NULL;
    char *dynamic_buffer = NULL;
    ssize_t ini_buffer_size, dynamic_buffer_size, bytes_read, current_size = 0;
    ssize_t file_size;
    char *found_value = NULL;
    ssize_t line_length = 0;

    pcre2_code *re = NULL;
    pcre2_match_data *match_data = NULL;

    // Проверка длины имени файла для предотвращения path-injection
    if (filename_len > PATH_MAX - 1) {
        php_error_docref(NULL, E_WARNING, "Filename too long");
        RETURN_FALSE;
    }

    // Парсинг аргументов: filename, line_key, [position, mode]
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "ss|ll",
                              &filename, &filename_len,
                              &line_key, &line_key_len,
                              &position, &mode) == FAILURE) {
        RETURN_FALSE;
    }

    fp = fopen(filename, "r");
    if (!fp) {
        php_error_docref(NULL, E_WARNING, "Failed to open file: %s", filename);
        RETURN_FALSE;
    }

    // Блокировка файла если не Log mode
    if (mode < 100) {
        if (flock(fileno(fp), LOCK_EX) == -1) {
            php_error_docref(NULL, E_WARNING, "Failed to lock file: %s", filename);
            fclose(fp);
            RETURN_FALSE;
        }
    }
    if (mode > 99)
        mode -= 100;

    // Установка позиции
    if (fseek(fp, 0, SEEK_END) != 0) {
        php_error_docref(NULL, E_WARNING, "Failed to seek in file: %s", filename);
        fclose(fp);
        RETURN_FALSE;
    }
    file_size = ftell(fp);
    if (position > 0) {
        if (position >= file_size) {
            php_error_docref(NULL, E_WARNING, "Invalid position in file: %s", filename);
            fclose(fp);
            RETURN_FALSE;
        }
        if (fseek(fp, position, SEEK_SET) != 0) {
            php_error_docref(NULL, E_WARNING, "Failed to set file position: %s", filename);
            fclose(fp);
            RETURN_FALSE;
        }
    } else {
        fseek(fp, 0, SEEK_SET);
    }

    ini_buffer_size = FAST_IO_G(buffer_size);
    if (file_size < ini_buffer_size)
        ini_buffer_size = file_size;
    if (ini_buffer_size < 16)
        ini_buffer_size = 16;
    dynamic_buffer_size = ini_buffer_size;

    dynamic_buffer = (char *)emalloc(dynamic_buffer_size + 1);
    if (!dynamic_buffer) {
        fclose(fp);
        zend_error(E_ERROR, "Out of memory to allocate %ld bytes", dynamic_buffer_size + 1);
    }

    // Если mode > 9, компилируем регулярное выражение через PCRE2
    if (mode > 9) {
        PCRE2_SIZE erroffset;
        int errorcode;
        re = pcre2_compile((PCRE2_SPTR)line_key, PCRE2_ZERO_TERMINATED, 0, &errorcode, &erroffset, NULL);
        if (!re) {
            PCRE2_UCHAR message[256];
            pcre2_get_error_message(errorcode, message, sizeof(message));
            php_error_docref(NULL, E_WARNING, "PCRE2 compilation failed at offset %d: %s", (int)erroffset, message);
            fclose(fp);
            efree(dynamic_buffer);
            RETURN_FALSE;
        }
        match_data = pcre2_match_data_create_from_pattern(re, NULL);
        if (!match_data) {
            pcre2_code_free(re);
            fclose(fp);
            efree(dynamic_buffer);
            zend_error(E_ERROR, "Out of memory to allocate PCRE2 match data");
        }
    }

    // Чтение файла блоками, обработка каждой строки
    while ((bytes_read = fread(dynamic_buffer + current_size, 1, ini_buffer_size, fp)) > 0) {
        current_size += bytes_read;
        dynamic_buffer[current_size] = '\0';

        char *line_start = dynamic_buffer;
        char *line_end;
        while ((line_end = strchr(line_start, '\n')) != NULL) {
            line_length = line_end - line_start + 1;
            *line_end = '\0';  // Завершаем текущую строку

            // Поиск подстроки (mode 0)
            if (mode == 0 && strstr(line_start, line_key) != NULL) {
                found_value = estrndup(line_start, line_length - 1);
                if (!found_value) {
                    if (re) pcre2_code_free(re);
                    if (match_data) pcre2_match_data_free(match_data);
                    efree(dynamic_buffer);
                    fclose(fp);
                    zend_error(E_ERROR, "Out of memory to allocate %ld bytes", line_length - 1);
                }
                goto found;
            }
            // Поиск по регулярному выражению (mode 10)
            if (mode == 10 && re && match_data &&
                pcre2_match(re, (PCRE2_SPTR)line_start, line_length - 1, 0, 0, match_data, NULL) > 0) {
                found_value = estrndup(line_start, line_length - 1);
                if (!found_value) {
                    pcre2_code_free(re);
                    pcre2_match_data_free(match_data);
                    efree(dynamic_buffer);
                    fclose(fp);
                    zend_error(E_ERROR, "Out of memory to allocate %ld bytes", line_length - 1);
                }
                goto found;
            }
            line_start = line_end + 1;
        }
        // Переносим незавершённую строку в начало буфера
        current_size -= (line_start - dynamic_buffer);
        memmove(dynamic_buffer, line_start, current_size);

        // Расширяем буфер при необходимости
        if (current_size + ini_buffer_size > dynamic_buffer_size) {
            dynamic_buffer_size += ini_buffer_size;
            char *temp_buffer = (char *)erealloc(dynamic_buffer, dynamic_buffer_size + 1);
            if (!temp_buffer) {
                if (re) pcre2_code_free(re);
                if (match_data) pcre2_match_data_free(match_data);
                efree(dynamic_buffer);
                fclose(fp);
                zend_error(E_ERROR, "Out of memory to allocate %ld bytes", dynamic_buffer_size + 1);
            }
            dynamic_buffer = temp_buffer;
        }
    }

found:
    fclose(fp);
    efree(dynamic_buffer);

    if (re)
        pcre2_code_free(re);
    if (match_data)
        pcre2_match_data_free(match_data);

    if (!found_value) {
        RETURN_FALSE;
    }

    // Обрезка пробелов и перевода строки справа, если mode == 1 или 11
    if (mode == 1 || mode == 11) {
        ssize_t i;
        for (i = line_length - 2; i >= 0; --i) {
            if (found_value[i] == ' ' || found_value[i] == '\n') {
                found_value[i] = '\0';
            } else {
                break;
            }
        }
    }
    RETURN_STRING(found_value);
}


PHP_FUNCTION(file_search_data) {
    char *filename, *line_key;
    size_t filename_len, line_key_len;
    ssize_t position = 0;
    size_t mode = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "ss|ll",
                              &filename, &filename_len,
                              &line_key, &line_key_len,
                              &position, &mode) == FAILURE) {
        RETURN_FALSE;
    }

    // Проверка длины имени файла
    if (filename_len > PATH_MAX - 7) {
        php_error_docref(NULL, E_WARNING, "Filename too long");
        RETURN_FALSE;
    }

    char index_filename[PATH_MAX];
    snprintf(index_filename, sizeof(index_filename), "%.*s.index", (int)filename_len, filename);

    FILE *index_fp = fopen(index_filename, "r");
    if (!index_fp) {
        php_error_docref(NULL, E_WARNING, "Failed to open file: %s", index_filename);
        RETURN_FALSE;
    }

    // Получаем размер индексного файла
    if (fseek(index_fp, 0, SEEK_END) != 0) {
        php_error_docref(NULL, E_WARNING, "Failed to seek file: %s", index_filename);
        fclose(index_fp);
        RETURN_FALSE;
    }
    ssize_t file_size = ftell(index_fp);

    if (position > 0) {
        if (position >= file_size) {
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

    ssize_t ini_buffer_size = FAST_IO_G(buffer_size);
    if (file_size < ini_buffer_size) ini_buffer_size = file_size;
    if (ini_buffer_size < 16) ini_buffer_size = 16;

    ssize_t dynamic_buffer_size = ini_buffer_size;
    char *dynamic_buffer = (char *)emalloc(dynamic_buffer_size + 1);
    if (!dynamic_buffer) {
        fclose(data_fp);
        fclose(index_fp);
        zend_error(E_ERROR, "Out of memory to allocate %ld bytes", dynamic_buffer_size + 1);
    }

    ssize_t bytes_read;
    ssize_t current_size = 0;
    bool found_match = false;
    char *found_value = NULL;

    while ((bytes_read = fread(dynamic_buffer + current_size, 1, ini_buffer_size, index_fp)) > 0) {
        current_size += bytes_read;
        dynamic_buffer[current_size] = '\0';

        char *line_start = dynamic_buffer;
        char *line_end;

        while ((line_end = strchr(line_start, '\n')) != NULL) {
            *line_end = '\0';
            // Точное сравнение ключа в начале строки и после ключа должен быть пробел
            if (strncmp(line_start, line_key, line_key_len) == 0 && line_start[line_key_len] == ' ') {
                found_value = estrdup(line_start + line_key_len + 1);
                if (!found_value) {
                    fclose(data_fp);
                    fclose(index_fp);
                    efree(dynamic_buffer);
                    zend_error(E_ERROR, "Out of memory to allocate value");
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

    if (!found_match || !found_value) {
        if (found_value) efree(found_value);
        fclose(data_fp);
        RETURN_FALSE;
    }

    // Парсим смещение и размер
    char *colon_ptr = strchr(found_value, ':');
    if (!colon_ptr) {
        efree(found_value);
        fclose(data_fp);
        RETURN_FALSE;
    }
    *colon_ptr = '\0';
    position = atol(found_value);
    ssize_t size = (ssize_t)strtoul(colon_ptr + 1, NULL, 10);
    efree(found_value);

    if (size > 0) {
        if (fseek(data_fp, position, SEEK_SET) == -1) {
            php_error_docref(NULL, E_WARNING, "Failed to seek file: %s", filename);
            fclose(data_fp);
            RETURN_FALSE;
        }

        char *data_buffer = emalloc(size + 1);
        if (!data_buffer) {
            fclose(data_fp);
            zend_error(E_ERROR, "Out of memory to allocate %ld bytes", size + 1);
        }

        bytes_read = fread(data_buffer, 1, size, data_fp);
        if (bytes_read != size) {
            php_error_docref(NULL, E_WARNING, "Failed to read the file: %s", filename);
            fclose(data_fp);
            efree(data_buffer);
            RETURN_FALSE;
        }

        data_buffer[bytes_read] = '\0';
        RETVAL_STRING(data_buffer);
        efree(data_buffer);
    } else {
        fclose(data_fp);
        RETURN_FALSE;
    }

    fclose(data_fp);
}


/* Реализация функции */
PHP_FUNCTION(file_push_data)
{
    char *filename, *line_key, *line_value;
    size_t filename_len, line_key_len;
    ssize_t line_value_len;
    size_t mode = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "sss|l",
                              &filename, &filename_len,
                              &line_key, &line_key_len,
                              &line_value, &line_value_len, &mode) == FAILURE) {
        RETURN_FALSE;
    }

    // Проверка длины имени файла для предотвращения path-injection
    if (filename_len > PATH_MAX - 7) {
        php_error_docref(NULL, E_WARNING, "Filename too long");
        RETURN_LONG(-8);
    }

    char index_filename[PATH_MAX];
    snprintf(index_filename, sizeof(index_filename), "%.*s.index", (int)filename_len, filename);

    FILE *data_fp = fopen(filename, "a+");
    if (!data_fp) {
        php_error_docref(NULL, E_WARNING, "Failed to open file: %s", filename);
        RETURN_LONG(-1);
    }
    FILE *index_fp = fopen(index_filename, "a+");
    if (!index_fp) {
        php_error_docref(NULL, E_WARNING, "Failed to open file: %s", index_filename);
        fclose(data_fp);
        RETURN_LONG(-1);
    }

    if (mode < 100) {
        if (flock(fileno(data_fp), LOCK_EX) == -1) {
            php_error_docref(NULL, E_WARNING, "Failed to lock file: %s", filename);
            fclose(data_fp);
            fclose(index_fp);
            RETURN_LONG(-2);
        }
        if (flock(fileno(index_fp), LOCK_EX) == -1) {
            php_error_docref(NULL, E_WARNING, "Failed to lock file: %s", index_filename);
            fclose(data_fp);
            fclose(index_fp);
            RETURN_LONG(-2);
        }
    }
    if (mode > 99)
        mode -= 100;

    if (fseek(data_fp, 0, SEEK_END) != 0) {
        php_error_docref(NULL, E_WARNING, "Failed to seek file: %s", filename);
        fclose(data_fp);
        fclose(index_fp);
        RETURN_LONG(-3);
    }
    ssize_t position = ftell(data_fp);

    if (fwrite(line_value, 1, line_value_len, data_fp) != (size_t)line_value_len) {
        php_error_docref(NULL, E_WARNING, "Failed to write to file: %s", filename);
        if (ftruncate(fileno(data_fp), position) == -1) {
            fclose(data_fp);
            fclose(index_fp);
            zend_error(E_ERROR, "Failed to truncate file: %s", filename);
        }
        fclose(data_fp);
        fclose(index_fp);
        RETURN_LONG(-3);
    }

    if (fseek(index_fp, 0, SEEK_END) != 0) {
        php_error_docref(NULL, E_WARNING, "Failed to seek file: %s", index_filename);
        fclose(data_fp);
        fclose(index_fp);
        RETURN_LONG(-3);
    }
    ssize_t index_size = ftell(index_fp);

    if (fprintf(index_fp, "%.*s %ld:%zu\n", (int)line_key_len, line_key, position, (size_t)line_value_len) < (int)(line_key_len + 5)) {
        php_error_docref(NULL, E_WARNING, "Failed to write to file: %s", index_filename);
        if (ftruncate(fileno(index_fp), index_size) == -1) {
            fclose(data_fp);
            fclose(index_fp);
            zend_error(E_ERROR, "Failed to truncate file: %s", index_filename);
        }
        if (ftruncate(fileno(data_fp), position) == -1) {
            fclose(data_fp);
            fclose(index_fp);
            zend_error(E_ERROR, "Failed to truncate file: %s", filename);
        }
        fclose(data_fp);
        fclose(index_fp);
        RETURN_LONG(-3);
    }

    fclose(data_fp);
    fclose(index_fp);
    RETURN_LONG(position);
}
// path-injection Unvalidated input in path value creation risks unintended file/directory access



/* Реализация функции */
PHP_FUNCTION(file_defrag_lines)
{
    char *filename = NULL, *line_key = NULL;
    size_t filename_len = 0, line_key_len = 0, mode = 0;
    size_t found_count = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "s|sl",
                              &filename, &filename_len, &line_key, &line_key_len, &mode) == FAILURE) {
        RETURN_FALSE;
    }

    // Проверка длины имени файла для предотвращения path-injection
    char temp_filename[PATH_MAX];
    if (filename_len + 5 >= sizeof(temp_filename)) {
        php_error_docref(NULL, E_WARNING, "Filename too long");
        RETURN_LONG(-8);
    }
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

    if (mode < 100) {
        if (flock(fileno(data_fp), LOCK_EX) == -1) {
            php_error_docref(NULL, E_WARNING, "Failed to lock the file: %s", filename);
            fclose(data_fp);
            fclose(temp_fp);
            unlink(temp_filename);
            RETURN_LONG(-3);
        }
    }
    if (mode > 99) {
        mode -= 100;
    }

    // Определяем размер файла и возвращаем указатель в начало
    if (fseek(data_fp, 0, SEEK_END) != 0) {
        php_error_docref(NULL, E_WARNING, "Failed to seek file: %s", filename);
        fclose(data_fp);
        fclose(temp_fp);
        unlink(temp_filename);
        RETURN_LONG(-3);
    }
    ssize_t file_size = ftell(data_fp);
    fseek(data_fp, 0, SEEK_SET);

    ssize_t ini_buffer_size = FAST_IO_G(buffer_size);
    if (file_size < ini_buffer_size) ini_buffer_size = file_size;
    if (ini_buffer_size < 16) ini_buffer_size = 16;

    ssize_t dynamic_buffer_size = ini_buffer_size;
    char *dynamic_buffer = (char *)emalloc(dynamic_buffer_size + 1);
    if (!dynamic_buffer) {
        fclose(data_fp);
        fclose(temp_fp);
        unlink(temp_filename);
        zend_error(E_ERROR, "Out of memory to allocate %ld bytes", dynamic_buffer_size + 1);
    }

    ssize_t bytes_read, bytes_write, current_size = 0;
    while ((bytes_read = fread(dynamic_buffer + current_size, 1, ini_buffer_size, data_fp)) > 0) {
        current_size += bytes_read;
        dynamic_buffer[current_size] = '\0';

        char *line_start = dynamic_buffer;
        char *line_end = NULL;
        while ((line_end = strchr(line_start, '\n')) != NULL) {
            ssize_t line_length = line_end - line_start + 1;
            *line_end = '\0'; // Завершаем строку

            bool found_match = false;
            // Если задан ключ, ищем либо спецсимвол, либо совпадение с ключом
            if (line_key && line_key_len > 0) {
                if ((unsigned char)*line_start == SPECIAL_CHAR ||
                    (strncmp(line_start, line_key, line_key_len) == 0 &&
                     (line_start[line_key_len] == ' ' || line_start[line_key_len] == '\0'))) {
                    found_match = true;
                }
            } else {
                if ((unsigned char)*line_start == SPECIAL_CHAR) {
                    found_match = true;
                }
            }

            if (!found_match) {
                *line_end = '\n'; // Восстанавливаем перевод строки
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

        // Переносим неполную строку в начало буфера
        current_size -= (line_start - dynamic_buffer);
        memmove(dynamic_buffer, line_start, current_size);

        // Расширяем буфер, если необходимо
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

    // Если mode == 0, копируем данные обратно в исходный файл
    if (mode == 0) {
        fseek(data_fp, 0, SEEK_SET);
        fseek(temp_fp, 0, SEEK_SET);
        ssize_t total_written = 0;
        while ((bytes_read = fread(dynamic_buffer, 1, ini_buffer_size, temp_fp)) > 0) {
            bytes_write = fwrite(dynamic_buffer, 1, bytes_read, data_fp);
            if (bytes_write != bytes_read) {
                php_error_docref(NULL, E_WARNING, "Failed to write to the file: %s", filename);
                fclose(data_fp);
                fclose(temp_fp);
                unlink(temp_filename);
                efree(dynamic_buffer);
                RETURN_LONG(-4);
            }
            total_written += bytes_write;
        }
        // Усечение файла до нового размера
        if (ftruncate(fileno(data_fp), total_written) == -1) {
            php_error_docref(NULL, E_WARNING, "Failed to truncate file: %s", filename);
            fclose(data_fp);
            fclose(temp_fp);
            unlink(temp_filename);
            efree(dynamic_buffer);
            RETURN_LONG(-5);
        }
        fclose(temp_fp);
        unlink(temp_filename);
    } else {
        efree(dynamic_buffer);
        fclose(data_fp);
        fclose(temp_fp);
        if (mode == 1) {
            if (rename(temp_filename, filename) != 0) {
                php_error_docref(NULL, E_WARNING, "Failed to rename temp file: %s", temp_filename);
                RETURN_LONG(-6);
            }
        }
        RETURN_LONG(found_count);
    }

    efree(dynamic_buffer);
    fclose(data_fp);

    RETURN_LONG(found_count);
}
// path-injection Unvalidated input in path value creation risks unintended file/directory access




PHP_FUNCTION(file_defrag_data)
{
    char *filename = NULL, *line_key = NULL;
    size_t filename_len = 0, line_key_len = 0, mode = 0;
    size_t found_count = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "s|sl",
                              &filename, &filename_len, &line_key, &line_key_len, &mode) == FAILURE) {
        RETURN_FALSE;
    }

    // Проверка длины имени файла для предотвращения path-injection
    char temp_filename[PATH_MAX];
    char index_filename[PATH_MAX];
    char temp_index_filename[PATH_MAX];
    if (filename_len + 11 >= sizeof(temp_filename) ||
        filename_len + 11 >= sizeof(index_filename) ||
        filename_len + 11 >= sizeof(temp_index_filename)) {
        php_error_docref(NULL, E_WARNING, "Filename too long");
        RETURN_LONG(-8);
    }
    snprintf(temp_filename, sizeof(temp_filename), "%s.tmp", filename);
    snprintf(index_filename, sizeof(index_filename), "%s.index", filename);
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

    // Блокировка файлов, если не Log mode
    if (mode < 100) {
        if (flock(fileno(data_fp), LOCK_EX) == -1) {
            php_error_docref(NULL, E_WARNING, "Failed to lock file: %s", filename);
            fclose(index_fp);
            fclose(data_fp);
            fclose(temp_fp);
            fclose(temp_index_fp);
            unlink(temp_filename);
            unlink(temp_index_filename);
            RETURN_LONG(-2);
        }
        if (flock(fileno(index_fp), LOCK_EX) == -1) {
            php_error_docref(NULL, E_WARNING, "Failed to lock file: %s", index_filename);
            fclose(index_fp);
            fclose(data_fp);
            fclose(temp_fp);
            fclose(temp_index_fp);
            unlink(temp_filename);
            unlink(temp_index_filename);
            RETURN_LONG(-2);
        }
    }
    if (mode > 99) {
        mode -= 100;
    }

    // Определяем размер индексного файла и подготавливаем буфер
    fseek(index_fp, 0, SEEK_END);
    ssize_t file_size = ftell(index_fp);
    fseek(index_fp, 0, SEEK_SET);
    ssize_t ini_buffer_size = FAST_IO_G(buffer_size);
    if (file_size < ini_buffer_size) ini_buffer_size = file_size;
    if (ini_buffer_size < 16) ini_buffer_size = 16;
    ssize_t dynamic_buffer_size = ini_buffer_size;
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

    ssize_t bytes_read, bytes_write, bytes_read_data, bytes_write_data, current_size = 0;
    bool found_match;
    off_t position;
    size_t size;

    // Читаем индексный файл блоками
    while ((bytes_read = fread(dynamic_buffer + current_size, 1, ini_buffer_size, index_fp)) > 0) {
        current_size += bytes_read;
        dynamic_buffer[current_size] = '\0';

        char *line_start = dynamic_buffer;
        char *line_end;
        while ((line_end = strchr(line_start, '\n')) != NULL) {
            ssize_t line_length = line_end - line_start + 1;
            *line_end = '\0'; // Завершаем строку

            // Определяем, нужно ли удалять запись
            found_match = false;
            if (line_key && line_key_len > 0) {
                if ((unsigned char)*line_start == SPECIAL_CHAR ||
                    (strncmp(line_start, line_key, line_key_len) == 0 &&
                     (line_start[line_key_len] == ' ' || line_start[line_key_len] == '\0'))) {
                    found_match = true;
                }
            } else {
                if ((unsigned char)*line_start == SPECIAL_CHAR) {
                    found_match = true;
                }
            }

            if (!found_match) {
                // Копируем строку в temp_index_fp
                *line_end = '\n';
                bytes_write = fwrite(line_start, 1, line_length, temp_index_fp);
                if (bytes_write != line_length) {
                    php_error_docref(NULL, E_WARNING, "Failed to write to file: %s", temp_index_filename);
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

                // Парсим строку формата "ключ offset:size"
                char *space = strchr(line_start, ' ');
                char *colon = space ? strchr(space + 1, ':') : NULL;
                if (!space || !colon) {
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
                position = strtoul(space + 1, NULL, 10);
                size = strtoul(colon + 1, NULL, 10);
                if (size == 0) {
                    line_start = line_end + 1;
                    continue;
                }
                // Читаем данные из оригинального файла
                if (fseek(data_fp, position, SEEK_SET) != 0) {
                    php_error_docref(NULL, E_WARNING, "Failed to seek file: %s", filename);
                    fclose(index_fp);
                    fclose(data_fp);
                    fclose(temp_fp);
                    fclose(temp_index_fp);
                    unlink(temp_filename);
                    unlink(temp_index_filename);
                    efree(dynamic_buffer);
                    RETURN_LONG(-6);
                }
                char *data_buffer = emalloc(size + 1);
                if (!data_buffer) {
                    fclose(index_fp);
                    fclose(data_fp);
                    fclose(temp_fp);
                    fclose(temp_index_fp);
                    unlink(temp_filename);
                    unlink(temp_index_filename);
                    efree(dynamic_buffer);
                    zend_error(E_ERROR, "Out of memory to allocate %ld bytes", size + 1);
                }
                bytes_read_data = fread(data_buffer, 1, size, data_fp);
                if (bytes_read_data != (ssize_t)size) {
                    php_error_docref(NULL, E_WARNING, "Failed to read from file: %s", filename);
                    fclose(index_fp);
                    fclose(data_fp);
                    fclose(temp_fp);
                    fclose(temp_index_fp);
                    unlink(temp_filename);
                    unlink(temp_index_filename);
                    efree(dynamic_buffer);
                    efree(data_buffer);
                    RETURN_LONG(-6);
                }
                data_buffer[bytes_read_data] = '\0';
                // Записываем данные во временный файл данных
                bytes_write_data = fwrite(data_buffer, 1, bytes_read_data, temp_fp);
                if (bytes_write_data != bytes_read_data) {
                    php_error_docref(NULL, E_WARNING, "Failed to write to file: %s", temp_filename);
                    fclose(index_fp);
                    fclose(data_fp);
                    fclose(temp_fp);
                    fclose(temp_index_fp);
                    unlink(temp_filename);
                    unlink(temp_index_filename);
                    efree(dynamic_buffer);
                    efree(data_buffer);
                    RETURN_LONG(-4);
                }
                efree(data_buffer);
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

    // В зависимости от режима обновления копируем или переименовываем файлы
    if (mode == 0) {
        fseek(data_fp, 0, SEEK_SET);
        fseek(temp_fp, 0, SEEK_SET);
        fseek(index_fp, 0, SEEK_SET);
        fseek(temp_index_fp, 0, SEEK_SET);
        ssize_t total_written = 0;
        while ((bytes_read = fread(dynamic_buffer, 1, ini_buffer_size, temp_fp)) > 0) {
            bytes_write = fwrite(dynamic_buffer, 1, bytes_read, data_fp);
            if (bytes_write != bytes_read) {
                php_error_docref(NULL, E_WARNING, "Failed to write to file: %s", filename);
                fclose(index_fp);
                fclose(data_fp);
                fclose(temp_fp);
                fclose(temp_index_fp);
                rename(temp_filename, filename);
                rename(temp_index_filename, index_filename);
                efree(dynamic_buffer);
                RETURN_LONG(-4);
            }
            total_written += bytes_write;
        }
        if (ftruncate(fileno(data_fp), total_written) == -1) {
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
        total_written = 0;
        while ((bytes_read = fread(dynamic_buffer, 1, ini_buffer_size, temp_index_fp)) > 0) {
            bytes_write = fwrite(dynamic_buffer, 1, bytes_read, index_fp);
            if (bytes_write != bytes_read) {
                php_error_docref(NULL, E_WARNING, "Failed to write to file: %s", index_filename);
                fclose(index_fp);
                fclose(data_fp);
                fclose(temp_fp);
                fclose(temp_index_fp);
                rename(temp_filename, filename);
                rename(temp_index_filename, index_filename);
                efree(dynamic_buffer);
                RETURN_LONG(-4);
            }
            total_written += bytes_write;
        }
        if (ftruncate(fileno(index_fp), total_written) == -1) {
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

    if (mode == 1) {
        fclose(temp_fp);
        rename(temp_filename, filename);
        fclose(temp_index_fp);
        rename(temp_index_filename, index_filename);
    }

    RETURN_LONG(found_count);
}

// path-injection Unvalidated input in path value creation risks unintended file/directory access




/* Функция для извлечения и удаления последней строки из файла */
PHP_FUNCTION(file_pop_line)
{
    char *filename = NULL;
    size_t filename_len = 0;
    ssize_t offset = -1;
    size_t mode = 0;
    ssize_t end = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "s|lll",
                              &filename, &filename_len, &offset, &mode, &end) == FAILURE) {
        RETURN_FALSE;
    }

    // Проверка длины имени файла для предотвращения path-injection
    if (filename_len > PATH_MAX - 1) {
        php_error_docref(NULL, E_WARNING, "Filename too long");
        RETURN_FALSE;
    }

    FILE *fp = fopen(filename, "r+");
    if (!fp) {
        php_error_docref(NULL, E_WARNING, "Failed to open file: %s", filename);
        RETURN_FALSE;
    }
    if (mode < 100) {
        if (flock(fileno(fp), LOCK_EX) == -1) {
            php_error_docref(NULL, E_WARNING, "Failed to lock the file: %s", filename);
            fclose(fp);
            RETURN_FALSE;
        }
    }
    if (mode > 99) {
        mode -= 100;
    }

    if (fseek(fp, 0, SEEK_END) != 0) {
        php_error_docref(NULL, E_WARNING, "Failed to seek file: %s", filename);
        fclose(fp);
        RETURN_FALSE;
    }
    ssize_t file_size = ftell(fp);

    // --- Байтовый pop: offset > 0 ---
    if (offset > 0) {
        if (offset > file_size) {
            php_error_docref(NULL, E_WARNING, "Offset is greater than file size: %s", filename);
            fclose(fp);
            RETURN_FALSE;
        }
        ssize_t new_size = file_size - offset;
        if (fseek(fp, new_size, SEEK_SET) != 0) {
            php_error_docref(NULL, E_WARNING, "Failed to seek file: %s", filename);
            fclose(fp);
            RETURN_FALSE;
        }
        char *buffer = (char *)emalloc(offset + 1);
        if (!buffer) {
            fclose(fp);
            zend_error(E_ERROR, "Out of memory to allocate %ld bytes", offset + 1);
        }
        ssize_t bytes_read = fread(buffer, 1, offset, fp);
        if (bytes_read != offset) {
            efree(buffer);
            fclose(fp);
            php_error_docref(NULL, E_WARNING, "Failed to read from file: %s", filename);
            RETURN_FALSE;
        }
        buffer[bytes_read] = '\0';
        if (mode < 1 || mode == 2) {
            for (ssize_t i = bytes_read - 1; i >= 0; --i) {
                if (buffer[i] == ' ' || buffer[i] == '\n')
                    buffer[i] = '\0';
                else
                    break;
            }
        }
        if (mode < 2) {
            if (ftruncate(fileno(fp), new_size) == -1) {
                efree(buffer);
                fclose(fp);
                php_error_docref(NULL, E_WARNING, "Failed to truncate file: %s", filename);
                RETURN_FALSE;
            }
        }
        fclose(fp);
        RETURN_STRING(buffer);
    }

    // --- Строковый pop: offset < 0 ---
    if (offset < 0) {
        int target_line = -offset;
        ssize_t ini_buffer_size = FAST_IO_G(buffer_size);
        if (file_size < ini_buffer_size)
            ini_buffer_size = file_size;
        if (ini_buffer_size < 16)
            ini_buffer_size = 16;
        ssize_t dynamic_buffer_size = ini_buffer_size;
        char *dynamic_buffer = (char *)emalloc(dynamic_buffer_size + 1);
        if (!dynamic_buffer) {
            fclose(fp);
            zend_error(E_ERROR, "Out of memory to allocate %ld bytes", dynamic_buffer_size + 1);
        }
        ssize_t current_size = 0;
        ssize_t pos = file_size;
        int newline_count = 0;
        ssize_t line_start_pos = 0;

        // Если end > 0, уменьшаем размер файла на end байт
        if (end > 0 && pos > end)
            pos -= end;

        // Чтение файла блоками с конца до тех пор, пока не найдено нужное число переводов строки
        while (pos > 0 && newline_count < target_line) {
            ssize_t read_size = (pos >= ini_buffer_size) ? ini_buffer_size : pos;
            pos -= read_size;
            if (fseek(fp, pos, SEEK_SET) != 0) {
                efree(dynamic_buffer);
                fclose(fp);
                php_error_docref(NULL, E_WARNING, "Failed to seek file: %s", filename);
                RETURN_FALSE;
            }
            ssize_t r = fread(dynamic_buffer + current_size, 1, read_size, fp);
            if (r != read_size) {
                efree(dynamic_buffer);
                fclose(fp);
                php_error_docref(NULL, E_WARNING, "Failed to read from file: %s", filename);
                RETURN_FALSE;
            }
            current_size += r;
            for (ssize_t i = r - 1; i >= 0; i--) {
                if (dynamic_buffer[current_size - r + i] == '\n') {
                    newline_count++;
                    if (newline_count == target_line) {
                        line_start_pos = pos + i + 1;
                        goto line_found;
                    }
                }
            }
            if (current_size + ini_buffer_size > dynamic_buffer_size) {
                dynamic_buffer_size += ini_buffer_size;
                char *temp_buffer = (char *)erealloc(dynamic_buffer, dynamic_buffer_size + 1);
                if (!temp_buffer) {
                    fclose(fp);
                    efree(dynamic_buffer);
                    zend_error(E_ERROR, "Out of memory to allocate %ld bytes", dynamic_buffer_size + 1);
                }
                dynamic_buffer = temp_buffer;
            }
        }
        line_start_pos = 0;
line_found:
        ssize_t line_length = file_size - line_start_pos;
        if (fseek(fp, line_start_pos, SEEK_SET) != 0) {
            efree(dynamic_buffer);
            fclose(fp);
            php_error_docref(NULL, E_WARNING, "Failed to seek file: %s", filename);
            RETURN_FALSE;
        }
        char *line_buffer = (char *)emalloc(line_length + 1);
        if (!line_buffer) {
            efree(dynamic_buffer);
            fclose(fp);
            zend_error(E_ERROR, "Out of memory to allocate %ld bytes", line_length + 1);
        }
        if (fread(line_buffer, 1, line_length, fp) != line_length) {
            efree(line_buffer);
            efree(dynamic_buffer);
            fclose(fp);
            php_error_docref(NULL, E_WARNING, "Failed to read line from file: %s", filename);
            RETURN_FALSE;
        }
        line_buffer[line_length] = '\0';
        if (mode < 1 || mode == 2) {
            for (ssize_t i = line_length - 1; i >= 0; --i) {
                if (line_buffer[i] == ' ' || line_buffer[i] == '\n')
                    line_buffer[i] = '\0';
                else
                    break;
            }
        }
        if (mode < 2) {
            if (ftruncate(fileno(fp), line_start_pos) == -1) {
                efree(line_buffer);
                efree(dynamic_buffer);
                fclose(fp);
                php_error_docref(NULL, E_WARNING, "Failed to truncate file: %s", filename);
                RETURN_FALSE;
            }
        }
        fclose(fp);
        efree(dynamic_buffer);
        RETURN_STRING(line_buffer);
    }

    fclose(fp);
    RETURN_FALSE;
}





/* Реализация функции */
PHP_FUNCTION(file_erase_line)
{
    char *filename = NULL, *line_key = NULL;
    size_t filename_len = 0, line_key_len = 0;
    ssize_t position = 0;
    size_t mode = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "ss|ll",
                              &filename, &filename_len, &line_key, &line_key_len,
                              &position, &mode) == FAILURE) {
        RETURN_FALSE;
    }

    // Проверка длины имени файла для предотвращения path-injection
    if (filename_len > PATH_MAX - 1) {
        php_error_docref(NULL, E_WARNING, "Filename too long");
        RETURN_LONG(-8);
    }

    FILE *fp = fopen(filename, "r+");
    if (!fp) {
        php_error_docref(NULL, E_WARNING, "Failed to open file: %s", filename);
        RETURN_LONG(-1);
    }

    if (mode < 100) {
        if (flock(fileno(fp), LOCK_EX) == -1) {
            php_error_docref(NULL, E_WARNING, "Failed to lock file: %s", filename);
            fclose(fp);
            RETURN_LONG(-2);
        }
    }
    if (mode > 99) {
        mode -= 100;
    }

    if (fseek(fp, 0, SEEK_END) != 0) {
        php_error_docref(NULL, E_WARNING, "Failed to seek end of file: %s", filename);
        fclose(fp);
        RETURN_LONG(-3);
    }
    ssize_t file_size = ftell(fp);
    ssize_t write_offset = 0;

    if (position > 0) {
        if (position > file_size) {
            php_error_docref(NULL, E_WARNING, "Specified position exceeds file size: %s", filename);
            fclose(fp);
            RETURN_LONG(-5);
        }
        if (fseek(fp, position, SEEK_SET) != 0) {
            php_error_docref(NULL, E_WARNING, "Failed to seek to position in file: %s", filename);
            fclose(fp);
            RETURN_LONG(-3);
        }
        write_offset = position;
    } else {
        fseek(fp, 0, SEEK_SET);
    }

    ssize_t ini_buffer_size = FAST_IO_G(buffer_size);
    if (file_size < ini_buffer_size)
        ini_buffer_size = file_size;
    if (ini_buffer_size < 16)
        ini_buffer_size = 16;

    ssize_t dynamic_buffer_size = ini_buffer_size;
    char *dynamic_buffer = (char *)emalloc(dynamic_buffer_size + 1);
    if (!dynamic_buffer) {
        fclose(fp);
        zend_error(E_ERROR, "Out of memory to allocate %ld bytes", dynamic_buffer_size + 1);
    }

    ssize_t bytes_read, bytes_write, current_size = 0;
    bool found_match = false;

    while ((bytes_read = fread(dynamic_buffer + current_size, 1, ini_buffer_size, fp)) > 0) {
        current_size += bytes_read;
        dynamic_buffer[current_size] = '\0';

        char *line_start = dynamic_buffer;
        char *line_end;
        while ((line_end = strchr(line_start, '\n')) != NULL) {
            ssize_t line_length = line_end - line_start + 1;
            *line_end = '\0'; // Завершаем строку

            // Точное сравнение ключа: строка должна начинаться с line_key и далее пробел или конец строки
            if (strncmp(line_start, line_key, line_key_len) == 0 &&
                (line_start[line_key_len] == ' ' || line_start[line_key_len] == '\0')) {

                // Заполняем строку спецсимволом, кроме последнего символа (перевод строки)
                char *replacement = (char *)emalloc(line_length + 1);
                if (!replacement) {
                    efree(dynamic_buffer);
                    fclose(fp);
                    zend_error(E_ERROR, "Out of memory to allocate %ld bytes", line_length + 1);
                }
                memset(replacement, SPECIAL_CHAR, line_length - 1);
                replacement[line_length - 1] = '\n';
                replacement[line_length] = '\0';

                // Переходим к позиции записи и пишем замену
                if (fseek(fp, write_offset, SEEK_SET) != 0) {
                    efree(replacement);
                    efree(dynamic_buffer);
                    fclose(fp);
                    php_error_docref(NULL, E_WARNING, "Failed to seek file for writing: %s", filename);
                    RETURN_LONG(-3);
                }
                bytes_write = fwrite(replacement, 1, line_length, fp);
                efree(replacement);
                if (bytes_write != line_length) {
                    efree(dynamic_buffer);
                    fclose(fp);
                    php_error_docref(NULL, E_WARNING, "Failed to write to the file: %s", filename);
                    RETURN_LONG(-3);
                }
                found_match = true;
                break;
            }
            write_offset += line_length;
            line_start = line_end + 1;
        }

        if (found_match)
            break;

        // Переносим незавершённую строку в начало буфера
        current_size -= (line_start - dynamic_buffer);
        memmove(dynamic_buffer, line_start, current_size);

        // Расширяем буфер при необходимости
        if (current_size + ini_buffer_size > dynamic_buffer_size) {
            dynamic_buffer_size += ini_buffer_size;
            char *temp_buffer = (char *)erealloc(dynamic_buffer, dynamic_buffer_size + 1);
            if (!temp_buffer) {
                fclose(fp);
                efree(dynamic_buffer);
                zend_error(E_ERROR, "Out of memory to allocate %ld bytes", dynamic_buffer_size + 1);
            }
            dynamic_buffer = temp_buffer;
        }
    }

    efree(dynamic_buffer);
    fclose(fp);

    if (found_match) {
        RETURN_LONG(write_offset);
    } else {
        RETURN_LONG(-4);
    }
}



/* Реализация функции */
PHP_FUNCTION(file_get_keys)
{
    char *filename = NULL;
    size_t filename_len = 0;
    size_t search_start = 0, search_limit = 1;
    ssize_t position = 0;
    size_t mode = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "s|llll",
                              &filename, &filename_len,
                              &search_start, &search_limit, &position, &mode) == FAILURE) {
        RETURN_FALSE;
    }

    // Проверка длины имени файла для предотвращения path-injection
    if (filename_len > PATH_MAX - 1) {
        php_error_docref(NULL, E_WARNING, "Filename too long");
        RETURN_FALSE;
    }

    FILE *fp = fopen(filename, "r");
    if (!fp) {
        php_error_docref(NULL, E_WARNING, "Failed to open file: %s", filename);
        RETURN_FALSE;
    }

    if (mode < 100) {
        if (flock(fileno(fp), LOCK_EX) == -1) {
            php_error_docref(NULL, E_WARNING, "Failed to lock file: %s", filename);
            fclose(fp);
            RETURN_FALSE;
        }
    } else if (mode > 99) {
        mode -= 100;
    }

    // Получаем размер файла и устанавливаем позицию
    if (fseek(fp, 0, SEEK_END) != 0) {
        php_error_docref(NULL, E_WARNING, "Failed to seek end of file: %s", filename);
        fclose(fp);
        RETURN_FALSE;
    }
    ssize_t file_size = ftell(fp);
    if (file_size < 0) {
        php_error_docref(NULL, E_WARNING, "Failed to get file size: %s", filename);
        fclose(fp);
        RETURN_FALSE;
    }

    ssize_t line_offset = 0;
    if (position > 0) {
        if (position > file_size) {
            php_error_docref(NULL, E_WARNING, "Position exceeds file size: %s", filename);
            fclose(fp);
            RETURN_FALSE;
        }
        fseek(fp, position, SEEK_SET);
        line_offset = position;
    } else {
        fseek(fp, 0, SEEK_SET);
    }

    ssize_t ini_buffer_size = FAST_IO_G(buffer_size);
    if (file_size < ini_buffer_size) ini_buffer_size = file_size;
    if (ini_buffer_size < 16) ini_buffer_size = 16;
    ssize_t dynamic_buffer_size = ini_buffer_size;
    char *dynamic_buffer = (char *)emalloc(dynamic_buffer_size + 1);
    if (!dynamic_buffer) {
        fclose(fp);
        zend_error(E_ERROR, "Out of memory to allocate %ld bytes", dynamic_buffer_size + 1);
    }

    ssize_t bytes_read, current_size = 0;
    size_t add_count = 0, line_count = 0;

    array_init(return_value);

    while ((bytes_read = fread(dynamic_buffer + current_size, 1, ini_buffer_size, fp)) > 0) {
        current_size += bytes_read;
        dynamic_buffer[current_size] = '\0';

        char *line_ptr = dynamic_buffer;
        char *newline;
        while ((newline = strchr(line_ptr, '\n')) != NULL) {
            ssize_t line_length = newline - line_ptr + 1;
            *newline = '\0';
            line_count++;

            if (line_count > search_start) {
                zval line_arr;
                array_init(&line_arr);

                // Mode 0 or 4: extract key (substring before first space)
                if (mode == 0 || mode == 4) {
                    char *space = strchr(line_ptr, ' ');
                    if (space) *space = '\0';
                    if (mode == 0) {
                        add_assoc_string(&line_arr, "key", line_ptr);
                    } else {
                        add_next_index_string(return_value, line_ptr);
                    }
                }
                // Mode 1: entire line
                else if (mode == 1) {
                    add_assoc_string(&line_arr, "line", line_ptr);
                }
                // Mode 2 or 5: trim right (spaces and newline)
                else if (mode == 2 || mode == 5) {
                    ssize_t i;
                    for (i = line_length - 2; i >= 0; i--) {
                        if (line_ptr[i] == ' ' || line_ptr[i] == '\n')
                            line_ptr[i] = '\0';
                        else
                            break;
                    }
                    if (mode == 2) {
                        add_assoc_string(&line_arr, "trim_line", line_ptr);
                        add_assoc_long(&line_arr, "trim_length", i + 1);
                    } else {
                        add_next_index_string(return_value, line_ptr);
                    }
                }
                // Для режимов < 4 (0,1,2,3) добавляем offset, length, count
                if (mode < 4 && mode != 4) {
                    add_assoc_long(&line_arr, "line_offset", line_offset);
                    add_assoc_long(&line_arr, "line_length", line_length);
                    add_assoc_long(&line_arr, "line_count", line_count - 1);
                    add_next_index_zval(return_value, &line_arr);
                }
                add_count++;
                if (add_count >= search_limit) {
                    goto done;
                }
            }
            line_offset += line_length;
            line_ptr = newline + 1;
        }
        // Переносим остаток в начало буфера
        current_size = dynamic_buffer + current_size - line_ptr;
        memmove(dynamic_buffer, line_ptr, current_size);
    }
done:
    fclose(fp);
    efree(dynamic_buffer);

    if (zend_hash_num_elements(Z_ARRVAL_P(return_value)) == 0) {
        zval_ptr_dtor(return_value);
        RETURN_FALSE;
    }
    RETURN_ZVAL(return_value, 0, 1);
}
// path-injection Unvalidated input in path value creation risks unintended file/directory access



// Функция обновления пары ключ-значение
PHP_FUNCTION(file_replace_line)
{
    char *filename = NULL, *line_key = NULL, *line = NULL;
    size_t filename_len = 0, line_key_len = 0, mode = 0;
    ssize_t line_len = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "sss|l",
                              &filename, &filename_len,
                              &line_key, &line_key_len,
                              &line, &line_len, &mode) == FAILURE) {
        RETURN_FALSE;
    }

    // Проверка длины имени файла для предотвращения path-injection
    char temp_filename[PATH_MAX];
    if (filename_len + 5 >= sizeof(temp_filename)) {
        php_error_docref(NULL, E_WARNING, "Filename too long");
        RETURN_LONG(-8);
    }
    snprintf(temp_filename, sizeof(temp_filename), "%s.tmp", filename);

    FILE *data_fp = fopen(filename, "r+");
    if (!data_fp) {
        php_error_docref(NULL, E_WARNING, "Failed to open file: %s", filename);
        RETURN_LONG(-1);
    }

    FILE *temp_fp = fopen(temp_filename, "w+");
    if (!temp_fp) {
        php_error_docref(NULL, E_WARNING, "Failed to open temp file: %s", temp_filename);
        fclose(data_fp);
        RETURN_LONG(-2);
    }

    // Блокировка файла, если режим не Log mode
    if (mode < 100) {
        if (flock(fileno(data_fp), LOCK_EX) == -1) {
            php_error_docref(NULL, E_WARNING, "Failed to lock file: %s", filename);
            fclose(data_fp);
            fclose(temp_fp);
            unlink(temp_filename);
            RETURN_LONG(-3);
        }
    }
    if (mode > 99)
        mode -= 100;

    // Получаем размер файла и возвращаем указатель в начало
    if (fseek(data_fp, 0, SEEK_END) != 0) {
        php_error_docref(NULL, E_WARNING, "Failed to seek end of file: %s", filename);
        fclose(data_fp);
        fclose(temp_fp);
        RETURN_LONG(-3);
    }
    ssize_t file_size = ftell(data_fp);
    fseek(data_fp, 0, SEEK_SET);

    ssize_t ini_buffer_size = FAST_IO_G(buffer_size);
    if (file_size < ini_buffer_size)
        ini_buffer_size = file_size;
    if (ini_buffer_size < 16)
        ini_buffer_size = 16;

    ssize_t dynamic_buffer_size = ini_buffer_size;
    char *dynamic_buffer = (char *)emalloc(dynamic_buffer_size + 1);
    if (!dynamic_buffer) {
        fclose(data_fp);
        fclose(temp_fp);
        unlink(temp_filename);
        zend_error(E_ERROR, "Out of memory to allocate %ld bytes", dynamic_buffer_size + 1);
    }

    size_t found_count = 0;
    ssize_t bytes_read, bytes_write, current_size = 0;
    int replaced = 0;

    // Читаем файл по блокам, обрабатывая каждую строку
    while ((bytes_read = fread(dynamic_buffer + current_size, 1, ini_buffer_size, data_fp)) > 0) {
        current_size += bytes_read;
        dynamic_buffer[current_size] = '\0';

        char *line_start = dynamic_buffer;
        char *line_end;
        while ((line_end = strchr(line_start, '\n')) != NULL) {
            ssize_t this_line_length = line_end - line_start + 1;
            *line_end = '\0';

            // Точное сравнение ключа: сравниваем первые line_key_len символов,
            // и следующий символ должен быть пробелом или концом строки
            bool is_match = false;
            if (strncmp(line_start, line_key, line_key_len) == 0 &&
                (line_start[line_key_len] == ' ' || line_start[line_key_len] == '\0')) {
                is_match = true;
            }

            if (is_match && !replaced) {
                // Создаем строку-заменитель: копируем новую строку и добавляем '\n'
                char *replacement = (char *)emalloc(line_len + 2);
                if (!replacement) {
                    fclose(data_fp);
                    fclose(temp_fp);
                    unlink(temp_filename);
                    efree(dynamic_buffer);
                    zend_error(E_ERROR, "Out of memory to allocate %ld bytes", line_len + 2);
                }
                memcpy(replacement, line, line_len);
                replacement[line_len] = '\n';
                replacement[line_len + 1] = '\0';

                bytes_write = fwrite(replacement, 1, line_len + 1, temp_fp);
                efree(replacement);
                if (bytes_write != line_len + 1) {
                    php_error_docref(NULL, E_WARNING, "Failed to write replacement to temp file: %s", temp_filename);
                    fclose(data_fp);
                    fclose(temp_fp);
                    unlink(temp_filename);
                    efree(dynamic_buffer);
                    RETURN_LONG(-4);
                }
                replaced = 1;
            } else {
                *line_end = '\n';
                bytes_write = fwrite(line_start, 1, this_line_length, temp_fp);
                if (bytes_write != this_line_length) {
                    php_error_docref(NULL, E_WARNING, "Failed to write original line to temp file: %s", temp_filename);
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

        // Оставляем оставшуюся (незавершённую) часть буфера для следующей итерации
        current_size -= (line_start - dynamic_buffer);
        memmove(dynamic_buffer, line_start, current_size);

        // Расширяем динамический буфер при необходимости
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

    // В зависимости от режима обновления:
    // Для mode == 0 копируем данные из временного файла обратно в оригинальный файл
    if (mode == 0) {
        fseek(data_fp, 0, SEEK_SET);
        fseek(temp_fp, 0, SEEK_SET);
        ssize_t total_written = 0;
        while ((bytes_read = fread(dynamic_buffer, 1, ini_buffer_size, temp_fp)) > 0) {
            bytes_write = fwrite(dynamic_buffer, 1, bytes_read, data_fp);
            if (bytes_write != bytes_read) {
                php_error_docref(NULL, E_WARNING, "Failed to write back to file: %s", filename);
                efree(dynamic_buffer);
                fclose(data_fp);
                fclose(temp_fp);
                rename(temp_filename, filename);
                RETURN_LONG(-4);
            }
            total_written += bytes_write;
        }
        if (ftruncate(fileno(data_fp), total_written) == -1) {
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

    // Для mode == 1 переименовываем временный файл в оригинальный
    if (mode == 1) {
        fclose(temp_fp);
        if (rename(temp_filename, filename) != 0) {
            php_error_docref(NULL, E_WARNING, "Failed to rename temp file: %s", temp_filename);
            RETURN_LONG(-6);
        }
    }

    RETURN_LONG(found_count);
}
// path-injection Unvalidated input in path value creation risks unintended file/directory access



PHP_FUNCTION(file_insert_line)
{
    char *filename;
    size_t filename_len;
    char *line;
    ssize_t line_len;
    size_t mode = 0;
    ssize_t line_length = 0;

    // Парсинг аргументов: filename, line, опционально mode и line_length
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "ss|ll",
                              &filename, &filename_len, &line, &line_len, &mode, &line_length) == FAILURE) {
        RETURN_FALSE;
    }

    // Проверка, что строка не пустая
    if (line_len == 0) {
        php_error_docref(NULL, E_WARNING, "An empty line");
        RETURN_LONG(-4);
    }

    // Проверка длины имени файла для предотвращения path-injection
    if (filename_len > PATH_MAX - 1) {
        php_error_docref(NULL, E_WARNING, "Filename too long");
        RETURN_LONG(-8);
    }

    // Если line_length не задан или меньше или равен 0, брать длину строки плюс 1 (для '\n')
    if (line_length <= 0) {
        line_length = line_len + 1;
    }

    FILE *fp = fopen(filename, "a");
    if (!fp) {
        php_error_docref(NULL, E_WARNING, "Failed to open file: %s", filename);
        RETURN_LONG(-1);
    }

    // Блокировка файла (если не включён log-режим)
    if (mode < 100) {
        if (flock(fileno(fp), LOCK_EX) == -1) {
            php_error_docref(NULL, E_WARNING, "Failed to lock the file: %s", filename);
            fclose(fp);
            RETURN_LONG(-2);
        }
    }
    if (mode > 99) {
        mode -= 100;
    }

    // Получаем текущее смещение (размер файла) для последующего возврата
    if (fseek(fp, 0, SEEK_END) != 0) {
        php_error_docref(NULL, E_WARNING, "Failed to seek to end of file: %s", filename);
        fclose(fp);
        RETURN_LONG(-3);
    }
    ssize_t file_size = ftell(fp);

    // Подготовка буфера фиксированной длины для записи
    char *buffer = (char *)emalloc(line_length + 1);
    if (!buffer) {
        fclose(fp);
        zend_error(E_ERROR, "Out of memory to allocate %ld bytes", line_length + 1);
    }
    memset(buffer, ' ', line_length);

    // Копируем строку в подготовленный буфер (если длина строки меньше line_length, остаток останется пробелами)
    memcpy(buffer, line, (line_len < line_length) ? line_len : line_length);

    // Для режимов 0 и 2 – добавляем символ перевода строки в конец буфера
    if (mode == 0 || mode == 2) {
        buffer[line_length - 1] = '\n';
    }
    buffer[line_length] = '\0';

    // Запись подготовленного буфера в файл
    ssize_t written = fwrite(buffer, 1, line_length, fp);
    if (written != line_length) {
        php_error_docref(NULL, E_WARNING, "Failed to write to the file: %s", filename);
        // Откат файла до прежнего размера
        if (ftruncate(fileno(fp), file_size) == -1) {
            efree(buffer);
            fclose(fp);
            zend_error(E_ERROR, "Failed to truncate the file: %s", filename);
        }
        efree(buffer);
        fclose(fp);
        RETURN_LONG(-3);
    }

    fclose(fp);
    efree(buffer);

    // В зависимости от режима возвращаем либо смещение вставленной строки, либо номер строки с выравниванием
    if (mode > 1)
        RETURN_LONG(file_size);           // смещение начала записи
    else
        RETURN_LONG(file_size / line_length);  // номер строки (с учётом выравнивания)
}




PHP_FUNCTION(file_select_line)
{
    char *filename;
    size_t filename_len;
    ssize_t row;
    ssize_t align;
    size_t mode = 0;

    // Парсинг аргументов: строка, два числа, опционально mode
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "sll|l",
                              &filename, &filename_len, &row, &align, &mode) == FAILURE) {
        RETURN_FALSE;
    }

    // Проверка валидности аргументов
    if (align <= 0 || row < 0) {
        php_error_docref(NULL, E_WARNING, "Invalid align or row value");
        RETURN_FALSE;
    }
    if (filename_len > PATH_MAX - 1) {
        php_error_docref(NULL, E_WARNING, "Filename too long");
        RETURN_FALSE;
    }

    FILE *fp = fopen(filename, "r");
    if (!fp) {
        php_error_docref(NULL, E_WARNING, "Failed to open file: %s", filename);
        RETURN_FALSE;
    }

    // Если не log-режим, блокируем файл
    if (mode < 100) {
        if (flock(fileno(fp), LOCK_EX) == -1) {
            php_error_docref(NULL, E_WARNING, "Failed to lock file: %s", filename);
            fclose(fp);
            RETURN_FALSE;
        }
    }
    if (mode > 99) {
        mode -= 100;
    }

    // Вычисление смещения для выборки
    ssize_t position = 0;
    if (mode == 0 || mode == 2)
        position = row * align;
    else if (mode == 1 || mode == 3)
        position = row;
    else {
        fclose(fp);
        php_error_docref(NULL, E_WARNING, "Unknown mode: %zu", mode);
        RETURN_FALSE;
    }

    // Проверка границ файла
    if (fseek(fp, 0, SEEK_END) != 0) {
        php_error_docref(NULL, E_WARNING, "Failed to seek to end of file: %s", filename);
        fclose(fp);
        RETURN_FALSE;
    }
    ssize_t file_size = ftell(fp);
    if (position < 0 || position + align > file_size) {
        php_error_docref(NULL, E_WARNING, "Position out of bounds in file: %s", filename);
        fclose(fp);
        RETURN_FALSE;
    }
    // Устанавливаем указатель на нужное место
    if (fseek(fp, position, SEEK_SET) != 0) {
        php_error_docref(NULL, E_WARNING, "Failed to seek file: %s", filename);
        fclose(fp);
        RETURN_FALSE;
    }

    // Выделяем буфер для чтения строки
    char *buffer = (char *)emalloc(align + 1);
    if (!buffer) {
        fclose(fp);
        zend_error(E_ERROR, "Out of memory to allocate %ld bytes", align + 1);
    }

    ssize_t bytes_read = fread(buffer, 1, align, fp);
    fclose(fp);

    if (bytes_read != align) {
        php_error_docref(NULL, E_WARNING, "Failed to read %ld bytes from file: %s", align, filename);
        efree(buffer);
        RETURN_FALSE;
    }
    buffer[bytes_read] = '\0';

    // Для режимов 0 и 1 производим обрезку пробелов и символа перевода строки справа
    if (mode == 0 || mode == 1) {
        if (mode == 1) {
            char *line_end = strchr(buffer, '\n');
            if (line_end) {
                *line_end = '\0';
            }
        }
        for (ssize_t i = bytes_read - 1; i >= 0; --i) {
            if (buffer[i] == ' ' || buffer[i] == '\n' || buffer[i] == '\r') {
                buffer[i] = '\0';
            } else {
                break;
            }
        }
    }

    RETURN_STRING(buffer);
}



PHP_FUNCTION(file_update_line)
{
    char *filename;
    size_t filename_len;
    char *line;
    ssize_t line_len;
    ssize_t position, line_length;
    size_t mode = 0;

    // Аргументы: строка, строка, два числа, опционально mode
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "ssll|l",
                              &filename, &filename_len, &line, &line_len, &position, &line_length, &mode) == FAILURE) {
        RETURN_FALSE;
    }

    // Проверка валидности параметров
    if (line_length <= 0 || position < 0) {
        php_error_docref(NULL, E_WARNING, "Invalid line_length or position");
        RETURN_LONG(-8);
    }
    if (filename_len > PATH_MAX - 1) {
        php_error_docref(NULL, E_WARNING, "Filename too long");
        RETURN_LONG(-8);
    }

    FILE *fp = fopen(filename, "r+");
    if (!fp) {
        php_error_docref(NULL, E_WARNING, "Failed to open file: %s", filename);
        RETURN_LONG(-1);
    }

    // Блокировка файла, если не включён log-режим
    if (mode < 100) {
        if (flock(fileno(fp), LOCK_EX) == -1) {
            php_error_docref(NULL, E_WARNING, "Failed to lock file: %s", filename);
            fclose(fp);
            RETURN_LONG(-2);
        }
    }
    if (mode > 99) {
        mode -= 100;
    }

    // Проверка границ файла
    if (fseek(fp, 0, SEEK_END) != 0) {
        php_error_docref(NULL, E_WARNING, "Failed to seek end of file: %s", filename);
        fclose(fp);
        RETURN_LONG(-3);
    }
    ssize_t file_size = ftell(fp);
    if (position > file_size || fseek(fp, position, SEEK_SET) != 0) {
        php_error_docref(NULL, E_WARNING, "Failed to set file pointer in file: %s", filename);
        fclose(fp);
        RETURN_LONG(-3);
    }

    // Подготовка буфера для записи
    char *buffer = (char *)emalloc(line_length + 1);
    if (!buffer) {
        fclose(fp);
        zend_error(E_ERROR, "Out of memory to allocate %ld bytes", line_length + 1);
    }
    // Инициализируем буфер пробелами
    memset(buffer, ' ', line_length);

    // Копируем строку (если она короче, остаток останется пробелами, если длиннее – лишнее обрезается)
    if (line_len > 0) {
        memcpy(buffer, line, (line_len < line_length) ? line_len : line_length);
    }

    // Для режима 0 добавляем символ перевода строки в конец записи
    if (mode == 0) {
        buffer[line_length - 1] = '\n';
    }
    buffer[line_length] = '\0';

    // Запись в файл
    ssize_t written = fwrite(buffer, 1, line_length, fp);
    efree(buffer);
    fclose(fp);

    if (written != line_length) {
        php_error_docref(NULL, E_WARNING, "Failed to write to the file: %s", filename);
        RETURN_LONG(-4);
    }

    RETURN_LONG(written);
}
// path-injection Unvalidated input in path value creation risks unintended file/directory access




PHP_FUNCTION(file_analize)
{
    char *filename;
    size_t filename_len;
    size_t mode = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "s|l", &filename, &filename_len, &mode) == FAILURE) {
        RETURN_FALSE;
    }

    // Проверка длины имени файла для предотвращения path-injection
    if (filename_len > PATH_MAX - 1) {
        php_error_docref(NULL, E_WARNING, "Filename too long");
        RETURN_LONG(-8);
    }

    FILE *fp = fopen(filename, "r");
    if (!fp) {
        php_error_docref(NULL, E_WARNING, "Failed to open file: %s", filename);
        RETURN_LONG(-1);
    }

    // Устанавливаем блокировку, если не включён log-режим
    if (mode < 100) {
        if (flock(fileno(fp), LOCK_EX) == -1) {
            php_error_docref(NULL, E_WARNING, "Failed to lock file: %s", filename);
            fclose(fp);
            RETURN_LONG(-2);
        }
    }
    if (mode > 99) {
        mode -= 100;
    }

    // Получаем размер файла
    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        RETURN_LONG(-3);
    }
    ssize_t file_size = ftell(fp);
    if (file_size < 0) {
        fclose(fp);
        RETURN_LONG(-3);
    }
    if (file_size == 0) {
        fclose(fp);
        array_init(return_value);
        add_assoc_long(return_value, "min_length", 0);
        add_assoc_long(return_value, "min_length_offset", 0);
        add_assoc_long(return_value, "max_length", 0);
        add_assoc_long(return_value, "max_length_offset", 0);
        add_assoc_double(return_value, "avg_length", 0.0);
        add_assoc_long(return_value, "line_count", 0);
        add_assoc_long(return_value, "total_characters", 0);
        add_assoc_long(return_value, "flow_interruption", 0);
        add_assoc_long(return_value, "last_symbol", 0);
        add_assoc_long(return_value, "file_size", 0);
        RETURN_ZVAL(return_value, 0, 1);
    }

    // Читаем последний символ для проверки "разрыва потока"
    char last_symbol = 0;
    if (file_size > 0) {
        fseek(fp, file_size - 1, SEEK_SET);
        last_symbol = fgetc(fp);
    }
    fseek(fp, 0, SEEK_SET);

    ssize_t ini_buffer_size = FAST_IO_G(buffer_size);
    if (file_size < ini_buffer_size)
        ini_buffer_size = file_size;
    if (ini_buffer_size < 16)
        ini_buffer_size = 16;

    char *buffer = (char *)emalloc(ini_buffer_size + 1);
    if (!buffer) {
        fclose(fp);
        zend_error(E_ERROR, "Out of memory to allocate %ld bytes", ini_buffer_size + 1);
    }

    // Инициализация переменных для анализа
    size_t max_length = 0, max_length_offset = 0;
    size_t min_length = SIZE_MAX, min_length_offset = 0;
    size_t line_count = 0, total_characters = 0;
    double avg_length = 0.0;
    ssize_t flow_interruption = 0;
    ssize_t offset = 0;

    array_init(return_value);

    ssize_t current_size = 0;
    ssize_t bytes_read = 0;

    // Если mode == 1, анализируем только первую строку
    if (mode == 1) {
        if (fseek(fp, 0, SEEK_SET) != 0) {
            efree(buffer);
            fclose(fp);
            RETURN_LONG(-3);
        }
        if (fgets(buffer, ini_buffer_size, fp) != NULL) {
            ssize_t line_len = strlen(buffer);
            if (line_len > 0 && buffer[line_len - 1] == '\n') {
                // строка с переводом строки
            } else if (line_len > 0 && buffer[line_len - 1] != '\n') {
                // строка без перевода строки
            }
            min_length = max_length = line_len;
            min_length_offset = max_length_offset = 0;
            total_characters = line_len;
            avg_length = (double)line_len;
            line_count = 0;
        } else {
            min_length = max_length = min_length_offset = max_length_offset = total_characters = 0;
            avg_length = 0.0;
            line_count = 0;
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
        add_assoc_long(return_value, "flow_interruption", 0);
        add_assoc_long(return_value, "last_symbol", last_symbol);
        add_assoc_long(return_value, "file_size", file_size);
        RETURN_ZVAL(return_value, 0, 1);
    }

    // Читаем файл порциями, обрабатывая полные строки и сохраняя незавершённую строку в начале буфера
    while ((bytes_read = fread(buffer + current_size, 1, ini_buffer_size - current_size, fp)) > 0) {
        current_size += bytes_read;
        buffer[current_size] = '\0';

        ssize_t pos = 0;
        while (pos < current_size) {
            char *newline = memchr(buffer + pos, '\n', current_size - pos);
            if (!newline) break;
            ssize_t line_len = newline - (buffer + pos) + 1; // включая символ перевода строки
            line_count++;

            if ((size_t)line_len > max_length) {
                max_length = line_len;
                max_length_offset = offset;
            }
            if ((size_t)line_len < min_length) {
                min_length = line_len;
                min_length_offset = offset;
            }
            total_characters += line_len;
            avg_length = (double)total_characters / line_count;

            offset += line_len;
            pos += line_len;
        }
        // Переносим остаток незавершённой строки в начало буфера
        if (pos < current_size) {
            ssize_t remain = current_size - pos;
            memmove(buffer, buffer + pos, remain);
            current_size = remain;
        } else {
            current_size = 0;
        }
    }

    // Если в конце файла осталась незавершённая строка, учитываем её
    if (current_size > 0) {
        line_count++;
        if ((size_t)current_size > max_length) {
            max_length = current_size;
            max_length_offset = offset;
        }
        if ((size_t)current_size < min_length) {
            min_length = current_size;
            min_length_offset = offset;
        }
        total_characters += current_size;
        avg_length = (double)total_characters / line_count;
        offset += current_size;
    }

    efree(buffer);
    fclose(fp);

    if (file_size > total_characters)
        flow_interruption = file_size - total_characters;
    else
        flow_interruption = 0;

    add_assoc_long(return_value, "min_length", (min_length == SIZE_MAX) ? 0 : min_length);
    add_assoc_long(return_value, "min_length_offset", min_length_offset);
    add_assoc_long(return_value, "max_length", max_length);
    add_assoc_long(return_value, "max_length_offset", max_length_offset);
    add_assoc_double(return_value, "avg_length", avg_length);
    add_assoc_long(return_value, "line_count", line_count);
    add_assoc_long(return_value, "total_characters", total_characters);
    add_assoc_long(return_value, "flow_interruption", flow_interruption);
    add_assoc_long(return_value, "last_symbol", last_symbol);
    add_assoc_long(return_value, "file_size", file_size);

    RETURN_ZVAL(return_value, 0, 1);
}
// path-injection Unvalidated input in path value creation risks unintended file/directory access



PHP_FUNCTION(find_matches_pcre2)
{
    char *pattern;
    size_t pattern_len;
    char *subject;
    size_t subject_len;
    size_t mode = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "ss|l",
                              &pattern, &pattern_len, &subject, &subject_len, &mode) == FAILURE) {
        RETURN_FALSE;
    }

    pcre2_code *re = NULL;
    pcre2_match_data *match_data = NULL;
    PCRE2_SIZE erroffset;
    int errorcode;

    // Компилируем паттерн (используем pattern_len, а не PCRE2_ZERO_TERMINATED)
    re = pcre2_compile((PCRE2_SPTR)pattern, pattern_len, 0, &errorcode, &erroffset, NULL);
    if (re == NULL) {
        PCRE2_UCHAR message[256];
        pcre2_get_error_message(errorcode, message, sizeof(message));
        php_error_docref(NULL, E_WARNING, "PCRE2 compilation failed at offset %d: %s", (int)erroffset, message);
        RETURN_FALSE;
    }

    match_data = pcre2_match_data_create_from_pattern(re, NULL);
    if (!match_data) {
        pcre2_code_free(re);
        zend_error(E_ERROR, "Out of memory to allocate PCRE2 match data");
    }

    int rc;
    PCRE2_SIZE *ovector;
    size_t start_offset = 0;

    array_init(return_value);

    while ((rc = pcre2_match(re, (PCRE2_SPTR)subject, subject_len, start_offset, 0, match_data, NULL)) > 0) {
        ovector = pcre2_get_ovector_pointer(match_data);
        for (int i = 0; i < rc; i++) {
            PCRE2_SIZE start = ovector[2 * i];
            PCRE2_SIZE end   = ovector[2 * i + 1];

            if (start > end || end > subject_len)
                continue;

            if (mode == 1) {
                zval match_arr;
                array_init(&match_arr);
                add_assoc_stringl(&match_arr, "line_match", subject + start, end - start);
                add_assoc_long(&match_arr, "match_offset", (zend_long)start);
                add_assoc_long(&match_arr, "match_length", (zend_long)(end - start));
                add_next_index_zval(return_value, &match_arr);
            } else {
                add_next_index_stringl(return_value, subject + start, end - start);
            }
        }
        // Продвигаем смещение для следующего поиска
        if (ovector[1] > start_offset) {
            start_offset = ovector[1];
        } else {
            start_offset++;
            if (start_offset >= subject_len) break;
        }
    }

    if (rc < 0 && rc != PCRE2_ERROR_NOMATCH) {
        php_error_docref(NULL, E_WARNING, "Matching error %d", rc);
        pcre2_match_data_free(match_data);
        pcre2_code_free(re);
        RETURN_FALSE;
    }

    pcre2_match_data_free(match_data);
    pcre2_code_free(re);

    RETURN_ZVAL(return_value, 0, 1);
}



PHP_FUNCTION(replicate_file)
{
    char *source, *destination;
    size_t source_len, destination_len;
    size_t mode = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "ss|l",
                              &source, &source_len, &destination, &destination_len, &mode) == FAILURE) {
        RETURN_FALSE;
    }

    // Проверка длины имени файла для предотвращения path-injection
    if (source_len > PATH_MAX - 1 || destination_len > PATH_MAX - 1) {
        php_error_docref(NULL, E_WARNING, "Filename too long");
        RETURN_LONG(-8);
    }

    FILE *source_fp = fopen(source, "r");
    if (!source_fp) {
        php_error_docref(NULL, E_WARNING, "Failed to open source file: %s", source);
        RETURN_LONG(-1);
    }

    FILE *destination_fp = fopen(destination, "w");
    if (!destination_fp) {
        php_error_docref(NULL, E_WARNING, "Failed to open destination file: %s", destination);
        fclose(source_fp);
        RETURN_LONG(-2);
    }

    // Блокировка исходного файла (и индексного, если mode==1)
    if (flock(fileno(source_fp), LOCK_EX) == -1) {
        php_error_docref(NULL, E_WARNING, "Failed to lock source file: %s", source);
        fclose(source_fp);
        fclose(destination_fp);
        unlink(destination);
        RETURN_LONG(-3);
    }

    FILE *index_source_fp = NULL;
    FILE *index_destination_fp = NULL;
    char index_source[PATH_MAX];
    char index_destination[PATH_MAX];

    if (mode == 1) {
        if (source_len + 7 >= sizeof(index_source) || destination_len + 7 >= sizeof(index_destination)) {
            php_error_docref(NULL, E_WARNING, "Filename too long for index file");
            fclose(source_fp);
            fclose(destination_fp);
            unlink(destination);
            RETURN_LONG(-8);
        }
        snprintf(index_source, sizeof(index_source), "%s.index", source);
        snprintf(index_destination, sizeof(index_destination), "%s.index", destination);

        index_source_fp = fopen(index_source, "r");
        if (!index_source_fp) {
            php_error_docref(NULL, E_WARNING, "Failed to open index source file: %s", index_source);
            fclose(source_fp);
            fclose(destination_fp);
            unlink(destination);
            unlink(index_destination);
            RETURN_LONG(-1);
        }

        index_destination_fp = fopen(index_destination, "w");
        if (!index_destination_fp) {
            php_error_docref(NULL, E_WARNING, "Failed to open index destination file: %s", index_destination);
            fclose(index_source_fp);
            fclose(source_fp);
            fclose(destination_fp);
            unlink(destination);
            unlink(index_destination);
            RETURN_LONG(-2);
        }

        if (flock(fileno(index_source_fp), LOCK_EX) == -1) {
            php_error_docref(NULL, E_WARNING, "Failed to lock index source file: %s", index_source);
            fclose(index_source_fp);
            fclose(index_destination_fp);
            fclose(source_fp);
            fclose(destination_fp);
            unlink(destination);
            unlink(index_destination);
            RETURN_LONG(-3);
        }
    }

    // Получаем размер исходного файла
    if (fseek(source_fp, 0, SEEK_END) != 0) {
        php_error_docref(NULL, E_WARNING, "Failed to seek source file: %s", source);
        if (index_source_fp) fclose(index_source_fp);
        if (index_destination_fp) fclose(index_destination_fp);
        fclose(source_fp);
        fclose(destination_fp);
        unlink(destination);
        if (mode == 1) unlink(index_destination);
        RETURN_LONG(-4);
    }
    ssize_t file_size = ftell(source_fp);
    fseek(source_fp, 0, SEEK_SET);

    ssize_t ini_buffer_size = FAST_IO_G(buffer_size);
    if (file_size < ini_buffer_size)
        ini_buffer_size = file_size;
    if (ini_buffer_size < 16)
        ini_buffer_size = 16;
    char *dynamic_buffer = (char *)emalloc(ini_buffer_size + 1);
    if (!dynamic_buffer) {
        if (index_source_fp) fclose(index_source_fp);
        if (index_destination_fp) fclose(index_destination_fp);
        fclose(source_fp);
        fclose(destination_fp);
        unlink(destination);
        if (mode == 1) unlink(index_destination);
        zend_error(E_ERROR, "Out of memory to allocate %ld bytes", ini_buffer_size + 1);
    }

    ssize_t bytes_read, bytes_write;
    ssize_t total_written = 0;

    // Копируем основной файл
    while ((bytes_read = fread(dynamic_buffer, 1, ini_buffer_size, source_fp)) > 0) {
        bytes_write = fwrite(dynamic_buffer, 1, bytes_read, destination_fp);
        if (bytes_read != bytes_write) {
            php_error_docref(NULL, E_WARNING, "Failed to write to destination file: %s", destination);
            fclose(source_fp);
            fclose(destination_fp);
            if (index_source_fp) fclose(index_source_fp);
            if (index_destination_fp) fclose(index_destination_fp);
            efree(dynamic_buffer);
            unlink(destination);
            if (mode == 1) unlink(index_destination);
            RETURN_LONG(-4);
        }
        total_written += bytes_write;
    }

    // Копируем индексный файл, если mode==1
    if (mode == 1 && index_source_fp && index_destination_fp) {
        while ((bytes_read = fread(dynamic_buffer, 1, ini_buffer_size, index_source_fp)) > 0) {
            bytes_write = fwrite(dynamic_buffer, 1, bytes_read, index_destination_fp);
            if (bytes_read != bytes_write) {
                php_error_docref(NULL, E_WARNING, "Failed to write to index destination file: %s", index_destination);
                fclose(source_fp);
                fclose(destination_fp);
                fclose(index_source_fp);
                fclose(index_destination_fp);
                efree(dynamic_buffer);
                unlink(destination);
                unlink(index_destination);
                RETURN_LONG(-4);
            }
        }
        fclose(index_source_fp);
        fclose(index_destination_fp);
    }

    fclose(source_fp);
    fclose(destination_fp);
    efree(dynamic_buffer);

    RETURN_LONG(total_written);
}
// path-injection Unvalidated input in path value creation risks unintended file/directory access




PHP_FUNCTION(file_select_array) {
    char  *filename;
    size_t filename_len;
    char *pattern = NULL;
    size_t pattern_len = 0;
    zval  *array = NULL;
    size_t mode = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "sa|sl",
                              &filename, &filename_len,
                              &array, &pattern, &pattern_len, &mode) == FAILURE) {
        RETURN_FALSE;
    }

    // Проверка длины имени файла
    if (filename_len > PATH_MAX - 1) {
        php_error_docref(NULL, E_WARNING, "Filename too long");
        RETURN_FALSE;
    }

    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        php_error_docref(NULL, E_WARNING, "Failed to open file: %s", filename);
        RETURN_FALSE;
    }

    if (mode < 100) {
        if (flock(fileno(fp), LOCK_EX) == -1) {
            php_error_docref(NULL, E_WARNING, "Failed to lock file: %s", filename);
            fclose(fp);
            RETURN_FALSE;
        }
    }
    if (mode > 99)
        mode -= 100;

    if (fseek(fp, 0, SEEK_END) != 0) {
        php_error_docref(NULL, E_WARNING, "Failed to seek file: %s", filename);
        fclose(fp);
        RETURN_FALSE;
    }
    ssize_t file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    array_init(return_value);
    zval *elem, *value;
    ssize_t bytes_read;
    size_t found_count = 0;
    size_t line_count = 0;

    pcre2_code *re = NULL;
    pcre2_match_data *match_data = NULL;

    if (mode > 9 && pattern && pattern_len > 0) {
        PCRE2_SIZE erroffset;
        int errorcode;
        re = pcre2_compile((PCRE2_SPTR)pattern, pattern_len, 0, &errorcode, &erroffset, NULL);
        if (re == NULL) {
            PCRE2_UCHAR message[256];
            pcre2_get_error_message(errorcode, message, sizeof(message));
            php_error_docref(NULL, E_WARNING, "PCRE2 compilation failed at offset %d: %s", (int)erroffset, message);
            fclose(fp);
            RETURN_FALSE;
        }
        match_data = pcre2_match_data_create_from_pattern(re, NULL);
        if (!match_data) {
            pcre2_code_free(re);
            fclose(fp);
            zend_error(E_ERROR, "Out of memory to allocate PCRE2 match data");
        }
    }

    if (array && Z_TYPE_P(array) == IS_ARRAY) {
        ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(array), elem) {
            if (Z_TYPE_P(elem) == IS_ARRAY) {
                int num_elem = 0;
                ssize_t select_pos = -1;
                ssize_t select_size = -1;

                ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(elem), value) {
                    if (Z_TYPE_P(value) == IS_LONG) {
                        if (num_elem == 0)
                            select_pos = Z_LVAL_P(value);
                        if (num_elem == 1)
                            select_size = Z_LVAL_P(value);
                        num_elem++;
                    }
                } ZEND_HASH_FOREACH_END();

                if (select_pos >= 0 && select_size > 0 && file_size >= select_pos + select_size) {
                    line_count++;
                    bool found_match = false;

                    char *buffer = (char *)emalloc(select_size + 1);
                    if (!buffer) {
                        fclose(fp);
                        if (re) pcre2_code_free(re);
                        if (match_data) pcre2_match_data_free(match_data);
                        zend_error(E_ERROR, "Out of memory to allocate %ld bytes", select_size + 1);
                    }

                    if (fseek(fp, select_pos, SEEK_SET) != 0) {
                        php_error_docref(NULL, E_WARNING, "Failed to seek file: %s", filename);
                        efree(buffer);
                        fclose(fp);
                        if (re) pcre2_code_free(re);
                        if (match_data) pcre2_match_data_free(match_data);
                        RETURN_FALSE;
                    }
                    bytes_read = fread(buffer, 1, select_size, fp);
                    if (bytes_read != select_size) {
                        php_error_docref(NULL, E_WARNING, "Failed to read file: %s", filename);
                        fclose(fp);
                        efree(buffer);
                        if (re) pcre2_code_free(re);
                        if (match_data) pcre2_match_data_free(match_data);
                        RETURN_FALSE;
                    }
                    buffer[bytes_read] = '\0';

                    zval line_arr;
                    array_init(&line_arr);

                    ssize_t i = 0;
                    if (mode == 0 || mode == 2 || mode == 5 || mode == 7 ||
                        mode == 10 || mode == 12 || mode == 20 || mode == 24) {
                        for (i = bytes_read - 1; i >= 0; --i) {
                            if (buffer[i] == ' ' || buffer[i] == '\n')
                                buffer[i] = '\0';
                            else
                                break;
                        }
                    }

                    if (mode == 0) {
                        add_assoc_string(&line_arr, "trim_line", buffer);
                        add_assoc_long(&line_arr, "trim_length", i + 1);
                        found_match = true;
                    }
                    if (mode == 1) {
                        add_assoc_string(&line_arr, "line", buffer);
                        found_match = true;
                    }
                    if (mode == 2) {
                        add_next_index_string(return_value, buffer);
                        found_match = false;
                    }
                    if (mode == 3 && pattern && strstr(buffer, pattern) != NULL) {
                        found_count++;
                        found_match = false;
                    }
                    if (mode == 5 && pattern && strstr(buffer, pattern) != NULL) {
                        add_assoc_string(&line_arr, "trim_line", buffer);
                        add_assoc_long(&line_arr, "trim_length", i + 1);
                        found_match = true;
                    }
                    if (mode == 6) {
                        add_assoc_string(&line_arr, "line", buffer);
                        found_match = true;
                    }
                    if (mode == 7 && pattern && strstr(buffer, pattern) != NULL) {
                        add_next_index_string(return_value, buffer);
                        found_match = false;
                    }
                    if (mode == 0 || mode == 5 || mode == 1 || mode == 6) {
                        add_assoc_long(&line_arr, "line_offset", select_pos);
                        add_assoc_long(&line_arr, "line_length", select_size);
                    }
                    if (mode > 9 && mode < 14 && re && match_data) {
                        int rc;
                        if ((rc = pcre2_match(re, (PCRE2_SPTR)buffer, select_size, 0, 0, match_data, NULL)) > 0) {
                            if (mode < 13) {
                                if (mode == 10) {
                                    add_assoc_string(&line_arr, "trim_line", buffer);
                                    add_assoc_long(&line_arr, "trim_length", i + 1);
                                    found_match = true;
                                }
                                if (mode == 11) {
                                    add_assoc_string(&line_arr, "line", buffer);
                                    found_match = true;
                                }
                                if (mode < 12) {
                                    add_assoc_long(&line_arr, "line_offset", select_pos);
                                    add_assoc_long(&line_arr, "line_length", select_size);
                                }
                                if (mode == 12) {
                                    add_next_index_string(return_value, buffer);
                                    found_match = false;
                                }
                            }
                            if (mode == 13) {
                                found_count++;
                                found_match = false;
                            }
                        }
                    }
                    if (mode > 19 && mode < 24 && re && match_data) {
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
                                if (mode > 21) {
                                    zval match_arr;
                                    array_init(&match_arr);
                                    if (mode == 23) {
                                        add_next_index_stringl(&return_matched, buffer + start, end - start);
                                    } else {
                                        add_assoc_stringl(&match_arr, "line_match", buffer + start, end - start);
                                    }
                                    if (mode != 23) {
                                        add_assoc_long(&match_arr, "match_offset", start);
                                        add_assoc_long(&match_arr, "match_length", end - start);
                                        add_next_index_zval(&return_matched, &match_arr);
                                    }
                                } else {
                                    add_next_index_stringl(&return_matched, buffer + start, end - start);
                                }
                            }
                            if (ovector[1] > start_offset) {
                                start_offset = ovector[1];
                            } else {
                                start_offset++;
                                if ((ssize_t)start_offset >= bytes_read)
                                    break;
                            }
                            found_match = true;
                        }
                        if (rc == PCRE2_ERROR_NOMATCH) {
                            /* Нет совпадений */
                        } else if (rc < 0) {
                            php_error_docref(NULL, E_WARNING, "Matching error %d", rc);
                            fclose(fp);
                            efree(buffer);
                            if (re) pcre2_code_free(re);
                            if (match_data) pcre2_match_data_free(match_data);
                            RETURN_FALSE;
                        }
                        if (found_match) {
                            if (mode == 20) {
                                add_assoc_string(&line_arr, "trim_line", buffer);
                                add_assoc_long(&line_arr, "trim_length", i + 1);
                            }
                            if (mode == 21) {
                                add_assoc_string(&line_arr, "line", buffer);
                            }
                            if (mode != 23) {
                                add_assoc_zval(&line_arr, "line_matches", &return_matched);
                                add_assoc_long(&line_arr, "line_offset", select_pos);
                                add_assoc_long(&line_arr, "line_length", select_size);
                            }
                            if (mode == 23) {
                                add_next_index_zval(return_value, &return_matched);
                                found_match = false;
                            }
                        }
                    }
                    if (found_match) {
                        add_next_index_zval(return_value, &line_arr);
                    }
                    efree(buffer);
                }
            }
        } ZEND_HASH_FOREACH_END();
    }
    fclose(fp);
    if (mode > 9 && re)
        pcre2_code_free(re);
    if (mode > 9 && match_data)
        pcre2_match_data_free(match_data);

    if (mode == 3 || mode == 13) {
        add_assoc_long(return_value, "line_count", line_count);
        add_assoc_long(return_value, "found_count", found_count);
    }
    if (Z_TYPE_P(return_value) == IS_ARRAY &&
        zend_hash_num_elements(Z_ARRVAL_P(return_value)) == 0) {
        zval_ptr_dtor(return_value);
        RETURN_FALSE;
    }
    RETURN_ZVAL(return_value, 0, 1);
}
// path-injection Unvalidated input in path value creation risks unintended file/directory access



PHP_FUNCTION(file_update_array) {
    char *filename;
    size_t filename_len;
    zval *query = NULL;
    size_t mode = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "sa|l", &filename, &filename_len, &query, &mode) == FAILURE) {
        RETURN_FALSE;
    }

    // Проверка длины имени файла для предотвращения path-injection
    if (filename_len > PATH_MAX - 1) {
        php_error_docref(NULL, E_WARNING, "Filename too long");
        RETURN_LONG(-8);
    }

    FILE *fp = fopen(filename, "r+");
    if (!fp) {
        php_error_docref(NULL, E_WARNING, "Failed to open file: %s", filename);
        RETURN_LONG(-1);
    }

    // Если не включён log-режим, блокируем файл для записи.
    if (mode < 100) {
        if (flock(fileno(fp), LOCK_EX) == -1) {
            php_error_docref(NULL, E_WARNING, "Failed to lock file: %s", filename);
            fclose(fp);
            RETURN_LONG(-2);
        }
    }
    if (mode > 99) {
        mode -= 100;
    }

    // Получаем размер файла для проверки границ.
    if (fseek(fp, 0, SEEK_END) != 0) {
        php_error_docref(NULL, E_WARNING, "Failed to seek file end: %s", filename);
        fclose(fp);
        RETURN_LONG(-3);
    }
    ssize_t file_size = ftell(fp);
    if (file_size < 0) {
        php_error_docref(NULL, E_WARNING, "Failed to get file size: %s", filename);
        fclose(fp);
        RETURN_LONG(-3);
    }

    ssize_t total_written = 0;

    if (query && Z_TYPE_P(query) == IS_ARRAY) {
        zval *elem;
        ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(query), elem) {
            if (Z_TYPE_P(elem) == IS_ARRAY) {
                zval *value;
                char *line_value = NULL;
                ssize_t update_pos = -1, update_size = -1;
                ssize_t str_len = 0;
                int idx = 0;

                ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(elem), value) {
                    if (idx == 0 && Z_TYPE_P(value) == IS_STRING) {
                        line_value = Z_STRVAL_P(value);
                        str_len = Z_STRLEN_P(value);
                    } else if (idx == 1 && Z_TYPE_P(value) == IS_LONG) {
                        update_pos = Z_LVAL_P(value);
                    } else if (idx == 2 && Z_TYPE_P(value) == IS_LONG) {
                        update_size = Z_LVAL_P(value);
                    }
                    idx++;
                } ZEND_HASH_FOREACH_END();

                if (update_pos >= 0 && update_size > 0 && line_value != NULL && update_pos + update_size <= file_size) {
                    if (update_size < str_len)
                        str_len = update_size;

                    char *buffer = (char *)emalloc(update_size + 1);
                    if (!buffer) {
                        fclose(fp);
                        zend_error(E_ERROR, "Out of memory to allocate %ld bytes", update_size + 1);
                    }
                    memset(buffer, ' ', update_size);
                    buffer[update_size] = '\0';
                    memcpy(buffer, line_value, str_len);
                    if (mode == 0 && update_size > 0)
                        buffer[update_size - 1] = '\n';

                    if (fseek(fp, update_pos, SEEK_SET) != 0) {
                        php_error_docref(NULL, E_WARNING, "Failed to seek in the file: %s", filename);
                        efree(buffer);
                        fclose(fp);
                        RETURN_LONG(-3);
                    }

                    ssize_t bytes_written = fwrite(buffer, 1, update_size, fp);
                    if (bytes_written != update_size) {
                        php_error_docref(NULL, E_WARNING, "Failed to write to the file: %s", filename);
                        efree(buffer);
                        fclose(fp);
                        RETURN_LONG(-4);
                    }
                    total_written += bytes_written;
                    efree(buffer);
                }
            }
        } ZEND_HASH_FOREACH_END();
    }

    fclose(fp);
    RETURN_LONG(total_written);
}




PHP_FUNCTION(file_callback_line) {
    char *filename;
    size_t filename_len;
    ssize_t position = 0;
    size_t mode = 0;
    zval *callback;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "sz|ll",
                              &filename, &filename_len, &callback, &position, &mode) == FAILURE) {
        RETURN_FALSE;
    }

    if (!zend_is_callable(callback, 0, NULL)) {
        php_error_docref(NULL, E_WARNING, "The passed parameter is not a callback function");
        RETURN_FALSE;
    }

    // Проверка длины имени файла для предотвращения path-injection
    if (filename_len > PATH_MAX - 1) {
        php_error_docref(NULL, E_WARNING, "Filename too long");
        RETURN_FALSE;
    }

    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        php_error_docref(NULL, E_WARNING, "Failed to open file: %s", filename);
        RETURN_FALSE;
    }

    if (mode < 100) {
        if (flock(fileno(fp), LOCK_EX) == -1) {
            php_error_docref(NULL, E_WARNING, "Failed to lock file: %s", filename);
            fclose(fp);
            RETURN_FALSE;
        }
    }
    if (mode > 99) {
        mode -= 100;
    }

    if (fseek(fp, 0, SEEK_END) != 0) {
        php_error_docref(NULL, E_WARNING, "Failed to seek file: %s", filename);
        fclose(fp);
        RETURN_FALSE;
    }
    ssize_t file_size = ftell(fp);
    ssize_t line_offset = 0;
    if (position > 0) {
        if (position >= file_size) {
            php_error_docref(NULL, E_WARNING, "Position exceeds file size: %s", filename);
            fclose(fp);
            RETURN_FALSE;
        }
        fseek(fp, position, SEEK_SET);
        line_offset = position;
    } else {
        fseek(fp, 0, SEEK_SET);
    }

    ssize_t ini_buffer_size = FAST_IO_G(buffer_size);
    if (file_size < ini_buffer_size) ini_buffer_size = file_size;
    if (ini_buffer_size < 16) ini_buffer_size = 16;
    ssize_t dynamic_buffer_size = ini_buffer_size;
    char *dynamic_buffer = (char *)emalloc(dynamic_buffer_size + 1);
    if (!dynamic_buffer) {
        fclose(fp);
        zend_error(E_ERROR, "Out of memory to allocate %ld bytes", dynamic_buffer_size + 1);
    }
    dynamic_buffer[0] = '\0';

    char *found_value = (char *)emalloc(1);
    if (!found_value) {
        efree(dynamic_buffer);
        fclose(fp);
        zend_error(E_ERROR, "Out of memory to allocate 1 byte");
    }
    found_value[0] = '\0';

    ssize_t bytes_read, current_size = 0;
    size_t line_count = 0;
    bool found_match = false, jump = false;

    while ((bytes_read = fread(dynamic_buffer + current_size, 1, ini_buffer_size, fp)) > 0) {
        current_size += bytes_read;
        dynamic_buffer[current_size] = '\0';

        char *line_start = dynamic_buffer;
        char *line_end = NULL;
        while ((line_end = strchr(line_start, '\n')) != NULL) {
            ssize_t line_length = line_end - line_start + 1;
            *line_end = '\0';

            zval retval;
            zval args[10];
            int num_args = (mode > 9 ? 10 : (int)mode + 1);

            ZVAL_STRING(&args[0], line_start);
            if (num_args > 1) ZVAL_STRING(&args[1], filename);
            if (num_args > 2) ZVAL_LONG(&args[2], line_offset);
            if (num_args > 3) ZVAL_LONG(&args[3], line_length);
            if (num_args > 4) ZVAL_LONG(&args[4], line_count);
            if (num_args > 5) ZVAL_LONG(&args[5], position);
            if (num_args > 6) ZVAL_STRING(&args[6], found_value);
            if (num_args > 7) ZVAL_LONG(&args[7], file_size);
            if (num_args > 8) ZVAL_LONG(&args[8], dynamic_buffer_size);
            if (num_args > 9) ZVAL_STRING(&args[9], dynamic_buffer);

            if (call_user_function(EG(function_table), NULL, callback, &retval, num_args, args) == SUCCESS) {
                if (Z_TYPE(retval) == IS_STRING) {
                    size_t new_len = Z_STRLEN(retval);
                    char *temp = erealloc(found_value, new_len + 1);
                    if (!temp) {
                        for (int i = 0; i < num_args; i++) zval_dtor(&args[i]);
                        zval_dtor(&retval);
                        efree(dynamic_buffer);
                        efree(found_value);
                        fclose(fp);
                        zend_error(E_ERROR, "Out of memory to allocate %zu bytes", new_len + 1);
                    }
                    found_value = temp;
                    memcpy(found_value, Z_STRVAL(retval), new_len);
                    found_value[new_len] = '\0';
                } else if (Z_TYPE(retval) == IS_LONG) {
                    ssize_t new_pos = Z_LVAL(retval);
                    if (new_pos < 0 || new_pos >= file_size) {
                        for (int i = 0; i < num_args; i++) zval_dtor(&args[i]);
                        zval_dtor(&retval);
                        efree(dynamic_buffer);
                        efree(found_value);
                        fclose(fp);
                        php_error_docref(NULL, E_WARNING, "Invalid new position from callback for file: %s", filename);
                        RETURN_FALSE;
                    }
                    position = new_pos;
                    fseek(fp, position, SEEK_SET);
                    line_offset = position;
                    jump = true;
                } else if (Z_TYPE(retval) == IS_FALSE) {
                    found_match = true;
                }
            } else {
                php_error_docref(NULL, E_WARNING, "Failed to call callback function");
                for (int i = 0; i < num_args; i++) zval_dtor(&args[i]);
                zval_dtor(&retval);
                efree(dynamic_buffer);
                efree(found_value);
                fclose(fp);
                RETURN_FALSE;
            }

            zval_dtor(&retval);
            for (int i = 0; i < num_args; i++) zval_dtor(&args[i]);

            line_count++;
            line_start = line_end + 1;
            if (jump) {
                jump = false;
                break;
            } else {
                line_offset += line_length;
            }
            if (found_match) break;
        }
        if (found_match) break;

        current_size -= (line_start - dynamic_buffer);
        memmove(dynamic_buffer, line_start, current_size);
        if (current_size + ini_buffer_size > dynamic_buffer_size) {
            dynamic_buffer_size += ini_buffer_size;
            char *temp_buffer = erealloc(dynamic_buffer, dynamic_buffer_size + 1);
            if (!temp_buffer) {
                efree(dynamic_buffer);
                efree(found_value);
                fclose(fp);
                zend_error(E_ERROR, "Out of memory to allocate %ld bytes", dynamic_buffer_size + 1);
            }
            dynamic_buffer = temp_buffer;
        }
    }

    efree(dynamic_buffer);
    fclose(fp);
    RETURN_STRING(found_value);
}
// path-injection Unvalidated input in path value creation risks unintended file/directory access
