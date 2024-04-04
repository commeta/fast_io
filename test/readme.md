# Test

Буферизованная версия fast_io.h
```
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
            [8] => index_19
            [9] => index_20
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
    [1] => 280:35
)
Array
(
    [0] => indexed_find_value_by_key
    [1] => 
)
write_key_value_pair: 0.12245011329651 (0.00001225)
find_value_by_key: 1.085972070694 (0.00010860)
find_value_by_key repeat: 0.051185846328735 (0.00000512)
delete_key_value_pair: 10.685581922531 (0.00106856)
indexed_write_key_value_pair: 0.18320202827454 (0.00001832)
indexed_find_value_by_key: 0.81330299377441 (0.00008133)
indexed_find_value_by_key repeat: 0.087224960327148 (0.00000872)
pop_key_value_pair: 0.21866512298584 (0.00002187)
```

Версия с выравниванием данных test/fast_io.h
```
write_key_value_pair: 0.073961973190308 (0.00000740)
find_value_by_key: 37.980912923813 (0.00379809)
find_value_by_key repeat: 0.12786817550659 (0.00001279)
delete_key_value_pair: 33.781598806381 (0.00337816)
indexed_write_key_value_pair: 0.18683004379272 (0.00001868)
indexed_find_value_by_key: 38.641878128052 (0.00386419)
indexed_find_value_by_key repeat: 0.18656396865845 (0.00001866)
indexed_delete_key: 38.810563087463 (0.00388106)
```
