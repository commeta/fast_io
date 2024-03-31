# Test

Буферизованная версия fast_io.h
```
commeta@api:~/project/kernel/fast_io$ php test.php
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
write_key_value_pair: 0.088172912597656 (0.00000882)
find_value_by_key: 1.1746139526367 (0.00011746)
find_value_by_key repeat: 0.061406850814819 (0.00000614)
delete_key_value_pair: 10.811213970184 (0.00108112)
indexed_write_key_value_pair: 0.22422909736633 (0.00002242)
indexed_find_value_by_key: 0.87611103057861 (0.00008761)
indexed_find_value_by_key repeat: 0.12141799926758 (0.00001214)
indexed_delete_key: 4.6194241046906 (0.00046194)

```

Версия с выравниванием данных test/fast_io.h
```
commeta@api:~/project/kernel/fast_io$ php test.php
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
