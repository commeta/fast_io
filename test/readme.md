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
write_key_value_pair: 0.11135983467102 (0.00001114)
find_value_by_key: 1.2018070220947 (0.00012018)
find_value_by_key repeat: 0.054724931716919 (0.00000547)
delete_key_value_pair: 10.749533891678 (0.00107495)
indexed_write_key_value_pair: 0.190997838974 (0.00001910)
indexed_find_value_by_key: 0.87064599990845 (0.00008706)
indexed_find_value_by_key repeat: 0.11166596412659 (0.00001117)


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
