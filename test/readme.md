# Test

## Результаты стресс теста:

```
=====================================================================
PHP         : /usr/bin/php8.3 
PHP_SAPI    : cli
PHP_VERSION : 8.3.4
ZEND_VERSION: 4.3.4
PHP_OS      : Linux - Linux api.webdevops.ru 6.8.0-11-generic #11-Ubuntu SMP PREEMPT_DYNAMIC Wed Feb 14 00:29:05 UTC 2024 x86_64
INI actual  : /home/commeta/project/kernel/fast_io/tmp-php.ini
More .INIs  :   
CWD         : /home/commeta/project/kernel/fast_io
Extra dirs  : 
VALGRIND    : Not used
=====================================================================
TIME START 2024-04-12 08:55:46
=====================================================================
No tests were run.
12345Array
(
    [0] => select_key_value
    [1] => index_2 insert_key_value_2
)
Array
(
    [0] => update_key_value
    [1] => 1
)
Array
(
    [0] => pop_key_value_pair
    [1] => index_4 insert_key_value_4
)
Array
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
0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, Array
(
    [0] => find_value_by_key
    [1] => data_write_key_value_pair_5
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
0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, Array
(
    [0] => indexed_find_value_by_key
    [1] => data_indexed_write_key_value_pair_10
)
Array
(
    [0] => indexed_find_value_by_key
    [1] => 
)
write_key_value_pair: 0.12382102012634 (0.00001238)
find_value_by_key: 1.2464380264282 (0.00012464)
find_value_by_key repeat: 0.051723003387451 (0.00000517)
delete_key_value_pair: 12.083899021149 (0.00120839)
indexed_write_key_value_pair: 0.24071311950684 (0.00002407)
indexed_find_value_by_key: 0.93093800544739 (0.00009309)
indexed_find_value_by_key repeat: 0.10538697242737 (0.00001054)
pop_key_value_pair: 0.27355885505676 (0.00002736)
Array
(
    [0] => memory_get_process_usage_kernel in Kilo Bytes
    [1] => 27604
    [2] => 27988
    [3] => 384
)
Array
(
    [0] => getProcessIOStats
    [1] => Array
        (
            [rchar] => 7050197499
            [wchar] => 2100566115
            [syscr] => 1860922
            [syscw] => 557708
            [read_bytes] => 40960
            [write_bytes] => 2121089024
            [cancelled_write_bytes] => 446464
        )

)

```

