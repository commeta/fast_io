<?php
print_r(__DIR__ . '/fast_io2.dat' . "\n");

for($i=0; $i <=10; $i++){
	write_key_value_pair(__DIR__ . '/fast_io2.dat', 'index_' . $i, 'data_write_key_value_pair_' . $i);

	print_r($i . ', ');
}

print_r([
	find_value_by_key(__DIR__ . '/fast_io2.dat', 'index_5')
]);

delete_key_value_pair(__DIR__ . '/fast_io2.dat', 'index_5');

print_r([
	find_value_by_key(__DIR__ . '/fast_io2.dat', 'index_5')
]);



print_r(__DIR__ . '/fast_io3.dat' . "\n");

for($i=0; $i <=10; $i++){
	indexed_write_key_value_pair(__DIR__ . '/fast_io3.dat', 'index_' . $i, 'data_indexed_write_key_value_pair_' . $i);

	print_r($i . ', ');
}

print_r([
	indexed_find_value_by_key(__DIR__ . '/fast_io3.dat', 'index_8')
]);

