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


function generate_utf8_string($length) {
    $result = '';

    $ranges = [
        '0020-007F', // Основная латиница
        '00A0-00FF', // Латиница-1 (дополнение)
        '0100-017F', // Расширенная латиница-А
        '0370-03FF', // Греческий и коптский
        '0400-04FF', // Кириллица
    ];

    foreach ($ranges as $range) {
        list($start, $end) = explode('-', $range);
        $start = hexdec($start);
        $end = hexdec($end);
        for ($i = $start; $i <= $end; $i++) {
            // Проверяем, является ли символ печатным
            $char = mb_chr($i, 'UTF-8');
            if (mb_ereg_match('\p{Print}', $char)) {
                $result .= $char;
            }
        }
    }

    // Если результат короче желаемой длины, повторяем его
    while (mb_strlen($result, 'UTF-8') < $length) {
        $result .= $result;
    }

    // Обрезаем строку до заданной длины
    $result = mb_substr($result, 0, $length, 'UTF-8');

    return $result;
}

function generate_utf8_random($length) {
    $result = '';
    $asciiRange = '';

    // Генерация печатных символов ASCII
    for ($i = 32; $i <= 126; $i++) {
        $asciiRange .= chr($i);
    }

    // Добавление ASCII символов в начало строки
    $result .= $asciiRange;

    // Добавление случайных символов UTF-8 до достижения заданной длины
    while (mb_strlen($result, 'UTF-8') < $length) {
        // Генерация случайного числа в диапазоне UTF-8
        $byte1 = mt_rand(0x80, 0xBF);
        $byte2 = mt_rand(0x80, 0xBF);
        $byte3 = mt_rand(0x80, 0xBF);
        $byte4 = mt_rand(0x80, 0xBF);

        // Сборка 4-байтового символа UTF-8
        $char = chr(0xF0 | ($byte1 >> 2)) . chr(0x80 | (($byte1 & 0x3) << 4) | ($byte2 >> 4)) . chr(0x80 | (($byte2 & 0xF) << 2) | ($byte3 >> 6)) . chr(0x80 | ($byte3 & 0x3F));

        // Добавление символа, если он печатный
        if (mb_check_encoding($char, 'UTF-8') && mb_strlen($char, 'UTF-8') + mb_strlen($result, 'UTF-8') <= $length) {
            $result .= $char;
        }
    }

    // Обрезка строки до заданной длины, если это необходимо
    $result = mb_substr($result, 0, $length, 'UTF-8');
    $result = str_replace([chr(127), chr(10), chr(0)], ' ', $result);

    return $result;
}



function mb_sec($time, $bytes, $k){
    $millions = $bytes / 1000000;
    if($k == 'syscr' || $k == 'syscw') {
        $speed = $bytes / $time;
        return number_format($speed, 2) . " per sec";
    } else {
        $speed = $millions / $time;
        return number_format($speed, 2) . " millions per sec";
    }
}

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

ini_set('error_reporting', E_ALL);
ini_set('display_errors', 0);
ini_set('html_errors', 0);
error_reporting(E_ALL);


$db_file = __DIR__ . '/fast_io.dat';



// #########################
// Check file_insert_line
$file_insert_line_passed = true;
$start = microtime(true);
$start_io = get_process_io_stats();

for($ii = 0; $ii < 100; $ii++){
    if(file_exists($db_file)) unlink($db_file);
    $align = mt_rand(32, 65536);

    ini_set('fast_io.buffer_size', mt_rand(16, 65536));

    $last_offset = 0;

    $c = mt_rand(10, 100);
    
    for($i=0; $i <= $c; $i++){
        $shuffle = mt_rand(1, $align * 2);

        $str = 'index_' . $i . ' file_insert_line_' . $i . ' ' . str_pad('', $shuffle, '1234567890');
        $file_offset = file_insert_line($db_file, $str, 2, $align);
        $file_line = file_get_keys($db_file, 0, 1, $last_offset, 1);

        if(
            empty($file_line[0]) ||
            $file_line[0]['line_length'] !== $align ||
            strlen($file_line[0]['line']) + 1 !== $align
        ){
            $file_insert_line_passed = false;
            break;
        }

        $file_str = $file_line[0]['line'];
        $file_array = explode(' ', $file_str);
        $str_array = explode(' ', $str);
        $analize = file_analize($db_file, 1);

        if(
            empty($file_array[0]) || 
            $file_array[0] !== 'index_' . $i ||
            $file_array[1] !== 'file_insert_line_' . $i ||
            trim($file_str) !== substr($str, 0, $align - 1) ||
            $analize['total_characters'] !== $align ||
            $analize['last_symbol'] !== 10 ||
            $analize['file_size'] !== ($i > 0 ? $align * ($i + 1) : $align) ||
            $analize['min_length'] !== $analize['max_length'] ||
            $analize['min_length'] != $analize['avg_length'] ||
            $analize['line_count'] !== 0
        ) {
            $file_insert_line_passed = false;           
            break;
        }

        if($file_offset == $last_offset) $last_offset += $align;  
    }
    
    $analize = file_analize($db_file);
     
    if(
        $file_insert_line_passed &&
        $last_offset - $align === $file_offset &&
        $analize['total_characters'] === $last_offset &&
        $analize['last_symbol'] === 10 &&
        $analize['file_size'] === $last_offset &&
        $analize['file_size'] === filesize($db_file) &&
        $analize['flow_interruption'] === 0 &&
        $analize['min_length'] === $analize['max_length'] &&
        $analize['min_length'] == $analize['avg_length'] &&
        $analize['line_count'] === $c + 1
    ) {
        $file_insert_line_passed = true;
    } else {
        $file_insert_line_passed = false;
        break;
    }
}
$time= microtime(true) - $start;

if($file_insert_line_passed) echo "\nCheck file_insert_line: time: ", $time, " - PASS",  "\n";
else echo "\nCheck file_insert_line - ERROR\n";

$end_io = get_process_io_stats();
foreach($end_io as $p=>$v)  echo $p, ': ', $v - $start_io[$p], ' (', mb_sec($time, $v - $start_io[$p], $p), ")\n";







// #########################
// Check file_analize
$file_analize_passed = true;
$start= microtime(true);
$start_io = get_process_io_stats();

for($ii = 0; $ii < 100; $ii++){
    if(file_exists($db_file)) unlink($db_file);
    $align = mt_rand(32, 65536);

    ini_set('fast_io.buffer_size', mt_rand(16, 65536));

    $last_offset = 0;

    $c = mt_rand(10, 100);
    
    for($i=0; $i <= $c; $i++){
        $shuffle = mt_rand(1, $align * 2);

        $str = 'index_' . $i . ' file_insert_line_' . $i . ' ' . str_pad('', $shuffle, '1234567890');
        $file_offset = file_insert_line($db_file, $str, 2, $align);

        $analize = file_analize($db_file);

        if(
            $analize['total_characters'] == $i > 0 ? $i * $align : $align &&
            $analize['last_symbol'] === 10 &&
            $analize['file_size'] === $last_offset + $align &&
            $analize['file_size'] === filesize($db_file) &&
            $analize['flow_interruption'] === 0 &&
            $analize['min_length'] === $analize['max_length'] &&
            $analize['min_length'] == $analize['avg_length'] &&
            $analize['line_count'] === $i + 1
        ) {
            $file_analize_passed = true;
        } else {
            $file_analize_passed = false;
            break;
        }
        if($file_offset == $last_offset) $last_offset += $align;  
    }
}
$time= microtime(true) - $start;

if($file_analize_passed) echo "\nCheck file_analize: time: ", $time, " - PASS",  "\n";
else echo "\nCheck file_analize - ERROR\n";

$end_io = get_process_io_stats();
foreach($end_io as $p=>$v)  echo $p, ': ', $v - $start_io[$p], ' (', mb_sec($time, $v - $start_io[$p], $p), ")\n";




// #########################
// Check file_get_keys
$file_get_keys_passed = true;
$start= microtime(true);
$start_io = get_process_io_stats();

for($ii = 0; $ii < 100; $ii++){
    if(file_exists($db_file)) unlink($db_file);
    $align = mt_rand(32, 65536);

    ini_set('fast_io.buffer_size', mt_rand(16, 65536));

    $c = mt_rand(10, 100);
    $insert_string = [];
    
    for($i=0; $i <= $c; $i++){
        $shuffle = mt_rand(1, $align * 2);

        $str = 'index_' . $i . ' file_insert_line_' . $i . ' ' . str_pad('', $shuffle, '1234567890');
        $file_offset = file_insert_line($db_file, $str, 2, $align);

        $trim_line = substr($str, 0, $align - 1);
        $insert_string[$i] = [
            'trim_line' => $trim_line,
            'trim_length' => strlen($trim_line),
            'line_offset' => $file_offset,
            'line_length' => $align,
            'line_count' => $i
        ];
    }

    $file_array = file_get_keys($db_file, 0, $c + 1, 0, 1);
    if(count($file_array) != count($insert_string)) {
        $file_get_keys_passed = false;
        break;
    }
    foreach($insert_string as $row_num=>$line_arr){
        if(
            $file_array[$row_num]['line_count'] !== $line_arr['line_count'] ||
            $file_array[$row_num]['line_offset'] !== $line_arr['line_offset'] ||
            $file_array[$row_num]['line_length'] !== $line_arr['line_length'] ||
            trim($file_array[$row_num]['line']) !== $line_arr['trim_line']
        ){
            $file_get_keys_passed = false;
            break;
        }
    }

    $file_array = file_get_keys($db_file, 0, $c + 1, 0, 0);
    if(count($file_array) != count($insert_string)) {
        $file_get_keys_passed = false;
        break;
    }
    foreach($insert_string as $row_num=>$line_arr){
        $str_array = explode(' ', $insert_string[$row_num]['trim_line']);

        if(
            $line_arr['line_count'] !== $file_array[$row_num]['line_count'] ||
            $line_arr['line_length'] !== $file_array[$row_num]['line_length'] ||
            $line_arr['line_offset'] !== $file_array[$row_num]['line_offset'] ||
            $file_array[$row_num]['key'] != $str_array[0]
        ){
            $file_get_keys_passed = false;
            break;
        }
    }

    $file_array = file_get_keys($db_file, 0, $c + 1, 0, 2);
    if(count($file_array) != count($insert_string)) {
        $file_get_keys_passed = false;
        break;
    }
    foreach($insert_string as $row_num=>$line_arr){
        if(
            $line_arr['line_count'] !== $file_array[$row_num]['line_count'] ||
            $line_arr['line_length'] !== $file_array[$row_num]['line_length'] ||
            $line_arr['line_offset'] !== $file_array[$row_num]['line_offset'] ||
            $line_arr['trim_length'] !== $file_array[$row_num]['trim_length'] ||
            $line_arr['trim_line'] !== $file_array[$row_num]['trim_line']
        ){
            $file_get_keys_passed = false;
            break;
        }
    }

    $file_array = file_get_keys($db_file, 0, $c + 1, 0, 3);
    if(count($file_array) != count($insert_string)) {
        $file_get_keys_passed = false;
        break;
    }
    foreach($insert_string as $row_num=>$line_arr){
        if(
            $line_arr['line_count'] !== $file_array[$row_num]['line_count'] ||
            $line_arr['line_length'] !== $file_array[$row_num]['line_length'] ||
            $line_arr['line_offset'] !== $file_array[$row_num]['line_offset'] 
        ){
            $file_get_keys_passed = false;
            break;
        }
    }

    $file_array = file_get_keys($db_file, 0, $c + 1, 0, 4);
    if(count($file_array) != count($insert_string)) {
        $file_get_keys_passed = false;
        break;
    }
    foreach($insert_string as $row_num=>$line_arr){
        $str_array = explode(' ', $insert_string[$row_num]['trim_line']);
        
        if(
            $file_array[$row_num] !== $str_array[0]
        ){
            $file_get_keys_passed = false;
            break;
        }
    }

    $file_array = file_get_keys($db_file, 0, $c + 1, 0, 5);
    if(count($file_array) != count($insert_string)) {
        $file_get_keys_passed = false;
        break;
    }
    foreach($insert_string as $row_num=>$line_arr){       
        if(
            $file_array[$row_num] !== $line_arr['trim_line']
        ){
            $file_get_keys_passed = false;
            break;
        }
    }
}
$time= microtime(true) - $start;

if($file_get_keys_passed) echo "\nCheck file_get_keys: time: ", $time, " - PASS",  "\n";
else echo "\nCheck file_get_keys - ERROR\n";

$end_io = get_process_io_stats();
foreach($end_io as $p=>$v)  echo $p, ': ', $v - $start_io[$p], ' (', mb_sec($time, $v - $start_io[$p], $p), ")\n";





// #########################
// Check file_search_array
$file_search_array_passed = true;
$start= microtime(true);
$start_io = get_process_io_stats();

for($ii = 0; $ii < 100; $ii++){
    if(file_exists($db_file)) unlink($db_file);
    $align = mt_rand(32, 65536);

    ini_set('fast_io.buffer_size', mt_rand(16, 65536));

    $c = mt_rand(10, 100);
    $insert_string = [];
    
    for($i=0; $i <= $c; $i++){
        $shuffle = mt_rand(1, $align * 2);

        $str = 'index_' . $i . ' file_insert_line_' . $i . ' ' . str_pad('', $shuffle, '1234567890');
        $file_offset = file_insert_line($db_file, $str, 2, $align);

        $trim_line = substr($str, 0, $align - 1);
        $insert_string[$i] = [
            'trim_line' => $trim_line,
            'trim_length' => strlen($trim_line),
            'line_offset' => $file_offset,
            'line_length' => $align,
            'line_count' => $i
        ];
    }

    $file_array = file_search_array($db_file, 'index_', 0, $c + 1, 0, 0);
    if(count($file_array) != count($insert_string)) {
        $file_search_array_passed = false;
        break;
    }
    foreach($insert_string as $row_num=>$line_arr){
        if(
            $file_array[$row_num]['trim_line'] !== $line_arr['trim_line'] ||
            $file_array[$row_num]['trim_length'] !== $line_arr['trim_length'] ||
            $file_array[$row_num]['line_offset'] !== $line_arr['line_offset'] ||
            $file_array[$row_num]['line_length'] !== $line_arr['line_length'] ||
            $file_array[$row_num]['line_count'] !== $line_arr['line_count']
        ){
            $file_search_array_passed = false;
            break;
        }
    }


    $file_array = file_search_array($db_file, 'index_', 0, $c + 1, 0, 1);
    if(count($file_array) != count($insert_string)) {
        $file_search_array_passed = false;
        break;
    }
    foreach($insert_string as $row_num=>$line_arr){
        if(
            $line_arr['line_count'] !== $file_array[$row_num]['line_count'] ||
            $line_arr['line_length'] !== $file_array[$row_num]['line_length'] ||
            $line_arr['line_offset'] !== $file_array[$row_num]['line_offset'] ||
            $line_arr['trim_line'] !== trim($file_array[$row_num]['line'])
        ){
            $file_search_array_passed = false;
            break;
        }
    }
    
    $file_array = file_search_array($db_file, 'index_', 0, $c + 1, 0, 2);
    if(count($file_array) != count($insert_string)) {
        $file_search_array_passed = false;
        break;
    }
    foreach($insert_string as $row_num=>$line_arr){       
        if(
            $line_arr['trim_line'] !== $file_array[$row_num]['trim_line']
        ){
            $file_search_array_passed = false;
            break;
        }
    }
    
    $file_array = file_search_array($db_file, 'index_', 0, $c + 1, 0, 3);
    if(
        empty($file_array['line_count']) ||
        empty($file_array['found_count']) ||
        $file_array['line_count'] !== $file_array['found_count'] ||
        $file_array['line_count'] !== $c + 1
    ){
        $file_search_array_passed = false;
        break;
    }

    $file_array = file_search_array($db_file, '\\w+_\\d+', 0, $c + 1, 0, 10);
    if(count($file_array) != count($insert_string)) {
        $file_search_array_passed = false;
        break;
    }
    foreach($insert_string as $row_num=>$line_arr){
        if(
            $file_array[$row_num]['trim_line'] !== $line_arr['trim_line'] ||
            $file_array[$row_num]['trim_length'] !== $line_arr['trim_length'] ||
            $file_array[$row_num]['line_offset'] !== $line_arr['line_offset'] ||
            $file_array[$row_num]['line_length'] !== $line_arr['line_length'] ||
            $file_array[$row_num]['line_count'] !== $line_arr['line_count']
        ){
            $file_search_array_passed = false;
            break;
        }
    }

    $file_array = file_search_array($db_file, '\\w+_\\d+', 0, $c + 1, 0, 11);
    if(count($file_array) != count($insert_string)) {
        $file_search_array_passed = false;
        break;
    }
    foreach($insert_string as $row_num=>$line_arr){
        if(
            $line_arr['line_count'] !== $file_array[$row_num]['line_count'] ||
            $line_arr['line_length'] !== $file_array[$row_num]['line_length'] ||
            $line_arr['line_offset'] !== $file_array[$row_num]['line_offset'] ||
            $line_arr['trim_line'] !== trim($file_array[$row_num]['line'])
        ){
            $file_search_array_passed = false;
            break;
        }
    }

    $file_array = file_search_array($db_file, '\\w+_\\d+', 0, $c + 1, 0, 12);
    if(count($file_array) != count($insert_string)) {
        $file_search_array_passed = false;
        break;
    }
    foreach($insert_string as $row_num=>$line_arr){       
        if(
            $line_arr['trim_line'] !== $file_array[$row_num]
        ){
            $file_search_array_passed = false;
            break;
        }
    }


    $file_array = file_search_array($db_file, '\\w+_\\d+', 0, $c + 1, 0, 13);
    if(
        empty($file_array['line_count']) ||
        empty($file_array['found_count']) ||
        $file_array['line_count'] !== $file_array['found_count'] ||
        $file_array['line_count'] !== $c + 1
    ){
        $file_search_array_passed = false;
        break;
    }


    $file_array = file_search_array($db_file, '\\w+_\\d+', 0, $c + 1, 0, 20);
    if(count($file_array) != count($insert_string)) {
        $file_search_array_passed = false;
        break;
    }
    foreach($insert_string as $row_num=>$line_arr){
        $str_array = explode(' ', $insert_string[$row_num]['trim_line']);

        if(
            $file_array[$row_num]['trim_line'] !== $line_arr['trim_line'] ||
            $file_array[$row_num]['trim_length'] !== $line_arr['trim_length'] ||
            $file_array[$row_num]['line_offset'] !== $line_arr['line_offset'] ||
            $file_array[$row_num]['line_length'] !== $line_arr['line_length'] ||
            $file_array[$row_num]['line_count'] !== $line_arr['line_count'] ||
            $file_array[$row_num]['line_matches'][0] !== $str_array[0] ||
            $file_array[$row_num]['line_matches'][1] !== $str_array[1]
        ){
            $file_search_array_passed = false;
            break;
        }
    }


    $file_array = file_search_array($db_file, '\\w+_\\d+', 0, $c + 1, 0, 21);
    if(count($file_array) != count($insert_string)) {
        $file_search_array_passed = false;
        break;
    }
    foreach($insert_string as $row_num=>$line_arr){
        $str_array = explode(' ', $insert_string[$row_num]['trim_line']);

        if(
            trim($file_array[$row_num]['line']) !== $line_arr['trim_line'] ||
            $file_array[$row_num]['line_offset'] !== $line_arr['line_offset'] ||
            $file_array[$row_num]['line_length'] !== $line_arr['line_length'] ||
            $file_array[$row_num]['line_count'] !== $line_arr['line_count'] ||
            $file_array[$row_num]['line_matches'][0] !== $str_array[0] ||
            $file_array[$row_num]['line_matches'][1] !== $str_array[1] 
        ){
            $file_search_array_passed = false;
            break;
        }
    }


    $file_array = file_search_array($db_file, '\\w+_\\d+', 0, $c + 1, 0, 22);
    if(count($file_array) != count($insert_string)) {
        $file_search_array_passed = false;
        break;
    }
    foreach($insert_string as $row_num=>$line_arr){
        $str_array = explode(' ', $insert_string[$row_num]['trim_line']);

        if(
            $file_array[$row_num]['line_offset'] !== $line_arr['line_offset'] ||
            $file_array[$row_num]['line_length'] !== $line_arr['line_length'] ||
            $file_array[$row_num]['line_count'] !== $line_arr['line_count'] ||
            $file_array[$row_num]['line_matches'][0]['line_match'] !== $str_array[0] ||
            $file_array[$row_num]['line_matches'][0]['match_offset'] !== strpos($insert_string[$row_num]['trim_line'], $str_array[0]) ||
            $file_array[$row_num]['line_matches'][0]['match_length'] !== strlen($str_array[0]) ||
            $file_array[$row_num]['line_matches'][1]['line_match'] !== $str_array[1] ||
            $file_array[$row_num]['line_matches'][1]['match_offset'] !== strpos($insert_string[$row_num]['trim_line'], $str_array[1]) ||
            $file_array[$row_num]['line_matches'][1]['match_length'] !== strlen($str_array[1]) 
        ){
            $file_search_array_passed = false;
            break;
        }
    }


    $file_array = file_search_array($db_file, '\\w+_\\d+', 0, $c + 1, 0, 23);
    if(count($file_array) != count($insert_string)) {
        $file_search_array_passed = false;
        break;
    }

    foreach($insert_string as $row_num=>$line_arr){
        $str_array = explode(' ', $insert_string[$row_num]['trim_line']);

        if(
            $file_array[$row_num][0] !== $str_array[0] ||
            $file_array[$row_num][1] !== $str_array[1]
        ){
            $file_search_array_passed = false;
            break;
        }
    }

}
$time= microtime(true) - $start;

if($file_search_array_passed) echo "\nCheck file_search_array: time: ", $time, " - PASS",  "\n";
else echo "\nCheck file_search_array - ERROR\n";

$end_io = get_process_io_stats();
foreach($end_io as $p=>$v)  echo $p, ': ', $v - $start_io[$p], ' (', mb_sec($time, $v - $start_io[$p], $p), ")\n";





// #########################
// Check file_select_array
$file_select_array_passed = true;
$start= microtime(true);
$start_io = get_process_io_stats();

for($ii = 0; $ii < 100; $ii++){
    if(file_exists($db_file)) unlink($db_file);
    $align = mt_rand(32, 65536);

    ini_set('fast_io.buffer_size', mt_rand(16, 65536));

    $c = mt_rand(10, 100);
    $insert_string = [];
    $query = [];
    
    for($i=0; $i <= $c; $i++){
        $shuffle = mt_rand(1, $align * 2);

        $str = 'index_' . $i . ' file_insert_line_' . $i . ' ' . str_pad('', $shuffle, '1234567890');
        $file_offset = file_insert_line($db_file, $str, 2, $align);

        $trim_line = substr($str, 0, $align - 1);
        $insert_string[$i] = [
            'trim_line' => $trim_line,
            'trim_length' => strlen($trim_line),
            'line_offset' => $file_offset,
            'line_length' => $align,
            'line_count' => $i
        ];

        $query[] = [
            $file_offset, $align
        ];
    }

    $file_array = file_select_array($db_file, $query);
    if(count($file_array) != count($insert_string)) {
        $file_select_array_passed = false;
        break;
    }
    foreach($insert_string as $row_num=>$line_arr){
        if(
            $file_array[$row_num]['trim_line'] !== $line_arr['trim_line'] ||
            $file_array[$row_num]['trim_length'] !== $line_arr['trim_length'] ||
            $file_array[$row_num]['line_offset'] !== $line_arr['line_offset'] ||
            $file_array[$row_num]['line_length'] !== $line_arr['line_length']
        ){           
            $file_select_array_passed = false;
            break;
        }
    }


    $file_array = file_select_array($db_file, $query, 'index', 1);
    if(count($file_array) != count($insert_string)) {
        $file_select_array_passed = false;
        break;
    }
    foreach($insert_string as $row_num=>$line_arr){
        if(
            $line_arr['line_length'] !== $file_array[$row_num]['line_length'] ||
            $line_arr['line_offset'] !== $file_array[$row_num]['line_offset'] ||
            $line_arr['trim_line'] !== trim($file_array[$row_num]['line'])
        ){           
            $file_select_array_passed = false;
            break;
        }
    }


    $file_array = file_select_array($db_file, $query, 'index', 2);
    if(count($file_array) != count($insert_string)) {
        $file_select_array_passed = false;
        break;
    }
    foreach($insert_string as $row_num=>$line_arr){  
        if(
            $file_array[$row_num] !== $line_arr['trim_line']
        ){            
            $file_select_array_passed = false;
            break;
        }
    }

    $file_array = file_select_array($db_file, $query, 'index', 3);
    if(
        empty($file_array['line_count']) ||
        empty($file_array['found_count']) ||
        $file_array['line_count'] !== $file_array['found_count'] ||
        $file_array['line_count'] !== $c + 1
    ){
        $file_select_array_passed = false;
        break;
    }

    $file_array = file_select_array($db_file, $query, 'index', 5);
    if(count($file_array) != count($insert_string)) {
        $file_select_array_passed = false;
        break;
    }
    foreach($insert_string as $row_num=>$line_arr){
        if(
            $file_array[$row_num]['trim_line'] !== $line_arr['trim_line'] ||
            $file_array[$row_num]['trim_length'] !== $line_arr['trim_length'] ||
            $file_array[$row_num]['line_offset'] !== $line_arr['line_offset'] ||
            $file_array[$row_num]['line_length'] !== $line_arr['line_length']
        ){           
            $file_select_array_passed = false;
            break;
        }
    }


    $file_array = file_select_array($db_file, $query, 'index', 6);
    if(count($file_array) != count($insert_string)) {
        $file_select_array_passed = false;
        break;
    }
    foreach($insert_string as $row_num=>$line_arr){
        if(
            $line_arr['line_length'] !== $file_array[$row_num]['line_length'] ||
            $line_arr['line_offset'] !== $file_array[$row_num]['line_offset'] ||
            $line_arr['trim_line'] !== trim($file_array[$row_num]['line'])
        ){           
            $file_select_array_passed = false;
            break;
        }
    }

    $file_array = file_select_array($db_file, $query, 'index', 7);
    if(count($file_array) != count($insert_string)) {
        $file_select_array_passed = false;
        break;
    }
    foreach($insert_string as $row_num=>$line_arr){       
        if(
            $file_array[$row_num] !== $line_arr['trim_line']
        ){
            $file_select_array_passed = false;
            break;
        }
    }


    $file_array = file_select_array($db_file, $query, 'index', 10);
    if(count($file_array) != count($insert_string)) {
        $file_select_array_passed = false;
        break;
    }
    foreach($insert_string as $row_num=>$line_arr){
        if(
            $file_array[$row_num]['trim_line'] !== $line_arr['trim_line'] ||
            $file_array[$row_num]['trim_length'] !== $line_arr['trim_length'] ||
            $file_array[$row_num]['line_offset'] !== $line_arr['line_offset'] ||
            $file_array[$row_num]['line_length'] !== $line_arr['line_length']
        ){
            $file_select_array_passed = false;
            break;
        }
    }


    $file_array = file_select_array($db_file, $query, 'index', 11);
    if(count($file_array) != count($insert_string)) {
        $file_select_array_passed = false;
        break;
    }
    foreach($insert_string as $row_num=>$line_arr){
        if(
            $line_arr['line_length'] !== $file_array[$row_num]['line_length'] ||
            $line_arr['line_offset'] !== $file_array[$row_num]['line_offset'] ||
            $line_arr['trim_line'] !== trim($file_array[$row_num]['line'])
        ){
            $file_select_array_passed = false;
            break;
        }
    }

    $file_array = file_select_array($db_file, $query, 'index', 12);
    if(count($file_array) != count($insert_string)) {
        $file_select_array_passed = false;
        break;
    }
    foreach($insert_string as $row_num=>$line_arr){       
        if(
            $line_arr['trim_line'] !== $file_array[$row_num]
        ){
            $file_select_array_passed = false;
            break;
        }
    }


    $file_array = file_select_array($db_file, $query, 'index', 13);
    if(
        empty($file_array['line_count']) ||
        empty($file_array['found_count']) ||
        $file_array['line_count'] !== $file_array['found_count'] ||
        $file_array['line_count'] !== $c + 1
    ){
        $file_select_array_passed = false;
        break;
    }


    $file_array = file_select_array($db_file, $query, '\\w+_\\d+', 20);
    if(count($file_array) != count($insert_string)) {
        $file_select_array_passed = false;
        break;
    }
    foreach($insert_string as $row_num=>$line_arr){
        $str_array = explode(' ', $insert_string[$row_num]['trim_line']);

        if(
            $file_array[$row_num]['trim_line'] !== $line_arr['trim_line'] ||
            $file_array[$row_num]['trim_length'] !== $line_arr['trim_length'] ||
            $file_array[$row_num]['line_offset'] !== $line_arr['line_offset'] ||
            $file_array[$row_num]['line_length'] !== $line_arr['line_length'] ||
            $file_array[$row_num]['line_matches'][0] !== $str_array[0] ||
            $file_array[$row_num]['line_matches'][1] !== $str_array[1]
        ){
            $file_select_array_passed = false;
            break;
        }
    }


    $file_array = file_select_array($db_file, $query, '\\w+_\\d+', 21);
    if(count($file_array) != count($insert_string)) {
        $file_select_array_passed = false;
        break;
    }
    foreach($insert_string as $row_num=>$line_arr){
        $str_array = explode(' ', $insert_string[$row_num]['trim_line']);

        if(
            trim($file_array[$row_num]['line']) !== $line_arr['trim_line'] ||
            $file_array[$row_num]['line_offset'] !== $line_arr['line_offset'] ||
            $file_array[$row_num]['line_length'] !== $line_arr['line_length'] ||
            $file_array[$row_num]['line_matches'][0] !== $str_array[0] ||
            $file_array[$row_num]['line_matches'][1] !== $str_array[1] 
        ){           
            $file_select_array_passed = false;
            break;
        }
    }


    $file_array = file_select_array($db_file, $query, '\\w+_\\d+', 22);
    if(count($file_array) != count($insert_string)) {
        $file_select_array_passed = false;
        break;
    }
    foreach($insert_string as $row_num=>$line_arr){
        $str_array = explode(' ', $insert_string[$row_num]['trim_line']);

        if(
            $file_array[$row_num]['line_offset'] !== $line_arr['line_offset'] ||
            $file_array[$row_num]['line_length'] !== $line_arr['line_length'] ||
            $file_array[$row_num]['line_matches'][0]['line_match'] !== $str_array[0] ||
            $file_array[$row_num]['line_matches'][0]['match_offset'] !== strpos($insert_string[$row_num]['trim_line'], $str_array[0]) ||
            $file_array[$row_num]['line_matches'][0]['match_length'] !== strlen($str_array[0]) ||
            $file_array[$row_num]['line_matches'][1]['line_match'] !== $str_array[1] ||
            $file_array[$row_num]['line_matches'][1]['match_offset'] !== strpos($insert_string[$row_num]['trim_line'], $str_array[1]) ||
            $file_array[$row_num]['line_matches'][1]['match_length'] !== strlen($str_array[1]) 
        ){
            $file_select_array_passed = false;
            break;
        }
    }


    $file_array = file_select_array($db_file, $query, '\\w+_\\d+', 23);
    if(count($file_array) != count($insert_string)) {
        $file_select_array_passed = false;
        break;
    }
    foreach($insert_string as $row_num=>$line_arr){
        $str_array = explode(' ', $insert_string[$row_num]['trim_line']);

        if(
            $file_array[$row_num][0] !== $str_array[0] ||
            $file_array[$row_num][1] !== $str_array[1]
        ){
            $file_select_array_passed = false;
            break;
        }
    }



}
$time= microtime(true) - $start;

if($file_select_array_passed) echo "\nCheck file_select_array: time: ", $time, " - PASS",  "\n";
else echo "\nCheck file_select_array - ERROR\n";

$end_io = get_process_io_stats();
foreach($end_io as $p=>$v)  echo $p, ': ', $v - $start_io[$p], ' (', mb_sec($time, $v - $start_io[$p], $p), ")\n";






// #########################
// Check file_search_line
$file_search_line_passed = true;
$start= microtime(true);
$start_io = get_process_io_stats();

for($ii = 0; $ii < 100; $ii++){
    if(file_exists($db_file)) unlink($db_file);
    $align = mt_rand(32, 65536);

    ini_set('fast_io.buffer_size', mt_rand(16, 65536));
    $c = mt_rand(10, 100);
    $insert_string = [];
    
    for($i=0; $i <= $c; $i++){
        $shuffle = mt_rand(1, $align * 2);

        $str = 'index_' . $i . ' file_insert_line_' . $i . ' ' . str_pad('', $shuffle, '1234567890');
        $file_offset = file_insert_line($db_file, $str, 2, $align);

        $trim_line = substr($str, 0, $align - 1);
        $insert_string[$i] = [
            'key' => 'index_' . $i,
            'trim_line' => $trim_line,
            'line_offset' => $file_offset,
        ];
    }

    
    foreach($insert_string as $row_num=>$line_arr){
        for($i = 0; $i < 2; $i++){
            $file_line = file_search_line($db_file, $line_arr['key'], $i == 0 ? 0 : $line_arr['line_offset'], 0);

            if(
                $file_line === false ||
                trim($file_line) !== $line_arr['trim_line']
            ){           
                $file_search_line_passed = false;
                break;
            }
        }
    }


    foreach($insert_string as $row_num=>$line_arr){
        for($i = 0; $i < 2; $i++){
            $file_line = file_search_line($db_file, $line_arr['key'], $i == 0 ? 0 : $line_arr['line_offset'], 10);

            if(
                $file_line === false ||
                trim($file_line) !== $line_arr['trim_line']
            ){           
                $file_search_line_passed = false;
                break;
            }
        }

    }

}

$time= microtime(true) - $start;

if($file_search_line_passed) echo "\nCheck file_search_line: time: ", $time, " - PASS",  "\n";
else echo "\nCheck file_search_line - ERROR\n";

$end_io = get_process_io_stats();
foreach($end_io as $p=>$v)  echo $p, ': ', $v - $start_io[$p], ' (', mb_sec($time, $v - $start_io[$p], $p), ")\n";






// #########################
// Check file_select_line
$file_select_line_passed = true;
$start= microtime(true);
$start_io = get_process_io_stats();

for($ii = 0; $ii < 100; $ii++){
    if(file_exists($db_file)) unlink($db_file);
    $align = mt_rand(32, 65536);

    ini_set('fast_io.buffer_size', mt_rand(16, 65536));
    $c = mt_rand(10, 100);
    $insert_string = [];
    
    for($i=0; $i <= $c; $i++){
        $shuffle = mt_rand(1, $align * 2);

        $str = 'index_' . $i . ' file_insert_line_' . $i . ' ' . str_pad('', $shuffle, '1234567890');
        $file_offset = file_insert_line($db_file, $str, 2, $align);

        $trim_line = substr($str, 0, $align - 1);
        $insert_string[$i] = [
            'trim_line' => $trim_line,
            'trim_length' => strlen($trim_line),
            'line_offset' => $file_offset,
            'line_count' => $i
        ];
    }

    
    foreach($insert_string as $row_num=>$line_arr){
        for($i = 0; $i < 2; $i++){
            if($i == 0) $file_line = file_select_line($db_file, $line_arr['line_count'], $align, 0);
            if($i == 1) $file_line = file_select_line($db_file, $line_arr['line_count'], $align, 2);

            if(
                $file_line === false ||
                trim($file_line) !== $line_arr['trim_line']
            ){           
                $file_select_line_passed = false;
                break;
            }
        }
    }


    foreach($insert_string as $row_num=>$line_arr){
        for($i = 0; $i < 2; $i++){
            if($i == 0) $file_line = file_select_line($db_file, $line_arr['line_offset'], $line_arr['trim_length'], 1);
            if($i == 1) $file_line = file_select_line($db_file, $line_arr['line_offset'], $line_arr['trim_length'], 3);

            if(
                $file_line === false ||
                trim($file_line) !== $line_arr['trim_line']
            ){           
                $file_select_line_passed = false;
                break;
            }
        }
    }

}

$time= microtime(true) - $start;

if($file_select_line_passed) echo "\nCheck file_select_line: time: ", $time, " - PASS",  "\n";
else echo "\nCheck file_select_line - ERROR\n";

$end_io = get_process_io_stats();
foreach($end_io as $p=>$v)  echo $p, ': ', $v - $start_io[$p], ' (', mb_sec($time, $v - $start_io[$p], $p), ")\n";





// #########################
// Check file_pop_line
$file_pop_line_passed = true;
$start= microtime(true);
$start_io = get_process_io_stats();


for($ii = 0; $ii < 50; $ii++){
    if(file_exists($db_file)) unlink($db_file);
    $align = mt_rand(32, 65536);


    $c = mt_rand(1, 50);
    $insert_string = [];
    $mode = 0;

    if(($ii % 10) == 0) {
        ini_set('fast_io.buffer_size', mt_rand($align - 10, $align + 10));
    } else {
        ini_set('fast_io.buffer_size', mt_rand(16, 65536));
    }
    

    for($i=0; $i <= $c; $i++){
        $shuffle = mt_rand(16, $align * 2);

        if($mode == 3) $mode = 2;
        else $mode = 3;

        $str = 'index_' . $i . ' file_insert_line_' . $i . ' ' . str_pad('', $shuffle, '1234567890_' . $i . '_');

        $file_offset = file_insert_line($db_file, $str, 2, $align);
        $trim_line = substr($str, 0, $align - 1);

        $file_last_str = file_pop_line($db_file, $align, $mode);

        if(
            ($mode == 2 && $file_last_str !== $trim_line) ||
            ($mode == 3 && trim($file_last_str) !== $trim_line) 
        ){
            $file_pop_line_passed = false;
            break;
        }

        $insert_string[$i] = [
            'trim_line' => $trim_line,
            'trim_length' => strlen($trim_line),
            'line_offset' => $file_offset,
            'line_count' => $i,
        ];
    }

    $insert_string_reverse = array_reverse($insert_string, true);
    foreach($insert_string_reverse as $row_num=>$line_arr){
        if($mode == 0) $mode = 1;
        else $mode = 0;

        $file_last_str = file_pop_line($db_file, $align, $mode);

        if($mode == 1) $file_last_str = trim($file_last_str);

        if(
            $file_last_str === false ||
            trim($file_last_str) !== $line_arr['trim_line']
        ){           
            $file_pop_line_passed = false;
            break;
        }
    }

    if(filesize($db_file) != 0){
        $file_pop_line_passed = false;
        break;
    }

    unlink($db_file);

    foreach($insert_string as $row_num=>$line_arr){
        file_insert_line($db_file, $line_arr['trim_line'], 2, $align);
    }  
    
    $insert_string_reverse = array_reverse($insert_string, true);
    foreach($insert_string_reverse as $row_num=>$line_arr){ 
        if($mode == 0) $mode = 1;
        else $mode = 0;

        $file_last_str = file_pop_line($db_file, -1, $mode);

        if(
            $file_last_str === false ||
            trim($file_last_str) !== $line_arr['trim_line']
        ){
            $file_pop_line_passed = false;
            break;
        }
    }

    if(filesize($db_file)!= 0){
        $file_pop_line_passed = false;
        break;
    }

    unlink($db_file);

    foreach($insert_string as $row_num=>$line_arr){
        file_insert_line($db_file, $line_arr['trim_line'], 2, $align);
    }

    $file_last_str = file_pop_line($db_file, 0 - $c - 1, 1);
    $file_str_array = array_slice(explode("\n", $file_last_str), 0, -1);
    foreach($insert_string as $row_num=>$line_arr){
        if(
            trim($file_str_array[$row_num]) !== $line_arr['trim_line']
        ){
            $file_pop_line_passed = false;
            break;
        }
    }

    if(filesize($db_file) != 0){
        $file_pop_line_passed = false;
        break;
    }

    unlink($db_file);
    

    foreach($insert_string as $row_num=>$line_arr){
        file_insert_line($db_file, $line_arr['trim_line'], 2, $align);
    }

    $file_last_str = file_pop_line($db_file, 0 - $c - 2, 3);
    $file_str_array = array_slice(explode("\n", $file_last_str), 0, -1);
    foreach($insert_string as $row_num=>$line_arr){
        if(
            trim($file_str_array[$row_num]) !== $line_arr['trim_line']
        ){
            $file_pop_line_passed = false;
            break;
        }
    }

    if(filesize($db_file) == 0){
        $file_pop_line_passed = false;
        break;
    }

    unlink($db_file);
}


$utf8_random_str = generate_utf8_random(65536);
$utf8_str = generate_utf8_string(65536);

for($i=0; $i <= 500; $i++){
    $align = mt_rand(1, 65536);
    $c = mt_rand(1, 65536);

    if(($i % 100) == 0) {
        $align = mt_rand(1, 10);
        $c = mt_rand(1, 10);
    }

    if($i < 250) $str = substr($utf8_random_str, 0, $c);
    else $str = substr($utf8_str, 0, $c);


    $trim_line = substr($str, 0, $align - 1);
    $file_offset = file_insert_line($db_file, $str, 2, $align);
   
    $file_last_str = file_pop_line($db_file, $align, $mode);
    
    if(
        ($mode == 2 && $file_last_str !== $trim_line) ||
        ($mode == 3 && trim($file_last_str) !== $trim_line) 
    ){
        $file_pop_line_passed = false;
        break;
    } 
}



$time= microtime(true) - $start;

if($file_pop_line_passed) echo "\nCheck file_pop_line: time: ", $time, " - PASS",  "\n";
else echo "\nCheck file_pop_line - ERROR\n";

$end_io = get_process_io_stats();
foreach($end_io as $p=>$v)  echo $p, ': ', $v - $start_io[$p], ' (', mb_sec($time, $v - $start_io[$p], $p), ")\n";





// #########################
// Check file_callback_line
$file_callback_line_passed = true;
$start= microtime(true);
$start_io = get_process_io_stats();


for($ii = 0; $ii < 100; $ii++){
    if(file_exists($db_file)) unlink($db_file);
    $align = mt_rand(32, 65536);

    ini_set('fast_io.buffer_size', mt_rand(16, 65536));

    $c = mt_rand(10, 100);
    $insert_string = [];

    for($i=0; $i <= $c; $i++){
        $shuffle = mt_rand(1, 65536);

        $str = 'index_' . $i . ' file_insert_line_' . $i . ' ' . str_pad('', $shuffle, '1234567890_' . $i . '_');
        $file_offset = file_insert_line($db_file, $str, 2, $align);
        $trim_line = substr($str, 0, $align - 1);
               
        $insert_string[$i] = [
            'trim_line' => $trim_line,
            'line_length' => $align,
            'line_offset' => $file_offset,
            'line_count' => $i
        ];
    }

    $callback_line_arr = [];

    file_callback_line(
        $db_file,
        function () use (&$callback_line_arr) {
            $return_arr = [];
            $args = func_num_args();

            if($args > 0){
                $return_arr[] = func_get_arg(0);

                if($args > 1) $return_arr[] = func_get_arg(1);
                if($args > 2) $return_arr[] = func_get_arg(2);
                if($args > 3) $return_arr[] = func_get_arg(3);
                if($args > 4) $return_arr[] = func_get_arg(4);
                if($args > 5) $return_arr[] = func_get_arg(5);
                if($args > 6) $return_arr[] = func_get_arg(6);
                if($args > 7) $return_arr[] = func_get_arg(7);
            }

            $callback_line_arr[] = $return_arr;

            return true;
        }, 0, 7
    );


    foreach($insert_string as $row_num=>$line_arr){
        if(
            trim($callback_line_arr[$row_num][0]) !== $line_arr['trim_line'] ||
            $db_file != $callback_line_arr[$row_num][1] ||
            $line_arr['line_offset'] != $callback_line_arr[$row_num][2] ||
            $line_arr['line_length'] != $callback_line_arr[$row_num][3] ||
            $line_arr['line_count'] != $callback_line_arr[$row_num][4] ||
            0 != $callback_line_arr[$row_num][5] ||
            filesize($db_file) != $callback_line_arr[$row_num][7] 
        ){  
            $file_callback_line_passed = false;
            break;
        }
    }


    foreach($insert_string as $row_num=>$line_arr){
        $file_line_str = file_callback_line(
            $db_file,
            function () use (&$line_arr){
                if(!empty(func_get_arg(6))) return false;

                if(
                    func_get_arg(2) == $line_arr['line_offset'] &&
                    func_get_arg(3) == $line_arr['line_length']
                ) return func_get_arg(0); 

                return true;
            }, $line_arr['line_offset'], 6
        );


        if(
            trim($file_line_str) !== $line_arr['trim_line']
        ){  
            $file_callback_line_passed = false;
            break;
        }
    }

}

$time= microtime(true) - $start;

if($file_callback_line_passed) echo "\nCheck file_callback_line: time: ", $time, " - PASS",  "\n";
else echo "\nCheck file_callback_line - ERROR\n";

$end_io = get_process_io_stats();
foreach($end_io as $p=>$v)  echo $p, ': ', $v - $start_io[$p], ' (', mb_sec($time, $v - $start_io[$p], $p), ")\n";


