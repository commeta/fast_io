# Test

Стресс тест [test.php](test.php) запускался в однопользовательском режиме, без использования механизма блокировок!

Для реализации транзакции достаточно использовать блокировку на уровне файла [пример](transaction/README.md)

[mysql-adapter](mysql-adapter/readme.md) использует [greenlion/PHP-SQL-Parser](https://github.com/greenlion/PHP-SQL-Parser), но подойдет любой другой [SQL / SQLI tokenizer parser analyzer](https://github.com/client9/libinjection)


Страница описания прокта [Fast_IO Engine](https://github.com/commeta/fast_io).



## Авто тест базы данных

[Скрипт автоматического тестирования базы данных Fast_IO Engine](auto_test.md)

Плюсы этого теста включают:
- Использование случайных значений для имитации различных сценариев использования в допустимых диапазонах.
- Проверка различных аспектов работы функций, включая размер строк, содержание и позицию в файле.
- Проверка сходимости базы данных, убеждаясь, что вставленные строки находятся в правильном месте и имеют правильные размеры.
- Тест выводит на экран все ошибки и предупреждения, которые могут возникнуть при работе с базой данных.
- Тест выявляет нестабильное поведение, если генерация случайных значений приведет к ошибкам в работе расширения.

Минусы этого теста включают:
- Тест может быть медленным из-за большого количества операций: вызов функций в цикле, чтение и запись.



### Адресация

```
<?php
$data_file = __DIR__ . '/fast_io.dat';
$align = 64; // line_number - длина 12 байт, 52 байта под данные.


// Данные с выравниванием
$last_line_number = 0;
if(file_exists($data_file) && filesize($data_file) > 0){
	$last_line_number = filesize($data_file) / $align;
}

$new_line_number = file_insert_line($data_file, 'insert_key_value_' . $last_line_number, 0, $align); // Добавить строку в файл с выравниванием
$str = file_select_line($data_file, $new_line_number, $align); // Получить строку из файла по номеру строки

print_r([$last_line_number, $new_line_number, $str]);


// Даннае без выравнивания
$last_offset = 0;
if(file_exists($data_file . '.dat') && filesize($data_file) > 0){
	$last_offset = filesize($data_file . '.dat');
}

$str = "write_key_value_pair_" . $last_offset;
$new_offset = file_insert_line($data_file . '.dat', $str, 2); // Добавить строку в файл без выравнивания
$new_str = file_select_line($data_file . '.dat', $new_offset, mb_strlen($str), 1); // Получить строку из файла по смещению

print_r([$last_offset, $new_offset, $new_str]);
```

Результат
```
Array
(
    [0] => 0
    [1] => 0
    [2] => insert_key_value_0
)
Array
(
    [0] => 0
    [1] => 0
    [2] => write_key_value_pair_0
)
```

второй запуск
```
Array
(
    [0] => 1
    [1] => 1
    [2] => insert_key_value_1
)
Array
(
    [0] => 23
    [1] => 23
    [2] => write_key_value_pair_23
)

```




### Результаты стресс теста: test.php

```
root@api:/home/commeta/project/kernel/fast_io# ./compile.sh
find . -name \*.gcno -o -name \*.gcda | xargs rm -f
find . -name \*.lo -o -name \*.o -o -name \*.dep | xargs rm -f
find . -name \*.la -o -name \*.a | xargs rm -f
find . -name \*.so | xargs rm -f
find . -name .libs -a -type d|xargs rm -rf
rm -f libphp.la      modules/* libs/*
rm -f ext/opcache/jit/zend_jit_x86.c
rm -f ext/opcache/jit/zend_jit_arm64.c
rm -f ext/opcache/minilua
/bin/bash /home/commeta/project/kernel/fast_io/libtool --tag=CC --mode=compile cc -I. -I/home/commeta/project/kernel/fast_io -I/home/commeta/project/kernel/fast_io/include -I/home/commeta/project/kernel/fast_io/main -I/home/commeta/project/kernel/fast_io -I/usr/include/php/20230831 -I/usr/include/php/20230831/main -I/usr/include/php/20230831/TSRM -I/usr/include/php/20230831/Zend -I/usr/include/php/20230831/ext -I/usr/include/php/20230831/ext/date/lib  -DHAVE_CONFIG_H  -g -O2 -D_GNU_SOURCE    -DZEND_COMPILE_DL_EXT=1 -c /home/commeta/project/kernel/fast_io/fast_io.c -o fast_io.lo  -MMD -MF fast_io.dep -MT fast_io.lo
libtool: compile:  cc -I. -I/home/commeta/project/kernel/fast_io -I/home/commeta/project/kernel/fast_io/include -I/home/commeta/project/kernel/fast_io/main -I/home/commeta/project/kernel/fast_io -I/usr/include/php/20230831 -I/usr/include/php/20230831/main -I/usr/include/php/20230831/TSRM -I/usr/include/php/20230831/Zend -I/usr/include/php/20230831/ext -I/usr/include/php/20230831/ext/date/lib -DHAVE_CONFIG_H -g -O2 -D_GNU_SOURCE -DZEND_COMPILE_DL_EXT=1 -c /home/commeta/project/kernel/fast_io/fast_io.c -MMD -MF fast_io.dep -MT fast_io.lo  -fPIC -DPIC -o .libs/fast_io.o
/bin/bash /home/commeta/project/kernel/fast_io/libtool --tag=CC --mode=link cc -shared -I/home/commeta/project/kernel/fast_io/include -I/home/commeta/project/kernel/fast_io/main -I/home/commeta/project/kernel/fast_io -I/usr/include/php/20230831 -I/usr/include/php/20230831/main -I/usr/include/php/20230831/TSRM -I/usr/include/php/20230831/Zend -I/usr/include/php/20230831/ext -I/usr/include/php/20230831/ext/date/lib  -DHAVE_CONFIG_H  -g -O2 -D_GNU_SOURCE    -o fast_io.la -export-dynamic -avoid-version -prefer-pic -module -rpath /home/commeta/project/kernel/fast_io/modules  fast_io.lo 
libtool: link: cc -shared  -fPIC -DPIC  .libs/fast_io.o    -g -O2   -Wl,-soname -Wl,fast_io.so -o .libs/fast_io.so
libtool: link: ( cd ".libs" && rm -f "fast_io.la" && ln -s "../fast_io.la" "fast_io.la" )
/bin/bash /home/commeta/project/kernel/fast_io/libtool --tag=CC --mode=install cp ./fast_io.la /home/commeta/project/kernel/fast_io/modules
libtool: install: cp ./.libs/fast_io.so /home/commeta/project/kernel/fast_io/modules/fast_io.so
libtool: install: cp ./.libs/fast_io.lai /home/commeta/project/kernel/fast_io/modules/fast_io.la
libtool: finish: PATH="/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/snap/bin:/sbin" ldconfig -n /home/commeta/project/kernel/fast_io/modules
----------------------------------------------------------------------
Libraries have been installed in:
   /home/commeta/project/kernel/fast_io/modules

If you ever happen to want to link against installed libraries
in a given directory, LIBDIR, you must either use libtool, and
specify the full pathname of the library, or use the '-LLIBDIR'
flag during linking and do at least one of the following:
   - add LIBDIR to the 'LD_LIBRARY_PATH' environment variable
     during execution
   - add LIBDIR to the 'LD_RUN_PATH' environment variable
     during linking
   - use the '-Wl,-rpath -Wl,LIBDIR' linker flag
   - have your system administrator add LIBDIR to '/etc/ld.so.conf'

See any operating system documentation about shared libraries for
more information, such as the ld(1) and ld.so(8) manual pages.
----------------------------------------------------------------------

Build complete.
Don't forget to run 'make test'.


Build complete.
Don't forget to run 'make test'.


=====================================================================
PHP         : /usr/bin/php8.3 
PHP_SAPI    : cli
PHP_VERSION : 8.3.6
ZEND_VERSION: 4.3.6
PHP_OS      : Linux - Linux api.webdevops.ru 6.8.0-31-generic #31-Ubuntu SMP PREEMPT_DYNAMIC Sat Apr 20 00:40:06 UTC 2024 x86_64
INI actual  : /home/commeta/project/kernel/fast_io/tmp-php.ini
More .INIs  :   
CWD         : /home/commeta/project/kernel/fast_io
Extra dirs  : 
VALGRIND    : Not used
=====================================================================
TIME START 2024-05-04 19:58:10
=====================================================================
No tests were run.
0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255, 256, 257, 258, 259, 260, 261, 262, 263, 264, 265, 266, 267, 268, 269, 270, 271, 272, 273, 274, 275, 276, 277, 278, 279, 280, 281, 282, 283, 284, 285, 286, 287, 288, 289, 290, 291, 292, 293, 294, 295, 296, 297, 298, 299, 300, 301, 302, 303, 304, 305, 306, 307, 308, 309, 310, 311, 312, 313, 314, 315, 316, 317, 318, 319, 320, 321, 322, 323, 324, 325, 326, 327, 328, 329, 330, 331, 332, 333, 334, 335, 336, 337, 338, 339, 340, 341, 342, 343, 344, 345, 346, 347, 348, 349, 350, 351, 352, 353, 354, 355, 356, 357, 358, 359, 360, 361, 362, 363, 364, 365, 366, 367, 368, 369, 370, 371, 372, 373, 374, 375, 376, 377, 378, 379, 380, 381, 382, 383, 384, 385, 386, 387, 388, 389, 390, 391, 392, 393, 394, 395, 396, 397, 398, 399, 400, 401, 402, 403, 404, 405, 406, 407, 408, 409, 410, 411, 412, 413, 414, 415, 416, 417, 418, 419, 420, 421, 422, 423, 424, 425, 426, 427, 428, 429, 430, 431, 432, 433, 434, 435, 436, 437, 438, 439, 440, 441, 442, 443, 444, 445, 446, 447, 448, 449, 450, 451, 452, 453, 454, 455, 456, 457, 458, 459, 460, 461, 462, 463, 464, 465, 466, 467, 468, 469, 470, 471, 472, 473, 474, 475, 476, 477, 478, 479, 480, 481, 482, 483, 484, 485, 486, 487, 488, 489, 490, 491, 492, 493, 494, 495, 496, 497, 498, 499, 500, Array
(
    [0] => file_get_keys
    [1] => Array
        (
            [0] => Array
                (
                    [key] => index_0
                    [line_offset] => 0
                    [line_length] => 8192
                    [line_count] => 1
                )

        )

)
Array
(
    [0] => file_search_array
    [1] => Array
        (
            [line_count] => 501
            [found_count] => 501
        )

    [2] => Array
        (
            [0] => Array
                (
                    [trim_line] => index_0 file_insert_line_0 12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012
                    [trim_length] => 119
                    [line_offset] => 0
                    [line_length] => 8192
                    [line_count] => 1
                )

            [1] => Array
                (
                    [trim_line] => index_1 file_insert_line_1 12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012
                    [trim_length] => 119
                    [line_offset] => 8192
                    [line_length] => 8192
                    [line_count] => 2
                )

        )

    [3] => Array
        (
            [0] => Array
                (
                    [trim_line] => index_0 file_insert_line_0 12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012
                    [trim_length] => 8192
                    [line_offset] => 0
                    [line_length] => 8192
                    [line_count] => 1
                )

            [1] => Array
                (
                    [trim_line] => index_1 file_insert_line_1 12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012
                    [trim_length] => 8192
                    [line_offset] => 8192
                    [line_length] => 8192
                    [line_count] => 2
                )

        )

)
Array
(
    [0] => file_select_array
    [1] => Array
        (
            [0] => Array
                (
                    [trim_line] => index_1 file_insert_line_1 12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012
                    [trim_length] => 119
                    [line_offset] => 8192
                    [line_length] => 8192
                )

            [1] => Array
                (
                    [trim_line] => index_2 file_insert_line_2 12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012
                    [trim_length] => 119
                    [line_offset] => 16384
                    [line_length] => 8192
                )

            [2] => Array
                (
                    [trim_line] => index_3 file_insert_line_3 12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012
                    [trim_length] => 119
                    [line_offset] => 24576
                    [line_length] => 8192
                )

        )

    [2] => Array
        (
            [0] => Array
                (
                    [trim_line] => index_1 file_insert_line_1 12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012
                    [trim_length] => 119
                    [line_offset] => 8192
                    [line_length] => 8192
                )

            [1] => Array
                (
                    [trim_line] => index_2 file_insert_line_2 12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012
                    [trim_length] => 119
                    [line_offset] => 16384
                    [line_length] => 8192
                )

            [2] => Array
                (
                    [trim_line] => index_3 file_insert_line_3 12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012
                    [trim_length] => 119
                    [line_offset] => 24576
                    [line_length] => 8192
                )

        )

    [3] => Array
        (
            [0] => Array
                (
                    [0] => index_0
                    [1] => file_insert_line_0
                )

            [1] => Array
                (
                    [0] => index_1
                    [1] => file_insert_line_1
                )

        )

)
Array
(
    [0] => file_select_line
    [1] => index_0 file_insert_line_0 12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012
    [2] => Array
        (
            [0] => index_
            [1] => file_insert_line_
        )

    [3] => Array
        (
            [0] => Array
                (
                    [line_match] => index_
                    [match_offset] => 0
                    [match_length] => 6
                )

            [1] => Array
                (
                    [line_match] => file_insert_line_
                    [match_offset] => 6
                    [match_length] => 17
                )

        )

)
Array
(
    [0] => file_update_line
    [1] => 8192
)
Array
(
    [0] => file_pop_line
    [1] => index_500 file_insert_line_500 12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012
)
Array
(
    [0] => file_pop_line
    [1] => index_499 file_insert_line_499 12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012
)
Array
(
    [0] => file_get_keys
    [1] => Array
        (
            [0] => Array
                (
                    [key] => index_1
                    [line_offset] => 8192
                    [line_length] => 8192
                    [line_count] => 2
                )

            [1] => Array
                (
                    [key] => index_2
                    [line_offset] => 16384
                    [line_length] => 8192
                    [line_count] => 3
                )

            [2] => Array
                (
                    [key] => file_update_line
                    [line_offset] => 24576
                    [line_length] => 8192
                    [line_count] => 4
                )

            [3] => Array
                (
                    [key] => index_4
                    [line_offset] => 32768
                    [line_length] => 8192
                    [line_count] => 5
                )

            [4] => Array
                (
                    [key] => index_5
                    [line_offset] => 40960
                    [line_length] => 8192
                    [line_count] => 6
                )

        )

)
Array
(
    [0] => file_erase_line
    [1] => 49152
)
Array
(
    [0] => file_defrag_lines
    [1] => 2
)
Array
(
    [0] => file_update_line
    [1] => 8192
)
Array
(
    [0] => file_search_line
    [1] => 
    [2] => update апдейт
    [3] => index_0 file_insert_line_0 12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012
    [4] => index_1 file_insert_line_1 12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012
    [5] => index_1 file_insert_line_1 12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012
)
0, 22, 44, 66, 88, 110, 132, 154, 176, 198, 220, 243, 266, 289, 312, 335, 358, 381, 404, 427, 450, Array
(
    [0] => file_defrag_data
    [1] => 11
)
Array
(
    [0] => file_get_keys
    [1] => Array
        (
            [0] => Array
                (
                    [key] => index_11
                    [line_offset] => 0
                    [line_length] => 16
                    [line_count] => 1
                )

        )

)
Array
(
    [0] => replicate_file mode 1
    [1] => 390
)
/home/commeta/project/kernel/fast_io/fast_io3.dat
0,1,2,3,4,5,6,7,8,9,9,Array
(
    [0] => file_search_line
    [1] => index_5 data_file_insert_line_5
)
Array
(
    [0] => file_search_line
    [1] => 
)
Array
(
    [0] => file_pop_line
    [1] => index_10 data_file_insert_line_10
)
Array
(
    [0] => file_replace_line
    [1] => 9
)
Array
(
    [0] => replicate_file
    [1] => 242
)
/home/commeta/project/kernel/fast_io/fast_io4.dat
0 offset:0, 1 offset:22, 2 offset:44, 3 offset:66, 4 offset:88, 5 offset:110, 6 offset:132, 7 offset:154, 8 offset:176, 9 offset:198, 10 offset:220, 11 offset:243, 12 offset:266, 13 offset:289, 14 offset:312, 15 offset:335, 16 offset:358, 17 offset:381, 18 offset:404, 19 offset:427, 20 offset:450, 21 offset:473, 22 offset:496, 23 offset:519, 24 offset:542, 25 offset:565, 26 offset:588, 27 offset:611, 28 offset:634, 29 offset:657, 30 offset:680, 31 offset:703, 32 offset:726, 33 offset:749, 34 offset:772, 35 offset:795, 36 offset:818, 37 offset:841, 38 offset:864, 39 offset:887, 40 offset:910, 41 offset:933, 42 offset:956, 43 offset:979, 44 offset:1002, 45 offset:1025, 46 offset:1048, 47 offset:1071, 48 offset:1094, 49 offset:1117, 50 offset:1140, 51 offset:1163, 52 offset:1186, 53 offset:1209, 54 offset:1232, 55 offset:1255, 56 offset:1278, 57 offset:1301, 58 offset:1324, 59 offset:1347, 60 offset:1370, 61 offset:1393, 62 offset:1416, 63 offset:1439, 64 offset:1462, 65 offset:1485, 66 offset:1508, 67 offset:1531, 68 offset:1554, 69 offset:1577, 70 offset:1600, 71 offset:1623, 72 offset:1646, 73 offset:1669, 74 offset:1692, 75 offset:1715, 76 offset:1738, 77 offset:1761, 78 offset:1784, 79 offset:1807, 80 offset:1830, 81 offset:1853, 82 offset:1876, 83 offset:1899, 84 offset:1922, 85 offset:1945, 86 offset:1968, 87 offset:1991, 88 offset:2014, 89 offset:2037, 90 offset:2060, 91 offset:2083, 92 offset:2106, 93 offset:2129, 94 offset:2152, 95 offset:2175, 96 offset:2198, 97 offset:2221, 98 offset:2244, 99 offset:2267, 100 offset:2290, 101 offset:2314, 102 offset:2338, 103 offset:2362, 104 offset:2386, 105 offset:2410, 106 offset:2434, 107 offset:2458, 108 offset:2482, 109 offset:2506, 110 offset:2530, Array
(
    [0] => file_search_data
    [1] => data_file_push_data_10
)
Array
(
    [0] => file_search_data
    [1] => data_file_push_data_100
)
Array
(
    [0] => file_defrag_lines
    [1] => 11
)
Array
(
    [0] => file_search_data
    [1] => data_file_push_data_20
)
file_insert_line: 0.099200010299683 (0.00000992)
file_search_line: 2.9493789672852 (0.00029494)
file_search_line repeat: 0.088409900665283 (0.00000884)
file_defrag_lines: 0.63556289672852 (0.00006356)
file_push_data: 0.21883606910706 (0.00002188)
file_search_data: 2.3972780704498 (0.00023973)
file_search_data repeat: 0.15858101844788 (0.00001586)
file_pop_line: 0.2496030330658 (0.00002496)
Array
(
    [0] => memory_get_process_usage_kernel in Kilo Bytes
    [1] => 28020
    [2] => 28276
    [3] => 256
    [4] => get_process_io_stats
    [5] => Array
        (
            [rchar] => 3355633208
            [wchar] => 17303726
            [syscr] => 889902
            [syscw] => 44556
            [read_bytes] => 0
            [write_bytes] => 11374592
            [cancelled_write_bytes] => 6795264
        )

)

```


## Отчет Valgrind leak-check

```
root@api:/home/commeta/project/kernel/fast_io# valgrind --leak-check=full php -dxdebug.mode=debug test.php
==127710== Memcheck, a memory error detector
==127710== Copyright (C) 2002-2022, and GNU GPL'd, by Julian Seward et al.
==127710== Using Valgrind-3.22.0 and LibVEX; rerun with -h for copyright info
==127710== Command: php -dxdebug.mode=debug test.php
==127710== 

==127710== 
==127710== HEAP SUMMARY:
==127710==     in use at exit: 77,809 bytes in 1,349 blocks
==127710==   total heap usage: 293,530 allocs, 292,181 frees, 603,494,748 bytes allocated
==127710== 
==127710== LEAK SUMMARY:
==127710==    definitely lost: 0 bytes in 0 blocks
==127710==    indirectly lost: 0 bytes in 0 blocks
==127710==      possibly lost: 0 bytes in 0 blocks
==127710==    still reachable: 77,809 bytes in 1,349 blocks
==127710==         suppressed: 0 bytes in 0 blocks
==127710== Reachable blocks (those to which a pointer was found) are not shown.
==127710== To see them, rerun with: --leak-check=full --show-leak-kinds=all
==127710== 
==127710== For lists of detected and suppressed errors, rerun with: -s
==127710== ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0)

```

### Отчет Valgrind leak-check show-leak-kinds

```
root@api:/home/commeta/project/kernel/fast_io# valgrind --leak-check=full --show-leak-kinds=all php test.php
==127532== Memcheck, a memory error detector
==127532== Copyright (C) 2002-2022, and GNU GPL'd, by Julian Seward et al.
==127532== Using Valgrind-3.22.0 and LibVEX; rerun with -h for copyright info
==127532== Command: php test.php
==127532== 
==127532== 
==127532== HEAP SUMMARY:
==127532==     in use at exit: 77,809 bytes in 1,349 blocks
==127532==   total heap usage: 293,523 allocs, 292,174 frees, 603,494,561 bytes allocated
==127532== 
==127532== 32 bytes in 1 blocks are still reachable in loss record 1 of 13
==127532==    at 0x4846828: malloc (in /usr/libexec/valgrind/vgpreload_memcheck-amd64-linux.so)
==127532==    by 0x26A304: ??? (in /usr/bin/php8.3)
==127532==    by 0x239BD4: ??? (in /usr/bin/php8.3)
==127532==    by 0x49A8CA: zend_register_ini_entries_ex (in /usr/bin/php8.3)
==127532==    by 0x23F4A3: ??? (in /usr/bin/php8.3)
==127532==    by 0x425252: zend_startup_module_ex (in /usr/bin/php8.3)
==127532==    by 0x4252FF: ??? (in /usr/bin/php8.3)
==127532==    by 0x43440A: zend_hash_apply (in /usr/bin/php8.3)
==127532==    by 0x3B1E6C: php_module_startup (in /usr/bin/php8.3)
==127532==    by 0x2385C9: ??? (in /usr/bin/php8.3)
==127532==    by 0x52391C9: (below main) (libc_start_call_main.h:58)
==127532== 
==127532== 32 bytes in 1 blocks are still reachable in loss record 2 of 13
==127532==    at 0x4846828: malloc (in /usr/libexec/valgrind/vgpreload_memcheck-amd64-linux.so)
==127532==    by 0x4001E04: malloc (rtld-malloc.h:56)
==127532==    by 0x4001E04: _dl_close_worker.part.0 (dl-close.c:354)
==127532==    by 0x40027BD: _dl_close_worker (dl-close.c:120)
==127532==    by 0x40027BD: _dl_close (dl-close.c:793)
==127532==    by 0x400151B: _dl_catch_exception (dl-catch.c:237)
==127532==    by 0x4001668: _dl_catch_error (dl-catch.c:256)
==127532==    by 0x52A6C72: _dlerror_run (dlerror.c:138)
==127532==    by 0x52A69A5: dlclose@@GLIBC_2.34 (dlclose.c:31)
==127532==    by 0x42755A: ??? (in /usr/bin/php8.3)
==127532==    by 0x4204D0: ??? (in /usr/bin/php8.3)
==127532==    by 0x3B2451: php_module_shutdown (in /usr/bin/php8.3)
==127532==    by 0x2386A4: ??? (in /usr/bin/php8.3)
==127532==    by 0x52391C9: (below main) (libc_start_call_main.h:58)
==127532== 
==127532== 72 bytes in 2 blocks are still reachable in loss record 3 of 13
==127532==    at 0x4846828: malloc (in /usr/libexec/valgrind/vgpreload_memcheck-amd64-linux.so)
==127532==    by 0x4028ABF: malloc (rtld-malloc.h:56)
==127532==    by 0x4028ABF: strdup (strdup.c:42)
==127532==    by 0x4016A95: _dl_load_cache_lookup (dl-cache.c:515)
==127532==    by 0x40097CA: _dl_map_object (dl-load.c:2135)
==127532==    by 0x4002A2C: openaux (dl-deps.c:64)
==127532==    by 0x400151B: _dl_catch_exception (dl-catch.c:237)
==127532==    by 0x4002E66: _dl_map_object_deps (dl-deps.c:232)
==127532==    by 0x400D944: dl_open_worker_begin (dl-open.c:638)
==127532==    by 0x400151B: _dl_catch_exception (dl-catch.c:237)
==127532==    by 0x400CD1F: dl_open_worker (dl-open.c:803)
==127532==    by 0x400151B: _dl_catch_exception (dl-catch.c:237)
==127532==    by 0x400D163: _dl_open (dl-open.c:905)
==127532== 
==127532== 72 bytes in 2 blocks are still reachable in loss record 4 of 13
==127532==    at 0x4846828: malloc (in /usr/libexec/valgrind/vgpreload_memcheck-amd64-linux.so)
==127532==    by 0x400CA68: malloc (rtld-malloc.h:56)
==127532==    by 0x400CA68: _dl_new_object (dl-object.c:199)
==127532==    by 0x4007ABE: _dl_map_object_from_fd (dl-load.c:1053)
==127532==    by 0x4009528: _dl_map_object (dl-load.c:2268)
==127532==    by 0x4002A2C: openaux (dl-deps.c:64)
==127532==    by 0x400151B: _dl_catch_exception (dl-catch.c:237)
==127532==    by 0x4002E66: _dl_map_object_deps (dl-deps.c:232)
==127532==    by 0x400D944: dl_open_worker_begin (dl-open.c:638)
==127532==    by 0x400151B: _dl_catch_exception (dl-catch.c:237)
==127532==    by 0x400CD1F: dl_open_worker (dl-open.c:803)
==127532==    by 0x400151B: _dl_catch_exception (dl-catch.c:237)
==127532==    by 0x400D163: _dl_open (dl-open.c:905)
==127532== 
==127532== 720 bytes in 2 blocks are still reachable in loss record 5 of 13
==127532==    at 0x484D953: calloc (in /usr/libexec/valgrind/vgpreload_memcheck-amd64-linux.so)
==127532==    by 0x401622F: calloc (rtld-malloc.h:44)
==127532==    by 0x401622F: _dl_check_map_versions (dl-version.c:280)
==127532==    by 0x400DC7C: dl_open_worker_begin (dl-open.c:646)
==127532==    by 0x400151B: _dl_catch_exception (dl-catch.c:237)
==127532==    by 0x400CD1F: dl_open_worker (dl-open.c:803)
==127532==    by 0x400151B: _dl_catch_exception (dl-catch.c:237)
==127532==    by 0x400D163: _dl_open (dl-open.c:905)
==127532==    by 0x52A7193: dlopen_doit (dlopen.c:56)
==127532==    by 0x400151B: _dl_catch_exception (dl-catch.c:237)
==127532==    by 0x4001668: _dl_catch_error (dl-catch.c:256)
==127532==    by 0x52A6C72: _dlerror_run (dlerror.c:138)
==127532==    by 0x52A724E: dlopen_implementation (dlopen.c:71)
==127532==    by 0x52A724E: dlopen@@GLIBC_2.34 (dlopen.c:81)
==127532== 
==127532== 816 bytes in 1 blocks are still reachable in loss record 6 of 13
==127532==    at 0x4846828: malloc (in /usr/libexec/valgrind/vgpreload_memcheck-amd64-linux.so)
==127532==    by 0x400CF3F: malloc (rtld-malloc.h:56)
==127532==    by 0x400CF3F: add_to_global_resize (dl-open.c:152)
==127532==    by 0x400DF0F: dl_open_worker_begin (dl-open.c:737)
==127532==    by 0x400151B: _dl_catch_exception (dl-catch.c:237)
==127532==    by 0x400CD1F: dl_open_worker (dl-open.c:803)
==127532==    by 0x400151B: _dl_catch_exception (dl-catch.c:237)
==127532==    by 0x400D163: _dl_open (dl-open.c:905)
==127532==    by 0x52A7193: dlopen_doit (dlopen.c:56)
==127532==    by 0x400151B: _dl_catch_exception (dl-catch.c:237)
==127532==    by 0x4001668: _dl_catch_error (dl-catch.c:256)
==127532==    by 0x52A6C72: _dlerror_run (dlerror.c:138)
==127532==    by 0x52A724E: dlopen_implementation (dlopen.c:71)
==127532==    by 0x52A724E: dlopen@@GLIBC_2.34 (dlopen.c:81)
==127532== 
==127532== 1,504 bytes in 1 blocks are still reachable in loss record 7 of 13
==127532==    at 0x4846828: malloc (in /usr/libexec/valgrind/vgpreload_memcheck-amd64-linux.so)
==127532==    by 0x26A349: ??? (in /usr/bin/php8.3)
==127532==    by 0x239BD4: ??? (in /usr/bin/php8.3)
==127532==    by 0x49A8CA: zend_register_ini_entries_ex (in /usr/bin/php8.3)
==127532==    by 0x23F4A3: ??? (in /usr/bin/php8.3)
==127532==    by 0x425252: zend_startup_module_ex (in /usr/bin/php8.3)
==127532==    by 0x4252FF: ??? (in /usr/bin/php8.3)
==127532==    by 0x43440A: zend_hash_apply (in /usr/bin/php8.3)
==127532==    by 0x3B1E6C: php_module_startup (in /usr/bin/php8.3)
==127532==    by 0x2385C9: ??? (in /usr/bin/php8.3)
==127532==    by 0x52391C9: (below main) (libc_start_call_main.h:58)
==127532== 
==127532== 2,508 bytes in 2 blocks are still reachable in loss record 8 of 13
==127532==    at 0x484D953: calloc (in /usr/libexec/valgrind/vgpreload_memcheck-amd64-linux.so)
==127532==    by 0x400C72C: calloc (rtld-malloc.h:44)
==127532==    by 0x400C72C: _dl_new_object (dl-object.c:92)
==127532==    by 0x4007ABE: _dl_map_object_from_fd (dl-load.c:1053)
==127532==    by 0x4009528: _dl_map_object (dl-load.c:2268)
==127532==    by 0x4002A2C: openaux (dl-deps.c:64)
==127532==    by 0x400151B: _dl_catch_exception (dl-catch.c:237)
==127532==    by 0x4002E66: _dl_map_object_deps (dl-deps.c:232)
==127532==    by 0x400D944: dl_open_worker_begin (dl-open.c:638)
==127532==    by 0x400151B: _dl_catch_exception (dl-catch.c:237)
==127532==    by 0x400CD1F: dl_open_worker (dl-open.c:803)
==127532==    by 0x400151B: _dl_catch_exception (dl-catch.c:237)
==127532==    by 0x400D163: _dl_open (dl-open.c:905)
==127532== 
==127532== 4,384 bytes in 418 blocks are still reachable in loss record 9 of 13
==127532==    at 0x4846828: malloc (in /usr/libexec/valgrind/vgpreload_memcheck-amd64-linux.so)
==127532==    by 0x52C334E: strdup (strdup.c:42)
==127532==    by 0x26972D: ??? (in /usr/bin/php8.3)
==127532==    by 0x26A326: ??? (in /usr/bin/php8.3)
==127532==    by 0x239BD4: ??? (in /usr/bin/php8.3)
==127532==    by 0x49A8CA: zend_register_ini_entries_ex (in /usr/bin/php8.3)
==127532==    by 0x23F4A3: ??? (in /usr/bin/php8.3)
==127532==    by 0x425252: zend_startup_module_ex (in /usr/bin/php8.3)
==127532==    by 0x4252FF: ??? (in /usr/bin/php8.3)
==127532==    by 0x43440A: zend_hash_apply (in /usr/bin/php8.3)
==127532==    by 0x3B1E6C: php_module_startup (in /usr/bin/php8.3)
==127532==    by 0x2385C9: ??? (in /usr/bin/php8.3)
==127532== 
==127532== 7,837 bytes in 499 blocks are still reachable in loss record 10 of 13
==127532==    at 0x4846828: malloc (in /usr/libexec/valgrind/vgpreload_memcheck-amd64-linux.so)
==127532==    by 0x52C334E: strdup (strdup.c:42)
==127532==    by 0x269A1C: ??? (in /usr/bin/php8.3)
==127532==    by 0x26A321: ??? (in /usr/bin/php8.3)
==127532==    by 0x239BD4: ??? (in /usr/bin/php8.3)
==127532==    by 0x49A8CA: zend_register_ini_entries_ex (in /usr/bin/php8.3)
==127532==    by 0x23F4A3: ??? (in /usr/bin/php8.3)
==127532==    by 0x425252: zend_startup_module_ex (in /usr/bin/php8.3)
==127532==    by 0x4252FF: ??? (in /usr/bin/php8.3)
==127532==    by 0x43440A: zend_hash_apply (in /usr/bin/php8.3)
==127532==    by 0x3B1E6C: php_module_startup (in /usr/bin/php8.3)
==127532==    by 0x2385C9: ??? (in /usr/bin/php8.3)
==127532== 
==127532== 8,168 bytes in 1 blocks are still reachable in loss record 11 of 13
==127532==    at 0x484D953: calloc (in /usr/libexec/valgrind/vgpreload_memcheck-amd64-linux.so)
==127532==    by 0x26955A: ??? (in /usr/bin/php8.3)
==127532==    by 0x26A326: ??? (in /usr/bin/php8.3)
==127532==    by 0x239BD4: ??? (in /usr/bin/php8.3)
==127532==    by 0x49A8CA: zend_register_ini_entries_ex (in /usr/bin/php8.3)
==127532==    by 0x23F4A3: ??? (in /usr/bin/php8.3)
==127532==    by 0x425252: zend_startup_module_ex (in /usr/bin/php8.3)
==127532==    by 0x4252FF: ??? (in /usr/bin/php8.3)
==127532==    by 0x43440A: zend_hash_apply (in /usr/bin/php8.3)
==127532==    by 0x3B1E6C: php_module_startup (in /usr/bin/php8.3)
==127532==    by 0x2385C9: ??? (in /usr/bin/php8.3)
==127532==    by 0x52391C9: (below main) (libc_start_call_main.h:58)
==127532== 
==127532== 8,192 bytes in 1 blocks are still reachable in loss record 12 of 13
==127532==    at 0x484DB80: realloc (in /usr/libexec/valgrind/vgpreload_memcheck-amd64-linux.so)
==127532==    by 0x269AF5: ??? (in /usr/bin/php8.3)
==127532==    by 0x26A321: ??? (in /usr/bin/php8.3)
==127532==    by 0x239BD4: ??? (in /usr/bin/php8.3)
==127532==    by 0x49A8CA: zend_register_ini_entries_ex (in /usr/bin/php8.3)
==127532==    by 0x23F4A3: ??? (in /usr/bin/php8.3)
==127532==    by 0x425252: zend_startup_module_ex (in /usr/bin/php8.3)
==127532==    by 0x4252FF: ??? (in /usr/bin/php8.3)
==127532==    by 0x43440A: zend_hash_apply (in /usr/bin/php8.3)
==127532==    by 0x3B1E6C: php_module_startup (in /usr/bin/php8.3)
==127532==    by 0x2385C9: ??? (in /usr/bin/php8.3)
==127532==    by 0x52391C9: (below main) (libc_start_call_main.h:58)
==127532== 
==127532== 43,472 bytes in 418 blocks are still reachable in loss record 13 of 13
==127532==    at 0x4846828: malloc (in /usr/libexec/valgrind/vgpreload_memcheck-amd64-linux.so)
==127532==    by 0x2696FA: ??? (in /usr/bin/php8.3)
==127532==    by 0x26A326: ??? (in /usr/bin/php8.3)
==127532==    by 0x239BD4: ??? (in /usr/bin/php8.3)
==127532==    by 0x49A8CA: zend_register_ini_entries_ex (in /usr/bin/php8.3)
==127532==    by 0x23F4A3: ??? (in /usr/bin/php8.3)
==127532==    by 0x425252: zend_startup_module_ex (in /usr/bin/php8.3)
==127532==    by 0x4252FF: ??? (in /usr/bin/php8.3)
==127532==    by 0x43440A: zend_hash_apply (in /usr/bin/php8.3)
==127532==    by 0x3B1E6C: php_module_startup (in /usr/bin/php8.3)
==127532==    by 0x2385C9: ??? (in /usr/bin/php8.3)
==127532==    by 0x52391C9: (below main) (libc_start_call_main.h:58)
==127532== 
==127532== LEAK SUMMARY:
==127532==    definitely lost: 0 bytes in 0 blocks
==127532==    indirectly lost: 0 bytes in 0 blocks
==127532==      possibly lost: 0 bytes in 0 blocks
==127532==    still reachable: 77,809 bytes in 1,349 blocks
==127532==         suppressed: 0 bytes in 0 blocks
==127532== 
==127532== For lists of detected and suppressed errors, rerun with: -s
==127532== ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0)

```

Из результатов анализа Valgrind видно, что не было обнаружено явных утечек памяти (definitely lost), косвенных утечек (indirectly lost) или возможных утечек (possibly lost) во время выполнения скрипта test.php, который использовал расширение fast_io.so.

Однако, Valgrind указывает на то, что 77,809 байт памяти остаются доступными (still reachable) в 1,349 блоках. Это может быть связано с тем, что некоторые объекты или структуры данных остаются в памяти на момент завершения программы, но при этом на них все еще есть указатели и они могут быть освобождены позже. 

В целом, результаты анализа Valgrind говорят о том, что программа завершилась без явных ошибок работы с памятью и без утечек памяти, но есть некоторые блоки памяти, которые остаются доступными и не были освобождены к моменту завершения программы. Эти блоки используются модулями из стандартной поставки, без загруженного fast_io.so (на пустом test.php) отчет не меняется.


#### PCRE2
Регулярные выражения на Ubuntu 24.04 были установлены в пакетах:
```
root@api:/home/commeta/project/kernel/fast_io# dpkg -l | grep pcre2
ii  libpcre2-16-0:amd64                              10.42-4ubuntu2                                amd64        New Perl Compatible Regular Expression Library - 16 bit runtime files
ii  libpcre2-32-0:amd64                              10.42-4ubuntu2                                amd64        New Perl Compatible Regular Expression Library - 32 bit runtime files
ii  libpcre2-8-0:amd64                               10.42-4ubuntu2                                amd64        New Perl Compatible Regular Expression Library- 8 bit runtime files
ii  libpcre2-dev:amd64                               10.42-4ubuntu2                                amd64        New Perl Compatible Regular Expression Library - development files
ii  libpcre2-posix3:amd64                            10.42-4ubuntu2                                amd64        New Perl Compatible Regular Expression Library - posix-compatible runtime files
```

```
root@api:/home/commeta/project/kernel/fast_io# ldconfig -p | grep pcre2
	libpcre2-32.so.0 (libc6,x86-64) => /lib/x86_64-linux-gnu/libpcre2-32.so.0
	libpcre2-32.so (libc6,x86-64) => /lib/x86_64-linux-gnu/libpcre2-32.so
	libpcre2-16.so.0 (libc6,x86-64) => /lib/x86_64-linux-gnu/libpcre2-16.so.0
	libpcre2-16.so (libc6,x86-64) => /lib/x86_64-linux-gnu/libpcre2-16.so
	libpcre2-8.so.0 (libc6,x86-64) => /lib/x86_64-linux-gnu/libpcre2-8.so.0
	libpcre2-8.so (libc6,x86-64) => /lib/x86_64-linux-gnu/libpcre2-8.so
	libpcre2-posix.so.3 (libc6,x86-64) => /lib/x86_64-linux-gnu/libpcre2-posix.so.3
	libpcre2-posix.so (libc6,x86-64) => /lib/x86_64-linux-gnu/libpcre2-posix.so
```

```
echo PCRE_VERSION; // 10.42 2022-12-11
```
