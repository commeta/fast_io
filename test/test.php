<?php
// Stress test
function memory_get_process_usage_kernel(){
    $statusFile = '/proc/' . getmypid() . '/status';
    
    if (!file_exists($statusFile)) {
        return 0;
    }
    
    $status = file_get_contents($statusFile);
    if ($status === false) {
        // Не удалось прочитать файл
        return 0;
    }
    
    $totalMemory = 0;
    
    // Извлечение VmRSS
    if (preg_match('~^VmRSS:\s*([0-9]+) kB~mi', $status, $matches)) {
        $totalMemory += intval($matches[1]);
    }
    
    // Извлечение VmSwap (если нужно учитывать)
    if (preg_match('~^VmSwap:\s*([0-9]+) kB~mi', $status, $matches)) {
        $totalMemory += intval($matches[1]);
    }
    
    return $totalMemory;
}



$r_total= memory_get_process_usage_kernel();

foreach(glob('fast_io*.dat') as $file) {
	unlink($file);
}
foreach(glob('fast_io*.index') as $file) {
	unlink($file);
}
foreach(glob('fast_io*.tmp') as $file) {
	unlink($file);
}


for($i=0; $i <=4; $i++){
	print_r(
		insert_key_value(__DIR__ . '/fast_io1.dat', 'index_' . $i . ' insert_key_value_' . $i, 32)
	);
}


print_r([
	'select_key_value',
	select_key_value(__DIR__ . '/fast_io1.dat', 2, 32)
]);
print_r([
	'update_key_value',
	update_key_value(__DIR__ . '/fast_io1.dat', 'update_key_value', 3, 32)
]);




print_r([
	'pop_key_value_pair',
	pop_key_value_pair(__DIR__ . '/fast_io1.dat', 32),
]);




for($i=0; $i <=20; $i++){
	indexed_write_key_value_pair(__DIR__ . '/fast_io2.dat', 'index_' . $i, 'data_write_key_value_pair_' . $i);
}
for($i=0; $i <=10; $i++){
	hide_key_value_pair(__DIR__ . '/fast_io2.dat.index', 'index_' . $i);
}

print_r([
	'rebuild_data_file',
	rebuild_data_file(__DIR__ . '/fast_io2.dat', 'index_19')
]);

print_r([
	'get_index_keys',
	get_index_keys(__DIR__ . '/fast_io2.dat.index')
]);






print_r(__DIR__ . '/fast_io3.dat' . "\n");
for($i=0; $i <=10; $i++){
	write_key_value_pair(__DIR__ . '/fast_io3.dat', 'index_' . $i, 'data_write_key_value_pair_' . $i);
	print_r($i . ', ');
}



print_r([
	'find_value_by_key',
	find_value_by_key(__DIR__ . '/fast_io3.dat', 'index_5')
]);
delete_key_value_pair(__DIR__ . '/fast_io3.dat', 'index_5');
print_r([
	'find_value_by_key',
	find_value_by_key(__DIR__ . '/fast_io3.dat', 'index_5')
]);

print_r([
	'pop_key_value_pair',
	pop_key_value_pair(__DIR__ . '/fast_io3.dat')
]);

print_r([
	'update_key_value_pair',
	update_key_value_pair(__DIR__ . '/fast_io3.dat', 'index_3', 'update_key_value_pair')
]);



print_r(__DIR__ . '/fast_io4.dat' . "\n");
for($i=0; $i <=10; $i++){
	indexed_write_key_value_pair(__DIR__ . '/fast_io4.dat', 'index_' . $i, 'data_indexed_write_key_value_pair_' . $i);

	print_r($i . ', ');
}

print_r([
	'indexed_find_value_by_key',
	indexed_find_value_by_key(__DIR__ . '/fast_io4.dat', 'index_8')
]);

delete_key_value_pair(__DIR__ . '/fast_io4.dat.index', 'index_8');

print_r([
	'indexed_find_value_by_key',
	indexed_find_value_by_key(__DIR__ . '/fast_io4.dat', 'index_8')
]);
sleep(10);


$start= microtime(true);
for($i=0; $i <=10000; $i++){
	write_key_value_pair(__DIR__ . '/fast_io5.dat', 'index_' . $i, 'data_write_key_value_pair_' . $i);
}
$time= microtime(true) - $start;
echo "write_key_value_pair: ", $time, " (", sprintf('%.8f', ($time / 10000)), ")",  "\n";


sleep(10);
$start= microtime(true);
for($i=0; $i <=10000; $i++){
	find_value_by_key(__DIR__ . '/fast_io5.dat', 'index_' . $i);
}
$time= microtime(true) - $start;
echo "find_value_by_key: ", $time, " (", sprintf('%.8f', ($time / 10000)), ")",  "\n";


sleep(10);
$start= microtime(true);
for($i=0; $i <=10000; $i++){
	find_value_by_key(__DIR__ . '/fast_io5.dat', 'index_10');
}
$time= microtime(true) - $start;
echo "find_value_by_key repeat: ", $time, " (", sprintf('%.8f', ($time / 10000)), ")",  "\n";


sleep(10);
$start= microtime(true);
for($i=0; $i <=10000; $i++){
	delete_key_value_pair(__DIR__ . '/fast_io5.dat', 'index_' . $i);
}
$time= microtime(true) - $start;
echo "delete_key_value_pair: ", $time, " (", sprintf('%.8f', ($time / 10000)), ")",  "\n";


sleep(10);
$start= microtime(true);
for($i=0; $i <=10000; $i++){
	indexed_write_key_value_pair(__DIR__ . '/fast_io6.dat', 'index_' . $i, 'data_write_key_value_pair_' . $i);
}
$time= microtime(true) - $start;
echo "indexed_write_key_value_pair: ", $time, " (", sprintf('%.8f', ($time / 10000)), ")",  "\n";


sleep(10);
$start= microtime(true);
for($i=0; $i <=10000; $i++){
	indexed_find_value_by_key(__DIR__ . '/fast_io6.dat', 'index_' . $i);
}
$time= microtime(true) - $start;
echo "indexed_find_value_by_key: ", $time, " (", sprintf('%.8f', ($time / 10000)), ")",  "\n";


sleep(10);
$start= microtime(true);
for($i=0; $i <=10000; $i++){
	indexed_find_value_by_key(__DIR__ . '/fast_io6.dat', 'index_10');
}
$time= microtime(true) - $start;
echo "indexed_find_value_by_key repeat: ", $time, " (", sprintf('%.8f', ($time / 10000)), ")",  "\n";


sleep(10);
for($i=0; $i <=10000; $i++){
	write_key_value_pair(__DIR__ . '/fast_io7.dat', 'index_' . $i, 'data_write_key_value_pair_' . $i);
}
$start= microtime(true);
for($i=0; $i <=10000; $i++){
	pop_key_value_pair(__DIR__ . '/fast_io7.dat');
}
$time= microtime(true) - $start;
echo "pop_key_value_pair: ", $time, " (", sprintf('%.8f', ($time / 10000)), ")",  "\n";

print_r([
	'memory_get_process_usage_kernel in Kilo Bytes',
	$r_total,
	memory_get_process_usage_kernel(),
	memory_get_process_usage_kernel() - $r_total
]);
