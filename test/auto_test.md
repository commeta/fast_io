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
Check file_insert_line: time: 1.9269127845764 - PASS
rchar: 674020973 (349.79 millions per sec)
wchar: 174840700 (90.74 millions per sec)
syscr: 71455 (37,082.63 per sec)
syscw: 10495 (5,446.54 per sec)
read_bytes: 0 (0.00 millions per sec)
write_bytes: 175058944 (90.85 millions per sec)
cancelled_write_bytes: 168718336 (87.56 millions per sec)

Check file_analize: time: 4.6234958171844 - PASS
rchar: 7684914475 (1,662.14 millions per sec)
wchar: 211675884 (45.78 millions per sec)
syscr: 600218 (129,819.09 per sec)
syscw: 11690 (2,528.39 per sec)
read_bytes: 0 (0.00 millions per sec)
write_bytes: 211869696 (45.82 millions per sec)
cancelled_write_bytes: 216678400 (46.86 millions per sec)

Check file_get_keys: time: 2.1775228977203 - PASS
rchar: 1179394176 (541.62 millions per sec)
wchar: 196357512 (90.17 millions per sec)
syscr: 119811 (55,021.69 per sec)
syscw: 11056 (5,077.33 per sec)
read_bytes: 0 (0.00 millions per sec)
write_bytes: 196562944 (90.27 millions per sec)
cancelled_write_bytes: 194015232 (89.10 millions per sec)

Check file_search_array: time: 2.6541838645935 - PASS
rchar: 2158531044 (813.26 millions per sec)
wchar: 179663771 (67.69 millions per sec)
syscr: 211077 (79,526.14 per sec)
syscw: 10118 (3,812.09 per sec)
read_bytes: 0 (0.00 millions per sec)
write_bytes: 179859456 (67.76 millions per sec)
cancelled_write_bytes: 180285440 (67.93 millions per sec)

Check file_select_array: time: 2.4541449546814 - PASS
rchar: 2465994254 (1,004.83 millions per sec)
wchar: 164208502 (66.91 millions per sec)
syscr: 148473 (60,498.87 per sec)
syscw: 10405 (4,239.77 per sec)
read_bytes: 0 (0.00 millions per sec)
write_bytes: 164421632 (67.00 millions per sec)
cancelled_write_bytes: 161804288 (65.93 millions per sec)

Check file_search_line: time: 5.820433139801 - PASS
rchar: 12680275214 (2,178.58 millions per sec)
wchar: 177966239 (30.58 millions per sec)
syscr: 1344481 (230,993.29 per sec)
syscw: 11131 (1,912.40 per sec)
read_bytes: 0 (0.00 millions per sec)
write_bytes: 178176000 (30.61 millions per sec)
cancelled_write_bytes: 180559872 (31.02 millions per sec)

Check file_select_line: time: 1.6426420211792 - PASS
rchar: 702575103 (427.71 millions per sec)
wchar: 164970973 (100.43 millions per sec)
syscr: 79011 (48,099.95 per sec)
syscw: 10222 (6,222.90 per sec)
read_bytes: 0 (0.00 millions per sec)
write_bytes: 165183488 (100.56 millions per sec)
cancelled_write_bytes: 163766272 (99.70 millions per sec)

Check file_pop_line: time: 5.3802130222321 - PASS
rchar: 1289819041 (239.73 millions per sec)
wchar: 773095295 (143.69 millions per sec)
syscr: 165699 (30,797.85 per sec)
syscw: 46892 (8,715.64 per sec)
read_bytes: 0 (0.00 millions per sec)
write_bytes: 773881856 (143.84 millions per sec)
cancelled_write_bytes: 774709248 (143.99 millions per sec)

Check file_callback_line: time: 1.8096261024475 - PASS
rchar: 578088413 (319.45 millions per sec)
wchar: 153451752 (84.80 millions per sec)
syscr: 55345 (30,583.67 per sec)
syscw: 10048 (5,552.53 per sec)
read_bytes: 0 (0.00 millions per sec)
write_bytes: 153649152 (84.91 millions per sec)
cancelled_write_bytes: 154738688 (85.51 millions per sec)


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


##### Профиль XDEBUG


###### Стоимость по времени
![auto_test_profile_time](https://raw.githubusercontent.com/commeta/fast_io/master/test/auto_test_profile_time.png "auto_test_profile_time")

###### Стоимость по памяти
![auto_test_profile_mem](https://raw.githubusercontent.com/commeta/fast_io/master/test/auto_test_profile_mem.png "auto_test_profile_mem")

