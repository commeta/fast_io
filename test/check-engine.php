<?php
/*
 * Fast_IO Extension for PHP 8
 * https://github.com/commeta/fast_io
 * 
 * Copyright 2026 commeta <dcs-spb@ya.ru>
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
 */

// ============================================
// РЕЖИМ ОТЛАДКИ - включите для подробного вывода
// ============================================
define('DEBUG_MODE', false);
define('TRACE_MODE', false);

function debug_log($message) {
    if (DEBUG_MODE) {
        echo "[DEBUG] " . $message . "\n";
    }
}

function trace_log($label, $value) {
    if (TRACE_MODE) {
        if (is_array($value)) {
            echo "  [TRACE] {$label}: [Array: " . count($value) . " items]\n";
        } elseif (is_bool($value)) {
            echo "  [TRACE] {$label}: " . ($value ? 'true' : 'false') . "\n";
        } else {
            echo "  [TRACE] {$label}: {$value}\n";
        }
    }
}

function error_log_detailed($test_name, $sub_test, $error_msg, $details = [], $trace = []) {
    $output = "\n❌ ERROR in {$test_name} - {$sub_test}:\n";
    $output .= "   {$error_msg}\n";
    
    if (!empty($details)) {
        $output .= "   Details:\n";
        foreach ($details as $key => $value) {
            if (is_array($value)) {
                $output .= "     {$key}: [Array with " . count($value) . " items]\n";
            } elseif (is_bool($value)) {
                $output .= "     {$key}: " . ($value ? 'true' : 'false') . "\n";
            } else {
                $output .= "     {$key}: {$value}\n";
            }
        }
    }
    
    if (!empty($trace)) {
        $output .= "   Trace:\n";
        foreach ($trace as $key => $value) {
            $output .= "     {$key}: {$value}\n";
        }
    }
    
    echo $output;
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
        return [];
    }
    
    $ioStats = [];
    
    $lines = explode("\n", trim($ioData));
    foreach ($lines as $line) {
        if (empty($line)) continue;
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
$db_data_file = __DIR__ . '/fast_io_data.dat';
$db_replica_file = __DIR__ . '/fast_io_replica.dat';



// #########################
// Check file_insert_line
$file_insert_line_passed = true;
$start = microtime(true);
$start_io = get_process_io_stats();

for($ii = 0; $ii < 100; $ii++){
    debug_log("file_insert_line: iteration {$ii}/100");
    
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
            error_log_detailed(
                'file_insert_line',
                "iteration {$ii}, line {$i}",
                'Line length validation failed',
                [
                    'expected_align' => $align,
                    'actual_line_length' => $file_line[0]['line_length'] ?? 'null',
                    'actual_strlen_plus_1' => (strlen($file_line[0]['line'] ?? '') + 1)
                ],
                ['string_len' => strlen($str), 'shuffle' => $shuffle]
            );
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
            error_log_detailed(
                'file_insert_line',
                "iteration {$ii}, line {$i} - content validation",
                'Content or analysis failed',
                [
                    'file_array[0]' => $file_array[0] ?? 'null',
                    'expected_key' => 'index_' . $i,
                    'total_characters' => $analize['total_characters'] ?? 'null',
                    'expected_total' => $align,
                    'line_count' => $analize['line_count'] ?? 'null'
                ],
                ['index' => $i]
            );
            break;
        }

        if($file_offset == $last_offset) $last_offset += $align;  
    }
    
    if(!$file_insert_line_passed) break;
    
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
        error_log_detailed(
            'file_insert_line',
            "iteration {$ii} - final validation",
            'Final analysis validation failed',
            [
                'total_characters' => $analize['total_characters'] ?? 'null',
                'expected_total' => $last_offset,
                'last_symbol' => $analize['last_symbol'] ?? 'null',
                'file_size' => $analize['file_size'] ?? 'null',
                'actual_filesize' => filesize($db_file),
                'flow_interruption' => $analize['flow_interruption'] ?? 'null',
                'line_count' => $analize['line_count'] ?? 'null',
                'expected_lines' => $c + 1
            ],
            ['align' => $align, 'lines_inserted' => $c + 1]
        );
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
    debug_log("file_analize: iteration {$ii}/100");
    
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
            $analize['total_characters'] !== ($i + 1) * $align ||
            $analize['last_symbol'] !== 10 ||
            $analize['file_size'] !== $last_offset + $align ||
            $analize['file_size'] !== filesize($db_file) ||
            $analize['flow_interruption'] !== 0 ||
            $analize['min_length'] !== $analize['max_length'] ||
            $analize['min_length'] != $analize['avg_length'] ||
            $analize['line_count'] !== $i + 1
        ) {
            $file_analize_passed = false;
            error_log_detailed(
                'file_analize',
                "iteration {$ii}, line {$i}",
                'Analysis validation failed',
                [
                    'total_characters' => $analize['total_characters'] ?? 'null',
                    'expected_total' => ($i + 1) * $align,
                    'line_count' => $analize['line_count'] ?? 'null',
                    'expected_lines' => $i + 1,
                    'file_size' => $analize['file_size'] ?? 'null',
                    'expected_size' => $last_offset + $align
                ],
                ['index' => $i, 'align' => $align]
            );
            break;
        }
        if($file_offset == $last_offset) $last_offset += $align;  
    }

    if(!$file_analize_passed) break;

    $analize_mode0 = file_analize($db_file, 0);
    $analize_mode1 = file_analize($db_file, 1);

    if(
        $analize_mode1['total_characters'] !== $align ||  // Проверка на первую строку!
        $analize_mode1['line_count'] !== 0 ||  // spec: mode=1 returns line_count=0
        $analize_mode1['min_length'] !== $align ||
        $analize_mode1['max_length'] !== $align ||
        $analize_mode1['file_size'] !== $analize_mode0['file_size'] ||
        $analize_mode1['last_symbol'] !== 10
    ) {
        $file_analize_passed = false;
        error_log_detailed(
            'file_analize',
            "iteration {$ii} - mode 1 comparison",
            'Mode 1 validation failed',
            [
                'mode1_total_chars' => $analize_mode1['total_characters'] ?? 'null',
                'expected_chars' => $align,
                'mode1_line_count' => $analize_mode1['line_count'] ?? 'null',
                'mode1_min_length' => $analize_mode1['min_length'] ?? 'null',
                'expected_min' => $align,
                'mode0_file_size' => $analize_mode0['file_size'] ?? 'null',
                'mode1_file_size' => $analize_mode1['file_size'] ?? 'null'
            ],
            ['mode0_total' => $analize_mode0['total_characters'], 'c' => $c]
        );
        break;
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
    debug_log("file_get_keys: iteration {$ii}/100");
    
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
        error_log_detailed(
            'file_get_keys',
            "iteration {$ii} - mode 1 count check",
            'Array count mismatch',
            [
                'returned_count' => count($file_array),
                'expected_count' => count($insert_string)
            ],
            ['c' => $c]
        );
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
            error_log_detailed(
                'file_get_keys',
                "iteration {$ii} - mode 1 row {$row_num}",
                'Row data mismatch',
                [
                    'line_count' => $file_array[$row_num]['line_count'] ?? 'null',
                    'expected_count' => $line_arr['line_count'],
                    'line_offset' => $file_array[$row_num]['line_offset'] ?? 'null',
                    'expected_offset' => $line_arr['line_offset']
                ],
                ['row' => $row_num]
            );
            break;
        }
    }

    if(!$file_get_keys_passed) break;

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

    if(!$file_get_keys_passed) break;

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

    if(!$file_get_keys_passed) break;

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

    if(!$file_get_keys_passed) break;

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

    if(!$file_get_keys_passed) break;

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
    
    if(!$file_get_keys_passed) break;
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
    debug_log("file_search_array: iteration {$ii}/100");
    
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
        error_log_detailed(
            'file_search_array',
            "iteration {$ii} - mode 0 count check",
            'Array count mismatch',
            [
                'returned_count' => count($file_array),
                'expected_count' => count($insert_string)
            ],
            ['c' => $c, 'align' => $align]
        );
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
    if(!$file_search_array_passed) break;


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
    if(!$file_search_array_passed) break;
    
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
    if(!$file_search_array_passed) break;
    
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
    if(!$file_search_array_passed) break;

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
    if(!$file_search_array_passed) break;

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
    if(!$file_search_array_passed) break;


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
    if(!$file_search_array_passed) break;


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
    if(!$file_search_array_passed) break;


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
    if(!$file_search_array_passed) break;


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
    if(!$file_search_array_passed) break;
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
    debug_log("file_select_array: iteration {$ii}/100");
    
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

        $query[] = [$file_offset, $align];
    }

    $file_array = file_select_array($db_file, $query);
    if(count($file_array) != count($insert_string)) {
        $file_select_array_passed = false;
        error_log_detailed(
            'file_select_array',
            "iteration {$ii} - default mode count check",
            'Array count mismatch',
            [
                'returned_count' => count($file_array),
                'expected_count' => count($insert_string)
            ],
            ['c' => $c]
        );
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
    
    if(!$file_select_array_passed) break;

    $modes_to_test = [1, 2, 3, 5, 6, 7, 10, 11, 12, 13, 20, 21, 22, 23];
    
    foreach ($modes_to_test as $mode) {
        $file_array = file_select_array($db_file, $query, 'index', $mode);
        
        if ($mode == 3 || $mode == 13) {
            if(
                empty($file_array['line_count']) ||
                $file_array['line_count'] !== $c + 1
            ){
                $file_select_array_passed = false;
                error_log_detailed(
                    'file_select_array',
                    "iteration {$ii} - mode {$mode}",
                    'Count mismatch',
                    [
                        'line_count' => $file_array['line_count'] ?? 'null',
                        'expected' => $c + 1
                    ],
                    ['mode' => $mode]
                );
                break;
            }
        } elseif (count($file_array) != count($insert_string)) {
            $file_select_array_passed = false;
            error_log_detailed(
                'file_select_array',
                "iteration {$ii} - mode {$mode} count check",
                'Array count mismatch',
                [
                    'returned_count' => count($file_array),
                    'expected_count' => count($insert_string),
                    'mode' => $mode
                ],
                ['c' => $c]
            );
            break;
        }
    }
    
    if(!$file_select_array_passed) break;
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
    debug_log("file_search_line: iteration {$ii}/100");
    
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
            'key' => 'index_' . $i . ' ',
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
                error_log_detailed(
                    'file_search_line',
                    "iteration {$ii}, row {$row_num}, search attempt {$i}",
                    'Search or content mismatch',
                    [
                        'offset_used' => $i == 0 ? 0 : $line_arr['line_offset'],
                        'found' => $file_line !== false ? 'yes' : 'no',
                        'expected_key' => $line_arr['key']
                    ],
                    ['row' => $row_num]
                );
                break;
            }
        }
        if(!$file_search_line_passed) break;
    }
    if(!$file_search_line_passed) break;

    foreach($insert_string as $row_num=>$line_arr){
        $file_line = file_search_line($db_file, $line_arr['key'], $line_arr['line_offset'], 1);
        if(
            $file_line === false ||
            $file_line !== $line_arr['trim_line']
        ){           
            $file_search_line_passed = false;
            break;
        }
    }
    if(!$file_search_line_passed) break;

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
        if(!$file_search_line_passed) break;
    }
    if(!$file_search_line_passed) break;

    foreach($insert_string as $row_num=>$line_arr){
        $file_line = file_search_line($db_file, $line_arr['key'], $line_arr['line_offset'], 11);
        if(
            $file_line === false ||
            $file_line !== $line_arr['trim_line']
        ){           
            $file_search_line_passed = false;
            break;
        }
    }
    if(!$file_search_line_passed) break;
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
    debug_log("file_select_line: iteration {$ii}/100");
    
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
        $file_line = file_select_line($db_file, $line_arr['line_count'], $align, 0);
        if(
            $file_line === false ||
            $file_line !== $line_arr['trim_line']
        ){           
            $file_select_line_passed = false;
            break;
        }
    }
    if(!$file_select_line_passed) break;

    foreach($insert_string as $row_num=>$line_arr){
        $file_line = file_select_line($db_file, $line_arr['line_count'], $align, 2);
        if(
            $file_line === false ||
            trim($file_line) !== $line_arr['trim_line']
        ){           
            $file_select_line_passed = false;
            break;
        }
    }
    if(!$file_select_line_passed) break;

    foreach($insert_string as $row_num=>$line_arr){
        $file_line = file_select_line($db_file, $line_arr['line_offset'], $align, 1);
        if(
            $file_line === false ||
            $file_line !== $line_arr['trim_line']
        ){           
            $file_select_line_passed = false;
            break;
        }
    }
    if(!$file_select_line_passed) break;

    foreach($insert_string as $row_num=>$line_arr){
        $file_line = file_select_line($db_file, $line_arr['line_offset'], $align, 3);
        if(
            $file_line === false ||
            trim($file_line) !== $line_arr['trim_line']
        ){           
            $file_select_line_passed = false;
            break;
        }
    }
    if(!$file_select_line_passed) break;
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


for($ii = 0; $ii < 100; $ii++){
    debug_log("file_pop_line: iteration {$ii}/100");
    
    if(file_exists($db_file)) unlink($db_file);
    $align = mt_rand(32, 65536);


    $c = mt_rand(1, 50);
    $insert_string = [];
    $mode = 0;

    if(($ii % 10) == 0) {
        ini_set('fast_io.buffer_size', mt_rand(($align - 10 < 16 ? 16 : $align - 10), ($align + 10 < 32 ? 32 : $align + 10)));
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
            error_log_detailed(
                'file_pop_line',
                "iteration {$ii}, insert {$i}, mode {$mode}",
                'Pop content mismatch after insert',
                [
                    'mode' => $mode,
                    'align' => $align
                ],
                ['index' => $i]
            );
            break;
        }

        $insert_string[$i] = [
            'trim_line' => $trim_line,
            'trim_length' => strlen($trim_line),
            'line_offset' => $file_offset,
            'line_count' => $i,
        ];
    }
    if(!$file_pop_line_passed) break;

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
    if(!$file_pop_line_passed) break;

    if(filesize($db_file) != 0){
        $file_pop_line_passed = false;
        error_log_detailed(
            'file_pop_line',
            "iteration {$ii}",
            'File not empty after popping all lines',
            [
                'file_size' => filesize($db_file),
                'expected' => 0
            ],
            ['c' => $c]
        );
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
    if(!$file_pop_line_passed) break;

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
            trim($file_str_array[$row_num] ?? '') !== $line_arr['trim_line']
        ){
            $file_pop_line_passed = false;
            break;
        }
    }
    if(!$file_pop_line_passed) break;

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
            trim($file_str_array[$row_num] ?? '') !== $line_arr['trim_line']
        ){
            $file_pop_line_passed = false;
            break;
        }
    }
    if(!$file_pop_line_passed) break;

    if(filesize($db_file) == 0){
        $file_pop_line_passed = false;
        break;
    }

    unlink($db_file);
}



for($ii=16; $ii<=64; $ii++){
    debug_log("file_pop_line: buffer size test {$ii}");
    
    ini_set('fast_io.buffer_size', $ii);

    $str = '';
    for($i=1; $i<= 32; $i++){
        $str = str_pad('', $i, substr(strval($i), 0, 1));
        
        file_insert_line($db_file, $str);
        $file_last_str = file_pop_line($db_file);

        clearstatcache();
        
        if(
            $file_last_str !== $str ||
            filesize($db_file) !== 0
        ){       
            $file_pop_line_passed = false;
            error_log_detailed(
                'file_pop_line',
                "buffer size {$ii}, line {$i}",
                'Buffer size test failed',
                [
                    'string_length' => $i,
                    'file_size_after_pop' => filesize($db_file)
                ],
                ['buffer_size' => $ii]
            );
            break;
        }
    }
    
    if(!$file_pop_line_passed) break;

    unlink($db_file);
    $str = '';
    for($i=1; $i<= 32; $i++){
        $str = str_pad('', $i, substr(strval($i), 0, 1));
        
        file_insert_line($db_file, $str);
        $file_last_str = file_pop_line($db_file, -1, 2);
        
        if(
            $file_last_str !== $str
        ){       
            $file_pop_line_passed = false;
            break;
        }
    }
    
    if(!$file_pop_line_passed) break;
    unlink($db_file);
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
    debug_log("file_callback_line: iteration {$ii}/100");
    
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
            trim($callback_line_arr[$row_num][0] ?? '') !== $line_arr['trim_line'] ||
            $db_file != $callback_line_arr[$row_num][1] ||
            $line_arr['line_offset'] != $callback_line_arr[$row_num][2] ||
            $line_arr['line_length'] != $callback_line_arr[$row_num][3] ||
            $line_arr['line_count'] != $callback_line_arr[$row_num][4] ||
            0 != $callback_line_arr[$row_num][5] ||
            filesize($db_file) != $callback_line_arr[$row_num][7] 
        ){  
            $file_callback_line_passed = false;
            error_log_detailed(
                'file_callback_line',
                "iteration {$ii}, row {$row_num}",
                'Callback argument mismatch',
                [
                    'line_offset' => $callback_line_arr[$row_num][2] ?? 'null',
                    'expected_offset' => $line_arr['line_offset'],
                    'line_length' => $callback_line_arr[$row_num][3] ?? 'null',
                    'expected_length' => $line_arr['line_length']
                ],
                ['row' => $row_num]
            );
            break;
        }
    }
    if(!$file_callback_line_passed) break;


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
            trim($file_line_str ?? '') !== $line_arr['trim_line']
        ){  
            $file_callback_line_passed = false;
            break;
        }
    }
    if(!$file_callback_line_passed) break;
}

$time= microtime(true) - $start;

if($file_callback_line_passed) echo "\nCheck file_callback_line: time: ", $time, " - PASS",  "\n";
else echo "\nCheck file_callback_line - ERROR\n";

$end_io = get_process_io_stats();
foreach($end_io as $p=>$v)  echo $p, ': ', $v - $start_io[$p], ' (', mb_sec($time, $v - $start_io[$p], $p), ")\n";




// #########################
// Check file_update_line
$file_update_line_passed = true;
$start = microtime(true);
$start_io = get_process_io_stats();

for($ii = 0; $ii < 100; $ii++){
    debug_log("file_update_line: iteration {$ii}/100");
    
    if(file_exists($db_file)) unlink($db_file);
    $align = mt_rand(32, 65536);

    ini_set('fast_io.buffer_size', mt_rand(16, 65536));

    $c = mt_rand(10, 100);
    $insert_string = [];

    for($i = 0; $i <= $c; $i++){
        $shuffle = mt_rand(1, $align * 2);
        $str = 'index_' . $i . ' original_' . $i . ' ' . str_pad('', $shuffle, 'ABCDEFGHIJ');
        $file_offset = file_insert_line($db_file, $str, 2, $align);
        $trim_line = substr($str, 0, $align - 1);
        $insert_string[$i] = [
            'trim_line' => $trim_line,
            'line_offset' => $file_offset,
            'line_length' => $align,
        ];
    }

    foreach($insert_string as $row_num=>$line_arr){
        $shuffle2 = mt_rand(1, $align * 2);
        $new_str = 'index_' . $row_num . ' updated_' . $row_num . ' ' . str_pad('', $shuffle2, '0987654321');
        $new_trim = substr($new_str, 0, $align - 1);

        $written = file_update_line($db_file, $new_str, $line_arr['line_offset'], $align, 0);

        if($written !== $align){
            $file_update_line_passed = false;
            error_log_detailed(
                'file_update_line',
                "iteration {$ii}, update row {$row_num}, mode 0",
                'Bytes written mismatch',
                [
                    'written' => $written,
                    'expected' => $align
                ],
                ['row' => $row_num]
            );
            break;
        }

        $read_back = file_select_line($db_file, $line_arr['line_offset'], $align, 1);
        if($read_back === false || $read_back !== $new_trim){
            $file_update_line_passed = false;
            error_log_detailed(
                'file_update_line',
                "iteration {$ii}, read back row {$row_num}, mode 0",
                'Read back validation failed',
                [
                    'found' => $read_back !== false ? 'yes' : 'no'
                ],
                ['row' => $row_num]
            );
            break;
        }

        $written2 = file_update_line($db_file, $new_str, $line_arr['line_offset'], $align, 1);
        if($written2 !== $align){
            $file_update_line_passed = false;
            error_log_detailed(
                'file_update_line',
                "iteration {$ii}, update row {$row_num}, mode 1",
                'Mode 1 bytes written mismatch',
                [
                    'written' => $written2,
                    'expected' => $align
                ],
                ['row' => $row_num]
            );
            break;
        }

        $read_back2 = file_select_line($db_file, $line_arr['line_offset'], $align, 3);
        if($read_back2 === false || rtrim($read_back2, "\n") !== $new_trim){
            $file_update_line_passed = false;
            error_log_detailed(
                'file_update_line',
                "iteration {$ii}, read back row {$row_num}, mode 1",
                'Mode 1 read back validation failed',
                [
                    'found' => $read_back2 !== false ? 'yes' : 'no',
                    'returned_len' => strlen($read_back2 ?? ''),
                    'expected_len' => strlen($new_trim)
                ],
                ['row' => $row_num, 'mode' => 3]
            );
            break;
        }
    }
    if(!$file_update_line_passed) break;
}
$time = microtime(true) - $start;

if($file_update_line_passed) echo "\nCheck file_update_line: time: ", $time, " - PASS", "\n";
else echo "\nCheck file_update_line - ERROR\n";

$end_io = get_process_io_stats();
foreach($end_io as $p=>$v) echo $p, ': ', $v - $start_io[$p], ' (', mb_sec($time, $v - $start_io[$p], $p), ")\n";




// #########################
// Check file_update_array
$file_update_array_passed = true;
$start = microtime(true);
$start_io = get_process_io_stats();

for($ii = 0; $ii < 100; $ii++){
    debug_log("file_update_array: iteration {$ii}/100");
    
    if(file_exists($db_file)) unlink($db_file);
    $align = mt_rand(32, 65536);

    ini_set('fast_io.buffer_size', mt_rand(16, 65536));

    $c = mt_rand(10, 100);
    $insert_string = [];
    $query = [];
    $new_strings = [];

    for($i = 0; $i <= $c; $i++){
        $shuffle = mt_rand(1, $align * 2);
        $str = 'index_' . $i . ' original_' . $i . ' ' . str_pad('', $shuffle, 'ABCDEFGHIJ');
        $file_offset = file_insert_line($db_file, $str, 2, $align);

        $shuffle2 = mt_rand(1, $align * 2);
        $new_str = 'index_' . $i . ' updated_' . $i . ' ' . str_pad('', $shuffle2, '0987654321');
        $new_trim = substr($new_str, 0, $align - 1);

        $insert_string[$i] = [
            'line_offset' => $file_offset,
            'line_length' => $align,
        ];
        $new_strings[$i] = $new_trim;
        $query[] = [$new_str, $file_offset, $align];
    }

    $written = file_update_array($db_file, $query, 0);
    $expected_written = ($c + 1) * $align;

    if($written !== $expected_written){
        $file_update_array_passed = false;
        error_log_detailed(
            'file_update_array',
            "iteration {$ii} - mode 0 total write",
            'Total bytes written mismatch',
            [
                'written' => $written,
                'expected' => $expected_written
            ],
            ['c' => $c, 'align' => $align]
        );
        break;
    }

    foreach($insert_string as $row_num=>$line_arr){
        $read_back = file_select_line($db_file, $line_arr['line_offset'], $align, 1);
        if($read_back === false || $read_back !== $new_strings[$row_num]){
            $file_update_array_passed = false;
            error_log_detailed(
                'file_update_array',
                "iteration {$ii} - mode 0 verify row {$row_num}",
                'Read back mismatch',
                [
                    'found' => $read_back !== false ? 'yes' : 'no'
                ],
                ['row' => $row_num]
            );
            break;
        }
    }
    if(!$file_update_array_passed) break;

    $query2 = [];
    $new_strings2 = [];
    foreach($insert_string as $row_num=>$line_arr){
        $shuffle3 = mt_rand(1, $align * 2);
        $new_str2 = 'index_' . $row_num . ' mode1_' . $row_num . ' ' . str_pad('', $shuffle3, 'XYZ');
        $new_trim2 = substr($new_str2, 0, $align - 1);
        $new_strings2[$row_num] = $new_trim2;
        $query2[] = [$new_str2, $line_arr['line_offset'], $align];
    }

    file_update_array($db_file, $query2, 1);

    foreach($insert_string as $row_num=>$line_arr){
        $read_back2 = file_select_line($db_file, $line_arr['line_offset'], $align, 3);
        if($read_back2 === false || rtrim($read_back2, "\n") !== $new_strings2[$row_num]){
            $file_update_array_passed = false;
            error_log_detailed(
                'file_update_array',
                "iteration {$ii} - mode 1 verify row {$row_num}",
                'Mode 1 read back mismatch',
                [
                    'found' => $read_back2 !== false ? 'yes' : 'no',
                    'returned_len' => strlen($read_back2 ?? ''),
                    'expected_len' => strlen($new_strings2[$row_num])
                ],
                ['row' => $row_num, 'mode' => 3]
            );
            break;
        }
    }
    if(!$file_update_array_passed) break;
}
$time = microtime(true) - $start;

if($file_update_array_passed) echo "\nCheck file_update_array: time: ", $time, " - PASS", "\n";
else echo "\nCheck file_update_array - ERROR\n";

$end_io = get_process_io_stats();
foreach($end_io as $p=>$v) echo $p, ': ', $v - $start_io[$p], ' (', mb_sec($time, $v - $start_io[$p], $p), ")\n";




// #########################
// Check file_erase_line
$file_erase_line_passed = true;
$start = microtime(true);
$start_io = get_process_io_stats();

for($ii = 0; $ii < 100; $ii++){
    debug_log("file_erase_line: iteration {$ii}/100");
    
    if(file_exists($db_file)) unlink($db_file);
    $align = mt_rand(32, 65536);

    ini_set('fast_io.buffer_size', mt_rand(16, 65536));

    $c = mt_rand(10, 100);
    $insert_string = [];

    for($i = 0; $i <= $c; $i++){
        $shuffle = mt_rand(1, $align * 2);
        $str = 'index_' . $i . ' value_' . $i . ' ' . str_pad('', $shuffle, 'ABCDEFGHIJ');
        $file_offset = file_insert_line($db_file, $str, 2, $align);
        $trim_line = substr($str, 0, $align - 1);
        $insert_string[$i] = [
            'key' => 'index_' . $i . ' ',
            'trim_line' => $trim_line,
            'line_offset' => $file_offset,
            'line_length' => $align,
        ];
    }

    foreach($insert_string as $row_num=>$line_arr){
        if($row_num % 2 !== 0) continue;

        $before = file_search_line($db_file, $line_arr['key'], 0, 0);
        if($before === false){
            $file_erase_line_passed = false;
            error_log_detailed(
                'file_erase_line',
                "iteration {$ii}, row {$row_num} - pre-erase check",
                'Line not found before erase',
                [
                    'row_num' => $row_num
                ],
                ['c' => $c]
            );
            break;
        }

        $erase_result = file_erase_line($db_file, $line_arr['key'], 0, 0);
        if($erase_result < 0){
            $file_erase_line_passed = false;
            error_log_detailed(
                'file_erase_line',
                "iteration {$ii}, row {$row_num} - erase",
                'Erase returned error',
                [
                    'erase_result' => $erase_result
                ],
                ['row' => $row_num]
            );
            break;
        }

        if($erase_result !== $line_arr['line_offset']){
            $file_erase_line_passed = false;
            error_log_detailed(
                'file_erase_line',
                "iteration {$ii}, row {$row_num} - erase offset",
                'Erase offset mismatch',
                [
                    'returned_offset' => $erase_result,
                    'expected_offset' => $line_arr['line_offset']
                ],
                ['row' => $row_num]
            );
            break;
        }

        clearstatcache();
        $expected_size = ($c + 1) * $align;
        if(filesize($db_file) !== $expected_size){
            $file_erase_line_passed = false;
            error_log_detailed(
                'file_erase_line',
                "iteration {$ii}, row {$row_num} - file size",
                'File size changed after erase',
                [
                    'current_size' => filesize($db_file),
                    'expected_size' => $expected_size
                ],
                ['row' => $row_num]
            );
            break;
        }
    }
    if(!$file_erase_line_passed) break;

    foreach($insert_string as $row_num=>$line_arr){
        $result = file_search_line($db_file, $line_arr['key'], 0, 0);
        if($row_num % 2 === 0){
            if($result !== false && $result !== null && $result !== ''){
                if(trim($result) === $line_arr['trim_line']){
                    $file_erase_line_passed = false;
                    error_log_detailed(
                        'file_erase_line',
                        "iteration {$ii}, row {$row_num} - erased line found",
                        'Erased line still has content',
                        [
                            'row_num' => $row_num
                        ],
                        ['c' => $c]
                    );
                    break;
                }
            }
        } else {
            if($result === false){
                $file_erase_line_passed = false;
                error_log_detailed(
                    'file_erase_line',
                    "iteration {$ii}, row {$row_num} - non-erased not found",
                    'Non-erased line not found',
                    [
                        'row_num' => $row_num
                    ],
                    ['c' => $c]
                );
                break;
            }
            if(trim($result) !== $line_arr['trim_line']){
                $file_erase_line_passed = false;
                error_log_detailed(
                    'file_erase_line',
                    "iteration {$ii}, row {$row_num} - non-erased content",
                    'Non-erased line content mismatch',
                    [
                        'row_num' => $row_num
                    ],
                    ['c' => $c]
                );
                break;
            }
        }
    }
    if(!$file_erase_line_passed) break;

    $target = $insert_string[1];
    $erase_result2 = file_erase_line($db_file, $target['key'], $target['line_offset'], 0);
    if($erase_result2 !== $target['line_offset']){
        $file_erase_line_passed = false;
        error_log_detailed(
            'file_erase_line',
            "iteration {$ii} - second erase",
            'Second erase offset mismatch',
            [
                'returned' => $erase_result2,
                'expected' => $target['line_offset']
            ],
            ['c' => $c]
        );
        break;
    }
}
$time = microtime(true) - $start;

if($file_erase_line_passed) echo "\nCheck file_erase_line: time: ", $time, " - PASS", "\n";
else echo "\nCheck file_erase_line - ERROR\n";

$end_io = get_process_io_stats();
foreach($end_io as $p=>$v) echo $p, ': ', $v - $start_io[$p], ' (', mb_sec($time, $v - $start_io[$p], $p), ")\n";




// #########################
// Check file_replace_line
$file_replace_line_passed = true;
$start = microtime(true);
$start_io = get_process_io_stats();

for($ii = 0; $ii < 100; $ii++){
    debug_log("file_replace_line: iteration {$ii}/100");
    
    if(file_exists($db_file)) unlink($db_file);
    $align = mt_rand(64, 4096);

    ini_set('fast_io.buffer_size', mt_rand(16, 65536));

    $c = mt_rand(5, 30);
    $insert_string = [];

    for($i = 0; $i <= $c; $i++){
        $shuffle = mt_rand(1, $align);
        $str = 'rkey_' . $i . ' original_val_' . $i . ' ' . str_pad('', $shuffle, 'ABC');
        file_insert_line($db_file, $str);
        $insert_string[$i] = [
            'key' => 'rkey_' . $i . ' ',
            'original' => $str,
        ];
    }

    $replace_map = [];
    foreach($insert_string as $row_num=>$line_arr){
        if($row_num % 2 === 0){
            $new_val = 'rkey_' . $row_num . ' replaced_val_' . $row_num . ' ' . str_pad('', mt_rand(1, 20), 'XYZ');
            $replace_map[$row_num] = $new_val;
            $result = file_replace_line($db_file, $line_arr['key'], $new_val, 0);
            if($result < 0){
                $file_replace_line_passed = false;
                error_log_detailed(
                    'file_replace_line',
                    "iteration {$ii}, replace row {$row_num}",
                    'Replace returned error',
                    [
                        'result' => $result
                    ],
                    ['row' => $row_num]
                );
                break;
            }
            if($result !== $c + 1){
                $file_replace_line_passed = false;
                error_log_detailed(
                    'file_replace_line',
                    "iteration {$ii}, replace row {$row_num}",
                    'Replace count mismatch',
                    [
                        'returned_count' => $result,
                        'expected_count' => $c + 1
                    ],
                    ['row' => $row_num]
                );
                break;
            }
        }
    }
    if(!$file_replace_line_passed) break;

    foreach($insert_string as $row_num=>$line_arr){
        $found = file_search_line($db_file, $line_arr['key'], 0, 0);
        if(isset($replace_map[$row_num])){
            if($found === false || trim($found) !== $replace_map[$row_num]){
                $file_replace_line_passed = false;
                error_log_detailed(
                    'file_replace_line',
                    "iteration {$ii}, verify replaced row {$row_num}",
                    'Replaced value not found',
                    [
                        'found' => $found !== false ? 'yes' : 'no'
                    ],
                    ['row' => $row_num]
                );
                break;
            }
        } else {
            if($found === false || trim($found) !== $line_arr['original']){
                $file_replace_line_passed = false;
                error_log_detailed(
                    'file_replace_line',
                    "iteration {$ii}, verify original row {$row_num}",
                    'Original value not preserved',
                    [
                        'found' => $found !== false ? 'yes' : 'no'
                    ],
                    ['row' => $row_num]
                );
                break;
            }
        }
    }
    if(!$file_replace_line_passed) break;

    if(file_exists($db_file)) unlink($db_file);
    for($i = 0; $i <= $c; $i++){
        $str = 'rkey_' . $i . ' original_val_' . $i;
        file_insert_line($db_file, $str);
    }

    $target_key = 'rkey_' . intval($c / 2) . ' ';
    $new_val_m1 = 'rkey_' . intval($c / 2) . ' mode1_replaced';
    $result_m1 = file_replace_line($db_file, $target_key, $new_val_m1, 1);
    if($result_m1 < 0){
        $file_replace_line_passed = false;
        error_log_detailed(
            'file_replace_line',
            "iteration {$ii} - mode 1",
            'Mode 1 replace error',
            [
                'result' => $result_m1
            ],
            ['c' => $c]
        );
        break;
    }

    $found_m1 = file_search_line($db_file, $target_key, 0, 0);
    if($found_m1 === false || trim($found_m1) !== $new_val_m1){
        $file_replace_line_passed = false;
        error_log_detailed(
            'file_replace_line',
            "iteration {$ii} - mode 1 verify",
            'Mode 1 value mismatch',
            [
                'found' => $found_m1 !== false ? 'yes' : 'no'
            ],
            ['c' => $c]
        );
        break;
    }
}
$time = microtime(true) - $start;

if($file_replace_line_passed) echo "\nCheck file_replace_line: time: ", $time, " - PASS", "\n";
else echo "\nCheck file_replace_line - ERROR\n";

$end_io = get_process_io_stats();
foreach($end_io as $p=>$v) echo $p, ': ', $v - $start_io[$p], ' (', mb_sec($time, $v - $start_io[$p], $p), ")\n";




// #########################
// Check file_defrag_lines
$file_defrag_lines_passed = true;
$start = microtime(true);
$start_io = get_process_io_stats();

for($ii = 0; $ii < 100; $ii++){
    debug_log("file_defrag_lines: iteration {$ii}/100");
    
    if(file_exists($db_file)) unlink($db_file);
    $align = mt_rand(64, 4096);

    ini_set('fast_io.buffer_size', mt_rand(16, 65536));

    $c = mt_rand(5, 30);
    $insert_string = [];

    for($i = 0; $i <= $c; $i++){
        $str = 'dkey_' . $i . ' value_' . $i . ' ' . str_pad('', mt_rand(1, 30), 'DATA');
        file_insert_line($db_file, $str);
        $insert_string[$i] = [
            'key' => 'dkey_' . $i . ' ',
            'value' => $str,
        ];
    }

    $erased_keys = [];
    $remaining_keys = [];
    foreach($insert_string as $row_num=>$line_arr){
        if($row_num % 2 === 0){
            $erase_result = file_erase_line($db_file, $line_arr['key'], 0, 0);
            if($erase_result < 0){
                $file_defrag_lines_passed = false;
                error_log_detailed(
                    'file_defrag_lines',
                    "iteration {$ii}, erase row {$row_num}",
                    'Erase before defrag failed',
                    [
                        'result' => $erase_result
                    ],
                    ['row' => $row_num]
                );
                break;
            }
            $erased_keys[] = $line_arr['key'];
        } else {
            $remaining_keys[] = $line_arr;
        }
    }
    if(!$file_defrag_lines_passed) break;

    $size_before_defrag = filesize($db_file);

    // Всегда передавайте оба параметра явно
    $deleted_count = file_defrag_lines($db_file, '', 0);
    if($deleted_count < 0){
        $file_defrag_lines_passed = false;
        error_log_detailed(
            'file_defrag_lines',
            "iteration {$ii}",
            'Defrag returned error',
            [
                'result' => $deleted_count
            ],
            ['c' => $c]
        );
        break;
    }

    $expected_erased = ceil(($c + 1) / 2);
    if($deleted_count !== $expected_erased){
        $file_defrag_lines_passed = false;
        error_log_detailed(
            'file_defrag_lines',
            "iteration {$ii}",
            'Deleted count mismatch',
            [
                'deleted_count' => $deleted_count,
                'expected_count' => $expected_erased
            ],
            ['c' => $c, 'total_lines' => $c + 1]
        );
        break;
    }

    clearstatcache();
    $size_after_defrag = filesize($db_file);

    if($size_after_defrag >= $size_before_defrag){
        $file_defrag_lines_passed = false;
        error_log_detailed(
            'file_defrag_lines',
            "iteration {$ii}",
            'File size not reduced',
            [
                'size_before' => $size_before_defrag,
                'size_after' => $size_after_defrag
            ],
            ['c' => $c]
        );
        break;
    }

    foreach($remaining_keys as $line_arr){
        $found = file_search_line($db_file, $line_arr['key'], 0, 0);
        if($found === false || trim($found) !== $line_arr['value']){
            $file_defrag_lines_passed = false;
            error_log_detailed(
                'file_defrag_lines',
                "iteration {$ii}, verify remaining",
                'Remaining line not found',
                [
                    'key' => $line_arr['key'],
                    'found' => $found !== false ? 'yes' : 'no'
                ],
                ['c' => $c]
            );
            break;
        }
    }
    if(!$file_defrag_lines_passed) break;

    if(file_exists($db_file)) unlink($db_file);
    for($i = 0; $i <= $c; $i++){
        $str = 'dkey_' . $i . ' value_' . $i;
        file_insert_line($db_file, $str);
    }

    file_erase_line($db_file, 'dkey_0 ', 0, 0);

    $deleted_m1 = file_defrag_lines($db_file, '', 1);
    if($deleted_m1 < 0){
        $file_defrag_lines_passed = false;
        error_log_detailed(
            'file_defrag_lines',
            "iteration {$ii} - mode 1",
            'Mode 1 defrag error',
            [
                'result' => $deleted_m1
            ],
            ['c' => $c]
        );
        break;
    }

    $found_m1 = file_search_line($db_file, 'dkey_1 ', 0, 0);
    if($found_m1 === false){
        $file_defrag_lines_passed = false;
        error_log_detailed(
            'file_defrag_lines',
            "iteration {$ii} - mode 1 verify",
            'Mode 1 verify failed',
            [
                'found' => 'no'
            ],
            ['c' => $c]
        );
        break;
    }
}
$time = microtime(true) - $start;

if($file_defrag_lines_passed) echo "\nCheck file_defrag_lines: time: ", $time, " - PASS", "\n";
else echo "\nCheck file_defrag_lines - ERROR\n";

$end_io = get_process_io_stats();
foreach($end_io as $p=>$v) echo $p, ': ', $v - $start_io[$p], ' (', mb_sec($time, $v - $start_io[$p], $p), ")\n";




// #########################
// Check file_push_data and file_search_data
$file_push_data_passed = true;
$start = microtime(true);
$start_io = get_process_io_stats();

for($ii = 0; $ii < 100; $ii++){
    debug_log("file_push_data + file_search_data: iteration {$ii}/100");
    
    if(file_exists($db_data_file)) unlink($db_data_file);
    if(file_exists($db_data_file . '.index')) unlink($db_data_file . '.index');

    ini_set('fast_io.buffer_size', mt_rand(16, 65536));

    $c = mt_rand(5, 30);
    $push_data = [];

    for($i = 0; $i <= $c; $i++){
        $key = 'datakey_' . $i;
        $value_size = mt_rand(1, 1024);
        $value = str_pad('', $value_size, 'Binary_Data_' . $i . '_');

        $offset = file_push_data($db_data_file, $key, $value, 0);
        if($offset < 0){
            $file_push_data_passed = false;
            error_log_detailed(
                'file_push_data',
                "iteration {$ii}, push row {$i}",
                'Push returned error',
                [
                    'result' => $offset
                ],
                ['index' => $i]
            );
            break;
        }

        $push_data[$i] = [
            'key' => $key,
            'value' => $value,
            'offset' => $offset,
        ];
    }
    if(!$file_push_data_passed) break;

    foreach($push_data as $row_num=>$data){
        $found = file_search_data($db_data_file, $data['key'], 0, 0);
        if($found === false || $found !== $data['value']){
            $file_push_data_passed = false;
            error_log_detailed(
                'file_push_data',
                "iteration {$ii}, search row {$row_num}",
                'Search returned mismatched value',
                [
                    'key' => $data['key'],
                    'found' => $found !== false ? 'yes' : 'no'
                ],
                ['row' => $row_num]
            );
            break;
        }
    }
    if(!$file_push_data_passed) break;

    $not_found = file_search_data($db_data_file, 'nonexistent_key_xyz', 0, 0);
    if($not_found !== false){
        $file_push_data_passed = false;
        error_log_detailed(
            'file_push_data',
            "iteration {$ii}",
            'Found nonexistent key',
            [
                'key' => 'nonexistent_key_xyz',
                'should_be' => false
            ],
            ['c' => $c]
        );
        break;
    }

    if(count($push_data) > 1){
        $second = $push_data[1];
        $found2 = file_search_data($db_data_file, $second['key'], 0, 0);
        if($found2 === false || $found2 !== $second['value']){
            $file_push_data_passed = false;
            error_log_detailed(
                'file_push_data',
                "iteration {$ii}, search second",
                'Second item search failed',
                [
                    'found' => $found2 !== false ? 'yes' : 'no'
                ],
                ['c' => $c]
            );
            break;
        }
    }
}

if(file_exists($db_data_file)) unlink($db_data_file);
if(file_exists($db_data_file . '.index')) unlink($db_data_file . '.index');

$time = microtime(true) - $start;

if($file_push_data_passed) echo "\nCheck file_push_data + file_search_data: time: ", $time, " - PASS", "\n";
else echo "\nCheck file_push_data + file_search_data - ERROR\n";

$end_io = get_process_io_stats();
foreach($end_io as $p=>$v) echo $p, ': ', $v - $start_io[$p], ' (', mb_sec($time, $v - $start_io[$p], $p), ")\n";




// #########################
// Check file_defrag_data
$file_defrag_data_passed = true;
$start = microtime(true);
$start_io = get_process_io_stats();

for($ii = 0; $ii < 100; $ii++){
    debug_log("file_defrag_data: iteration {$ii}/100");
    
    if(file_exists($db_data_file)) unlink($db_data_file);
    if(file_exists($db_data_file . '.index')) unlink($db_data_file . '.index');

    ini_set('fast_io.buffer_size', mt_rand(16, 65536));

    $c = mt_rand(5, 20);
    $push_data = [];

    for($i = 0; $i <= $c; $i++){
        $key = 'fddkey_' . $i;
        $value_size = mt_rand(10, 256);
        $value = str_pad('', $value_size, 'Data_' . $i . '_');

        $offset = file_push_data($db_data_file, $key, $value, 0);
        if($offset < 0){
            $file_defrag_data_passed = false;
            error_log_detailed(
                'file_defrag_data',
                "iteration {$ii}, push row {$i}",
                'Push error',
                [
                    'result' => $offset
                ],
                ['index' => $i]
            );
            break;
        }
        $push_data[$i] = ['key' => $key, 'value' => $value];
    }
    if(!$file_defrag_data_passed) break;

    $index_file = $db_data_file . '.index';
    $erased_keys = [];
    $kept_keys = [];

    foreach($push_data as $row_num=>$data){
        if($row_num % 2 === 0){
            file_erase_line($index_file, $data['key'] . ' ', 0, 0);
            $erased_keys[] = $data['key'];
        } else {
            $kept_keys[] = $data;
        }
    }

    $size_before = filesize($db_data_file);

    $defrag_result = file_defrag_data($db_data_file);
    if($defrag_result < 0){
        $file_defrag_data_passed = false;
        error_log_detailed(
            'file_defrag_data',
            "iteration {$ii}",
            'Defrag error',
            [
                'result' => $defrag_result
            ],
            ['c' => $c]
        );
        break;
    }

    clearstatcache();
    $size_after = filesize($db_data_file);

    if($size_after > $size_before){
        $file_defrag_data_passed = false;
        error_log_detailed(
            'file_defrag_data',
            "iteration {$ii}",
            'Size increased after defrag',
            [
                'size_before' => $size_before,
                'size_after' => $size_after
            ],
            ['c' => $c]
        );
        break;
    }

    // Проверка должна быть строже - сравнивать значения
    foreach($kept_keys as $data){
        $found = file_search_data($db_data_file, $data['key'], 0, 0);
        if($found === false){
            $file_defrag_data_passed = false;
            error_log_detailed(
                'file_defrag_data',
                "iteration {$ii}, verify kept key",
                'Kept key not found',
                [
                    'key' => $data['key'],
                    'found' => 'no'
                ],
                ['c' => $c]
            );
            break;
        }
        // Дополнительная проверка значения
        if($found !== $data['value']){
            $file_defrag_data_passed = false;
            error_log_detailed(
                'file_defrag_data',
                "iteration {$ii}, verify kept key value",
                'Kept key value mismatch',
                [
                    'key' => $data['key'],
                    'expected_value_len' => strlen($data['value']),
                    'found_value_len' => strlen($found)
                ],
                ['c' => $c]
            );
            break;
        }
    }
    if(!$file_defrag_data_passed) break;

    if(file_exists($db_data_file)) unlink($db_data_file);
    if(file_exists($db_data_file . '.index')) unlink($db_data_file . '.index');

    for($i = 0; $i <= 5; $i++){
        file_push_data($db_data_file, 'mkey_' . $i, 'value_data_' . $i, 0);
    }

    file_erase_line($db_data_file . '.index', 'mkey_0 ', 0, 0);

    $defrag_m1 = file_defrag_data($db_data_file, '', 1);
    if($defrag_m1 < 0){
        $file_defrag_data_passed = false;
        error_log_detailed(
            'file_defrag_data',
            "iteration {$ii} - mode 1",
            'Mode 1 defrag error',
            [
                'result' => $defrag_m1
            ]
        );
        break;
    }

    $found_m1 = file_search_data($db_data_file, 'mkey_1', 0, 0);
    if($found_m1 === false || $found_m1 !== 'value_data_1'){
        $file_defrag_data_passed = false;
        error_log_detailed(
            'file_defrag_data',
            "iteration {$ii} - mode 1 verify",
            'Mode 1 verify failed',
            [
                'found' => $found_m1 !== false ? 'yes' : 'no'
            ]
        );
        break;
    }
}

if(file_exists($db_data_file)) unlink($db_data_file);
if(file_exists($db_data_file . '.index')) unlink($db_data_file . '.index');

$time = microtime(true) - $start;

if($file_defrag_data_passed) echo "\nCheck file_defrag_data: time: ", $time, " - PASS", "\n";
else echo "\nCheck file_defrag_data - ERROR\n";

$end_io = get_process_io_stats();
foreach($end_io as $p=>$v) echo $p, ': ', $v - $start_io[$p], ' (', mb_sec($time, $v - $start_io[$p], $p), ")\n";




// #########################
// Check replicate_file
$file_replicate_passed = true;
$start = microtime(true);
$start_io = get_process_io_stats();

for($ii = 0; $ii < 100; $ii++){
    debug_log("replicate_file: iteration {$ii}/100");
    
    if(file_exists($db_file)) unlink($db_file);
    if(file_exists($db_replica_file)) unlink($db_replica_file);

    ini_set('fast_io.buffer_size', mt_rand(16, 65536));

    $c = mt_rand(5, 50);
    $align = mt_rand(32, 4096);
    $insert_string = [];

    for($i = 0; $i <= $c; $i++){
        $str = 'repkey_' . $i . ' value_' . $i . ' ' . str_pad('', mt_rand(1, $align), 'REPL');
        file_insert_line($db_file, $str, 2, $align);
        $insert_string[] = substr($str, 0, $align - 1);
    }

    $bytes_copied = replicate_file($db_file, $db_replica_file, 0);

    if($bytes_copied < 0){
        $file_replicate_passed = false;
        error_log_detailed(
            'replicate_file',
            "iteration {$ii}",
            'Replicate returned error',
            [
                'result' => $bytes_copied
            ],
            ['c' => $c, 'align' => $align]
        );
        break;
    }

    clearstatcache();
    $src_size = filesize($db_file);
    $dst_size = filesize($db_replica_file);

    if($bytes_copied !== $src_size || $src_size !== $dst_size){
        $file_replicate_passed = false;
        error_log_detailed(
            'replicate_file',
            "iteration {$ii}",
            'Size mismatch',
            [
                'bytes_copied' => $bytes_copied,
                'src_size' => $src_size,
                'dst_size' => $dst_size
            ],
            ['c' => $c]
        );
        break;
    }

    foreach($insert_string as $row_num=>$trim_line){
        $found_original = file_select_line($db_file, $row_num, $align, 0);
        $found_replica  = file_select_line($db_replica_file, $row_num, $align, 0);

        if(
            $found_original === false ||
            $found_replica === false ||
            $found_original !== $found_replica ||
            $found_original !== $trim_line
        ){
            $file_replicate_passed = false;
            error_log_detailed(
                'replicate_file',
                "iteration {$ii}, verify row {$row_num}",
                'Content mismatch',
                [
                    'original_found' => $found_original !== false ? 'yes' : 'no',
                    'replica_found' => $found_replica !== false ? 'yes' : 'no'
                ],
                ['row' => $row_num, 'c' => $c]
            );
            break;
        }
    }
    if(!$file_replicate_passed) break;

    if(file_exists($db_data_file)) unlink($db_data_file);
    if(file_exists($db_data_file . '.index')) unlink($db_data_file . '.index');
    if(file_exists($db_replica_file)) unlink($db_replica_file);
    if(file_exists($db_replica_file . '.index')) unlink($db_replica_file . '.index');

    for($i = 0; $i <= 5; $i++){
        file_push_data($db_data_file, 'repdata_' . $i, 'replication_value_' . $i, 0);
    }

    $bytes_m1 = replicate_file($db_data_file, $db_replica_file, 1);
    if($bytes_m1 < 0){
        $file_replicate_passed = false;
        error_log_detailed(
            'replicate_file',
            "iteration {$ii} - mode 1",
            'Mode 1 replicate error',
            [
                'result' => $bytes_m1
            ],
            ['c' => $c]
        );
        break;
    }

    if(!file_exists($db_replica_file) || !file_exists($db_replica_file . '.index')){
        $file_replicate_passed = false;
        error_log_detailed(
            'replicate_file',
            "iteration {$ii} - mode 1 files",
            'Replica files not created',
            [
                'data_exists' => file_exists($db_replica_file) ? 'yes' : 'no',
                'index_exists' => file_exists($db_replica_file . '.index') ? 'yes' : 'no'
            ],
            ['c' => $c]
        );
        break;
    }

    $found_replica = file_search_data($db_replica_file, 'repdata_1', 0, 0);
    if($found_replica === false || $found_replica !== 'replication_value_1'){
        $file_replicate_passed = false;
        error_log_detailed(
            'replicate_file',
            "iteration {$ii} - mode 1 verify",
            'Replica data mismatch',
            [
                'found' => $found_replica !== false ? 'yes' : 'no'
            ],
            ['c' => $c]
        );
        break;
    }
}

if(file_exists($db_data_file)) unlink($db_data_file);
if(file_exists($db_data_file . '.index')) unlink($db_data_file . '.index');
if(file_exists($db_replica_file)) unlink($db_replica_file);
if(file_exists($db_replica_file . '.index')) unlink($db_replica_file . '.index');

$time = microtime(true) - $start;

if($file_replicate_passed) echo "\nCheck replicate_file: time: ", $time, " - PASS", "\n";
else echo "\nCheck replicate_file - ERROR\n";

$end_io = get_process_io_stats();
foreach($end_io as $p=>$v) echo $p, ': ', $v - $start_io[$p], ' (', mb_sec($time, $v - $start_io[$p], $p), ")\n";




// #########################
// Check find_matches_pcre2
$find_matches_pcre2_passed = true;
$start = microtime(true);
$start_io = get_process_io_stats();

for($ii = 0; $ii < 100; $ii++){
    debug_log("find_matches_pcre2: iteration {$ii}/100");
    
    $subject = 'index_42 file_insert_line_42 1234567890';
    $pattern = '\\w+_\\d+';

    $matches = find_matches_pcre2($pattern, $subject, 0);

    if(
        !is_array($matches) ||
        count($matches) < 2 ||
        $matches[0] !== 'index_42' ||
        $matches[1] !== 'file_insert_line_42'
    ){
        $find_matches_pcre2_passed = false;
        error_log_detailed(
            'find_matches_pcre2',
            "iteration {$ii} - mode 0",
            'Mode 0 match failed',
            [
                'is_array' => is_array($matches) ? 'yes' : 'no',
                'count' => count($matches ?? []),
                'match_0' => $matches[0] ?? 'null',
                'match_1' => $matches[1] ?? 'null'
            ]
        );
        break;
    }

    $matches1 = find_matches_pcre2($pattern, $subject, 1);

    if(
        !is_array($matches1) ||
        count($matches1) < 2 ||
        $matches1[0]['line_match'] !== 'index_42' ||
        $matches1[0]['match_offset'] !== 0 ||
        $matches1[0]['match_length'] !== strlen('index_42') ||
        $matches1[1]['line_match'] !== 'file_insert_line_42' ||
        $matches1[1]['match_offset'] !== strpos($subject, 'file_insert_line_42') ||
        $matches1[1]['match_length'] !== strlen('file_insert_line_42')
    ){
        $find_matches_pcre2_passed = false;
        error_log_detailed(
            'find_matches_pcre2',
            "iteration {$ii} - mode 1",
            'Mode 1 detailed match failed',
            [
                'match_0' => $matches1[0]['line_match'] ?? 'null',
                'match_1' => $matches1[1]['line_match'] ?? 'null'
            ]
        );
        break;
    }

    $no_match = find_matches_pcre2('ZZZNOMATCH\\d{20}', $subject, 0);
    if(!is_array($no_match) || count($no_match) !== 0){
        $find_matches_pcre2_passed = false;
        error_log_detailed(
            'find_matches_pcre2',
            "iteration {$ii} - no match test",
            'No match should return empty array',
            [
                'is_array' => is_array($no_match) ? 'yes' : 'no',
                'count' => count($no_match ?? [])
            ]
        );
        break;
    }

    $shuffle = mt_rand(1, 100);
    $random_subject = 'key_' . $shuffle . ' val_' . $shuffle . ' extra_' . mt_rand(1, 999);
    $random_matches = find_matches_pcre2('\\w+_\\d+', $random_subject, 0);

    if(
        !is_array($random_matches) ||
        $random_matches[0] !== 'key_' . $shuffle ||
        $random_matches[1] !== 'val_' . $shuffle
    ){
        $find_matches_pcre2_passed = false;
        error_log_detailed(
            'find_matches_pcre2',
            "iteration {$ii} - random subject",
            'Random subject match failed',
            [
                'match_0' => $random_matches[0] ?? 'null',
                'expected_0' => 'key_' . $shuffle,
                'match_1' => $random_matches[1] ?? 'null',
                'expected_1' => 'val_' . $shuffle
            ],
            ['shuffle' => $shuffle]
        );
        break;
    }

    $random_matches1 = find_matches_pcre2('\\w+_\\d+', $random_subject, 1);
    if(
        !is_array($random_matches1) ||
        $random_matches1[0]['line_match'] !== 'key_' . $shuffle ||
        $random_matches1[0]['match_offset'] !== 0 ||
        $random_matches1[0]['match_length'] !== strlen('key_' . $shuffle)
    ){
        $find_matches_pcre2_passed = false;
        error_log_detailed(
            'find_matches_pcre2',
            "iteration {$ii} - random detailed",
            'Random detailed match failed',
            [
                'line_match' => $random_matches1[0]['line_match'] ?? 'null',
                'match_offset' => $random_matches1[0]['match_offset'] ?? 'null',
                'match_length' => $random_matches1[0]['match_length'] ?? 'null'
            ],
            ['shuffle' => $shuffle]
        );
        break;
    }

    $num_subject = 'price: 123.45 qty: 67 total: 890';
    $num_matches = find_matches_pcre2('\\d+', $num_subject, 0);
    if(
        !is_array($num_matches) ||
        $num_matches[0] !== '123' ||
        $num_matches[1] !== '45'
    ){
        $find_matches_pcre2_passed = false;
        error_log_detailed(
            'find_matches_pcre2',
            "iteration {$ii} - numeric",
            'Numeric match failed',
            [
                'match_0' => $num_matches[0] ?? 'null',
                'match_1' => $num_matches[1] ?? 'null'
            ]
        );
        break;
    }
}
$time = microtime(true) - $start;

if($find_matches_pcre2_passed) echo "\nCheck find_matches_pcre2: time: ", $time, " - PASS", "\n";
else echo "\nCheck find_matches_pcre2 - ERROR\n";

$end_io = get_process_io_stats();
foreach($end_io as $p=>$v) echo $p, ': ', $v - $start_io[$p], ' (', mb_sec($time, $v - $start_io[$p], $p), ")\n";


// Cleanup
if(file_exists($db_file)) unlink($db_file);
if(file_exists($db_data_file)) unlink($db_data_file);
if(file_exists($db_data_file . '.index')) unlink($db_data_file . '.index');
if(file_exists($db_replica_file)) unlink($db_replica_file);
if(file_exists($db_replica_file . '.index')) unlink($db_replica_file . '.index');

echo "\n✅ All tests completed!\n";
