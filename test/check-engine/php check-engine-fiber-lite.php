<?php
/*
 * Fast_IO Extension for PHP 8
 * https://github.com/commeta/fast_io
 * 
 * Copyright 2026 commeta <dcs-spb@ya.ru>
 * 
 * Fast_IO Async Fiber Stress Test
 * Асинхронная версия на Fibers (PHP 8.1+)
 *
 * Запуск: php check-engine-fiber-lite.php [workers=8] [iterations=150]
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

define('DEBUG', true);

$workers    = isset($argv[1]) ? max(2, (int)$argv[1]) : 8;   // fibers легче процессов
$iterations = isset($argv[2]) ? max(10, (int)$argv[2]) : 150;

$db_file = __DIR__ . '/fast_io_async_stress.dat';
if (file_exists($db_file)) unlink($db_file);

if (PHP_VERSION_ID < 80100) {
    die("❌ Требуется PHP 8.1+ для Fibers\n");
}

echo "=== fast_io ASYNC FIBER СТРЕСС-ТЕСТ v1.0 ===\n";
echo "Fibers (workers)   : $workers\n";
echo "Итераций на fiber  : $iterations\n\n";

$start_time = microtime(true);

$fibers = [];
$failed = 0;

for ($w = 0; $w < $workers; $w++) {
    $fiber = new Fiber(function (int $worker_id) use ($iterations, $db_file) {
        mt_srand(getmypid() + hrtime(true) + $worker_id);

        $align = mt_rand(512, 4096);
        ini_set('fast_io.buffer_size', mt_rand(8192, 65536));

        $my_offsets = [];
        $ok = true;

        for ($i = 0; $i < $iterations && $ok; $i++) {
            $r = mt_rand(1, 100);

            if ($r <= 40 || empty($my_offsets)) {
                // INSERT (mode 2)
                $str = "f{$worker_id}_i{$i}_" . str_pad('', mt_rand(40, $align * 2), 'X');
                $offset = file_insert_line($db_file, $str, 2, $align);
                if ($offset !== false && $offset >= 0) {
                    $my_offsets[] = $offset;
                } else {
                    $ok = false;
                }
            } elseif ($r <= 65 && !empty($my_offsets)) {
                // UPDATE single (mode 0)
                $offset = $my_offsets[array_rand($my_offsets)];
                $str = "UPD_f{$worker_id}_" . str_pad('', mt_rand(50, $align), 'Y');
                $written = file_update_line($db_file, $str, $offset, $align, 0);
                if ($written !== $align) $ok = false;
            } elseif ($r <= 80 && !empty($my_offsets)) {
                // SELECT single (mode 1)
                $offset = $my_offsets[array_rand($my_offsets)];
                $line = file_select_line($db_file, $offset, $align, 1);
                if ($line === false) $ok = false;
            } elseif (count($my_offsets) >= 2) {
                // SELECT ARRAY + UPDATE ARRAY (безопасные вызовы)
                $num = min(4, count($my_offsets));
                $sample = array_rand($my_offsets, $num);
                if (!is_array($sample)) $sample = [$sample];

                // SELECT ARRAY
                $query = [];
                foreach ($sample as $idx) $query[] = [$my_offsets[$idx], $align];
                $result = file_select_array($db_file, $query);
                if (!is_array($result) || count($result) !== count($query)) $ok = false;

                // UPDATE ARRAY
                $query = [];
                foreach ($sample as $idx) {
                    $off = $my_offsets[$idx];
                    $str = "BULK_f{$worker_id}_" . str_pad('', mt_rand(35, $align * 2), 'Z');
                    $query[] = [$str, $off, $align];
                }
                $written = file_update_array($db_file, $query, 0);
                if ($written <= 0) $ok = false;
            }

            // Кооперативный yield — даём поработать другим fibers
            if ($i % 5 === 0) {
                Fiber::suspend();
            }
        }

        return $ok ? 0 : 1;
    });

    $fiber->start($w);
    $fibers[] = $fiber;
}

// Планировщик: переключаем fibers пока все не завершатся
while (!empty($fibers)) {
    foreach ($fibers as $k => $fiber) {
        if ($fiber->isTerminated()) {
            if ($fiber->getReturn() !== 0) $failed++;
            unset($fibers[$k]);
        } elseif (!$fiber->isSuspended()) {
            // если fiber ещё не стартовал или завершён — пропускаем
            continue;
        } else {
            try {
                $fiber->resume();
            } catch (Throwable $e) {
                // игнорируем редкие ошибки suspend/resume
            }
        }
    }
    // небольшая пауза, чтобы CPU не горел
    usleep(100);
}

$time = microtime(true) - $start_time;
$analize = function_exists('file_analize') ? file_analize($db_file) : [];

echo "\n" . str_repeat("=", 75) . "\n";
echo "ASYNC FIBER ТЕСТ ЗАВЕРШЁН\n";
echo "Время выполнения      : " . round($time, 3) . " сек\n";
echo "Неудачных fibers      : $failed / $workers\n";
echo "Строк в файле         : " . ($analize['line_count'] ?? 0) . "\n";
echo "Прерываний потока     : " . ($analize['flow_interruption'] ?? 0) . "\n";

if ($failed === 0 && ($analize['flow_interruption'] ?? 0) === 0 && ($analize['line_count'] ?? 0) > 50) {
    echo "\n✅ check-engine-fiber-lite.php — УСПЕШНО ПРОЙДЕН\n";
    echo "Fibers работают корректно под нагрузкой (кооперативно)\n";
} else {
    echo "\n⚠️  Обнаружены проблемы (или недостаточно конкуренции)\n";
}

echo "\nГотово! Запускайте сколько угодно раз.\n";
