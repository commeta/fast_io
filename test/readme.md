# Test

Стресс тест [test.php](test.php) запускался в однопользовательском режиме, без использования механизма блокировок!

Для реализации транзакции достаточно использовать блокировку на уровне файла [пример](transaction/README.md)

[mysql-adapter](mysql-adapter/readme.md) использует [greenlion/PHP-SQL-Parser](https://github.com/greenlion/PHP-SQL-Parser), но подойдет любой другой [SQL / SQLI tokenizer parser analyzer](https://github.com/client9/libinjection)



## Адресация

```
<?php
$data_file = __DIR__ . '/fast_io.dat';
$align = 64; // line_number - длина 12 байт, 52 байта под данные.


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


print_r([$last_line_number, $new_line_number, $str]);
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




## Результаты стресс теста: test.php

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
PHP_OS      : Linux - Linux api.webdevops.ru 6.8.0-11-generic #11-Ubuntu SMP PREEMPT_DYNAMIC Wed Feb 14 00:29:05 UTC 2024 x86_64
INI actual  : /home/commeta/project/kernel/fast_io/tmp-php.ini
More .INIs  :   
CWD         : /home/commeta/project/kernel/fast_io
Extra dirs  : 
VALGRIND    : Not used
=====================================================================
TIME START 2024-04-24 03:21:51
=====================================================================
No tests were run.
0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255, 256, 257, 258, 259, 260, 261, 262, 263, 264, 265, 266, 267, 268, 269, 270, 271, 272, 273, 274, 275, 276, 277, 278, 279, 280, 281, 282, 283, 284, 285, 286, 287, 288, 289, 290, 291, 292, 293, 294, 295, 296, 297, 298, 299, 300, 301, 302, 303, 304, 305, 306, 307, 308, 309, 310, 311, 312, 313, 314, 315, 316, 317, 318, 319, 320, 321, 322, 323, 324, 325, 326, 327, 328, 329, 330, 331, 332, 333, 334, 335, 336, 337, 338, 339, 340, 341, 342, 343, 344, 345, 346, 347, 348, 349, 350, 351, 352, 353, 354, 355, 356, 357, 358, 359, 360, 361, 362, 363, 364, 365, 366, 367, 368, 369, 370, 371, 372, 373, 374, 375, 376, 377, 378, 379, 380, 381, 382, 383, 384, 385, 386, 387, 388, 389, 390, 391, 392, 393, 394, 395, 396, 397, 398, 399, 400, 401, 402, 403, 404, 405, 406, 407, 408, 409, 410, 411, 412, 413, 414, 415, 416, 417, 418, 419, 420, 421, 422, 423, 424, 425, 426, 427, 428, 429, 430, 431, 432, 433, 434, 435, 436, 437, 438, 439, 440, 441, 442, 443, 444, 445, 446, 447, 448, 449, 450, 451, 452, 453, 454, 455, 456, 457, 458, 459, 460, 461, 462, 463, 464, 465, 466, 467, 468, 469, 470, 471, 472, 473, 474, 475, 476, 477, 478, 479, 480, 481, 482, 483, 484, 485, 486, 487, 488, 489, 490, 491, 492, 493, 494, 495, 496, 497, 498, 499, 500, Array
(
    [0] => index_1 insert_key_value_1 12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012
    [1] => index_400 insert_key_value_400 12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012
)
Array
(
    [0] => Array
        (
            [0] => index_2 insert_key_value_2 12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012
            [1] => index_3 insert_key_value_3 12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012
            [2] => index_4 insert_key_value_4 12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012
            [3] => index_5 insert_key_value_5 12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012
            [4] => index_6 insert_key_value_6 12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012
        )

    [1] => Array
        (
            [0] => Array
                (
                    [0] => 4
                    [1] => 8192
                )

        )

    [2] => Array
        (
            [0] => index_
            [1] => insert_key_value_
        )

)
Array
(
    [0] => index_2 inser
    [1] => index_1 insert_key_value_1 12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012
)
Array
(
    [0] => detect_align_size
    [1] => 8192
)
Array
(
    [0] => select_key_value
    [1] => index_2 insert_key_value_2 12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012
)
Array
(
    [0] => update_key_value
    [1] => 8193
    [2] => 8193
)
Array
(
    [0] => pop_key_value_pair
    [1] => index_500 insert_key_value_500 12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012
)
Array
(
    [0] => update_key_value_pair
    [1] => 1
)
Array
(
    [0] => pop_key_value_pair
    [1] => index_499 insert_key_value_499 12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012
)
Array
(
    [0] => get_index_keys
    [1] => Array
        (
            [0] => index_0
        )

    [2] => Array
        (
            [0] => Array
                (
                    [0] => 0
                    [1] => 8192
                )

        )

)
Array
(
    [0] => hide_key_value_pair
    [1] => 49158
)
Array
(
    [0] => delete_key_value_pair
    [1] => 3
)
Array
(
    [0] => update_key_value
    [1] => 8193
)
Array
(
    [0] => find_value_by_key
    [1] => 
    [2] => update апдейт
    [3] => index_0 insert_key_value_0 12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012
    [4] => index_10 insert_key_value_10 12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012
    [5] => index_10 insert_key_value_10 12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012
)
0, 28, 56, 84, 112, 140, 168, 196, 224, 252, 280, 309, 338, 367, 396, 425, 454, 483, 512, 541, 570, Array
(
    [0] => rebuild_data_file
    [1] => 1
)
Array
(
    [0] => get_index_keys
    [1] => Array
        (
            [0] => index_11
        )

)
/home/commeta/project/kernel/fast_io/fast_io3.dat
0,36,72,108,144,180,216,252,288,324,360,Array
(
    [0] => find_value_by_key
    [1] => index_5 data_write_key_value_pair_5
)
Array
(
    [0] => find_value_by_key
    [1] => 
)
Array
(
    [0] => pop_key_value_pair
    [1] => index_10 data_write_key_value_pair_10
)
Array
(
    [0] => update_key_value_pair
    [1] => 1
)
/home/commeta/project/kernel/fast_io/fast_io4.dat
0 offset:0, 1 offset:36, 2 offset:72, 3 offset:108, 4 offset:144, 5 offset:180, 6 offset:216, 7 offset:252, 8 offset:288, 9 offset:324, 10 offset:360, 11 offset:397, 12 offset:434, 13 offset:471, 14 offset:508, 15 offset:545, 16 offset:582, 17 offset:619, 18 offset:656, 19 offset:693, 20 offset:730, 21 offset:767, 22 offset:804, 23 offset:841, 24 offset:878, 25 offset:915, 26 offset:952, 27 offset:989, 28 offset:1026, 29 offset:1063, 30 offset:1100, 31 offset:1137, 32 offset:1174, 33 offset:1211, 34 offset:1248, 35 offset:1285, 36 offset:1322, 37 offset:1359, 38 offset:1396, 39 offset:1433, 40 offset:1470, 41 offset:1507, 42 offset:1544, 43 offset:1581, 44 offset:1618, 45 offset:1655, 46 offset:1692, 47 offset:1729, 48 offset:1766, 49 offset:1803, 50 offset:1840, 51 offset:1877, 52 offset:1914, 53 offset:1951, 54 offset:1988, 55 offset:2025, 56 offset:2062, 57 offset:2099, 58 offset:2136, 59 offset:2173, 60 offset:2210, 61 offset:2247, 62 offset:2284, 63 offset:2321, 64 offset:2358, 65 offset:2395, 66 offset:2432, 67 offset:2469, 68 offset:2506, 69 offset:2543, 70 offset:2580, 71 offset:2617, 72 offset:2654, 73 offset:2691, 74 offset:2728, 75 offset:2765, 76 offset:2802, 77 offset:2839, 78 offset:2876, 79 offset:2913, 80 offset:2950, 81 offset:2987, 82 offset:3024, 83 offset:3061, 84 offset:3098, 85 offset:3135, 86 offset:3172, 87 offset:3209, 88 offset:3246, 89 offset:3283, 90 offset:3320, 91 offset:3357, 92 offset:3394, 93 offset:3431, 94 offset:3468, 95 offset:3505, 96 offset:3542, 97 offset:3579, 98 offset:3616, 99 offset:3653, 100 offset:3690, 101 offset:3728, 102 offset:3766, 103 offset:3804, 104 offset:3842, 105 offset:3880, 106 offset:3918, 107 offset:3956, 108 offset:3994, 109 offset:4032, 110 offset:4070, Array
(
    [0] => indexed_find_value_by_key
    [1] => data_indexed_write_key_value_pair_10
)
Array
(
    [0] => indexed_find_value_by_key
    [1] => data_indexed_write_key_value_pair_100

)
Array
(
    [0] => delete_key_value_pair
    [1] => 11
)
Array
(
    [0] => indexed_find_value_by_key
    [1] => data_indexed_write_key_value_pair_20

)
write_key_value_pair: 0.14092111587524 (0.00001409)
find_value_by_key: 2.9333550930023 (0.00029334)
find_value_by_key repeat: 0.54570698738098 (0.00005457)
delete_key_value_pair: 0.83563804626465 (0.00008356)
indexed_write_key_value_pair: 0.23253393173218 (0.00002325)
indexed_find_value_by_key: 2.3777570724487 (0.00023778)
indexed_find_value_by_key repeat: 0.36295485496521 (0.00003630)
pop_key_value_pair: 0.55416321754456 (0.00005542)
Array
(
    [0] => memory_get_process_usage_kernel in Kilo Bytes
    [1] => 28276
    [2] => 29136
    [3] => 860
    [4] => get_process_io_stats
    [5] => Array
        (
            [rchar] => 14773012641
            [wchar] => 21910281
            [syscr] => 223422
            [syscw] => 46130
            [read_bytes] => 0
            [write_bytes] => 20283392
            [cancelled_write_bytes] => 15220736
        )

)

```

