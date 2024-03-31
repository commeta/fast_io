# Test

Буферизованная версия fast_io.h
```
/home/commeta/project/kernel/fast_io/fast_io2.dat
0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, Array
(
    [0] => data_write_key_value_pair_5
)
Array
(
    [0] => 
)
Array
(
    [0] => index_10 data_write_key_value_pair_10

)
/home/commeta/project/kernel/fast_io/fast_io3.dat
0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, Array
(
    [0] => data_indexed_write_key_value_pair_8
)
Array
(
    [0] => 
)
write_key_value_pair: 0.11487889289856 (0.00001149)
find_value_by_key: 1.1644461154938 (0.00011644)
find_value_by_key repeat: 0.059406042098999 (0.00000594)
delete_key_value_pair: 10.655761003494 (0.00106558)
indexed_write_key_value_pair: 0.21924114227295 (0.00002192)
indexed_find_value_by_key: 0.91544914245605 (0.00009154)
indexed_find_value_by_key repeat: 0.11506295204163 (0.00001151)
pop_key_value_pair: 0.20079398155212 (0.00002008)
```

Версия с выравниванием данных test/fast_io.h
```
/home/commeta/project/kernel/fast_io/fast_io2.dat
0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, Array
(
    [0] => data_write_key_value_pair_5                                                                                                                                                                                                                            
)
Array
(
    [0] => 
)
/home/commeta/project/kernel/fast_io/fast_io3.dat
0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, Array
(
    [0] => data_indexed_write_key_value_pair_8
)
Array
(
    [0] => 
)
write_key_value_pair: 0.073961973190308 (0.00000740)
find_value_by_key: 37.980912923813 (0.00379809)
find_value_by_key repeat: 0.12786817550659 (0.00001279)
delete_key_value_pair: 33.781598806381 (0.00337816)
indexed_write_key_value_pair: 0.18683004379272 (0.00001868)
indexed_find_value_by_key: 38.641878128052 (0.00386419)
indexed_find_value_by_key repeat: 0.18656396865845 (0.00001866)
indexed_delete_key: 38.810563087463 (0.00388106)
```
