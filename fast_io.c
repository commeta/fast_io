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
PHP_FUNCTION(update_key_value_pair);
PHP_FUNCTION(insert_key_value);
PHP_FUNCTION(select_key_value);
PHP_FUNCTION(update_key_value);


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

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_select_key_value, 0, 3, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, index_row, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, index_align, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_update_key_value, 0, 3, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, index_key, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, index_align, IS_LONG, 0)
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
    PHP_FE(update_key_value_pair, arginfo_update_key_value_pair)
    PHP_FE(insert_key_value, arginfo_insert_key_value)
    PHP_FE(select_key_value, arginfo_select_key_value)
    PHP_FE(update_key_value, arginfo_update_key_value)
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


PHP_FUNCTION(update_key_value_pair) {
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

    result = update_key_value_pair(filename, index_key, index_val);
    
    if (result) {
        RETURN_LONG(result);
    } else {
        RETURN_NULL();
    }
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

    int fd = open(filename, O_WRONLY | O_APPEND | O_CREAT, 0666);
    if (fd == -1) {
        php_error_docref(NULL, E_WARNING, "Unable to open file: %s", strerror(errno));
        RETURN_LONG(-1);
    }

    // Блокировка файла для записи
    if (lock_file(fd, F_WRLCK) == -1) {
        php_error_docref(NULL, E_WARNING, "Unable to lock file: %s", strerror(errno));
        close(fd);
        RETURN_LONG(-2);
    }

    // Получение размера файла для определения номера строки
    struct stat st;
    if (fstat(fd, &st) != 0) {
        php_error_docref(NULL, E_WARNING, "Unable to get file size: %s", strerror(errno));
        close(fd);
        RETURN_LONG(-3);
    }
    off_t file_size = st.st_size;
    long line_number = file_size / (index_align + 1); // Учет символа перевода строки

    // Подготовка строки к записи с учетом выравнивания
    char *buffer = (char *)emalloc(index_align + 2); // +1 для '\n' и +1 для '\0'
    memset(buffer, ' ', index_align); // Заполнение пробелами
    buffer[index_align] = '\n'; // Добавление перевода строки
    buffer[index_align + 1] = '\0';
    
    // Копирование index_key в буфер с учетом выравнивания
    strncpy(buffer, index_key, index_align < index_key_len ? index_align : index_key_len);

    // Запись в файл
    ssize_t written = write(fd, buffer, index_align + 1); // +1 для записи '\n'
    efree(buffer);
    close(fd); // Это также разблокирует файл

    if (written != index_align + 1) {
        php_error_docref(NULL, E_WARNING, "Error writing to file: %s", strerror(errno));
        RETURN_LONG(-4);
    }

    // Возврат номера добавленной строки
    RETURN_LONG(line_number + 1); // Нумерация строк начинается с 1
}

PHP_FUNCTION(select_key_value) {
    char *filename;
    size_t filename_len;
    zend_long index_row;
    zend_long index_align;

    // Парсинг переданных аргументов
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "sll", &filename, &filename_len, &index_row, &index_align) == FAILURE) {
        RETURN_FALSE;
    }

    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        php_error_docref(NULL, E_WARNING, "Unable to open file: %s", strerror(errno));
        RETURN_FALSE;
    }

    // Блокировка файла для чтения
    if (lock_file(fd, F_RDLCK) == -1) {
        php_error_docref(NULL, E_WARNING, "Unable to lock file: %s", strerror(errno));
        close(fd);
        RETURN_FALSE;
    }

    // Учет символа перевода строки при вычислении смещения
    off_t offset = index_row * (index_align + 1); // +1 для '\n'
    if (lseek(fd, offset, SEEK_SET) == -1) {
        php_error_docref(NULL, E_WARNING, "Error seeking in file: %s", strerror(errno));
        close(fd);
        RETURN_FALSE;
    }

    // Увеличиваем размер буфера на 1 для возможного символа перевода строки
    char *buffer = (char *)emalloc(index_align + 2); // +1 для '\0' и +1 для '\n'
    ssize_t read_bytes = read(fd, buffer, index_align + 1); // Чтение строки вместе с '\n'
    if (read_bytes == -1) {
        php_error_docref(NULL, E_WARNING, "Error reading file: %s", strerror(errno));
        efree(buffer);
        close(fd);
        RETURN_FALSE;
    }

    // Убедимся, что строка нуль-терминирована
    buffer[read_bytes] = '\0';

    // Обрезка пробелов справа и символа перевода строки
    for (int i = read_bytes - 1; i >= 0 && (buffer[i] == ' ' || buffer[i] == '\n'); --i) {
        buffer[i] = '\0';
    }

    close(fd);

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

    int fd = open(filename, O_RDWR);
    if (fd == -1) {
        php_error_docref(NULL, E_WARNING, "Unable to open file: %s", strerror(errno));
        RETURN_LONG(-1);
    }

    // Блокировка файла для записи
    if (lock_file(fd, F_WRLCK) == -1) {
        php_error_docref(NULL, E_WARNING, "Unable to lock file: %s", strerror(errno));
        close(fd);
        RETURN_LONG(-2);
    }

    // Рассчитываем смещение для записи в файл, учитывая перевод строки
    off_t offset = (index_row - 1) * (index_align + 1); // +1 для '\n'
    if (lseek(fd, offset, SEEK_SET) == -1) {
        php_error_docref(NULL, E_WARNING, "Unable to seek in file: %s", strerror(errno));
        close(fd);
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
    ssize_t written = write(fd, buffer, index_align + 1); // +1 для '\n'
    efree(buffer);
    close(fd); // Это также разблокирует файл

    if (written != index_align + 1) {
        php_error_docref(NULL, E_WARNING, "Error writing to file: %s", strerror(errno));
        RETURN_LONG(-4);
    }

    RETURN_TRUE;
}

