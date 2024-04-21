# Test

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
TIME START 2024-04-21 02:58:41
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
1, 29, 57, 85, 113, 141, 169, 197, 225, 253, 281, 310, 339, 368, 397, 426, 455, 484, 513, 542, 571, Array
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
    [0] => 1
)
Array
(
    [0] => 37
)
Array
(
    [0] => 73
)
Array
(
    [0] => 109
)
Array
(
    [0] => 145
)
Array
(
    [0] => 181
)
Array
(
    [0] => 217
)
Array
(
    [0] => 253
)
Array
(
    [0] => 289
)
Array
(
    [0] => 325
)
Array
(
    [0] => 361
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
0 offset:1, 1 offset:37, 2 offset:73, 3 offset:109, 4 offset:145, 5 offset:181, 6 offset:217, 7 offset:253, 8 offset:289, 9 offset:325, 10 offset:361, 11 offset:398, 12 offset:435, 13 offset:472, 14 offset:509, 15 offset:546, 16 offset:583, 17 offset:620, 18 offset:657, 19 offset:694, 20 offset:731, 21 offset:768, 22 offset:805, 23 offset:842, 24 offset:879, 25 offset:916, 26 offset:953, 27 offset:990, 28 offset:1027, 29 offset:1064, 30 offset:1101, 31 offset:1138, 32 offset:1175, 33 offset:1212, 34 offset:1249, 35 offset:1286, 36 offset:1323, 37 offset:1360, 38 offset:1397, 39 offset:1434, 40 offset:1471, 41 offset:1508, 42 offset:1545, 43 offset:1582, 44 offset:1619, 45 offset:1656, 46 offset:1693, 47 offset:1730, 48 offset:1767, 49 offset:1804, 50 offset:1841, 51 offset:1878, 52 offset:1915, 53 offset:1952, 54 offset:1989, 55 offset:2026, 56 offset:2063, 57 offset:2100, 58 offset:2137, 59 offset:2174, 60 offset:2211, 61 offset:2248, 62 offset:2285, 63 offset:2322, 64 offset:2359, 65 offset:2396, 66 offset:2433, 67 offset:2470, 68 offset:2507, 69 offset:2544, 70 offset:2581, 71 offset:2618, 72 offset:2655, 73 offset:2692, 74 offset:2729, 75 offset:2766, 76 offset:2803, 77 offset:2840, 78 offset:2877, 79 offset:2914, 80 offset:2951, 81 offset:2988, 82 offset:3025, 83 offset:3062, 84 offset:3099, 85 offset:3136, 86 offset:3173, 87 offset:3210, 88 offset:3247, 89 offset:3284, 90 offset:3321, 91 offset:3358, 92 offset:3395, 93 offset:3432, 94 offset:3469, 95 offset:3506, 96 offset:3543, 97 offset:3580, 98 offset:3617, 99 offset:3654, 100 offset:3691, 101 offset:3729, 102 offset:3767, 103 offset:3805, 104 offset:3843, 105 offset:3881, 106 offset:3919, 107 offset:3957, 108 offset:3995, 109 offset:4033, 110 offset:4071, Array
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
write_key_value_pair: 0.15005207061768 (0.00001501)
find_value_by_key: 3.3552169799805 (0.00033552)
find_value_by_key repeat: 0.070140838623047 (0.00000701)
delete_key_value_pair: 3.7821300029755 (0.00037821)
indexed_write_key_value_pair: 0.27245593070984 (0.00002725)
indexed_find_value_by_key: 2.6449208259583 (0.00026449)
indexed_find_value_by_key repeat: 0.14556407928467 (0.00001456)
pop_key_value_pair: 0.3146641254425 (0.00003147)
Array
(
    [0] => memory_get_process_usage_kernel in Kilo Bytes
    [1] => 28276
    [2] => 60916
    [3] => 32640
    [4] => get_process_io_stats
    [5] => Array
        (
            [rchar] => 3828463965
            [wchar] => 392485728
            [syscr] => 994623
            [syscw] => 146215
            [read_bytes] => 0
            [write_bytes] => 431951872
            [cancelled_write_bytes] => 4976640
        )

)

```

