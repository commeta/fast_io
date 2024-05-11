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
Check file_insert_line: time: 2.0206029415131 - PASS
rchar: 668356378 (330.77 millions per sec)
wchar: 169830513 (84.05 millions per sec)
syscr: 71381 (35,326.58 per sec)
syscw: 10422 (5,157.87 per sec)
read_bytes: 0 (0.00 millions per sec)
write_bytes: 170061824 (84.16 millions per sec)
cancelled_write_bytes: 166739968 (82.52 millions per sec)

Check file_analize: time: 4.5036659240723 - PASS
rchar: 7088363107 (1,573.91 millions per sec)
wchar: 201598125 (44.76 millions per sec)
syscr: 778566 (172,873.84 per sec)
syscw: 10640 (2,362.52 per sec)
read_bytes: 0 (0.00 millions per sec)
write_bytes: 201822208 (44.81 millions per sec)
cancelled_write_bytes: 204009472 (45.30 millions per sec)

Check file_get_keys: time: 1.9767808914185 - PASS
rchar: 1123353516 (568.27 millions per sec)
wchar: 186991393 (94.59 millions per sec)
syscr: 105705 (53,473.30 per sec)
syscw: 11342 (5,737.61 per sec)
read_bytes: 0 (0.00 millions per sec)
write_bytes: 187166720 (94.68 millions per sec)
cancelled_write_bytes: 187129856 (94.66 millions per sec)

Check file_search_array: time: 2.339879989624 - PASS
rchar: 1907223540 (815.09 millions per sec)
wchar: 158733488 (67.84 millions per sec)
syscr: 215737 (92,200.03 per sec)
syscw: 9908 (4,234.41 per sec)
read_bytes: 0 (0.00 millions per sec)
write_bytes: 158941184 (67.93 millions per sec)
cancelled_write_bytes: 153329664 (65.53 millions per sec)

Check file_select_array: time: 2.6338698863983 - PASS
rchar: 2686374164 (1,019.93 millions per sec)
wchar: 178893952 (67.92 millions per sec)
syscr: 160128 (60,795.71 per sec)
syscw: 11026 (4,186.24 per sec)
read_bytes: 0 (0.00 millions per sec)
write_bytes: 179105792 (68.00 millions per sec)
cancelled_write_bytes: 179871744 (68.29 millions per sec)

Check file_search_line: time: 5.644623041153 - PASS
rchar: 13996705470 (2,479.65 millions per sec)
wchar: 188313363 (33.36 millions per sec)
syscr: 1356663 (240,346.08 per sec)
syscw: 11565 (2,048.85 per sec)
read_bytes: 0 (0.00 millions per sec)
write_bytes: 188534784 (33.40 millions per sec)
cancelled_write_bytes: 187154432 (33.16 millions per sec)

Check file_select_line: time: 1.7338440418243 - PASS
rchar: 763166643 (440.16 millions per sec)
wchar: 181497808 (104.68 millions per sec)
syscr: 85299 (49,196.47 per sec)
syscw: 10955 (6,318.33 per sec)
read_bytes: 0 (0.00 millions per sec)
write_bytes: 181723136 (104.81 millions per sec)
cancelled_write_bytes: 180125696 (103.89 millions per sec)

Check file_pop_line: time: 2.3454391956329 - PASS
rchar: 710848655 (303.08 millions per sec)
wchar: 441092267 (188.06 millions per sec)
syscr: 85833 (36,595.70 per sec)
syscw: 24924 (10,626.58 per sec)
read_bytes: 0 (0.00 millions per sec)
write_bytes: 445120512 (189.78 millions per sec)
cancelled_write_bytes: 443826176 (189.23 millions per sec)

Check file_callback_line: time: 1.5155608654022 - PASS
rchar: 197454865 (130.29 millions per sec)
wchar: 175354555 (115.70 millions per sec)
syscr: 21195 (13,984.92 per sec)
syscw: 10706 (7,064.05 per sec)
read_bytes: 0 (0.00 millions per sec)
write_bytes: 175566848 (115.84 millions per sec)
cancelled_write_bytes: 175894528 (116.06 millions per sec)
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


