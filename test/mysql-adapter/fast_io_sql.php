<?php
/*
 * Fast_IO mysql-adapter use Extension for PHP 8
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
 * use greenlion/PHP-SQL-Parser
 * Copyright (c) 2014, Justin Swanhart <greenlion@gmail.com> and André Rothe <phosco@gmx.de>
 * https://github.com/greenlion/PHP-SQL-Parser
 * 
*/

// https://github.com/greenlion/PHP-SQL-Parser
// composer require greenlion/PHP-SQL-Parser

namespace PHPSQLParser;
require_once __DIR__ . '/vendor/autoload.php';

$data_dir = __DIR__;
$index_align = 32;


function fast_io_mysql_adapter(& $parser, $sql){
    global $data_dir, $index_align;

    $parsed = $parser->parse($sql, true);

    $table = null;
    $colref = null;
    $operator = null;
    $const = null;
    $column_list = [];
    $values = [];

    if(isset($parsed['INSERT'])){ // INSERT
        foreach($parsed['INSERT'] as $expr){
            if($expr['expr_type'] == 'table') {
                if(isset($expr['no_quotes']['parts'][0])) $table = $expr['no_quotes']['parts'][0];
                else $table = trim($expr['table'], '"' . "'" . '`');
            }

            if($expr['expr_type'] == 'column-list') {
                if(isset($expr['sub_tree'][0])){
                    foreach($expr['sub_tree'] as $sub_tree){
                        if(isset($sub_tree['no_quotes']['parts'][0])) $column_list[] = $sub_tree['no_quotes']['parts'][0];
                        else $column_list[] = trim($sub_tree['base_expr'], '"' . "'" . '`'); 
                    }
                }                
            }
        }

        if(isset($parsed['VALUES'])){
            foreach($parsed['VALUES'] as $value){
                if(isset($value['data'][0])){
                    foreach($value['data'] as $data){
                        if($data['expr_type'] == 'const') $values[]= trim($data['base_expr'], '"' . "'" . '`');
                    }
                }
            }
        }

        if(
            $table && 
            count($column_list) && 
            count($values) && 
            count($column_list) == count($values)
        ){
            $count = count($column_list);
            $data_file= $data_dir . "/" . $table . ".sql.dat";
            $last_id = 1;

            $insert_values= implode(" ", $values);

            if(file_exists($data_file) && filesize($data_file) > 0){
                $last_id = filesize($data_file) / ($index_align + 1);
                $last_id ++;
            }
                
            $last_id = insert_key_value($data_file, $column_list[0] . "_" . $last_id . ' ' . $insert_values, $index_align);
            if ($last_id < 0) {
                echo "Произошла ошибка при добавлении записи. Код ошибки: $last_id";
                return 1;
            } else {
                return $last_id;
            }
        }

        return 0;
    }




    if(isset($parsed['SELECT'])){ // SELECT
        foreach($parsed['SELECT'] as $expr){
            if($expr['expr_type'] == 'colref') {
                if(isset($expr['no_quotes']['parts'][0])) $column_list[] = $expr['no_quotes']['parts'][0];
                else $column_list[] = trim($expr['base_expr'], '"' . "'" . '`'); 
            }
        }

        foreach($parsed['FROM'] as $expr){
            if($expr['expr_type'] == 'table') {
                if(isset($expr['no_quotes']['parts'][0])) $table = $expr['no_quotes']['parts'][0];
                else $table = trim($expr['table'], '"' . "'" . '`');
            }
        }

        foreach($parsed['WHERE'] as $expr){
            if($expr['expr_type'] == 'colref') {
                if(isset($expr['no_quotes']['parts'][0])) $colref = $expr['no_quotes']['parts'][0];
                else $colref = trim($expr['base_expr'], '"' . "'" . '`'); 
            }
            if($expr['expr_type'] == 'operator') {
                $operator = $expr['base_expr'];
            }
            if($expr['expr_type'] == 'const') {
                $const = $expr['base_expr'];
            }
        }

        if(
            $table && 
            count($column_list) && 
            $colref != null && 
            $operator != null && 
            $const != null
        ){
            $data_file= $data_dir . "/" . $table . ".sql.dat";
            
            if(file_exists($data_file) && in_array('*', $column_list)){
                if($colref == 'id' && $operator == "="){
                    $return = select_key_value($data_file, $const, $index_align);
                    return $return;
                }
            }
        }
    }

    return false;
}


$parser = new PHPSQLParser();

$sql = <<<SQL
    INSERT 
        INTO fast_io (index_key) 
        VALUES ("index_value")
    ;
SQL;
print_r([
    'last_id',
    fast_io_mysql_adapter($parser, $sql)
]);


$sql = <<<SQL
    SELECT *
        FROM fast_io
    WHERE id = 0;
SQL;
print_r([
    'select',
    fast_io_mysql_adapter($parser, $sql)
]);

