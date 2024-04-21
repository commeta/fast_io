# Test

Стресс тест [test.php](test.php) запускался в однопользовательском режиме, без использования механизма блокировок!

Для реализации транзакции достаточно использовать блокировку на уровне файла, пример: [ipc-example.php](ipc-example.php)

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




## Результаты стресс теста:

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
TIME START 2024-04-21 03:09:49
=====================================================================
No tests were run.
123456789101112131415161718192021222324252627282930313233343536373839404142434445464748495051525354555657585960616263646566676869707172737475767778798081828384858687888990919293949596979899100101102103104105106107108109110111112113114115116117118119120121122123124125126127128129130131132133134135136137138139140141142143144145146147148149150151152153154155156157158159160161162163164165166167168169170171172173174175176177178179180181182183184185186187188189190191192193194195196197198199200201202203204205206207208209210211212213214215216217218219220221222223224225226227228229230231232233234235236237238239240241242243244245246247248249250251252253254255256257258259260261262263264265266267268269270271272273274275276277278279280281282283284285286287288289290291292293294295296297298299300301302303304305306307308309310311312313314315316317318319320321322323324325326327328329330331332333334335336337338339340341342343344345346347348349350351352353354355356357358359360361362363364365366367368369370371372373374375376377378379380381382383384385386387388389390391392393394395396397398399400401402403404405406407408409410411412413414415416417418419420421422423424425426427428429430431432433434435436437438439440441442443444445446447448449450451452453454455456457458459460461462463464465466467468469470471472473474475476477478479480481482483484485486487488489490491492493494495496497498499500501Array
(
    [0] => index_1 insert_key_value_1 12345678
    [1] => index_500 insert_key_value_500 12345678
)
Array
(
    [0] => Array
        (
            [0] => index_2 insert_key_value_2 12345678
            [1] => index_3 insert_key_value_3 12345678
            [2] => index_4 insert_key_value_4 12345678
            [3] => index_5 insert_key_value_5 12345678
            [4] => index_6 insert_key_value_6 12345678
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
    [1] => index_1 insert_key_value_1 12345678
)
Array
(
    [0] => detect_align_size
    [1] => 8192
)
Array
(
    [0] => select_key_value
    [1] => index_2 insert_key_value_2 12345678
)
Array
(
    [0] => update_key_value
    [1] => 1
    [2] => 1
)
Array
(
    [0] => pop_key_value_pair
    [1] => index_500 insert_key_value_500 12345678
)
Array
(
    [0] => update_key_value_pair
    [1] => 1
)
Array
(
    [0] => pop_key_value_pair
    [1] => index_499 insert_key_value_499 12345678
)
Array
(
    [0] => get_index_keys
    [1] => 499
    [2] => 499
)
Array
(
    [0] => delete_key_value_pair
    [1] => 1
)
Array
(
    [0] => hide_key_value_pair
    [1] => 1
)
Array
(
    [0] => update_key_value
    [1] => 1
)
Array
(
    [0] => find_value_by_key
    [1] => index_360 insert_key_value_360 12345678
    [2] => update апдейт
    [3] => index_0 insert_key_value_0 12345678
    [4] => index_11 insert_key_value_11 12345678
    [5] => index_11 insert_key_value_11 12345678
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
            [1] => index_12
            [2] => index_13
            [3] => index_14
            [4] => index_15
            [5] => index_16
            [6] => index_17
            [7] => index_18
            [8] => index_20
        )

)
/home/commeta/project/kernel/fast_io/fast_io3.dat
Array
(
    [0] => 0
)
Array
(
    [0] => 36
)
Array
(
    [0] => 72
)
Array
(
    [0] => 108
)
Array
(
    [0] => 144
)
Array
(
    [0] => 180
)
Array
(
    [0] => 216
)
Array
(
    [0] => 252
)
Array
(
    [0] => 288
)
Array
(
    [0] => 324
)
Array
(
    [0] => 360
)
Array
(
    [0] => find_value_by_key
    [1] => index_5 data_write_key_value_pair_
)
Array
(
    [0] => find_value_by_key
    [1] => index_5 data_write_key_value_pair_
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
    [0] => indexed_find_value_by_key
    [1] => data_indexed_write_key_value_pair_20

)
write_key_value_pair: 0.12476110458374 (0.00001248)
find_value_by_key: 3.1536099910736 (0.00031536)
find_value_by_key repeat: 0.069421052932739 (0.00000694)
delete_key_value_pair: 4.0489981174469 (0.00040490)
indexed_write_key_value_pair: 0.2558388710022 (0.00002558)
indexed_find_value_by_key: 2.936863899231 (0.00029369)
indexed_find_value_by_key repeat: 0.14635014533997 (0.00001464)
pop_key_value_pair: 0.33754801750183 (0.00003375)
Array
(
    [0] => memory_get_process_usage_kernel in Kilo Bytes
    [1] => 28532
    [2] => 61300
    [3] => 32768
    [4] => get_process_io_stats
    [5] => Array
        (
            [rchar] => 3828463965
            [wchar] => 392485727
            [syscr] => 994623
            [syscw] => 146215
            [read_bytes] => 0
            [write_bytes] => 431951872
            [cancelled_write_bytes] => 4554752
        )

)

```

