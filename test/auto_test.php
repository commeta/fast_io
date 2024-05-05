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

ini_set('error_reporting', E_ALL);
ini_set('display_errors', 1);
ini_set('html_errors', 1);
error_reporting(E_ALL);


$db_file = __DIR__ . '/fast_io.dat';


// #########################
// Check file_insert_line
$file_insert_line_passed = true;

for($ii = 0; $ii < 100; $ii++){
    if(file_exists($db_file)) unlink($db_file);
    $align = mt_rand(16, 65534);

    ini_set('fast_io.buffer_size', mt_rand(16, 65534));

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
            mb_strlen($file_line[0]['line']) + 1 !== $align
        ){
            $file_insert_line_passed = false;
            break;
        }

        $file_str = $file_line[0]['line'];
        $file_array = explode(' ', $file_str);
        $str_array = explode(' ', $str);

        if(
            empty($file_array[0]) || 
            $file_array[0] !== 'index_' . $i ||
            $file_array[1] !== 'file_insert_line_' . $i ||
            trim($file_str) !== mb_substr($str, 0, $align - 1)
            
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


if($file_insert_line_passed) echo "Check file_insert_line alignment data - PASS\n";
else echo "Check file_insert_line alignment data - ERROR\n";



// #########################
// Check file_analize
$file_analize_passed = true;

for($ii = 0; $ii < 100; $ii++){
    if(file_exists($db_file)) unlink($db_file);
    $align = mt_rand(16, 65534);

    ini_set('fast_io.buffer_size', mt_rand(4096, 65534));

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


if($file_analize_passed) echo "Check file_analize - PASS\n";
else echo "Check file_analize - ERROR\n";



// #########################
// Check file_get_keys
$file_get_keys_passed = true;

for($ii = 0; $ii < 100; $ii++){
    if(file_exists($db_file)) unlink($db_file);
    $align = mt_rand(16, 65534);

    ini_set('fast_io.buffer_size', mt_rand(4096, 65534));

    $last_offset = 0;

    $c = mt_rand(10, 100);

    $insert_string = [];
    
    for($i=0; $i <= $c; $i++){
        $shuffle = mt_rand(1, $align * 2);

        $str = 'index_' . $i . ' file_insert_line_' . $i . ' ' . str_pad('', $shuffle, '1234567890');
        $file_offset = file_insert_line($db_file, $str, 2, $align);

        $trim_line = mb_substr($str, 0, $align - 1);
        $insert_string[$i] = [
            'trim_line' => $trim_line,
            'line_offset' => $file_offset,
            'line_length' => $align,
            'line_count' => $i
        ];

        if($file_offset == $last_offset) $last_offset += $align;  
    }


    foreach($insert_string as $row_num=>$line_arr){
        $file_array = file_get_keys($db_file, 0, $c + 1, 0, 1);

        if(
            $file_array[$row_num]['line_count'] - 1 !== $line_arr['line_count'] ||
            $file_array[$row_num]['line_offset'] !== $line_arr['line_offset'] ||
            $file_array[$row_num]['line_length'] !== $line_arr['line_length'] ||
            trim($file_array[$row_num]['line']) !== $line_arr['trim_line']
        ){
            $file_get_keys_passed = false;
            break;
        }
    }

    $file_array = file_get_keys($db_file, 0, $c + 1, 0, 0);
    foreach($file_array as $row_num=>$line_arr){
        $str_array = explode(' ', $insert_string[$row_num]['trim_line']);

        if(
            $line_arr['line_count'] - 1 !== $insert_string[$row_num]['line_count'] ||
            $line_arr['line_length'] !== $insert_string[$row_num]['line_length'] ||
            $line_arr['line_offset'] !== $insert_string[$row_num]['line_offset'] ||
            $line_arr['key'] !== $str_array[0]
        ){
            $file_get_keys_passed = false;
            break;
        }
    }

    $file_array = file_get_keys($db_file, 0, $c + 1, 0, 2);
    foreach($file_array as $row_num=>$line_arr){
        if(
            $line_arr['line_count'] - 1 !== $insert_string[$row_num]['line_count'] ||
            $line_arr['line_length'] !== $insert_string[$row_num]['line_length'] ||
            $line_arr['line_offset'] !== $insert_string[$row_num]['line_offset'] ||
            $line_arr['trim_length'] !== strlen($insert_string[$row_num]['trim_line']) ||
            $line_arr['trim_line'] !== $insert_string[$row_num]['trim_line']
        ){
            $file_get_keys_passed = false;
            break;
        }
    }

    $file_array = file_get_keys($db_file, 0, $c + 1, 0, 3);
    foreach($file_array as $row_num=>$line_arr){
        if(
            $line_arr['line_count'] - 1 !== $insert_string[$row_num]['line_count'] ||
            $line_arr['line_length'] !== $insert_string[$row_num]['line_length'] ||
            $line_arr['line_offset'] !== $insert_string[$row_num]['line_offset'] 
        ){
            $file_get_keys_passed = false;
            break;
        }
    }

    $file_array = file_get_keys($db_file, 0, $c + 1, 0, 4);
    foreach($file_array as $row_num=>$line_arr){
        $str_array = explode(' ', $insert_string[$row_num]['trim_line']);
        
        if(
            $line_arr !== $str_array[0]
        ){
            $file_get_keys_passed = false;
            break;
        }
    }

    $file_array = file_get_keys($db_file, 0, $c + 1, 0, 5);
    foreach($file_array as $row_num=>$line_arr){       
        if(
            $line_arr !== $insert_string[$row_num]['trim_line']
        ){
            $file_get_keys_passed = false;
            break;
        }
    }

}


if($file_get_keys_passed) echo "Check file_get_keys - PASS\n";
else echo "Check file_get_keys - ERROR\n";


