# Test

Буферизованная версия fast_io.h
```
commeta@api:~/project/kernel/fast_io$ php fast_io.php
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
write_key_value_pair: 0.073850870132446 (0.00000739)
find_value_by_key: 1.0971190929413 (0.00010971)
find_value_by_key repeat: 0.053118228912354 (0.00000531)
delete_key_value_pair: 9.4632499217987 (0.00094632)
indexed_write_key_value_pair: 0.18798398971558 (0.00001880)
indexed_find_value_by_key: 1.0934419631958 (0.00010934)
indexed_find_value_by_key repeat: 0.20405077934265 (0.00002041)
indexed_delete_key: 4.9545240402222 (0.00049545)
```

Версия с выравниванием данных test/fast_io.h
```
commeta@api:~/project/kernel/fast_io$ php fast_io.php
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
