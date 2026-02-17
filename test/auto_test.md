# Авто тест базы данных


Файл скрипта auto_test.php

Страница описания прокта [Fast_IO Engine](https://github.com/commeta/fast_io).


## Процедура тестирования

Тест для функции file_insert_line начинается с создания файла базы данных fast_io.dat. Затем генерируется случайный размер выравнивания и устанавливается размер буфера. После этого выполняется цикл, в котором в файл базы данных вставляются строки с использованием функции file_insert_line. Каждая вставленная строка проверяется на соответствие ожидаемым параметрам, включая размер и содержание. Также проверяется, что позиции вставленных строк корректны.

Плюсы этого теста включают:
- Использование случайных значений для имитации различных сценариев использования в допустимых диапазонах.
- Проверка различных аспектов работы функций, включая размер строк, содержание и позицию в файле.
- Проверка сходимости базы данных, убеждаясь, что вставленные строки находятся в правильном месте и имеют правильные размеры.
- Тест выводит на экран все ошибки и предупреждения, которые могут возникнуть при работе с базой данных.
- Тест выявляет нестабильное поведение, если генерация случайных значений приведет к ошибкам в работе расширения.

Минусы этого теста включают:
- Тест может быть медленным из-за большого количества операций: вызов функций в цикле, чтение и запись.


### Тестирование функций
- file_insert_line - Тест сходимости с участием: file_get_keys, file_analize, +рандомный размер буфера.
- file_analize - Тест сходимости 1 режима, с участием: file_insert_line, +рандомный размер буфера.
- file_get_keys - Тест сходимости транзакционных режимов, с участием: file_insert_line, +рандомный размер буфера.
- file_search_array - Тест сходимости транзакционных режимов, с участием: file_insert_line, +рандомный размер буфера.
- file_select_array - Тест сходимости транзакционных режимов, с участием: file_insert_line, +рандомный размер буфера.
- file_search_line - Тест сходимости транзакционных режимов, с участием: file_insert_line, +рандомный размер буфера.
- file_select_line - Тест сходимости транзакционных режимов, с участием: file_insert_line, +рандомный размер буфера.
- file_pop_line - Тест сходимости транзакционных режимов с авто-поиском строк, с участием: file_insert_line, +рандомный размер буфера.


### Результат выполнения
Результат тестирования: Ubuntu 24.04, Ryzen 12 Cores, 16GB RAM, SATA 3 SSD.
```
Check file_insert_line: time: 2.2905550003052 - PASS
rchar: 720775340 (314.67 millions per sec)
wchar: 197162612 (86.08 millions per sec)
syscr: 100019 (43,665.84 per sec)
syscw: 10976 (4,791.85 per sec)
read_bytes: 0 (0.00 millions per sec)
write_bytes: 197373952 (86.17 millions per sec)
cancelled_write_bytes: 195022848 (85.14 millions per sec)

Check file_analize: time: 4.8935198783875 - PASS
rchar: 5835507507 (1,192.50 millions per sec)
wchar: 176730792 (36.12 millions per sec)
syscr: 561243 (114,691.06 per sec)
syscw: 10559 (2,157.75 per sec)
read_bytes: 0 (0.00 millions per sec)
write_bytes: 176922624 (36.15 millions per sec)
cancelled_write_bytes: 169136128 (34.56 millions per sec)

Check file_get_keys: time: 1.9434449672699 - PASS
rchar: 1143084975 (588.17 millions per sec)
wchar: 188534502 (97.01 millions per sec)
syscr: 109596 (56,392.64 per sec)
syscw: 10593 (5,450.63 per sec)
read_bytes: 0 (0.00 millions per sec)
write_bytes: 188727296 (97.11 millions per sec)
cancelled_write_bytes: 190926848 (98.24 millions per sec)

Check file_search_array: time: 2.5149509906769 - PASS
rchar: 2263308193 (899.94 millions per sec)
wchar: 187432928 (74.53 millions per sec)
syscr: 201246 (80,019.85 per sec)
syscw: 11514 (4,578.22 per sec)
read_bytes: 0 (0.00 millions per sec)
write_bytes: 187641856 (74.61 millions per sec)
cancelled_write_bytes: 187138048 (74.41 millions per sec)

Check file_select_array: time: 2.9724228382111 - PASS
rchar: 3105559570 (1,044.79 millions per sec)
wchar: 206048455 (69.32 millions per sec)
syscr: 180310 (60,660.95 per sec)
syscw: 11865 (3,991.69 per sec)
read_bytes: 0 (0.00 millions per sec)
write_bytes: 206258176 (69.39 millions per sec)
cancelled_write_bytes: 202211328 (68.03 millions per sec)

Check file_search_line: time: 5.6465239524841 - PASS
rchar: 13536257872 (2,397.27 millions per sec)
wchar: 183461315 (32.49 millions per sec)
syscr: 966747 (171,211.00 per sec)
syscw: 10792 (1,911.26 per sec)
read_bytes: 0 (0.00 millions per sec)
write_bytes: 183656448 (32.53 millions per sec)
cancelled_write_bytes: 185110528 (32.78 millions per sec)

Check file_select_line: time: 1.6696162223816 - PASS
rchar: 781708335 (468.20 millions per sec)
wchar: 181673626 (108.81 millions per sec)
syscr: 91457 (54,777.26 per sec)
syscw: 11292 (6,763.23 per sec)
read_bytes: 0 (0.00 millions per sec)
write_bytes: 181874688 (108.93 millions per sec)
cancelled_write_bytes: 182554624 (109.34 millions per sec)

Check file_pop_line: time: 3.5449759960175 - PASS
rchar: 643707344 (181.58 millions per sec)
wchar: 350561535 (98.89 millions per sec)
syscr: 96229 (27,145.18 per sec)
syscw: 24132 (6,807.38 per sec)
read_bytes: 0 (0.00 millions per sec)
write_bytes: 357945344 (100.97 millions per sec)
cancelled_write_bytes: 357724160 (100.91 millions per sec)

Check file_callback_line: time: 1.6356911659241 - PASS
rchar: 654522459 (400.15 millions per sec)
wchar: 172482778 (105.45 millions per sec)
syscr: 70608 (43,167.07 per sec)
syscw: 10746 (6,569.70 per sec)
read_bytes: 0 (0.00 millions per sec)
write_bytes: 172675072 (105.57 millions per sec)
cancelled_write_bytes: 169607168 (103.69 millions per sec)

```


### Статистика ядра Linux Kernel

Источник данных: /proc/pid/io — файл, который содержит информацию о вводе-выводе процесса. Он используется для мониторинга использования дискового ввода-вывода процессом.

- time - время выполнения теста, в секундах.

- rchar (read chars) — количество прочитанных символов процессом. Этот параметр показывает, сколько байт было прочитано процессом с момента запуска теста.

- wchar (written chars) — количество записанных символов процессом. Этот параметр показывает, сколько байт было записано процессом с момента запуска теста.

- syscr (system calls read) — количество системных вызовов чтения, сделанных процессом. Этот параметр показывает, сколько раз процесс обращался к операционной системе с запросом на чтение данных.

- syscw (system calls write) — количество системных вызовов записи, сделанных процессом. Этот параметр показывает, сколько раз процесс обращался к операционной системе с запросом на запись данных.

- read_bytes — общее количество байт, прочитанных процессом с диска. Этот параметр показывает, сколько байт было прочитано процессом с диска с момента запуска теста.

- write_bytes — общее количество байт, записанных процессом на диск. Этот параметр показывает, сколько байт было записано процессом на диск с момента запуска теста.

- cancelled_write_bytes — количество байт, которые были отменены процессом при записи на диск. Этот параметр показывает, сколько байт было отменено процессом при попытке записи на диск.


### Профиль XDEBUG

###### Основные элементы интерфейса KCachegrind:

1. **Overview (Обзор)**:
    - **Call Graph (Граф вызовов)**: показывает иерархию вызовов функций в виде графа.
    - **Callers/Callees (Вызывающие/Вызванные)**: показывает функции, которые вызывают данную функцию и функции, которые вызываются данной функцией.
    - **Flat Profile (Плоский профиль)**: список всех функций с их метриками.

2. **Detail View (Детальный вид)**:
    - Отображает подробную информацию о выбранной функции, включая метрики времени и памяти.

### Метрики:

1. **time_(10ns)**:
    - Это время выполнения функции, измеренное в 10 наносекундных интервалах. Например, значение 10000 соответствует 100 микросекундам.
    - **Self (Собственное время)**: время, затраченное непосредственно на выполнение этой функции без учета времени, затраченного на вызванные ею функции.
    - **Inclusive (Включительное время)**: общее время выполнения функции, включая все вызовы других функций из этой функции.

2. **memory_(bytes)**:
    - Количество памяти, используемой функцией.
    - **Self**: память, используемая непосредственно этой функцией.
    - **Inclusive**: память, используемая этой функцией и всеми вызванными ею функциями.

3. **Calls (Вызовы)**:
    - Количество раз, когда функция была вызвана.


### Стоимость по времени
![auto_test_profile_time](https://raw.githubusercontent.com/commeta/fast_io/master/test/auto_test_profile_time.png "auto_test_profile_time")

### Стоимость по памяти
![auto_test_profile_mem](https://raw.githubusercontent.com/commeta/fast_io/master/test/auto_test_profile_mem.png "auto_test_profile_mem")

