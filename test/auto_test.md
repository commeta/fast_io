# Авто тест базы данных


Файл скрипта auto_test.php

Страница описания прокта [Fast_IO Engine](https://github.com/commeta/fast_io).


### Процедура тестирования

Тест для функции file_insert_line начинается с создания файла базы данных fast_io.dat. Затем генерируется случайный размер выравнивания и устанавливается размер буфера. После этого выполняется цикл, в котором в файл базы данных вставляются строки с использованием функции file_insert_line. Каждая вставленная строка проверяется на соответствие ожидаемым параметрам, включая размер и содержание. Также проверяется, что позиции вставленных строк корректны.

Плюсы этого теста включают:
- Использование случайных значений для имитации различных сценариев использования в допустимых диапазонах.
- Проверка различных аспектов работы функций, включая размер строк, содержание и позицию в файле.
- Проверка сходимости базы данных, убеждаясь, что вставленные строки находятся в правильном месте и имеют правильные размеры.
- Тест выводит на экран все ошибки и предупреждения, которые могут возникнуть при работе с базой данных.
- Тест выявляет нестабильное поведение, если генерация случайных значений приведет к ошибкам в работе расширения.

Минусы этого теста включают:
- Тест может быть медленным из-за большого количества операций: вызов функций в цикле, чтение и запись.


#### Тестирование функций
- file_insert_line - Тест сходимости с участием: file_get_keys, file_analize, +рандомный размер буфера.
- file_analize - Тест сходимости 1 режима, с участием: file_insert_line, +рандомный размер буфера.
- file_get_keys - Тест сходимости транзакционных режимов, с участием: file_insert_line, +рандомный размер буфера.
- file_search_array - Тест сходимости транзакционных режимов, с участием: file_insert_line, +рандомный размер буфера.
- file_select_array - Тест сходимости транзакционных режимов, с участием: file_insert_line, +рандомный размер буфера.
- file_search_line - Тест сходимости транзакционных режимов, с участием: file_insert_line, +рандомный размер буфера.
- file_select_line - Тест сходимости транзакционных режимов, с участием: file_insert_line, +рандомный размер буфера.
- file_pop_line - Тест сходимости транзакционных режимов с авто-поиском строк, с участием: file_insert_line, +рандомный размер буфера.


#### Результат выполнения
Результат тестирования: Ubuntu 24.04, Ryzen 12 Cores, 16GB RAM, SATA 3 SSD.
```
Check file_insert_line: time: 2.3249299526215 - PASS
rchar: 734991037 (316.13 millions per sec)
wchar: 197211103 (84.82 millions per sec)
syscr: 82689 (35,566.23 per sec)
syscw: 10742 (4,620.35 per sec)
read_bytes: 0 (0.00 millions per sec)
write_bytes: 197427200 (84.92 millions per sec)
cancelled_write_bytes: 196390912 (84.47 millions per sec)

Check file_analize: time: 6.8016469478607 - PASS
rchar: 7602825353 (1,117.79 millions per sec)
wchar: 215786744 (31.73 millions per sec)
syscr: 768615 (113,004.25 per sec)
syscw: 11951 (1,757.07 per sec)
read_bytes: 0 (0.00 millions per sec)
write_bytes: 216023040 (31.76 millions per sec)
cancelled_write_bytes: 216506368 (31.83 millions per sec)

Check file_get_keys: time: 1.927946805954 - PASS
rchar: 1077162792 (558.71 millions per sec)
wchar: 179332885 (93.02 millions per sec)
syscr: 92253 (47,850.39 per sec)
syscw: 10206 (5,293.71 per sec)
read_bytes: 0 (0.00 millions per sec)
write_bytes: 179548160 (93.13 millions per sec)
cancelled_write_bytes: 176144384 (91.36 millions per sec)

Check file_search_array: time: 2.6650650501251 - PASS
rchar: 2204118948 (827.04 millions per sec)
wchar: 183480195 (68.85 millions per sec)
syscr: 195569 (73,382.45 per sec)
syscw: 10288 (3,860.32 per sec)
read_bytes: 0 (0.00 millions per sec)
write_bytes: 183689216 (68.92 millions per sec)
cancelled_write_bytes: 182661120 (68.54 millions per sec)

Check file_select_array: time: 3.3443758487701 - PASS
rchar: 3093490454 (924.98 millions per sec)
wchar: 206018159 (61.60 millions per sec)
syscr: 182073 (54,441.55 per sec)
syscw: 12269 (3,668.55 per sec)
read_bytes: 0 (0.00 millions per sec)
write_bytes: 206213120 (61.66 millions per sec)
cancelled_write_bytes: 207093760 (61.92 millions per sec)

Check file_search_line: time: 7.2499101161957 - PASS
rchar: 16776156810 (2,313.98 millions per sec)
wchar: 216642676 (29.88 millions per sec)
syscr: 1770967 (244,274.34 per sec)
syscw: 11680 (1,611.05 per sec)
read_bytes: 0 (0.00 millions per sec)
write_bytes: 216854528 (29.91 millions per sec)
cancelled_write_bytes: 206196736 (28.44 millions per sec)

Check file_select_line: time: 1.8261711597443 - PASS
rchar: 785417171 (430.09 millions per sec)
wchar: 186901077 (102.35 millions per sec)
syscr: 85439 (46,785.87 per sec)
syscw: 10949 (5,995.60 per sec)
read_bytes: 0 (0.00 millions per sec)
write_bytes: 187133952 (102.47 millions per sec)
cancelled_write_bytes: 186793984 (102.29 millions per sec)

Check file_pop_line: time: 5.9825229644775 - PASS
rchar: 1425209869 (238.23 millions per sec)
wchar: 901270799 (150.65 millions per sec)
syscr: 154466 (25,819.54 per sec)
syscw: 46904 (7,840.17 per sec)
read_bytes: 0 (0.00 millions per sec)
write_bytes: 902180864 (150.80 millions per sec)
cancelled_write_bytes: 897216512 (149.97 millions per sec)

Check file_callback_line: time: 2.1984159946442 - PASS
rchar: 674857333 (306.97 millions per sec)
wchar: 185078722 (84.19 millions per sec)
syscr: 71290 (32,427.89 per sec)
syscw: 11074 (5,037.26 per sec)
read_bytes: 0 (0.00 millions per sec)
write_bytes: 185290752 (84.28 millions per sec)
cancelled_write_bytes: 186834944 (84.99 millions per sec)

```


##### Статистика ядра Linux Kernel

Источник данных: /proc/pid/io — файл, который содержит информацию о вводе-выводе процесса. Он используется для мониторинга использования дискового ввода-вывода процессом.

- time - время выполнения теста, в секундах.

- rchar (read chars) — количество прочитанных символов процессом. Этот параметр показывает, сколько байт было прочитано процессом с момента запуска теста.

- wchar (written chars) — количество записанных символов процессом. Этот параметр показывает, сколько байт было записано процессом с момента запуска теста.

- syscr (system calls read) — количество системных вызовов чтения, сделанных процессом. Этот параметр показывает, сколько раз процесс обращался к операционной системе с запросом на чтение данных.

- syscw (system calls write) — количество системных вызовов записи, сделанных процессом. Этот параметр показывает, сколько раз процесс обращался к операционной системе с запросом на запись данных.

- read_bytes — общее количество байт, прочитанных процессом с диска. Этот параметр показывает, сколько байт было прочитано процессом с диска с момента запуска теста.

- write_bytes — общее количество байт, записанных процессом на диск. Этот параметр показывает, сколько байт было записано процессом на диск с момента запуска теста.

- cancelled_write_bytes — количество байт, которые были отменены процессом при записи на диск. Этот параметр показывает, сколько байт было отменено процессом при попытке записи на диск.


