#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "fast_io.h"

/* Декларация функций */
PHP_FUNCTION(find_value_by_key);
PHP_FUNCTION(indexed_find_value_by_key);
PHP_FUNCTION(write_key_value_pair);
PHP_FUNCTION(indexed_write_key_value_pair);
PHP_FUNCTION(delete_key_value_pair);
PHP_FUNCTION(rebuild_data_file);
PHP_FUNCTION(pop_key_value_pair);
PHP_FUNCTION(hide_key_value_pair);
PHP_FUNCTION(get_index_keys);


/* Запись аргументов функций */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_find_value_by_key, 0, 2, IS_STRING, 1)
    ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, index_key, IS_STRING, 0)
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

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_delete_key_value_pair, 0, 2, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, index_key, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_rebuild_data_file, 0, 2, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, index_key, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_pop_key_value_pair, 0, 1, IS_STRING, 1)
    ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_hide_key_value_pair, 0, 2, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, index_key, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_get_index_keys, 0, 1, IS_ARRAY, 0)
    ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
ZEND_END_ARG_INFO()


/* Регистрация функций */
const zend_function_entry fast_io_functions[] = {
    PHP_FE(find_value_by_key, arginfo_find_value_by_key)
    PHP_FE(indexed_find_value_by_key, arginfo_indexed_find_value_by_key)
    PHP_FE(write_key_value_pair, arginfo_write_key_value_pair)
    PHP_FE(indexed_write_key_value_pair, arginfo_indexed_write_key_value_pair)
    PHP_FE(delete_key_value_pair, arginfo_delete_key_value_pair)
    PHP_FE(rebuild_data_file, arginfo_rebuild_data_file)
    PHP_FE(pop_key_value_pair, arginfo_pop_key_value_pair)
    PHP_FE(hide_key_value_pair, arginfo_hide_key_value_pair)
    PHP_FE(get_index_keys, arginfo_get_index_keys)
    PHP_FE_END
};

/* Определение модуля */
zend_module_entry fast_io_module_entry = {
    STANDARD_MODULE_HEADER,
    "fast_io",
    fast_io_functions,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NO_VERSION_YET,
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_FAST_IO
ZEND_GET_MODULE(fast_io)
#endif

/* Реализация функций PHP */
PHP_FUNCTION(find_value_by_key) {
    char *filename;
    size_t filename_len;
    char *index_key;
    size_t index_key_len;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "ss", &filename, &filename_len, &index_key, &index_key_len) == FAILURE) {
        RETURN_FALSE;
    }

    char *result = find_value_by_key(filename, index_key);
    
    if (result != NULL) {
        RETVAL_STRING(result);
        free(result);
    } else {
        RETURN_NULL();
    }
}

PHP_FUNCTION(indexed_find_value_by_key) {
    char *filename;
    size_t filename_len;
    char *index_key;
    size_t index_key_len;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "ss", &filename, &filename_len, &index_key, &index_key_len) == FAILURE) {
        RETURN_FALSE;
    }

    char *result = indexed_find_value_by_key(filename, index_key);
    
    if (result != NULL) {
        RETVAL_STRING(result);
        free(result);
    } else {
        RETURN_NULL();
    }
}

PHP_FUNCTION(write_key_value_pair) {
    char *filename;
    size_t filename_len;
    char *index_key;
    size_t index_key_len;
    char *index_val;
    size_t index_val_len;
    long result;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "sss", &filename, &filename_len, &index_key, &index_key_len, &index_val, &index_val_len) == FAILURE) {
        RETURN_FALSE;
    }

    result = write_key_value_pair(filename, index_key, index_val);

    if (result) {
        RETURN_LONG(result);
    } else {
        RETURN_NULL();
    }
}

PHP_FUNCTION(indexed_write_key_value_pair) {
    char *filename;
    size_t filename_len;
    char *index_key;
    size_t index_key_len;
    char *index_val;
    size_t index_val_len;
    long result;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "sss", &filename, &filename_len, &index_key, &index_key_len, &index_val, &index_val_len) == FAILURE) {
        RETURN_FALSE;
    }

    result = indexed_write_key_value_pair(filename, index_key, index_val);
    
    if (result) {
        RETURN_LONG(result);
    } else {
        RETURN_NULL();
    }
}

PHP_FUNCTION(delete_key_value_pair) {
    char *filename;
    size_t filename_len;
    char *index_key;
    size_t index_key_len;
    long result;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "ss", &filename, &filename_len, &index_key, &index_key_len) == FAILURE) {
        RETURN_FALSE;
    }

    result = delete_key_value_pair(filename, index_key);
    
    if (result) {
        RETURN_LONG(result);
    } else {
        RETURN_NULL();
    }
} 

PHP_FUNCTION(rebuild_data_file) {
    char *filename;
    size_t filename_len;
    char *index_key;
    size_t index_key_len;
    long result;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "ss", &filename, &filename_len,  &index_key, &index_key_len) == FAILURE) {
        RETURN_FALSE;
    }

    result = rebuild_data_file(filename, index_key);
    
    if (result) {
        RETURN_LONG(result);
    } else {
        RETURN_NULL();
    }
} 


PHP_FUNCTION(pop_key_value_pair) {
    char *filename;
    size_t filename_len;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &filename, &filename_len) == FAILURE) {
        RETURN_FALSE;
    }

    char *result = pop_key_value_pair(filename);
    
    if (result != NULL) {
        RETVAL_STRING(result);
        free(result);
    } else {
        RETURN_NULL();
    }
}

PHP_FUNCTION(hide_key_value_pair) {
    char *filename;
    size_t filename_len;
    char *index_key;
    size_t index_key_len;
    long result;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "ss", &filename, &filename_len, &index_key, &index_key_len) == FAILURE) {
        RETURN_FALSE;
    }

    result = hide_key_value_pair(filename, index_key);
    
    if (result) {
        RETURN_LONG(result);
    } else {
        RETURN_NULL();
    }
} 


PHP_FUNCTION(get_index_keys) {
    char *filename;
    size_t filename_len;

    // Парсинг и проверка аргументов
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &filename, &filename_len) == FAILURE) {
        return; // Если аргументы не соответствуют ожидаемым, возвращаемся
    }

    // Вызов вашей функции C для получения массива ключей
    KeyArray keys = get_index_keys(filename);

    // Инициализация возвращаемого массива
    array_init(return_value);

    // Добавление ключей в возвращаемый массив
    for (size_t i = 0; i < keys.count; i++) {
        add_next_index_string(return_value, keys.keys[i]);
    }

    // Освобождение памяти
    free_key_array(&keys);
}

