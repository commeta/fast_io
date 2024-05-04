<?php
/*
 * Fast_IO (pre-release beta) Extension for PHP 8
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

ini_set('fast_io.buffer_size', 4096);

//date_default_timezone_set ( 'Europe/Moscow' );
//setlocale (LC_ALL, "ru_RU.UTF-8");
//setlocale (LC_NUMERIC, "C");



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


for($i=0; $i <=500; $i++){
	print_r(
		file_insert_line(__DIR__ . '/fast_io1.dat', 'index_' . $i . ' file_insert_line_' . $i . ' ' . str_pad('', 92, '1234567890'), 8192) . ', '
	);
}


print_r([
	'file_get_keys',
	file_get_keys(__DIR__ . '/fast_io1.dat')
]);


print_r([
	'file_search_array',
	file_search_array(__DIR__ . '/fast_io1.dat', '\\w+_\\d+', 0, 2, 0, 13),
	file_search_array(__DIR__ . '/fast_io1.dat', 'index', 0, 2),
	file_search_array(__DIR__ . '/fast_io1.dat', '\\w+_\\d+', 0, 2, 0, 10),
]);


$array= [
	[8192, 8192], // Адрес и размер строки 1 в файле
	[16384, 8192], // Адрес и размер строки 2 в файле
	[24576, 8192] // Адрес и размер строки 3 в файле
];

print_r([
	'file_select_array',
	file_select_array(__DIR__ . '/fast_io1.dat', $array, '\\w+_\\d+', 10),
	file_select_array(__DIR__ . '/fast_io1.dat', $array, 'index'),
	file_search_array(__DIR__ . '/fast_io1.dat', '\\w+_\\d+', 0, 2, 0, 23),
]);


print_r([
	'file_select_line',
	file_select_line(__DIR__ . '/fast_io1.dat', 0, 1391, 0),
	find_matches_pcre2('\\w+_', file_select_line(__DIR__ . '/fast_io1.dat', 0, 8192, 1), 0),
	find_matches_pcre2('\\w+_', file_select_line(__DIR__ . '/fast_io1.dat', 0, 8192, 1), 1),
]);


print_r([
	'file_update_line',
	file_update_line(__DIR__ . '/fast_io1.dat', 'file_update_line mode 1', 8192, 8192, 1),
	file_update_line(__DIR__ . '/fast_io1.dat', 'file_update_line mode 0', 3, 8192),
	file_update_line(__DIR__ . '/fast_io1.dat', chr(127), 2, 8192),
]);


print_r([
	'file_pop_line',
	file_pop_line(__DIR__ . '/fast_io1.dat', 8192),
]);


print_r([
	'file_pop_line',
	file_pop_line(__DIR__ . '/fast_io1.dat'),
]);



print_r([
	'file_get_keys',
	file_get_keys(__DIR__ . '/fast_io1.dat', 1, 5),
]);




print_r([
	'file_erase_line',
	file_erase_line(__DIR__ . '/fast_io1.dat', 'index_6')
]);



print_r([
	'file_defrag_lines',
	file_defrag_lines(__DIR__ . '/fast_io1.dat', 'index_360')
]);




print_r([
	'file_update_line',
	file_update_line(__DIR__ . '/fast_io1.dat', 'update апдейт', 50, 8192),
]);



print_r([
	'file_search_line',
	file_search_line(__DIR__ . '/fast_io1.dat', 'index_360'),
	file_search_line(__DIR__ . '/fast_io1.dat', 'апдейт'),
	file_search_line(__DIR__ . '/fast_io1.dat', 'index'),
	file_search_line(__DIR__ . '/fast_io1.dat', '^\\w+_1', 0, 10),
	file_search_line(__DIR__ . '/fast_io1.dat', '^\\w+_1', 0, 10),
]);



for($i=0; $i <=20; $i++){
	print_r(
		file_push_data(__DIR__ . '/fast_io2.dat', 'index_' . $i, 'data_file_push_line_' . $i . "\n") . ", "
	);
}




for($i=0; $i <=10; $i++){
	file_erase_line(__DIR__ . '/fast_io2.dat.index', 'index_' . $i);
}


print_r([
	'file_defrag_data',
	file_defrag_data(__DIR__ . '/fast_io2.dat', 'index_32', 0)
]);




print_r([
	'file_get_keys',
	file_get_keys(__DIR__ . '/fast_io2.dat.index')
]);

print_r([
	'replicate_file mode 1',
	replicate_file(__DIR__ . '/fast_io2.dat', __DIR__ . '/fast_io22.dat', 1)
]);



print_r(__DIR__ . '/fast_io3.dat' . "\n");
for($i=0; $i <=10; $i++){
	print_r(
		file_push_line(__DIR__ . '/fast_io3.dat', 'index_' . $i . ' data_file_push_line_' . $i) . ','
	);
}




print_r([
	'file_search_line',
	file_search_line(__DIR__ . '/fast_io3.dat', 'index_5')
]);
file_defrag_lines(__DIR__ . '/fast_io3.dat', 'index_5');
print_r([
	'file_search_line',
	file_search_line(__DIR__ . '/fast_io3.dat', 'index_5')
]);

print_r([
	'file_pop_line',
	file_pop_line(__DIR__ . '/fast_io3.dat')
]);



print_r([
	'file_replace_line',
	file_replace_line(__DIR__ . '/fast_io3.dat', 'index_3', 'file_replace_line')
]);


file_erase_line(__DIR__ . '/fast_io3.dat', 'index_7');
file_defrag_lines(__DIR__ . '/fast_io3.dat');

print_r([
	'replicate_file',
	replicate_file(__DIR__ . '/fast_io3.dat', __DIR__ . '/fast_io33.dat')
]);




print_r(__DIR__ . '/fast_io4.dat' . "\n");
for($i=0; $i <=110; $i++){
	$offset = file_push_data(__DIR__ . '/fast_io4.dat', 'index_' . $i, 'data_file_push_data_' . $i . "\n");

	print_r($i . ' offset:' . $offset . ', ');
}


print_r([
	'file_search_data',
	trim(file_search_data(__DIR__ . '/fast_io4.dat', 'index_10'))
]);

print_r([
	'file_search_data',
	trim(file_search_data(__DIR__ . '/fast_io4.dat', 'index_100'))
]);



print_r([
	'file_defrag_lines',
	file_defrag_lines(__DIR__ . '/fast_io4.dat.index', 'index_8')
]);



print_r([
	'file_search_data',
	trim(file_search_data(__DIR__ . '/fast_io4.dat', 'index_20'))
]);




sleep(10);

$start= microtime(true);
for($i=0; $i <=10000; $i++){
	file_push_line(__DIR__ . '/fast_io5.dat', 'index_' . $i . ' data_file_push_line_' . $i);
}
$time= microtime(true) - $start;
echo "file_push_line: ", $time, " (", sprintf('%.8f', ($time / 10000)), ")",  "\n";


sleep(10);
$start= microtime(true);
for($i=0; $i <=10000; $i++){
	file_search_line(__DIR__ . '/fast_io5.dat', 'index_' . $i);
}
$time= microtime(true) - $start;
echo "file_search_line: ", $time, " (", sprintf('%.8f', ($time / 10000)), ")",  "\n";


sleep(10);
$start= microtime(true);
for($i=0; $i <=10000; $i++){
	file_search_line(__DIR__ . '/fast_io5.dat', 'index_10');
}
$time= microtime(true) - $start;
echo "file_search_line repeat: ", $time, " (", sprintf('%.8f', ($time / 10000)), ")",  "\n";


sleep(10);
$start= microtime(true);
for($i=0; $i <=10000; $i++){
	file_defrag_lines(__DIR__ . '/fast_io5.dat', 'index_' . $i);
}
$time= microtime(true) - $start;
echo "file_defrag_lines: ", $time, " (", sprintf('%.8f', ($time / 10000)), ")",  "\n";


sleep(10);
$start= microtime(true);
for($i=0; $i <=10000; $i++){
	file_push_data(__DIR__ . '/fast_io6.dat', 'index_' . $i, 'data_file_push_line_' . $i . "\n");
}
$time= microtime(true) - $start;
echo "file_push_data: ", $time, " (", sprintf('%.8f', ($time / 10000)), ")",  "\n";


sleep(10);
$start= microtime(true);
for($i=0; $i <=10000; $i++){
	file_search_data(__DIR__ . '/fast_io6.dat', 'index_' . $i);
}
$time= microtime(true) - $start;
echo "file_search_data: ", $time, " (", sprintf('%.8f', ($time / 10000)), ")",  "\n";


sleep(10);
$start= microtime(true);
for($i=0; $i <=10000; $i++){
	file_search_data(__DIR__ . '/fast_io6.dat', 'index_10');
}
$time= microtime(true) - $start;
echo "file_search_data repeat: ", $time, " (", sprintf('%.8f', ($time / 10000)), ")",  "\n";





sleep(10);
for($i=0; $i <=10000; $i++){
	file_push_line(__DIR__ . '/fast_io7.dat', 'index_' . $i . ' data_file_push_line_' . $i);
}
$start= microtime(true);
for($i=0; $i <=10000; $i++){
	file_pop_line(__DIR__ . '/fast_io7.dat');
}
$time= microtime(true) - $start;
echo "file_pop_line: ", $time, " (", sprintf('%.8f', ($time / 10000)), ")",  "\n";

print_r([
	'memory_get_process_usage_kernel in Kilo Bytes',
	$r_total,
	memory_get_process_usage_kernel(),
	memory_get_process_usage_kernel() - $r_total,
	'get_process_io_stats',
	get_process_io_stats()
]);

