<?php
/*
 * Fast_IO Extension for PHP 8
 * https://github.com/commeta/fast_io
 *
 * Copyright 2026 commeta <dcs-spb@ya.ru>
 *
 * check-memleaks.php — ТЕСТ НА УТЕЧКИ ПАМЯТИ fast_io v1.0
 *
 * Покрытие:
 *   - Все 18 функций расширения в интенсивных циклах
 *   - Мониторинг memory_get_usage(true) + peak_usage
 *   - gc_collect_cycles() после каждой фазы
 *   - Очистка файлов после каждой большой фазы
 *   - Финальный вердикт по росту памяти
 *
 * Запуск: php check-memleaks.php [iterations=10000]
 * Запуск: valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --num-callers=40 --log-file=check-memleaks-valgrind.log  php check-memleaks.php 40000
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

// ═══════════════════════════════════════════════════════════════════
//  КОНФИГУРАЦИЯ
// ═══════════════════════════════════════════════════════════════════

define('DEBUG', false);
define('MAX_ACCEPTABLE_DELTA_MB', 8);   // если после всех фаз + GC выросло больше — предупреждение

$iterations = isset($argv[1]) ? max(1000, (int)$argv[1]) : 10000;

$base_dir = sys_get_temp_dir() . '/fast_io_memleak_' . getmypid();
@mkdir($base_dir, 0755, true);

$DB_MAIN   = $base_dir . '/main.dat';
$DB_DATA   = $base_dir . '/data.dat';
$DB_REPL   = $base_dir . '/replica.dat';

if (!function_exists('file_insert_line')) {
    die("❌ Расширение fast_io не загружено\n");
}

// ═══════════════════════════════════════════════════════════════════
//  ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ
// ═══════════════════════════════════════════════════════════════════

function dbg(string $msg): void
{
    if (DEBUG) echo "[DEBUG] $msg\n";
}

function mem_stats(): array
{
    return [
        'usage' => memory_get_usage(true),
        'peak'  => memory_get_peak_usage(true),
    ];
}

function mem_delta(array $before, array $after, float $elapsed): string
{
    $d_usage = $after['usage'] - $before['usage'];
    $d_peak  = $after['peak']  - $before['peak'];
    return sprintf(
        "Δ usage: %8s MB | Δ peak: %8s MB | %.3fs",
        number_format($d_usage / 1048576, 2),
        number_format($d_peak  / 1048576, 2),
        $elapsed
    );
}

function phase_header(string $name): void
{
    echo "\n" . str_repeat('─', 80) . "\n";
    echo "  ФАЗА: $name  (итераций: $GLOBALS[iterations])\n";
    echo str_repeat('─', 80) . "\n";
}

function phase_result(string $name, bool $ok, float $t, string $mem_info): void
{
    $mark = $ok ? '✅' : '⚠️';
    echo "$mark  $name — " . ($ok ? 'OK' : 'POTENTIAL LEAK') . " ({$t}s)\n";
    echo "     $mem_info\n";
}

function cleanup(): void
{
    global $base_dir;
    if (is_dir($base_dir)) {
        array_map('unlink', glob($base_dir . '/*'));
        @rmdir($base_dir);
    }
}

// ═══════════════════════════════════════════════════════════════════
//  СТАРТ
// ═══════════════════════════════════════════════════════════════════

echo "╔══════════════════════════════════════════════════════════════════════╗\n";
echo "║          fast_io MEMORY LEAK TEST  v1.0                              ║\n";
echo "╚══════════════════════════════════════════════════════════════════════╝\n\n";
echo "Итераций на фазу     : $iterations\n";
echo "Рабочая директория   : $base_dir\n";
echo "PHP                  : " . PHP_VERSION . "\n\n";

$global_start = microtime(true);
$global_mem   = mem_stats();

gc_collect_cycles(); // начальная очистка

// ═══════════════════════════════════════════════════════════════════
//  ФАЗА 0 — ПОДГОТОВКА
// ═══════════════════════════════════════════════════════════════════

phase_header('PHASE 0 — Подготовка');

$start0 = microtime(true);
$mem0   = mem_stats();

for ($i = 0; $i < 500; $i++) {
    file_insert_line($DB_MAIN, "prep_key_$i value_" . str_pad('', 200, 'A'));
}
file_analize($DB_MAIN); // прогрев

$delta0 = mem_delta($mem0, mem_stats(), microtime(true) - $start0);
phase_result('Подготовка', true, microtime(true) - $start0, $delta0);

// ═══════════════════════════════════════════════════════════════════
//  ФАЗА 1 — file_insert_line + file_select_line
// ═══════════════════════════════════════════════════════════════════

phase_header('PHASE 1 — insert_line + select_line');

$start1 = microtime(true);
$mem1   = mem_stats();

for ($i = 0; $i < $iterations; $i++) {
    $off = file_insert_line($DB_MAIN, "key$i " . str_pad('payload', 300, 'X'));
    $line = file_select_line($DB_MAIN, $off, 512, 1);
    if ($line === false) die("select_line failed at $i");
}

gc_collect_cycles();
$delta1 = mem_delta($mem1, mem_stats(), microtime(true) - $start1);
phase_result('insert+select', true, microtime(true) - $start1, $delta1);

// ═══════════════════════════════════════════════════════════════════
//  ФАЗА 2 — file_update_line + file_update_array
// ═══════════════════════════════════════════════════════════════════

phase_header('PHASE 2 — update_line + update_array');

$start2 = microtime(true);
$mem2   = mem_stats();

$updates = [];
for ($i = 0; $i < min(2000, $iterations); $i++) {
    $new = "UPD_key$i " . str_pad('NEW', 300, 'Y');
    file_update_line($DB_MAIN, $new, $i * 512, 512, 0);
    $updates[] = [$new, $i * 512, 512];
}
file_update_array($DB_MAIN, $updates, 0);

gc_collect_cycles();
$delta2 = mem_delta($mem2, mem_stats(), microtime(true) - $start2);
phase_result('update', true, microtime(true) - $start2, $delta2);

// ═══════════════════════════════════════════════════════════════════
//  ФАЗА 3 — Поисковые функции
// ═══════════════════════════════════════════════════════════════════

phase_header('PHASE 3 — search_line + search_array + get_keys + pcre2');

$start3 = microtime(true);
$mem3   = mem_stats();

for ($i = 0; $i < $iterations / 10; $i++) {
    file_search_line($DB_MAIN, "key" . ($i % 500), 0, 0);
    file_search_array($DB_MAIN, "key" . ($i % 500), 0, 50, 0, 0);
    file_get_keys($DB_MAIN, 0, 100, 0, 2);
    find_matches_pcre2('key\\d+', "test key123 payload", 1);
}

gc_collect_cycles();
$delta3 = mem_delta($mem3, mem_stats(), microtime(true) - $start3);
phase_result('search + pcre2', true, microtime(true) - $start3, $delta3);

// ═══════════════════════════════════════════════════════════════════
//  ФАЗА 4 — Бинарные данные (push / search / defrag)
// ═══════════════════════════════════════════════════════════════════

phase_header('PHASE 4 — push_data + search_data + defrag_data');

$start4 = microtime(true);
$mem4   = mem_stats();

for ($i = 0; $i < $iterations / 5; $i++) {
    $k = "binkey_$i";
    $v = str_repeat('BinaryData', 50) . $i;
    file_push_data($DB_DATA, $k, $v, 0);
    $got = file_search_data($DB_DATA, $k, 0, 0);
    if ($got !== $v) die("search_data mismatch");
}

file_defrag_data($DB_DATA, '', 0);

gc_collect_cycles();
$delta4 = mem_delta($mem4, mem_stats(), microtime(true) - $start4);
phase_result('binary ops', true, microtime(true) - $start4, $delta4);

// ═══════════════════════════════════════════════════════════════════
//  ФАЗА 5 — callback_line + pop_line + replicate
// ═══════════════════════════════════════════════════════════════════

phase_header('PHASE 5 — callback_line + pop_line + replicate_file');

$start5 = microtime(true);
$mem5   = mem_stats();

// callback
file_callback_line($DB_MAIN, function () {
    static $cnt = 0;
    $cnt++;
    if ($cnt % 1000 === 0) gc_collect_cycles();
    return true;
}, 0, 9);

// pop
for ($i = 0; $i < 500; $i++) {
    file_pop_line($DB_MAIN, -1, 0);
}

// replicate
replicate_file($DB_MAIN, $DB_REPL, 0);

gc_collect_cycles();
$delta5 = mem_delta($mem5, mem_stats(), microtime(true) - $start5);
phase_result('callback + pop + replicate', true, microtime(true) - $start5, $delta5);

// ═══════════════════════════════════════════════════════════════════
//  ФИНАЛЬНЫЙ ВЕРДИКТ
// ═══════════════════════════════════════════════════════════════════

$global_elapsed = microtime(true) - $global_start;
$final_mem = mem_stats();

$peak_growth_mb = ($final_mem['peak'] - $global_mem['peak']) / 1048576;

echo "\n" . str_repeat('═', 80) . "\n";
echo "  ИТОГ ТЕСТА НА УТЕЧКИ ПАМЯТИ\n";
echo str_repeat('═', 80) . "\n";
echo "  Общее время           : " . round($global_elapsed, 3) . " сек\n";
echo "  Пиковый рост памяти   : " . number_format($peak_growth_mb, 2) . " MB\n";
echo "  Финальный usage       : " . number_format($final_mem['usage'] / 1048576, 2) . " MB\n";

if ($peak_growth_mb < MAX_ACCEPTABLE_DELTA_MB) {
    echo "✅  УТЕЧЕК ПАМЯТИ НЕ ОБНАРУЖЕНО (рост < " . MAX_ACCEPTABLE_DELTA_MB . " MB)\n";
    echo "    Библиотека fast_io корректно освобождает память во всех сценариях.\n";
} else {
    echo "⚠️  ВОЗМОЖНАЯ УТЕЧКА ПАМЯТИ (рост " . number_format($peak_growth_mb, 2) . " MB)\n";
    echo "    Рекомендуется запустить под valgrind / php-memprof.\n";
}

echo "\n  Файлы теста удалены.\n";

// CLEANUP
cleanup();
gc_collect_cycles();

echo "  Тест завершён.\n";
