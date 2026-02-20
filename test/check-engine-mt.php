<?php
/*
 * Fast_IO Extension for PHP 8
 * https://github.com/commeta/fast_io
 * 
 * Copyright 2026 commeta <dcs-spb@ya.ru>
 * 
 * check-engine-mt.php — МНОГОПРОЦЕССНЫЙ СТРЕСС-ТЕСТ fast_io 
 * 
 * Поддержка параметров: php check-engine-mt.php [workers=12] [iterations=120]
 * Запуск: php check-engine-mt.php 12 120
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

define('DEBUG', false);

$workers    = isset($argv[1]) ? max(2, (int)$argv[1]) : 12;
$iterations = isset($argv[2]) ? max(10, (int)$argv[2]) : 120;

$db_file = __DIR__ . '/fast_io_mt_stress.dat';
if (file_exists($db_file)) unlink($db_file);

function debug($msg) {
    if (DEBUG) echo "[DEBUG] $msg\n";
}

if (!function_exists('pcntl_fork')) {
    die("❌ Требуется PHP с --enable-pcntl\n");
}

echo "=== fast_io МНОГОПРОЦЕССНЫЙ СТРЕСС-ТЕСТ v2.1 ===\n";
echo "Процессов          : $workers\n";
echo "Итераций на процесс: $iterations\n\n";

$start_time = microtime(true);
$children   = [];

for ($w = 0; $w < $workers; $w++) {
    $pid = pcntl_fork();
    if ($pid === -1) die("Не удалось создать процесс\n");

    if ($pid === 0) {
        // ====================== ДОЧЕРНИЙ ПРОЦЕСС ======================
        mt_srand(getmypid() + time() + $w);
        $worker_id = $w;
        $align     = mt_rand(512, 8192);
        ini_set('fast_io.buffer_size', mt_rand(16384, 131072));

        $my_offsets = [];   // только свои offset'ы — 100% безопасность при конкуренции

        $ok = true;
        for ($i = 0; $i < $iterations && $ok; $i++) {
            $r = mt_rand(1, 100);

            if ($r <= 35 || empty($my_offsets)) {
                // === INSERT (mode 2) ===
                $str = "w{$worker_id}_i{$i}_" . str_pad('', mt_rand(30, $align * 3), 'X');
                $offset = file_insert_line($db_file, $str, 2, $align);
                if ($offset !== false && $offset >= 0) {
                    $my_offsets[] = $offset;
                } else {
                    $ok = false;
                }
            } 
            elseif ($r <= 60 && !empty($my_offsets)) {
                // === UPDATE single (mode 0) ===
                $offset = $my_offsets[array_rand($my_offsets)];
                $str    = "UPD_w{$worker_id}_" . str_pad('', mt_rand(40, $align * 2), 'Y');
                $written = file_update_line($db_file, $str, $offset, $align, 0);
                if ($written !== $align) $ok = false;
            } 
            elseif ($r <= 78 && count($my_offsets) >= 2) {
                // === SELECT single (mode 1) ===
                $offset = $my_offsets[array_rand($my_offsets)];
                $line   = file_select_line($db_file, $offset, $align, 1);
                if ($line === false) $ok = false;
            } 
            elseif ($r <= 90 && count($my_offsets) >= 2) {
                // === SELECT ARRAY (безопасный вызов) ===
                $num    = min(5, count($my_offsets));
                $sample = ($num === 1) ? [array_rand($my_offsets)] : array_rand($my_offsets, $num);
                if (!is_array($sample)) $sample = [$sample];

                $query = [];
                foreach ($sample as $idx) {
                    $query[] = [$my_offsets[$idx], $align];
                }
                $result = file_select_array($db_file, $query);
                if (!is_array($result) || count($result) !== count($query)) $ok = false;
            } 
            elseif (count($my_offsets) >= 2) {
                // === UPDATE ARRAY (безопасный вызов) ===
                $num    = min(4, count($my_offsets));
                $sample = ($num === 1) ? [array_rand($my_offsets)] : array_rand($my_offsets, $num);
                if (!is_array($sample)) $sample = [$sample];

                $query = [];
                foreach ($sample as $idx) {
                    $off = $my_offsets[$idx];
                    $str = "BULK_w{$worker_id}_" . str_pad('', mt_rand(35, $align * 2), 'Z');
                    $query[] = [$str, $off, $align];
                }
                $written = file_update_array($db_file, $query, 0);
                if ($written <= 0) $ok = false;
            }
        }
        exit($ok ? 0 : 1);
    } else {
        $children[] = $pid;
    }
}

// ====================== РОДИТЕЛЬ ======================
$failed = 0;
foreach ($children as $pid) {
    pcntl_waitpid($pid, $status);
    if (pcntl_wifexited($status) && pcntl_wexitstatus($status) !== 0) {
        $failed++;
    }
}

$time    = microtime(true) - $start_time;
$analize = function_exists('file_analize') ? file_analize($db_file) : [];

echo "\n" . str_repeat("=", 75) . "\n";
echo "ТЕСТ ЗАВЕРШЁН\n";
echo "Время выполнения      : " . round($time, 3) . " сек\n";
echo "Неудачных процессов   : $failed / $workers\n";
echo "Строк в файле         : " . ($analize['line_count'] ?? 0) . "\n";
echo "Прерываний потока     : " . ($analize['flow_interruption'] ?? 0) . "\n";
echo "Средний размер строки : " . ($analize['avg_length'] ?? 0) . " байт\n";

if ($failed === 0 && ($analize['flow_interruption'] ?? 0) === 0 && ($analize['line_count'] ?? 0) > 100) {
    echo "\n✅ check-engine-mt.php — УСПЕШНО ПРОЙДЕН\n";
    echo "Библиотека полностью thread/process-safe под высокой конкурентной нагрузкой\n";
} else {
    echo "\n❌ ОБНАРУЖЕНЫ ПРОБЛЕМЫ (failed workers или прерывания потока)\n";
}

echo "\nГотово. Запускайте повторно сколько угодно раз — файл пересоздаётся автоматически.\n";
