

<?php
$data_file = __DIR__ . '/fast_io.dat';
$data_file_lock = $data_file . '.lock';
$align = 64; // line_number - длина 12 байт, 52 байта под данные.


if(file_exists($data_file_lock) && filesize($data_file_lock) > 0){ // Реализация IPC
	// Это условие сработает если параллельный процесс удерживает блокировку или вышел аварийно.
	$last_process_id = intval(file_get_contents($data_file_lock));
	$statFile = "/proc/$last_process_id/stat";

	if (file_exists($statFile)) {// Процесс с PID $last_process_id существует
		$statData = file_get_contents($statFile);
	
		// Разбиваем данные статистики ядра на массив
		$statArray = explode(" ", $statData);

		if (count($statArray) > 21) {// Получаем значения из массива
			$utime = intval($statArray[13]); // user time
			$stime = intval($statArray[14]); // system time
			$cutime = intval($statArray[15]); // user time дочерних процессов
			$cstime = intval($statArray[16]); // system time дочерних процессов
		
			// Вычисляем общее время
			$total_time = $utime + $stime;
			if ($cutime > 0 || $cstime > 0) {
				$total_time += $cutime + $cstime;
			}
		
			$avg = sys_getloadavg();
			$log_file = $data_file . '.race_condition.log';
			if(filectime($log_file) + 120 < time()) {
                file_put_contents(
                    $log_file,
                    print_r([
                        time(),
                        $total_time,
                        $avg,
                        $statArray
                    ], true),
                    FILE_APPEND
                );
            }

		}
	}
}


$lock= fopen($data_file_lock, "c+");

if(flock($lock, LOCK_EX | LOCK_NB)) { 
	// Это условие сработает без ожидания снятия блокировки, если параллельный процесс удерживает блокировку 
	$is_locked = false;
} else {
	$is_locked = true; // Признак удержания блокировки параллельным процессом
}

    
if(flock($lock, LOCK_EX)) { 
	// В этом месте функция ждет в очереди, пока параллельные процессы снимут блокировку

	// Реализация IPC
	ftruncate($lock, 0); // Усекаем файл
	fwrite($lock, strval(getmypid())); // Id запущенного процесса, для реализации IPC
	fflush($lock);

	// Данные с выравниванием
	$last_line_number = 0;
	if(file_exists($data_file) && filesize($data_file) > 0){
		$last_line_number = filesize($data_file) / ($align + 1);
	}

	$new_line_number = insert_key_value($data_file, 'insert_key_value_' . $last_line_number, $align); // Добавить строку в файл с выравниванием
	$str = select_key_value($data_file, $new_line_number, $align); // Получить строку из файла по номеру строки



	// Даннае без выравнивания
	$last_offset = 0;
	if(file_exists($data_file . '.dat') && filesize($data_file) > 0){
		$last_offset = filesize($data_file . '.dat');
	}

	$new_offset = write_key_value_pair($data_file . '.dat', "write_key_value_pair_" . $last_offset); // Добавить строку в файл без выравнивания
	$new_str = select_key_value($data_file . '.dat', $new_offset, mb_strlen($str), 1); // Получить строку из файла по смещению


	// Выход
	ftruncate($lock, 0);
	flock($lock, LOCK_UN); // Снимает блокировку
}

fclose($lock); // Тоже снимает блокировку  


print_r([$last_line_number, $new_line_number, $str]);
print_r([$last_offset, $new_offset, $new_str]);

































#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define BUFFER_SIZE 4096

int main() {
    const char *filename = "example.txt";
    FILE *fp = fopen(filename, "r");
    
    if (fp == NULL) {
        perror("Error opening file");
        return 1;
    }

    char *buffer = (char *)malloc(BUFFER_SIZE);
    if (buffer == NULL) {
        perror("Error allocating memory");
        fclose(fp);
        return 1;
    }

    char *dynamic_buffer = NULL;
    size_t dynamic_buffer_size = 0;

    bool isEOF = false;
    bool found_match = false;
    char *found_value = NULL;

    // Считываем данные из конца файла порциями по BUFFER_SIZE байт
    fseek(fp, 0, SEEK_END);
    long fileSize = ftell(fp);
    long position = fileSize;

    while (position > 0) {
        if (position < BUFFER_SIZE) {
            fseek(fp, 0, SEEK_SET); // Перемещаем указатель в начало файла
            fread(buffer, sizeof(char), position, fp);
            buffer[position] = '\0';
        } else {
            fseek(fp, position - BUFFER_SIZE, SEEK_SET); // Перемещаем указатель на предыдущую порцию
            fread(buffer, sizeof(char), BUFFER_SIZE, fp);
            buffer[BUFFER_SIZE] = '\0';
        }

        // Обработка буфера
        dynamic_buffer = (char *)realloc(dynamic_buffer, dynamic_buffer_size + BUFFER_SIZE + 1);
        if (dynamic_buffer == NULL) {
            perror("Error reallocating memory");
            break;
        }

        strcat(dynamic_buffer, buffer);
        dynamic_buffer_size += BUFFER_SIZE;

        // Дополнительная обработка динамического буфера
        // Ваш текущий код сюда

        // Условие выхода из цикла (например, если найдено совпадение)
        if (found_match) {
            break;
        }

        // Обновляем позицию для следующей порции данных
        position -= BUFFER_SIZE;
    }

    free(buffer);
    free(dynamic_buffer);
    fclose(fp);

    return 0;
}
































    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        php_error_docref(NULL, E_WARNING, "Failed to open file: %s", filename);
        RETURN_FALSE;
    }

    zend_long ini_buffer_size = FAST_IO_G(buffer_size);
    zend_long dynamic_buffer_size = ini_buffer_size;
    char *dynamic_buffer = (char *)emalloc(dynamic_buffer_size + 1);

    ssize_t bytesRead;
    size_t current_size = 0;
    bool found_match = false;
    char *found_value = NULL;
    bool isEOF;


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





















// Авто поиск последней строки
zend_long ini_buffer_size = FAST_IO_G(buffer_size);
char *buffer = (char *)emalloc(ini_buffer_size + 1);
char *result_str = NULL;
size_t result_len = 0;
int found_line_start = 0;

while (pos > 0 && !found_line_start) {
    pos -= ini_buffer_size;
    pos = pos < 0 ? 0 : pos;

    lseek(fd, pos, SEEK_SET);
    ssize_t bytesRead = read(fd, buffer, ini_buffer_size);

    if (bytesRead <= 0) {
        php_error_docref(NULL, E_WARNING, "Error reading file: %s", filename);
        close(fd);
        efree(buffer);
        if (result_str) efree(result_str);
        RETURN_FALSE;
    }

    for (ssize_t i = bytesRead - 1; i >= 0; --i) {
        if (buffer[i] == '\n') {
            if (!result_str) { // Найден первый перенос строки с конца
                result_len = bytesRead - i - 1;
                result_str = emalloc(result_len + 1);
                memcpy(result_str, buffer + i + 1, result_len);
                result_str[result_len] = '\0';
            } else if (i != bytesRead - 1 || pos + bytesRead < fileSize) { // Найдено начало строки
                size_t new_len = result_len + bytesRead - i - 1;
                char *new_result_str = emalloc(new_len + 1);
                memcpy(new_result_str, buffer + i + 1, bytesRead - i - 1);
                memcpy(new_result_str + bytesRead - i - 1, result_str, result_len);
                efree(result_str);
                result_str = new_result_str;
                result_len = new_len;
                result_str[result_len] = '\0';

                found_line_start = 1;
                break;
            }
        }
    }

    if (!found_line_start && bytesRead > 0 && result_str != NULL && pos == 0) {
        size_t new_len = result_len + bytesRead;
        char *new_result_str = emalloc(new_len + 1);
        memcpy(new_result_str, buffer, bytesRead);
        if (result_len > 0) {
            memcpy(new_result_str + bytesRead, result_str, result_len);
        }
        efree(result_str);
        result_str = new_result_str;
        result_len = new_len;
        result_str[result_len] = '\0';

        found_line_start = 1;
        break;
    }
}

if (result_str) {
    // Возвращаем результат через механизмы PHP
    RETVAL_STRINGL(result_str, result_len);
    efree(result_str);
} else {
    // Если строка не найдена
    RETURN_FALSE;
}

efree(buffer);






































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















































// Авто поиск последней строки
zend_long ini_buffer_size = FAST_IO_G(buffer_size);
char *buffer = (char *)emalloc(ini_buffer_size + 1);
char *result_str = NULL;
size_t result_len = 0;
int found_line_start = 0;

while (pos > 0 && !found_line_start) {
    pos -= ini_buffer_size;
    pos = pos < 0 ? 0 : pos;

    lseek(fd, pos, SEEK_SET);
    ssize_t bytesRead = read(fd, buffer, ini_buffer_size);

    if (bytesRead <= 0) {
        php_error_docref(NULL, E_WARNING, "Error reading file: %s", filename);
        close(fd);
        efree(buffer);
        if (result_str) efree(result_str);
        RETURN_FALSE;
    }

    for (ssize_t i = bytesRead - 1; i >= 0; --i) {
        if (buffer[i] == '\n') {
            if (!result_str) { // Найден первый перенос строки с конца
                result_len = bytesRead - i - 1;
                result_str = emalloc(result_len + 1);
                memcpy(result_str, buffer + i + 1, result_len);
                result_str[result_len] = '\0';
            } else if (i != bytesRead - 1 || pos + bytesRead < fileSize) { // Найдено начало строки
                size_t new_len = result_len + bytesRead - i - 1;
                char *new_result_str = emalloc(new_len + 1);
                memcpy(new_result_str, buffer + i + 1, bytesRead - i - 1);
                memcpy(new_result_str + bytesRead - i - 1, result_str, result_len);
                efree(result_str);
                result_str = new_result_str;
                result_len = new_len;
                result_str[result_len] = '\0';

                found_line_start = 1;
                break;
            }
        }
    }

    if (!found_line_start && bytesRead > 0 && result_str != NULL && pos == 0) {
        size_t new_len = result_len + bytesRead;
        char *new_result_str = emalloc(new_len + 1);
        memcpy(new_result_str, buffer, bytesRead);
        if (result_len > 0) {
            memcpy(new_result_str + bytesRead, result_str, result_len);
        }
        efree(result_str);
        result_str = new_result_str;
        result_len = new_len;
        result_str[result_len] = '\0';

        found_line_start = 1;
        break;
    }
}

if (result_str) {
    // Возвращаем результат через механизмы PHP
    RETVAL_STRINGL(result_str, result_len);
    efree(result_str);
} else {
    // Если строка не найдена
    RETURN_FALSE;
}

efree(buffer);









































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

    zend_long ini_buffer_size = FAST_IO_G(buffer_size);
    zend_long dynamic_buffer_size = ini_buffer_size;
    char *dynamic_buffer = (char *)emalloc(dynamic_buffer_size + 1);
    ssize_t bytesRead;
    size_t current_size = 0;

    off_t search_offset = 0;

    zend_long found_count = 0;
    zend_long add_count = 0;
    zend_long line_count = 0;
    bool found_match = false;

    array_init(return_value);
    KeyArray keys = {0};
    KeyValueArray keys_values = {0};
    int value[2];
    bool isEOF;

    while ((bytesRead = fread(dynamic_buffer + current_size, 1, ini_buffer_size, fp)) > 0) {
        current_size += bytesRead;

        isEOF = feof(fp);
        if (isEOF && dynamic_buffer[current_size - 1] != '\n') {
            dynamic_buffer[current_size] = '\n'; // Добавляем символ перевода строки в конец, если его нет
            current_size++;
        }
        dynamic_buffer[current_size] = '\0';

        char *lineStart = dynamic_buffer;
        char *lineEnd;
        while ((lineEnd = strchr(lineStart, '\n')) != NULL) {
            ssize_t lineLength = lineEnd - lineStart + 1;
            *lineEnd = '\0';
            line_count++;

            if (search_state == 0 && strstr(lineStart, index_key) != NULL) {
                found_count++;
                if (search_start < found_count) {
                    add_count++;
                    for (int i = lineLength - 2; i >= 0; --i) {
                        if (lineStart[i] == ' ') lineStart[i] = '\0';
                        else break;
                    }
                    if (!add_key(&keys, lineStart)) {
                        php_error_docref(NULL, E_WARNING, "Out of memory");
                        found_match = true;
                        break;
                    }
                }
            }

            search_offset += lineLength;

            lineStart = lineEnd + 1;
            if (add_count >= search_limit) {
                found_match = true;
                break;
            }
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

    for (size_t i = 0; i < keys.count; i++) {
        add_next_index_string(return_value, keys.keys[i]);
    }

    free_key_array(&keys);
    free_key_value_array(&keys_values);
}






































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



    array_init(return_value);

    KeyArray keys = {0};
    KeyValueArray keys_values = {0};

    int value[2];
    bool isEOF;




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
    fclose(fp);
    


        for (size_t i = 0; i < keys.count; i++) {
            add_next_index_string(return_value, keys.keys[i]);
        }
    

    free_key_array(&keys);
    free_key_value_array(&keys_values);
}



























































PHP_FUNCTION(find_array_by_key) {
    char *filename, *index_key;
    size_t filename_len, index_key_len;
    zend_long search_state = 0;
    zend_long search_start = 0;
    zend_long search_limit = 1;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "ss|lll", &filename, &filename_len, &index_key, &index_key_len, &search_state, &search_start, &search_limit) == FAILURE) {
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
        dynamic_buffer[current_size] = '\0';

        char *lineStart = dynamic_buffer;
        char *lineEnd;
        while ((lineEnd = strchr(lineStart, '\n')) != NULL) {
            ssize_t lineLength = lineEnd - lineStart + 1;
            *lineEnd = '\0';
            line_count++;

            if(search_state == 14 && pcre2_match(re, lineStart, lineLength, 0, 0, match_data, NULL) > 0){
                found_count++;

                if(search_start < found_count){
                    add_count++;

                    int rc;
                    PCRE2_SIZE *ovector;
                    size_t start_offset = 0;

                    zval return_matched; // Объявляем переменную типа zval для хранения массива совпадений
                    array_init(&return_matched); // Инициализируем массив совпадений

                    while ((rc = pcre2_match(re, lineStart, lineLength, start_offset, 0, match_data, NULL)) > 0) {
                        ovector = pcre2_get_ovector_pointer(match_data);
                        
                        for (int i = 0; i < rc; i++) {
                            long start = (long)ovector[2*i];
                            long end = (long)ovector[2*i+1];
                            add_next_index_stringl(&return_matched, (char *)lineStart + start, end - start);    
                        }
                        
                        start_offset = ovector[1]; // Обновляем позицию начала поиска для следующего совпадения
                    }

                    add_next_index_zval(return_value, &return_matched);
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

    efree(dynamic_buffer);

    if (search_state > 9 && re != NULL) pcre2_code_free(re);
    if (search_state > 9 && match_data != NULL) pcre2_match_data_free(match_data);

    fclose(fp);
    
    free_key_array(&keys);
    free_key_value_array(&keys_values);
}















































/* Предполагается, что все необходимые заголовки и переменные уже объявлены */

array_init(return_value); // Инициализируем возвращаемое значение как массив

while ((lineEnd = strchr(lineStart, '\n')) != NULL) {
    ssize_t lineLength = lineEnd - lineStart + 1;
    *lineEnd = '\0';
    line_count++;
    if(search_state == 14 && pcre2_match(re, lineStart, lineLength, 0, 0, match_data, NULL) > 0){
        found_count++;

        zval return_match; // Объявляем переменную типа zval для хранения массива совпадений
        array_init(&return_match); // Инициализируем массив совпадений

        size_t start_offset = 0;
        while ((rc = pcre2_match(re, lineStart, lineLength, start_offset, 0, match_data, NULL)) > 0) {
            ovector = pcre2_get_ovector_pointer(match_data);
            for (int i = 0; i < rc; i++) {
                long start = (long)ovector[2*i];
                long end = (long)ovector[2*i+1];
                add_next_index_stringl(&return_match, (char *)lineStart + start, end - start);
            }
            start_offset = ovector[1]; // Обновляем позицию начала поиска для следующего совпадения
        }

        add_next_index_zval(return_value, &return_match); // Добавляем массив совпадений в возвращаемый массив
    }
    lineStart = lineEnd + 1; // Перемещаем указатель на начало следующей строки
}

/* После завершения всех операций не забудьте освободить ресурсы */
































#include <php.h>
#include <pcre2.h>

/* Предположим, что функция уже объявлена и подготовлена к использованию */

PHP_FUNCTION(example_pcre2_match_all) {
    char *filename, *index_key;
    size_t filename_len, index_key_len;
    zend_long search_state = 0;
    zend_long search_start = 0;
    zend_long search_limit = 1; // Этот параметр не используется в данном примере
    KeyArray keys = {0}; // Предполагается, что KeyArray это структура или тип данных, определенный где-то в вашем коде

    /* Получение аргументов (пример) */
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "ss", &filename, &filename_len, &index_key, &index_key_len) == FAILURE) {
        return;
    }





    pcre2_code *re;
    pcre2_match_data *match_data; 

    PCRE2_SIZE erroffset;
    int errorcode;

    /* Компиляция регулярного выражения */
    re = pcre2_compile((PCRE2_SPTR)index_key, PCRE2_ZERO_TERMINATED, 0, &errorcode, &erroffset, NULL);

    if (re == NULL) {
        PCRE2_UCHAR message[256];
        pcre2_get_error_message(errorcode, message, sizeof(message));
        php_error_docref(NULL, E_WARNING, "PCRE2 compilation failed at offset %d: %s", (int)erroffset, message);
        RETURN_FALSE;
    }

    match_data = pcre2_match_data_create_from_pattern(re, NULL);

    /* Предположим, что lineStart и lineLength уже определены где-то в вашем коде */
    PCRE2_SPTR subject = (PCRE2_SPTR)filename; // Пример использования строки
    size_t subject_length = filename_len; // Длина строки

    int rc;
    PCRE2_SIZE *ovector;
    size_t start_offset = 0;

    array_init(return_match);
    array_init(return_value); // Инициализируем возвращаемое значение как массив

        while ((lineEnd = strchr(lineStart, '\n')) != NULL) {
            ssize_t lineLength = lineEnd - lineStart + 1;
            *lineEnd = '\0';
            line_count++;
            if(search_state == 14 && pcre2_match(re, lineStart, lineLength, 0, 0, match_data, NULL) > 0){
                found_count++;

                int rc;
                PCRE2_SIZE *ovector;
                size_t start_offset = 0;
                while ((rc = pcre2_match(re, lineStart, lineLength, start_offset, 0, match_data, NULL)) > 0) {
                    ovector = pcre2_get_ovector_pointer(match_data);
                    for (int i = 0; i < rc; i++) {
                        long start = (long)ovector[2*i];
                        long end = (long)ovector[2*i+1];
                        add_next_index_stringl(return_match, (char *)lineStart + start, end - start);
                    }

                    add_next_index_zval(return_value, return_match);
                    start_offset = ovector[1]; // Обновляем позицию начала поиска для следующего совпадения
                }
            }
        }

    add_next_index_zval(return_value, return_match);











    if (re != NULL) pcre2_code_free(re);
    if (match_data != NULL) pcre2_match_data_free(match_data);
}




































#include <php.h>
#include <pcre2.h>










    char *filename, *index_key;
    size_t filename_len, index_key_len;
    zend_long search_state = 0;
    zend_long search_start = 0;
    zend_long search_limit = 1;
    KeyArray keys = {0};


    pcre2_code *re;
    pcre2_match_data *match_data; 

    PCRE2_SIZE erroffset;
    int errorcode;
    re = pcre2_compile((PCRE2_SPTR)index_key, PCRE2_ZERO_TERMINATED, 0, &errorcode, &erroffset, NULL);

    if (re == NULL) {
            PCRE2_UCHAR message[256];
            pcre2_get_error_message(errorcode, message, sizeof(message));
            php_error_docref(NULL, E_WARNING, "PCRE2 compilation failed at offset %d: %s", (int)erroffset, message);
            RETURN_FALSE;
    }

    match_data = pcre2_match_data_create_from_pattern(re, NULL);


    if(search_state == 14 && pcre2_match(re, lineStart, lineLength, 0, 0, match_data, NULL) > 0){
        PCRE2_SIZE *ovector = pcre2_get_ovector_pointer(match_data);
        /* Вывод первого совпадения. Для всех совпадений используйте цикл */
        long start = (long)ovector[0];
        long end = (long)ovector[1];

        RETURN_STRINGL(subject + start, end - start);
    }

    if (re != NULL) pcre2_code_free(re);
    if (match_data != NULL) pcre2_match_data_free(match_data);
    return;








/* Пример функции, использующей PCRE2 для поиска совпадений */
PHP_FUNCTION(example_pcre2_match) {
    char *pattern;
    size_t pattern_len;
    char *subject;
    size_t subject_len;

    /* Получаем аргументы функции */
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "ss", &pattern, &pattern_len, &subject, &subject_len) == FAILURE) {
        return;
    }

    pcre2_code *re;
    PCRE2_SPTR pattern_sp = (PCRE2_SPTR)pattern;
    PCRE2_SPTR subject_sp = (PCRE2_SPTR)subject;
    PCRE2_SIZE erroffset;
    int errorcode;

    /* Компилируем регулярное выражение */
    re = pcre2_compile(pattern_sp, PCRE2_ZERO_TERMINATED, 0, &errorcode, &erroffset, NULL);
    if (re == NULL) {
        PCRE2_UCHAR message[256];
        pcre2_get_error_message(errorcode, message, sizeof(message));
        php_error_docref(NULL, E_WARNING, "PCRE2 compilation failed at offset %d: %s", (int)erroffset, message);
        RETURN_FALSE;
    }

    pcre2_match_data *match_data = pcre2_match_data_create_from_pattern(re, NULL);
    
    /* Исполняем поиск совпадений */
    int rc = pcre2_match(re, subject_sp, subject_len, 0, 0, match_data, NULL);

    if (rc < 0) {
        /* Если совпадений нет или произошла ошибка */
        switch(rc) {
            case PCRE2_ERROR_NOMATCH:
                php_error_docref(NULL, E_NOTICE, "No match");
                break;
            default:
                php_error_docref(NULL, E_WARNING, "Matching error %d", rc);
                break;
        }
        pcre2_match_data_free(match_data);
        pcre2_code_free(re);
        RETURN_FALSE;
    }

    if (rc > 0) {
        PCRE2_SIZE *ovector = pcre2_get_ovector_pointer(match_data);
        /* Вывод первого совпадения. Для всех совпадений используйте цикл */
        long start = (long)ovector[0];
        long end = (long)ovector[1];
        RETURN_STRINGL(subject + start, end - start);
    }

    pcre2_match_data_free(match_data);
    pcre2_code_free(re);
}
