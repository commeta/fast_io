<?php
/*
 * Fast_IO (pre-release) Extension for PHP 8
 * https://github.com/commeta/fast_io
 * 
 * Copyright 2024 commeta <dcs-spb@ya.ru>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */

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

/*
Каждый ключ массива представляет определённый тип статистики ввода-вывода:
- rchar: количество байт, которые процесс прочитал из ядра (не обязательно с диска).
- wchar: количество байт, которые процесс записал в ядро.
- syscr: количество вызовов чтения, выполненных процессом.
- syscw: количество вызовов записи, выполненных процессом.
- и так далее, в зависимости от содержимого файла /proc/[pid]/io.
*/

function get_process_io_stats() {
    $ioFile = '/proc/' . getmypid() . '/io';
    
    if (!file_exists($ioFile)) {
        return [];
    }
    
    $ioData = file_get_contents($ioFile);
    if ($ioData === false) {
        // Не удалось прочитать файл
        return [];
    }
    
    $ioStats = [];
    
    // Разбор данных файла
    $lines = explode("\n", trim($ioData));
    foreach ($lines as $line) {
        list($key, $value) = explode(':', $line, 2);
        $key = trim($key);
        $value = trim($value);
        $ioStats[$key] = intval($value);
    }
    
    return $ioStats;
}


foreach(glob('fast_io*.dat') as $file) {
	unlink($file);
}
foreach(glob('fast_io*.index') as $file) {
	unlink($file);
}
foreach(glob('fast_io*.tmp') as $file) {
	unlink($file);
}


$r_total= memory_get_process_usage_kernel();


for($i=0; $i <=400; $i++){
	print_r(
		insert_key_value(__DIR__ . '/fast_io1.dat', 'index_' . $i . ' insert_key_value_' . $i, 8192)
	);
}

$align_size = detect_align_size(__DIR__ . '/fast_io1.dat');
if((int) ini_get('fast_io.buffer_size') < $align_size + 1) ini_set('fast_io.buffer_size', $align_size + 1);

print_r([
	'detect_align_size',
	$align_size,
]);


print_r([
	'select_key_value',
	select_key_value(__DIR__ . '/fast_io1.dat', 2, 8192)
]);


print_r([
	'update_key_value',
	update_key_value(__DIR__ . '/fast_io1.dat', 'update_key_value', 3, 8192),
	update_key_value(__DIR__ . '/fast_io1.dat', chr(127), 2, 8192),
]);


print_r([
	'pop_key_value_pair',
	pop_key_value_pair(__DIR__ . '/fast_io1.dat', 8192),
]);

print_r([
	'update_key_value_pair',
	update_key_value_pair(__DIR__ . '/fast_io1.dat', 'index_30', str_pad('update_key_value_pair', 8192, ' ') ),
]);


print_r([
	'pop_key_value_pair',
	pop_key_value_pair(__DIR__ . '/fast_io1.dat'),
]);


print_r([
	'memory_get_process_usage_kernel in Kilo Bytes',
	$r_total,
	memory_get_process_usage_kernel(),
	memory_get_process_usage_kernel() - $r_total,
	'get_process_io_stats',
	get_process_io_stats()
]);





for($i=0; $i <=20; $i++){
	indexed_write_key_value_pair(__DIR__ . '/fast_io2.dat', 'index_' . $i, 'data_write_key_value_pair_' . $i . "\n");
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


hide_key_value_pair(__DIR__ . '/fast_io3.dat', 'index_7');
delete_key_value_pair(__DIR__ . '/fast_io3.dat');



print_r(__DIR__ . '/fast_io4.dat' . "\n");
for($i=0; $i <=10; $i++){
	indexed_write_key_value_pair(__DIR__ . '/fast_io4.dat', 'index_' . $i, 'data_indexed_write_key_value_pair_' . $i . "\n");

	print_r($i . ', ');
}

print_r([
	'indexed_find_value_by_key',
	trim(indexed_find_value_by_key(__DIR__ . '/fast_io4.dat', 'index_10'))
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
	indexed_write_key_value_pair(__DIR__ . '/fast_io6.dat', 'index_' . $i, 'data_write_key_value_pair_' . $i . "\n");
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
	memory_get_process_usage_kernel() - $r_total,
	'get_process_io_stats',
	get_process_io_stats()
]);

