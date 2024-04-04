#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_streams.h" // Для работы с потоками PHP
#include <errno.h>
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

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_select_key_value, 0, 3, IS_STRING, 1)
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



// Функция поиска значения по ключу с чтением файла порциями
PHP_FUNCTION(find_value_by_key) {
    char *filename, *index_key;
    size_t filename_len, index_key_len;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "ss", &filename, &filename_len, &index_key, &index_key_len) == FAILURE) {
        RETURN_FALSE; // Неправильные параметры вызова функции
    }

    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        php_error_docref(NULL, E_WARNING, "Failed to open file: %s", strerror(errno));
        RETURN_FALSE;
    }

    // Попытка установить блокирующую блокировку на запись
    if (lock_file(fd, LOCK_EX) == -1) {
        php_error_docref(NULL, E_WARNING, "Failed to lock file: %s", strerror(errno));
        close(fd);
        RETURN_FALSE;
    }    


    char *buffer = (char *)emalloc(BUFFER_SIZE + 1);
    ssize_t bytesRead;
    char *found_value = NULL;
    char *lineStart = buffer;

    while ((bytesRead = read(fd, buffer, BUFFER_SIZE)) > 0) {
        buffer[bytesRead] = '\0'; // Завершаем прочитанный буфер нуль-терминатором

        // Обработка данных в буфере
        char *lineEnd;
        while ((lineEnd = strchr(lineStart, '\n')) != NULL) {
            *lineEnd = '\0'; // Завершаем строку нуль-терминатором

            // Проверяем, начинается ли строка с ключа
            if (strncmp(lineStart, index_key, index_key_len) == 0 && lineStart[index_key_len] == ' ') {
                found_value = estrndup(lineStart + index_key_len + 1, strlen(lineStart + index_key_len + 1)); // Пропускаем ключ и пробел
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
    efree(buffer);

    if (found_value == NULL) {
        RETURN_FALSE;
    } else {
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

    char index_filename[256];
    snprintf(index_filename, sizeof(index_filename), "%s.index", filename);

    int index_fd = open(index_filename, O_RDONLY);
    int data_fd = open(filename, O_RDONLY);

    
    if (index_fd == -1 || data_fd == -1) {
        if (index_fd != -1) close(index_fd);
        if (data_fd != -1) close(data_fd);
        php_error_docref(NULL, E_WARNING, "Failed to open index or data file");
        RETURN_FALSE;
    }

    // Блокировка индексного файла
    if (lock_file(index_fd, LOCK_EX) == -1 || lock_file(data_fd, LOCK_EX) == -1) {
        close(index_fd);
        if (data_fd != -1) close(data_fd);
        php_error_docref(NULL, E_WARNING, "Failed to lock index file");
        RETURN_FALSE;
    }


    char *buffer = (char *)emalloc(BUFFER_SIZE + 1);
    ssize_t bytesRead;
    char *found_value = NULL;
    char *lineStart = buffer;

    while ((bytesRead = read(index_fd, buffer, BUFFER_SIZE)) > 0) {
        buffer[bytesRead] = '\0'; // Завершаем прочитанный буфер нуль-терминатором

        // Обработка данных в буфере
        char *lineEnd;
        while ((lineEnd = strchr(lineStart, '\n')) != NULL) {
            *lineEnd = '\0'; // Завершаем строку нуль-терминатором

            // Проверяем, начинается ли строка с ключа
            if (strncmp(lineStart, index_key, index_key_len) == 0 && lineStart[index_key_len] == ' ') {
                found_value = estrdup(lineStart + index_key_len + 1); // Пропускаем ключ и пробел
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

    // Разблокировка и закрытие файлов
    close(index_fd);
    close(data_fd);
    efree(buffer);

    if (found_value == NULL) {
        RETURN_FALSE;
    } else {
        RETVAL_STRING(found_value);
        efree(found_value);
    }
}


/* Реализация функции */
PHP_FUNCTION(write_key_value_pair) {
    char *filename, *index_key, *index_val;
    size_t filename_len, index_key_len, index_val_len;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "sss", &filename, &filename_len, &index_key, &index_key_len, &index_val, &index_val_len) == FAILURE) {
        RETURN_FALSE;
    }

    int fd = open(filename, O_WRONLY | O_APPEND | O_CREAT, 0666);
    
    if (fd == -1) {
        php_error_docref(NULL, E_WARNING, "Failed to open the file: %s", strerror(errno));
        RETURN_LONG(-1);
    }

    if (lock_file(fd, LOCK_EX) == -1) {
        php_error_docref(NULL, E_WARNING, "Failed to lock the file: %s", strerror(errno));
        close(fd);
        RETURN_LONG(-2);
    }

    if (dprintf(fd, "%s %s\n", index_key, index_val) < 0) {
        php_error_docref(NULL, E_WARNING, "Failed to write to the file: %s", strerror(errno));
        close(fd); // Это также разблокирует файл
        RETURN_LONG(-3);
    }

    close(fd); // Это также разблокирует файл
    RETURN_LONG(1);
}


/* Реализация функции */
PHP_FUNCTION(indexed_write_key_value_pair) {
    char *filename, *index_key, *index_val;
    size_t filename_len, index_key_len, index_val_len;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "sss", &filename, &filename_len, &index_key, &index_key_len, &index_val, &index_val_len) == FAILURE) {
        RETURN_FALSE;
    }

    char *index_filename = emalloc(filename_len + 6); // Дополнительные символы для ".index" и терминирующего нуля
    snprintf(index_filename, filename_len + 7, "%s.index", filename);

    int data_fd = open(filename, O_RDWR | O_CREAT | O_APPEND, 0644);
    int index_fd = open(index_filename, O_RDWR | O_CREAT | O_APPEND, 0644);
    if (data_fd == -1 || index_fd == -1) {
        php_error_docref(NULL, E_WARNING, "Error opening file");
        if (data_fd != -1) close(data_fd);
        if (index_fd != -1) close(index_fd);
        efree(index_filename);
        RETURN_LONG(-1);
    }

    // Блокировка обоих файлов
    if (lock_file(index_fd, LOCK_EX) == -1 || lock_file(data_fd, LOCK_EX) == -1) {
        php_error_docref(NULL, E_WARNING, "Error locking file");
        close(data_fd);
        close(index_fd);
        efree(index_filename);
        RETURN_LONG(-2);
    }

    // Запись значения в файл данных
    off_t offset = lseek(data_fd, 0, SEEK_END); // Получаем текущее смещение в файле данных
    size_t size = strlen(index_val);
    if (write(data_fd, index_val, size) != size) {
        php_error_docref(NULL, E_WARNING, "Error writing to data file");
        close(data_fd);
        close(index_fd);
        efree(index_filename);
        RETURN_LONG(-3);
    }

    // Запись индекса в индексный файл
    dprintf(index_fd, "%s %ld:%zu\n", index_key, offset, size);

    // Разблокировка и закрытие файлов
    close(data_fd);
    close(index_fd);
    efree(index_filename);
    RETURN_LONG(1);
}


/* Реализация функции */
PHP_FUNCTION(delete_key_value_pair) {
    char *filename, *index_key = NULL;
    size_t filename_len, index_key_len = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "ss", &filename, &filename_len, &index_key, &index_key_len) == FAILURE) {
        RETURN_FALSE;
    }

    char temp_filename[256];
    snprintf(temp_filename, sizeof(temp_filename), "%s.tmp", filename);

    int fd = open(filename, O_RDWR);
    if (fd == -1) {
        php_error_docref(NULL, E_WARNING, "Failed to open original file.");
        RETURN_LONG(-1);
    }

    int temp_fd = open(temp_filename, O_RDWR | O_CREAT | O_APPEND, 0644);
    if (temp_fd == -1) {
        close(fd);
        php_error_docref(NULL, E_WARNING, "Failed to open or create temporary file.");
        RETURN_LONG(-2);
    }

    // Блокировка файлов
    if (lock_file(fd, LOCK_EX) == -1) {
        php_error_docref(NULL, E_WARNING, "Failed to lock files.");
        close(fd);
        close(temp_fd);
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
        RETURN_LONG(-4);
    }

    char *buffer = (char *)emalloc(BUFFER_SIZE + 1);
    size_t bytesRead;
    while ((bytesRead = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
        buffer[bytesRead] = '\0'; // Добавляем нуль-терминатор для безопасной работы со строками
        
        char *lineStart = buffer;
        char *lineEnd;
        while ((lineEnd = strchr(lineStart, '\n')) != NULL) {
            *lineEnd = '\0'; // Заменяем символ новой строки на нуль-терминатор
            
            if (strncmp(lineStart, index_key, strlen(index_key)) != 0 || lineStart[strlen(index_key)] != ' ') {
                fprintf(temp_file, "%s\n", lineStart);
            }
            
            lineStart = lineEnd + 1; // Переходим к следующей строке
        }
        
        if (lineStart != buffer + bytesRead) {
            fseek(file, -(long)(bytesRead - (lineStart - buffer)), SEEK_CUR); // Возвращаемся назад в исходном файле
        }
    }
    
    fclose(file);
    fclose(temp_file);
    close(fd); 
    close(temp_fd);
    efree(buffer);

    // Заменяем оригинальный файл временным файлом
    if (rename(temp_filename, filename) == -1) {
        php_error_docref(NULL, E_WARNING, "Failed to replace the original file with the temporary file.");
        RETURN_LONG(-5);
    }

    RETURN_TRUE;
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


/* Функция для извлечения и удаления последней строки из файла */
PHP_FUNCTION(pop_key_value_pair) {
    char *filename;
    size_t filename_len;
    zend_long index_align = -1; // Значение по умолчанию для необязательного аргумента

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "s|l", &filename, &filename_len, &index_align) == FAILURE) {
        RETURN_FALSE;
    }

    int fd = open(filename, O_RDWR);
    if (fd == -1) {
        php_error_docref(NULL, E_WARNING, "Failed to open file: %s", filename);
        RETURN_FALSE;
    }

    if (lock_file(fd, LOCK_EX) == -1) {
        close(fd);
        php_error_docref(NULL, E_WARNING, "Failed to lock file: %s", filename);
        RETURN_FALSE;
    }

    off_t fileSize = lseek(fd, 0, SEEK_END);
    if (fileSize == -1) {
        close(fd);
        php_error_docref(NULL, E_WARNING, "Failed to seek file: %s", filename);
        RETURN_FALSE;
    }

    off_t pos = fileSize;
    ssize_t bytesRead;


    if (index_align != -1) {
        pos -= index_align + 1;
        lseek(fd, pos, SEEK_SET);

        // Увеличиваем размер буфера на 1 для возможного символа перевода строки
        char *buffer = (char *)emalloc(index_align + 1); // +1 для '\0'

        ssize_t read_bytes = read(fd, buffer, index_align); // Чтение строки
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

        // Усекаем файл
        if(ftruncate(fd, pos)) {
            efree(buffer);
            close(fd);
            php_error_docref(NULL, E_WARNING, "Failed to truncate file: %s", filename);
            RETURN_FALSE;
        }
        
        close(fd);

        // Возврат строки в PHP
        RETVAL_STRING(buffer);
        efree(buffer);
        return;
    }


    char *buffer = (char *)emalloc(BUFFER_SIZE + 1);
    char *line = NULL;
    int state = 0; // Состояние конечного автомата


    // Читаем файл с конца, пока не найдем начало строки
    while (pos > 0 && state != 2) {
        pos -= BUFFER_SIZE;
        pos = pos < 0 ? 0 : pos; // Адаптируем позицию, если вышли за начало файла

        lseek(fd, pos, SEEK_SET);
        bytesRead = read(fd, buffer, BUFFER_SIZE);
        if (bytesRead <= 0) {
            php_error_docref(NULL, E_WARNING, "Failed to read file: %s", filename);
            close(fd);
            RETURN_FALSE;
        }

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
                        size_t len = bytesRead - i - 1;
                        line = emalloc(len + 1);
                        memcpy(line, buffer + i + 1, len);
                        line[len] = '\0';
                        state = 2; // Переходим к финальному состоянию

                        // Усекаем файл
                        if(ftruncate(fd, pos + i + (pos + i == 0 ? 0 : 1))) {
                            efree(line);
                            efree(buffer);
                            close(fd);
                            php_error_docref(NULL, E_WARNING, "Failed to truncate file: %s", filename);
                            RETURN_FALSE;
                        }
                    }
                    break;
            }
        }
    }

    close(fd);
    efree(buffer);

    if (line != NULL) {
        RETVAL_STRING(line);
        efree(line);
    } else {
        RETVAL_FALSE;
    }
}




/* Реализация функции */
PHP_FUNCTION(hide_key_value_pair) {
    char *filename, *index_key;
    size_t filename_len, index_key_len;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "ss", &filename, &filename_len, &index_key, &index_key_len) == FAILURE) {
        RETURN_FALSE;
    }

    int fd = open(filename, O_RDWR);
    if (fd == -1) {
        php_error_docref(NULL, E_WARNING, "Failed to open file: %s", filename);
        RETURN_FALSE;
    }

    if (lock_file(fd, LOCK_EX) == -1) { // Блокировка файла на запись
        close(fd);
        php_error_docref(NULL, E_WARNING, "Failed to lock file: %s", filename);
        RETURN_FALSE;
    }

    char *buffer = (char *)emalloc(BUFFER_SIZE + 1);
    ssize_t bytesRead, totalRead = 0;
    off_t writeOffset = 0; // Смещение для записи обновленных данных

    while ((bytesRead = read(fd, buffer + totalRead, BUFFER_SIZE - totalRead)) > 0) {
        totalRead += bytesRead;
        buffer[totalRead] = '\0';

        char *lineStart = buffer;
        char *lineEnd;

        while ((lineEnd = strchr(lineStart, '\n')) != NULL) {
            *lineEnd = '\0'; // Завершаем строку нуль-терминатором
            ssize_t lineLength = lineEnd - lineStart + 1;

            if (strncmp(lineStart, index_key, index_key_len) == 0 && lineStart[index_key_len] == ' ') {
                // Найдено совпадение ключа, подготавливаем замену
                char *replacement = emalloc(index_key_len + 1);
                memset(replacement, 127, index_key_len); // Заполнение символами DEL
                replacement[index_key_len] = ' '; // Сохраняем пробел после ключа
                
                // Перемещаемся к началу найденной строки и записываем замену
                lseek(fd, writeOffset, SEEK_SET);
                if(write(fd, replacement, index_key_len + 1) == -1) {
                    efree(replacement);
                    efree(buffer);
                    close(fd);
                    php_error_docref(NULL, E_WARNING, "Failed to write to file: %s", filename);
                    RETURN_FALSE;
                }
                efree(replacement);
            }

            writeOffset += lineLength; // Обновляем смещение для записи
            lineStart = lineEnd + 1; // Переходим к следующей строке
        }

        // Подготовка к следующему чтению, если остались данные в буфере
        totalRead -= (lineStart - buffer);
        if (totalRead > 0) {
            memmove(buffer, lineStart, totalRead);
        }
    }

    close(fd); // Это также разблокирует файл
    efree(buffer);
    RETURN_TRUE;
}



/* Реализация функции */
PHP_FUNCTION(get_index_keys) {
    char *filename;
    size_t filename_len;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &filename, &filename_len) == FAILURE) {
        RETURN_FALSE;
    }

    KeyArray keys = {0};

    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        php_error_docref(NULL, E_WARNING, "Failed to open file: %s", filename);
        RETURN_FALSE;
    }

    if (lock_file(fd, LOCK_EX) == -1) { // Блокировка файла на чтение
        close(fd);
        php_error_docref(NULL, E_WARNING, "Failed to lock file: %s", filename);
        RETURN_FALSE;
    }

    char *buffer = (char *)emalloc(BUFFER_SIZE + 1);
    ssize_t bytesRead;
    char *lineStart = buffer;
    char *lineEnd;

    while ((bytesRead = read(fd, buffer, BUFFER_SIZE)) > 0) {
        buffer[bytesRead] = '\0'; // Завершаем прочитанный буфер нуль-терминатором

        while ((lineEnd = strchr(lineStart, '\n')) != NULL) {
            *lineEnd = '\0'; // Завершаем строку нуль-терминатором

            // Извлекаем ключ из строки
            char *spacePos = strchr(lineStart, ' ');
            if (spacePos) {
                *spacePos = '\0'; // Обрезаем строку до первого пробела
                add_key(&keys, lineStart);
            }

            lineStart = lineEnd + 1; // Переходим к следующей строке
        }

        // Подготовка к чтению следующего буфера
        size_t remaining = bytesRead - (lineStart - buffer);
        memmove(buffer, lineStart, remaining); // Перемещаем оставшуюся часть в начало буфера
        lineStart = buffer;
    }

    close(fd);

    // Возвращаем результат в виде массива PHP
    array_init(return_value);
    for (size_t i = 0; i < keys.count; i++) {
        add_next_index_string(return_value, keys.keys[i]);
    }


    // Освобождаем выделенную память
    free_key_array(&keys);
    efree(buffer);
}




// Функция обновления пары ключ-значение
PHP_FUNCTION(update_key_value_pair) {
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
        php_error_docref(NULL, E_WARNING, "Failed to create temporary file: %s", temp_filename);
        close(fd);
        efree(temp_filename);
        RETURN_LONG(-2);
    }

    // Блокировка файлов
    if (lock_file(fd, LOCK_EX) == -1) {
        php_error_docref(NULL, E_WARNING, "Failed to lock files");
        close(fd);
        close(temp_fd);
        efree(temp_filename);
        RETURN_LONG(-3);
    }

    FILE *file = fdopen(fd, "r");
    FILE *temp_file = fdopen(temp_fd, "w");

    if (!file || !temp_file) {
        php_error_docref(NULL, E_WARNING, "Failed to open file descriptors");
        if (file) fclose(file);
        if (temp_file) fclose(temp_file);
        close(fd);
        close(temp_fd);
        unlink(temp_filename);
        efree(temp_filename);
        RETURN_LONG(-4);
    }

    char *buffer = (char *)emalloc(BUFFER_SIZE + 1);
    size_t bytesRead;
    while ((bytesRead = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
        buffer[bytesRead] = '\0';

        char *lineStart = buffer;
        char *lineEnd;
        while ((lineEnd = strchr(lineStart, '\n')) != NULL) {
            *lineEnd = '\0';
            
            if (strncmp(lineStart, index_key, index_key_len) == 0 && lineStart[index_key_len] == ' ') {
                fprintf(temp_file, "%s %s\n", index_key, index_value);
            } else {
                fprintf(temp_file, "%s\n", lineStart);
            }
            
            lineStart = lineEnd + 1;
        }
        
        if (lineStart != buffer + bytesRead) {
            fseek(file, -(long)(bytesRead - (lineStart - buffer)), SEEK_CUR);
        }
    }
    
    fclose(file);
    fclose(temp_file);
    close(fd); 
    close(temp_fd);

    rename(temp_filename, filename);

    efree(buffer);
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

    int fd = open(filename, O_WRONLY | O_APPEND | O_CREAT, 0666);
    if (fd == -1) {
        php_error_docref(NULL, E_WARNING, "Unable to open file: %s", strerror(errno));
        RETURN_LONG(-1);
    }

    // Блокировка файла для записи
    if (lock_file(fd, LOCK_EX) == -1) {
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
    if (lock_file(fd, LOCK_EX) == -1) {
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
    if (lock_file(fd, LOCK_EX) == -1) {
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

