<?php

for($i=0; $i <=10; $i++){
	write_key_value_pair('fast_io2.dat', 'index' . $i, 'data' . $i);

	print_r([$i]);
}

print_r([
	find_value_by_key('fast_io2.dat', 'index5')
]);

delete_key_value_pair('fast_io2.dat', 'index5');

print_r([
	find_value_by_key('fast_io2.dat', 'index5')
]);


for($i=0; $i <=10; $i++){
	indexed_write_key_value_pair('fast_io3.dat', 'index' . $i, 'data' . $i);

	print_r([$i]);
}

print_r([
	indexed_find_value_by_key('fast_io3.dat', 'index5')
]);






