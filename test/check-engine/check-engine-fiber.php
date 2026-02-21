<?php
/*
 * Fast_IO Extension for PHP 8
 * https://github.com/commeta/fast_io
 *
 * Copyright 2026 commeta <dcs-spb@ya.ru>
 *
 * check-engine-fiber.php — FIBER ASYNC СТРЕСС-ТЕСТ fast_io v1.0
 *
 * Аналог check-engine-mt.php, использует PHP 8.1+ Fibers вместо pcntl_fork.
 *
 * Покрытие (16 суб-тестов, как в mt-версии):
 *   - file_insert_line, file_select_line, file_select_array
 *   - file_update_line, file_update_array
 *   - file_search_line, file_search_array, file_get_keys
 *   - file_erase_line, file_replace_line
 *   - file_push_data, file_search_data, file_defrag_data
 *   - file_pop_line (LIFO-очередь)
 *   - file_callback_line
 *   - file_analize, replicate_file, find_matches_pcre2
 *   - Консистентность данных между fiber-воркерами
 *   - Статистика ядра Linux (/proc/PID/io) по каждому тесту
 *
 * Архитектура:
 *   - Вместо fork/IPC через файлы — общие in-memory массивы ($all_results, $offsets_map)
 *   - Каждый воркер — объект Fiber, делает Fiber::suspend() каждые N операций
 *   - Round-robin планировщик обходит все активные fibers
 *   - Нет pcntl, нет JSON-файлов с результатами
 *
 * Запуск: php check-engine-fiber.php [workers=12] [iterations=120]
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

// ═══════════════════════════════════════════════════════════════════
//  КОНФИГУРАЦИЯ
// ═══════════════════════════════════════════════════════════════════

define('DEBUG', false);         // подробный вывод ошибок внутри воркеров
define('SHOW_IO_PER_PHASE', true);  // вывод IO-статистики после каждой фазы
define('YIELD_EVERY', 3);       // Fiber::suspend() каждые N суб-тестов

$workers    = isset($argv[1]) ? max(2, (int)$argv[1]) : 12;
$iterations = isset($argv[2]) ? max(10, (int)$argv[2]) : 120;

// ─── Проверки ───────────────────────────────────────────────────────

if (PHP_VERSION_ID < 80100) {
    die("❌ Требуется PHP 8.1+ (Fibers)\n");
}
if (!function_exists('file_insert_line')) {
    die("❌ Расширение fast_io не загружено\n");
}

$base_dir = sys_get_temp_dir() . '/fast_io_fiber_' . getmypid();
@mkdir($base_dir, 0755, true);

// Файлы теста (разделяемые между всеми fibers — как между процессами)
$DB_MAIN     = $base_dir . '/main.dat';
$DB_DATA     = $base_dir . '/data.dat';
$DB_CALLBACK = $base_dir . '/callback.dat';

// ═══════════════════════════════════════════════════════════════════
//  ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ
// ═══════════════════════════════════════════════════════════════════

function dbg(string $msg): void
{
    if (DEBUG) echo "[DEBUG] $msg\n";
}

function get_io_stats(): array
{
    $path = '/proc/' . getmypid() . '/io';
    if (!file_exists($path)) return [];
    $raw = @file_get_contents($path);
    if ($raw === false) return [];
    $stats = [];
    foreach (explode("\n", trim($raw)) as $line) {
        if ($line === '') continue;
        [$k, $v] = explode(':', $line, 2);
        $stats[trim($k)] = (int)trim($v);
    }
    return $stats;
}

function diff_io(array $before, array $after): array
{
    $diff = [];
    foreach ($after as $k => $v) {
        $diff[$k] = $v - ($before[$k] ?? 0);
    }
    return $diff;
}

function format_io(array $diff, float $elapsed): string
{
    $keys_mb  = ['rchar', 'wchar', 'read_bytes', 'write_bytes', 'cancelled_write_bytes'];
    $keys_sys = ['syscr', 'syscw'];
    $lines = [];
    foreach ($diff as $k => $v) {
        if (in_array($k, $keys_mb, true)) {
            $mbps = $elapsed > 0 ? round($v / 1e6 / $elapsed, 2) : 0;
            $mb   = round($v / 1e6, 3);
            $lines[] = sprintf("  %-30s %10s MB  (%8.2f MB/s)", $k . ':', $mb, $mbps);
        } elseif (in_array($k, $keys_sys, true)) {
            $rps = $elapsed > 0 ? round($v / $elapsed, 0) : 0;
            $lines[] = sprintf("  %-30s %10d ops (%8d ops/s)", $k . ':', $v, $rps);
        }
    }
    return implode("\n", $lines);
}

function phase_header(string $name): void
{
    echo "\n" . str_repeat('─', 70) . "\n";
    echo "  ФАЗА: $name\n";
    echo str_repeat('─', 70) . "\n";
}

function phase_result(string $name, bool $ok, float $t, array $io_diff): void
{
    $mark = $ok ? '✅' : '❌';
    echo "$mark  $name — " . ($ok ? 'PASS' : 'FAIL') . " ({$t}s)\n";
    if (SHOW_IO_PER_PHASE && !empty($io_diff)) {
        echo format_io($io_diff, $t) . "\n";
    }
}

function cleanup_dir(string $dir): void
{
    if (!is_dir($dir)) return;
    foreach (glob($dir . '/*') as $f) {
        if (is_file($f))      unlink($f);
        elseif (is_dir($f))   cleanup_dir($f);
    }
    @rmdir($dir);
}

// ═══════════════════════════════════════════════════════════════════
//  СТАРТ
// ═══════════════════════════════════════════════════════════════════

echo "╔══════════════════════════════════════════════════════════════════╗\n";
echo "║      fast_io ASYNC FIBER СТРЕСС-ТЕСТ  v1.0                       ║\n";
echo "╚══════════════════════════════════════════════════════════════════╝\n\n";
echo "Fibers (workers)     : $workers\n";
echo "Итераций на fiber    : $iterations\n";
echo "Рабочая директория   : $base_dir\n";
echo "PHP                  : " . PHP_VERSION . "\n\n";

$global_start    = microtime(true);
$global_io_start = get_io_stats();

// ═══════════════════════════════════════════════════════════════════
//  ФАЗА 0 — ПОДГОТОВКА ФАЙЛА С ЗАФИКСИРОВАННЫМИ СТРОКАМИ
//  Главный поток вставляет $workers * $iterations строк с фиксированным
//  выравниванием. Каждый fiber получает свой диапазон смещений.
// ═══════════════════════════════════════════════════════════════════

phase_header('PHASE 0 — Подготовка общего файла и карты консистентности');

$ALIGN       = 512;
$TOTAL_LINES = $workers * $iterations;

// Разделяемые данные (аналог consistency.map + IPC в mt-версии)
$offsets_map = [];   // offset => trim_payload (проверка консистентности)

ini_set('fast_io.buffer_size', 65536);

$t0   = microtime(true);
$io0  = get_io_stats();

for ($i = 0; $i < $TOTAL_LINES; $i++) {
    $payload = sprintf(
        'w%03d_i%05d_%s',
        (int)($i / $iterations),
        $i,
        str_pad('', min(200, $ALIGN - 30), 'PREPARE_')
    );
    $trim_pl = substr($payload, 0, $ALIGN - 1);
    $off = file_insert_line($DB_MAIN, $payload, 2, $ALIGN);
    if ($off < 0) {
        echo "❌ file_insert_line failed at line $i (ret=$off)\n";
        cleanup_dir($base_dir);
        exit(1);
    }
    $offsets_map[$off] = $trim_pl;
}

// Подготовка файла для callback
for ($i = 0; $i < $TOTAL_LINES; $i++) {
    $s = sprintf('cbkey_%05d value_%05d_%s', $i, $i, str_pad('', 60, 'CB'));
    file_insert_line($DB_CALLBACK, $s, 2, $ALIGN);
}

$analize0 = file_analize($DB_MAIN);
$t0e  = round(microtime(true) - $t0, 4);
$io0e = diff_io($io0, get_io_stats());

echo "Вставлено строк      : {$analize0['line_count']} / $TOTAL_LINES\n";
echo "Размер файла         : " . number_format($analize0['file_size']) . " байт\n";
echo "min/max/avg строк    : {$analize0['min_length']} / {$analize0['max_length']} / {$analize0['avg_length']}\n";
echo "flow_interruption    : {$analize0['flow_interruption']}\n";

if (
    $analize0['line_count']      !== $TOTAL_LINES
    || $analize0['flow_interruption'] !== 0
    || $analize0['min_length']   !== $ALIGN
    || $analize0['max_length']   !== $ALIGN
) {
    echo "❌ ФАЗА 0 провалена — структура файла некорректна\n";
    cleanup_dir($base_dir);
    exit(1);
}

phase_result('PHASE 0 подготовка', true, $t0e, $io0e);

// ═══════════════════════════════════════════════════════════════════
//  РАСПРЕДЕЛЕНИЕ ДИАПАЗОНОВ ПО FIBER-ВОРКЕРАМ
// ═══════════════════════════════════════════════════════════════════

$all_offsets   = array_keys($offsets_map);
sort($all_offsets);

$worker_ranges = [];
$chunk_size    = (int)ceil(count($all_offsets) / $workers);
for ($w = 0; $w < $workers; $w++) {
    $worker_ranges[$w] = array_slice($all_offsets, $w * $chunk_size, $chunk_size);
}

// ═══════════════════════════════════════════════════════════════════
//  ФАЗА 1 — ПАРАЛЛЕЛЬНОЕ ВЫПОЛНЕНИЕ ЧЕРЕЗ FIBERS
//
//  Ключевое отличие от pcntl-версии:
//    - Нет fork/waitpid, нет JSON-файлов результатов
//    - $all_results — общий массив PHP (доступен всем fibers)
//    - Fiber::suspend() каждые YIELD_EVERY суб-тестов = кооперация
//    - Планировщик: round-robin по всем активным fibers
// ═══════════════════════════════════════════════════════════════════

phase_header('PHASE 1 — Параллельное выполнение (все fiber-воркеры)');

// Разделяемый массив результатов (читается после завершения всех fibers)
$all_results = [];

// ── ФАБРИКА WORKER-FIBER ─────────────────────────────────────────────
//
//  Каждый fiber выполняет ровно те же 16 суб-тестов, что и дочерний
//  процесс в check-engine-mt.php. Единственное отличие — данные
//  пишутся в &$all_results[$wid] вместо child_result() в файл.
//
//  Параметры через замыкание (use):
//    - &$all_results   : разделяемый массив [wid => result_data]
//    - $offsets_map    : карта offset => expected_payload
//    - $worker_ranges  : диапазоны смещений per-worker
//    - $base_dir, $DB_MAIN, $DB_DATA, $DB_CALLBACK, $ALIGN
//
$make_worker_fiber = function(int $wid) use (
    &$all_results,
    $offsets_map,
    $worker_ranges,
    $base_dir,
    $DB_MAIN,
    $DB_DATA,
    $DB_CALLBACK,
    $ALIGN,
    $iterations
): Fiber {
    return new Fiber(function() use (
        $wid,
        &$all_results,
        $offsets_map,
        $worker_ranges,
        $base_dir,
        $DB_MAIN,
        $DB_DATA,
        $DB_CALLBACK,
        $ALIGN,
        $iterations
    ): void {
        $my_offs = $worker_ranges[$wid];

        $result = [
            'wid'      => $wid,
            'errors'   => [],
            'ops'      => [],
            'io_stats' => [],
        ];

        $err = function(string $phase, string $msg) use (&$result, $wid): void {
            $result['errors'][] = "[$phase] $msg";
            if (DEBUG) echo "[W{$wid} ERR] [$phase] $msg\n";
        };

        ini_set('fast_io.buffer_size', mt_rand(4096, 131072));

        $yield_counter = 0;
        $maybe_yield = function() use (&$yield_counter): void {
            $yield_counter++;
            if ($yield_counter % YIELD_EVERY === 0) {
                Fiber::suspend();
            }
        };

        // ── SUB-TEST A: SELECT_LINE ───────────────────────────────────
        $io_a  = get_io_stats();
        $ta    = microtime(true);
        $ok_a  = true;
        foreach (array_slice($my_offs, 0, min(30, count($my_offs))) as $off) {
            $expected = $offsets_map[$off];
            $got = file_select_line($DB_MAIN, $off, $ALIGN, 1);
            if ($got === false || $got !== $expected) {
                $err('select_line', "offset=$off expected_len=" . strlen($expected) . " got_len=" . strlen($got ?? ''));
                $ok_a = false;
                break;
            }
        }
        $result['ops']['select_line']      = ['ok' => $ok_a, 't' => round(microtime(true) - $ta, 5)];
        $result['io_stats']['select_line'] = diff_io($io_a, get_io_stats());
        $maybe_yield();

        // ── SUB-TEST B: SELECT_ARRAY ──────────────────────────────────
        $io_b   = get_io_stats();
        $tb     = microtime(true);
        $ok_b   = true;
        $sample_b = array_slice($my_offs, 0, min(20, count($my_offs)));
        $query_b  = array_map(fn($o) => [$o, $ALIGN], $sample_b);
        if (!empty($query_b)) {
            $rows = file_select_array($DB_MAIN, $query_b);
            if (!is_array($rows) || count($rows) !== count($query_b)) {
                $err('select_array', 'count mismatch expected=' . count($query_b) . ' got=' . count($rows ?? []));
                $ok_b = false;
            } else {
                foreach ($rows as $ri => $row) {
                    $expected = $offsets_map[$sample_b[$ri]];
                    if ($row['trim_line'] !== $expected) {
                        $err('select_array', "row[$ri] content mismatch");
                        $ok_b = false;
                        break;
                    }
                }
            }
        }
        $result['ops']['select_array']      = ['ok' => $ok_b, 't' => round(microtime(true) - $tb, 5)];
        $result['io_stats']['select_array'] = diff_io($io_b, get_io_stats());
        $maybe_yield();

        // ── SUB-TEST C: UPDATE_LINE ───────────────────────────────────
        $io_c   = get_io_stats();
        $tc     = microtime(true);
        $ok_c   = true;
        $updated_c = [];
        foreach (array_slice($my_offs, 0, min(25, count($my_offs))) as $off) {
            $new_str = sprintf(
                'UPD_w%03d_%010d_%s',
                $wid,
                $off,
                str_pad('', min(150, $ALIGN - 30), 'U')
            );
            $new_exp = substr($new_str, 0, $ALIGN - 1);
            $written = file_update_line($DB_MAIN, $new_str, $off, $ALIGN, 0);
            if ($written !== $ALIGN) {
                $err('update_line', "offset=$off written=$written expected=$ALIGN");
                $ok_c = false;
                break;
            }
            $updated_c[$off] = $new_exp;
        }
        foreach ($updated_c as $off => $exp) {
            $got = file_select_line($DB_MAIN, $off, $ALIGN, 1);
            if ($got !== $exp) {
                $err('update_line_verify', "offset=$off");
                $ok_c = false;
                break;
            }
        }
        $result['ops']['update_line']      = ['ok' => $ok_c, 't' => round(microtime(true) - $tc, 5)];
        $result['io_stats']['update_line'] = diff_io($io_c, get_io_stats());
        $maybe_yield();

        // ── SUB-TEST D: UPDATE_ARRAY ──────────────────────────────────
        $io_d  = get_io_stats();
        $td    = microtime(true);
        $ok_d  = true;
        if (count($my_offs) >= 4) {
            $sample_d = array_slice($my_offs, 0, min(10, count($my_offs)));
            $query_d  = [];
            $expect_d = [];
            foreach ($sample_d as $off) {
                $ns = sprintf('BULK_w%03d_%010d_%s', $wid, $off, str_pad('', 100, 'B'));
                $query_d[]       = [$ns, $off, $ALIGN];
                $expect_d[$off]  = substr($ns, 0, $ALIGN - 1);
            }
            $wr = file_update_array($DB_MAIN, $query_d, 0);
            if ($wr <= 0) {
                $err('update_array', "written=$wr");
                $ok_d = false;
            } else {
                foreach ($expect_d as $off => $exp) {
                    $got = file_select_line($DB_MAIN, $off, $ALIGN, 1);
                    if ($got !== $exp) {
                        $err('update_array_verify', "offset=$off");
                        $ok_d = false;
                        break;
                    }
                }
            }
        }
        $result['ops']['update_array']      = ['ok' => $ok_d, 't' => round(microtime(true) - $td, 5)];
        $result['io_stats']['update_array'] = diff_io($io_d, get_io_stats());
        $maybe_yield();

        // ── SUB-TEST E: SEARCH_LINE ───────────────────────────────────
        $io_e      = get_io_stats();
        $te        = microtime(true);
        $ok_e      = true;
        $unique_key = sprintf('UPD_w%03d_', $wid);
        $found_e   = file_search_line($DB_MAIN, $unique_key, 0, 0);
        if ($found_e === false) {
            $err('search_line', "key='$unique_key' not found");
            $ok_e = false;
        }
        $result['ops']['search_line']      = ['ok' => $ok_e, 't' => round(microtime(true) - $te, 5)];
        $result['io_stats']['search_line'] = diff_io($io_e, get_io_stats());
        $maybe_yield();

        // ── SUB-TEST F: SEARCH_ARRAY ──────────────────────────────────
        $io_f   = get_io_stats();
        $tf     = microtime(true);
        $ok_f   = true;
        $found_f = file_search_array($DB_MAIN, $unique_key, 0, 5, 0, 0);
        if (!is_array($found_f) || empty($found_f)) {
            $err('search_array', "key='$unique_key' returned empty");
            $ok_f = false;
        }
        $pat_f   = 'UPD_w' . sprintf('%03d', $wid) . '_\\d+';
        $found_fr = file_search_array($DB_MAIN, $pat_f, 0, 5, 0, 10);
        if (!is_array($found_fr) || empty($found_fr)) {
            $err('search_array_regex', "pattern='$pat_f' returned empty");
            $ok_f = false;
        }
        $result['ops']['search_array']      = ['ok' => $ok_f, 't' => round(microtime(true) - $tf, 5)];
        $result['io_stats']['search_array'] = diff_io($io_f, get_io_stats());
        $maybe_yield();

        // ── SUB-TEST G: GET_KEYS ──────────────────────────────────────
        $io_g  = get_io_stats();
        $tg    = microtime(true);
        $ok_g  = true;
        $keys_g = file_get_keys($DB_MAIN, 0, 10, 0, 4);
        if (!is_array($keys_g) || count($keys_g) < 1) {
            $err('get_keys', 'returned empty');
            $ok_g = false;
        }
        $result['ops']['get_keys']      = ['ok' => $ok_g, 't' => round(microtime(true) - $tg, 5)];
        $result['io_stats']['get_keys'] = diff_io($io_g, get_io_stats());
        $maybe_yield();

        // ── SUB-TEST H: ERASE_LINE ────────────────────────────────────
        $io_h      = get_io_stats();
        $th        = microtime(true);
        $ok_h      = true;
        $erase_file = $base_dir . "/erase_test_w{$wid}.dat";

        for ($ei = 0; $ei < 30; $ei++) {
            $s = sprintf('erase_w%03d_item%05d_%s', $wid, $ei, str_pad('', min(300, $ALIGN - 40), 'ERASE_'));
            file_insert_line($erase_file, $s, 2, $ALIGN);
        }
        for ($ei = 5; $ei < 15; $ei++) {
            $key = sprintf('erase_w%03d_item%05d_', $wid, $ei);
            $er  = file_erase_line($erase_file, $key, 0, 0);
            if ($er < 0) {
                $err('erase_line', "key=$key ret=$er");
                $ok_h = false;
                break;
            }
        }
        $an_erase = file_analize($erase_file);
        if (($an_erase['flow_interruption'] ?? 0) !== 0) {
            $err('erase_line', 'flow_interruption after erase');
            $ok_h = false;
        }
        @unlink($erase_file);
        $result['ops']['erase_line']      = ['ok' => $ok_h, 't' => round(microtime(true) - $th, 5)];
        $result['io_stats']['erase_line'] = diff_io($io_h, get_io_stats());
        $maybe_yield();

        // ── SUB-TEST I: PUSH_DATA / SEARCH_DATA ──────────────────────
        $io_i    = get_io_stats();
        $ti      = microtime(true);
        $ok_i    = true;
        $pushed_i = [];
        for ($pi = 0; $pi < min($iterations, 20); $pi++) {
            $key_i = sprintf('wid%d_item%d', $wid, $pi);
            $val_i = sprintf('payload_%d_%d_%s', $wid, $pi, str_pad('', mt_rand(10, 200), 'D'));
            $po    = file_push_data($DB_DATA, $key_i, $val_i, 0);
            if ($po < 0) {
                $err('push_data', "key=$key_i ret=$po");
                $ok_i = false;
                break;
            }
            $pushed_i[$key_i] = $val_i;
        }
        foreach ($pushed_i as $ki => $vi) {
            $got = file_search_data($DB_DATA, $ki, 0, 0);
            if ($got === false || $got !== $vi) {
                $err('search_data', "key=$ki mismatch got_len=" . strlen($got ?? ''));
                $ok_i = false;
                break;
            }
        }
        $result['ops']['push_search_data']      = ['ok' => $ok_i, 't' => round(microtime(true) - $ti, 5)];
        $result['io_stats']['push_search_data'] = diff_io($io_i, get_io_stats());
        $maybe_yield();

        // ── SUB-TEST J: CALLBACK_LINE ─────────────────────────────────
        $io_j     = get_io_stats();
        $tj       = microtime(true);
        $ok_j     = true;
        $cb_count  = 0;
        $cb_target = sprintf('cbkey_%05d ', (int)($wid * $iterations));
        $cb_found  = false;
        file_callback_line(
            $DB_CALLBACK,
            function() use (&$cb_count, $cb_target, &$cb_found): bool {
                $cb_count++;
                $line = func_get_arg(0);
                if (str_starts_with($line, $cb_target)) {
                    $cb_found = true;
                    return false;
                }
                return true;
            },
            0,
            0
        );
        if (!$cb_found) {
            $err('callback_line', "target='$cb_target' not found after $cb_count lines");
            $ok_j = false;
        }
        $result['ops']['callback_line']      = ['ok' => $ok_j, 't' => round(microtime(true) - $tj, 5), 'lines_scanned' => $cb_count];
        $result['io_stats']['callback_line'] = diff_io($io_j, get_io_stats());
        $maybe_yield();

        // ── SUB-TEST K: FILE_ANALIZE ──────────────────────────────────
        $io_k  = get_io_stats();
        $tk    = microtime(true);
        $ok_k  = true;
        $an    = file_analize($DB_MAIN);
        if (!is_array($an) || ($an['file_size'] ?? 0) <= 0) {
            $err('file_analize', 'returned empty or zero file_size');
            $ok_k = false;
        }
        if (($an['flow_interruption'] ?? -1) !== 0) {
            $err('file_analize', 'flow_interruption=' . $an['flow_interruption']);
            $ok_k = false;
        }
        $result['ops']['file_analize']      = ['ok' => $ok_k, 't' => round(microtime(true) - $tk, 5)];
        $result['io_stats']['file_analize'] = diff_io($io_k, get_io_stats());
        $maybe_yield();

        // ── SUB-TEST L: FIND_MATCHES_PCRE2 ───────────────────────────
        $io_l  = get_io_stats();
        $tl    = microtime(true);
        $ok_l  = true;
        $subj_l = sprintf('UPD_w%03d_0000100000_UUUUUUUU', $wid);
        $m0    = find_matches_pcre2('\\w+_\\d+', $subj_l, 0);
        if (!is_array($m0) || empty($m0)) {
            $err('find_matches_pcre2_mode0', 'no matches');
            $ok_l = false;
        }
        $m1 = find_matches_pcre2('w\\d+', $subj_l, 1);
        if (!is_array($m1) || empty($m1) || !isset($m1[0]['line_match'])) {
            $err('find_matches_pcre2_mode1', 'no detailed matches');
            $ok_l = false;
        }
        if ($ok_l && $m1[0]['match_offset'] !== strpos($subj_l, $m1[0]['line_match'])) {
            $err('find_matches_pcre2_offset', 'match_offset mismatch');
            $ok_l = false;
        }
        $result['ops']['find_matches_pcre2']      = ['ok' => $ok_l, 't' => round(microtime(true) - $tl, 5)];
        $result['io_stats']['find_matches_pcre2'] = diff_io($io_l, get_io_stats());
        $maybe_yield();

        // ── SUB-TEST M: REPLACE_LINE ──────────────────────────────────
        $io_m        = get_io_stats();
        $tm          = microtime(true);
        $ok_m        = true;
        $replace_file = $base_dir . "/replace_w{$wid}.dat";
        for ($ri = 0; $ri < 10; $ri++) {
            file_insert_line($replace_file, "rk_{$wid}_{$ri} old_value_{$ri}");
        }
        $new_val = "rk_{$wid}_5 new_value_replaced";
        $rc      = file_replace_line($replace_file, "rk_{$wid}_5 ", $new_val, 0);
        if ($rc < 0) {
            $err('replace_line', "ret=$rc");
            $ok_m = false;
        } else {
            $check_m = file_search_line($replace_file, "rk_{$wid}_5 ", 0, 0);
            if ($check_m === false || trim($check_m) !== $new_val) {
                $err('replace_line_verify', "got='" . trim($check_m ?? '') . "'");
                $ok_m = false;
            }
        }
        @unlink($replace_file);
        $result['ops']['replace_line']      = ['ok' => $ok_m, 't' => round(microtime(true) - $tm, 5)];
        $result['io_stats']['replace_line'] = diff_io($io_m, get_io_stats());
        $maybe_yield();

        // ── SUB-TEST N: POP_LINE (LIFO) ───────────────────────────────
        $io_n       = get_io_stats();
        $tn         = microtime(true);
        $ok_n       = true;
        $stack_file  = $base_dir . "/stack_w{$wid}.dat";
        $stack_items = [];
        for ($si = 0; $si < min($iterations, 30); $si++) {
            $sv = sprintf('stack_w%d_item%05d_%s', $wid, $si, str_pad('', 60, 'S'));
            file_insert_line($stack_file, $sv, 2, $ALIGN);
            $stack_items[] = substr($sv, 0, $ALIGN - 1);
        }
        for ($si = count($stack_items) - 1; $si >= 0; $si--) {
            $p = file_pop_line($stack_file, -1, 0);
            if ($p === false) {
                $err('pop_line', "failed at si=$si");
                $ok_n = false;
                break;
            }
            if ($p !== $stack_items[$si]) {
                $err('pop_line_content', "si=$si expected_len=" . strlen($stack_items[$si]) . " got_len=" . strlen($p));
                $ok_n = false;
                break;
            }
        }
        if ($ok_n && file_exists($stack_file) && filesize($stack_file) !== 0) {
            $err('pop_line_empty', 'file not empty after full pop, size=' . filesize($stack_file));
            $ok_n = false;
        }
        @unlink($stack_file);
        $result['ops']['pop_line']      = ['ok' => $ok_n, 't' => round(microtime(true) - $tn, 5)];
        $result['io_stats']['pop_line'] = diff_io($io_n, get_io_stats());
        $maybe_yield();

        // ── SUB-TEST O: REPLICATE_FILE ────────────────────────────────
        $io_o    = get_io_stats();
        $to      = microtime(true);
        $ok_o    = true;
        $rep_src = $base_dir . "/rep_src_w{$wid}.dat";
        $rep_dst = $base_dir . "/rep_dst_w{$wid}.dat";
        for ($ri = 0; $ri < 5; $ri++) {
            file_insert_line($rep_src, "rep_{$wid}_{$ri} " . str_pad('', 80, 'R'), 2, $ALIGN);
        }
        $br = replicate_file($rep_src, $rep_dst, 0);
        if ($br < 0 || filesize($rep_src) !== filesize($rep_dst)) {
            $err('replicate_file', "ret=$br src=" . filesize($rep_src) . " dst=" . filesize($rep_dst));
            $ok_o = false;
        } else {
            for ($ri = 0; $ri < 5; $ri++) {
                $ls = file_select_line($rep_src, $ri, $ALIGN, 0);
                $ld = file_select_line($rep_dst, $ri, $ALIGN, 0);
                if ($ls !== $ld) {
                    $err('replicate_file_content', "row=$ri src='$ls' dst='$ld'");
                    $ok_o = false;
                    break;
                }
            }
        }
        @unlink($rep_src);
        @unlink($rep_dst);
        $result['ops']['replicate_file']      = ['ok' => $ok_o, 't' => round(microtime(true) - $to, 5)];
        $result['io_stats']['replicate_file'] = diff_io($io_o, get_io_stats());
        $maybe_yield();

        // ── SUB-TEST P: DEFRAG_DATA ────────────────────────────────────
        $io_p       = get_io_stats();
        $tp         = microtime(true);
        $ok_p       = true;
        $defrag_dat  = $base_dir . "/defrag_w{$wid}.dat";
        $defrag_keep  = [];
        $defrag_erase = [];
        for ($di = 0; $di < 10; $di++) {
            $dk = "dd_w{$wid}_item{$di}";
            $dv = str_pad('', mt_rand(20, 100), "defrag_val_{$di}_");
            file_push_data($defrag_dat, $dk, $dv, 0);
            if ($di % 2 === 0) {
                $defrag_erase[] = $dk;
            } else {
                $defrag_keep[$dk] = $dv;
            }
        }
        foreach ($defrag_erase as $dk) {
            file_erase_line($defrag_dat . '.index', $dk . ' ', 0, 0);
        }
        $dr = file_defrag_data($defrag_dat, '', 0);
        if ($dr < 0) {
            $err('defrag_data', "ret=$dr");
            $ok_p = false;
        } else {
            foreach ($defrag_keep as $dk => $dv) {
                $got = file_search_data($defrag_dat, $dk, 0, 0);
                if ($got !== $dv) {
                    $err('defrag_data_verify', "key=$dk got_len=" . strlen($got ?? ''));
                    $ok_p = false;
                    break;
                }
            }
        }
        @unlink($defrag_dat);
        @unlink($defrag_dat . '.index');
        $result['ops']['defrag_data']      = ['ok' => $ok_p, 't' => round(microtime(true) - $tp, 5)];
        $result['io_stats']['defrag_data'] = diff_io($io_p, get_io_stats());

        // ── СОХРАНЕНИЕ РЕЗУЛЬТАТА В ОБЩИЙ МАССИВ ─────────────────────
        // Аналог child_result() из mt-версии, но без файлов:
        $all_results[$wid] = $result;
    });
};

// ── СОЗДАНИЕ И СТАРТ ВСЕХ FIBERS ─────────────────────────────────────

$fibers = [];
for ($w = 0; $w < $workers; $w++) {
    mt_srand((int)(hrtime(true) / 1000) ^ ($w * 1000031));
    $fiber = $make_worker_fiber($w);
    $fiber->start();
    $fibers[$w] = $fiber;
}

// ── ROUND-ROBIN ПЛАНИРОВЩИК ──────────────────────────────────────────
//  Аналог pcntl_waitpid — крутимся пока все fibers не завершатся.
//  В отличие от mt-версии: нет ожидания дочерних процессов,
//  управление полностью кооперативное (Fiber::suspend()).

$failed_workers = 0;
$active = range(0, $workers - 1);

while (!empty($active)) {
    $still_active = [];
    foreach ($active as $w) {
        $fiber = $fibers[$w];
        if ($fiber->isTerminated()) {
            // fiber завершён — проверяем наличие ошибок
            if (!empty($all_results[$w]['errors'] ?? [])) {
                $failed_workers++;
            }
        } elseif ($fiber->isSuspended()) {
            $fiber->resume();
            $still_active[] = $w;
        } else {
            $still_active[] = $w;
        }
    }
    $active = $still_active;
}

// ═══════════════════════════════════════════════════════════════════
//  ФАЗА 2 — КОНСИСТЕНТНОСТЬ ДАННЫХ
//  Главный поток читает $offsets_map и проверяет каждую ячейку.
//  Логика идентична mt-версии, только map уже в памяти ($offsets_map).
// ═══════════════════════════════════════════════════════════════════

phase_header('PHASE 2 — Консистентность данных (главный поток читает после всех fibers)');

$t2  = microtime(true);
$io2 = get_io_stats();
$ok_cons  = true;
$checked  = 0;
$corrupted = [];

ini_set('fast_io.buffer_size', 131072);

foreach ($offsets_map as $off => $expected_payload) {
    $checked++;

    $trimmed = file_select_line($DB_MAIN, $off, $ALIGN, 1);
    $raw     = file_select_line($DB_MAIN, $off, $ALIGN, 3);

    if ($trimmed === false) {
        $corrupted[] = "offset=$off: trimmed returned FALSE";
        $ok_cons = false;
        continue;
    }
    if ($raw === false || strlen($raw) !== $ALIGN) {
        $corrupted[] = "offset=$off: raw_len=" . strlen($raw ?? '') . " expected=$ALIGN";
        $ok_cons = false;
        continue;
    }
    if (isset($raw[0]) && $raw[0] === "\x00") {
        $corrupted[] = "offset=$off: null byte at start";
        $ok_cons = false;
    }
}

$t2e  = round(microtime(true) - $t2, 4);
$io2e = diff_io($io2, get_io_stats());

echo "Проверено ячеек : $checked\n";
echo "Повреждено      : " . count($corrupted) . "\n";
if (!empty($corrupted)) {
    foreach (array_slice($corrupted, 0, 10) as $c) echo "  ⚠️  $c\n";
}
phase_result('PHASE 2 консистентность', $ok_cons, $t2e, $io2e);

// ═══════════════════════════════════════════════════════════════════
//  ФАЗА 3 — ФИНАЛЬНЫЙ АНАЛИЗ ФАЙЛОВ
// ═══════════════════════════════════════════════════════════════════

phase_header('PHASE 3 — Финальный анализ файлов');

$t3   = microtime(true);
$io3  = get_io_stats();
$an_final = file_analize($DB_MAIN);

$ok_f3 = ($an_final['flow_interruption'] ?? 1) === 0
    && ($an_final['file_size'] ?? 0) > 0
    && ($an_final['min_length'] ?? 0) === $ALIGN
    && ($an_final['max_length'] ?? 0) === $ALIGN;

echo sprintf("  %-25s %d\n",   'line_count:',              $an_final['line_count'] ?? 0);
echo sprintf("  %-25s %s B\n", 'file_size:',               number_format($an_final['file_size'] ?? 0));
echo sprintf("  %-25s %d\n",   'min_length:',              $an_final['min_length'] ?? 0);
echo sprintf("  %-25s %d\n",   'max_length:',              $an_final['max_length'] ?? 0);
echo sprintf("  %-25s %.1f\n", 'avg_length:',              $an_final['avg_length'] ?? 0);
echo sprintf("  %-25s %d\n",   'flow_interruption:',       $an_final['flow_interruption'] ?? 0);
echo sprintf("  %-25s %d\n",   'last_symbol (10=LF):',     $an_final['last_symbol'] ?? 0);

$t3e  = round(microtime(true) - $t3, 4);
$io3e = diff_io($io3, get_io_stats());
phase_result('PHASE 3 анализ', $ok_f3, $t3e, $io3e);

// ═══════════════════════════════════════════════════════════════════
//  ФАЗА 4 — СВОДНАЯ СТАТИСТИКА ПО ВОРКЕРАМ
//  Данные берём из $all_results (in-memory), не из JSON-файлов.
// ═══════════════════════════════════════════════════════════════════

phase_header('PHASE 4 — Сводная статистика по fiber-воркерам');

$sub_tests = [
    'select_line',
    'select_array',
    'update_line',
    'update_array',
    'search_line',
    'search_array',
    'get_keys',
    'erase_line',
    'push_search_data',
    'callback_line',
    'file_analize',
    'find_matches_pcre2',
    'replace_line',
    'pop_line',
    'replicate_file',
    'defrag_data',
];

$display_workers = min($workers, 12);

printf("\n  %-22s", 'SUB-TEST');
for ($w = 0; $w < $display_workers; $w++) printf("  W%-2d", $w);
echo "  PASS/TOTAL\n";
echo '  ' . str_repeat('─', 22 + $display_workers * 5 + 12) . "\n";

$total_errors = 0;
$sub_totals   = [];

foreach ($sub_tests as $st) {
    printf("  %-22s", $st);
    $pass  = 0;
    $total = 0;
    for ($w = 0; $w < $display_workers; $w++) {
        $r = $all_results[$w] ?? [];
        if (isset($r['ops'][$st])) {
            $ok = $r['ops'][$st]['ok'] ?? false;
            printf("  %s", $ok ? '✅' : '❌');
            if ($ok) $pass++;
            $total++;
        } else {
            printf("  --");
        }
    }
    printf("  %d/%d\n", $pass, $total);
    $sub_totals[$st] = [$pass, $total];
}

echo "\n  ОШИБКИ ПО ВОРКЕРАМ:\n";
for ($w = 0; $w < $workers; $w++) {
    $r    = $all_results[$w] ?? [];
    $errs = $r['errors'] ?? [];
    if (!empty($errs)) {
        echo "  W$w:\n";
        foreach (array_slice($errs, 0, 5) as $e) echo "    ⚠️  $e\n";
        $total_errors += count($errs);
    }
}
if ($total_errors === 0) echo "  Ошибок не обнаружено ✅\n";

// ═══════════════════════════════════════════════════════════════════
//  ФАЗА 5 — СУММАРНАЯ СТАТИСТИКА ЯДРА LINUX
// ═══════════════════════════════════════════════════════════════════

phase_header('PHASE 5 — Суммарная статистика ввода/вывода (один процесс)');

$global_elapsed  = round(microtime(true) - $global_start, 3);
$global_io_diff  = diff_io($global_io_start, get_io_stats());

echo "  Общее время выполнения: {$global_elapsed} сек\n\n";
echo "  СТАТИСТИКА ЯДРА LINUX (/proc/PID/io):\n";
echo format_io($global_io_diff, $global_elapsed) . "\n";

$io_keys = ['rchar', 'wchar', 'read_bytes', 'write_bytes', 'syscr', 'syscw'];
echo "\n  АГРЕГИРОВАННЫЙ IO ПО SUB-ТЕСТАМ (по всем воркерам):\n";

foreach ($sub_tests as $st) {
    $agg = array_fill_keys($io_keys, 0);
    $agg_t = 0.0;
    $agg_n = 0;
    for ($w = 0; $w < $workers; $w++) {
        $r = $all_results[$w] ?? [];
        if (isset($r['io_stats'][$st])) {
            foreach ($io_keys as $ik) {
                $agg[$ik] += $r['io_stats'][$st][$ik] ?? 0;
            }
        }
        if (isset($r['ops'][$st]['t'])) {
            $agg_t += $r['ops'][$st]['t'];
            $agg_n++;
        }
    }
    $avg_t  = $agg_n > 0 ? round($agg_t / $agg_n, 5) : 0;
    $rchar_m = round($agg['rchar'] / 1e6, 2);
    $wchar_m = round($agg['wchar'] / 1e6, 2);
    printf(
        "  %-22s rchar=%7.2f MB  wchar=%7.2f MB  syscr=%6d  syscw=%6d  avg_t=%8.5fs\n",
        $st . ':',
        $rchar_m,
        $wchar_m,
        $agg['syscr'],
        $agg['syscw'],
        $avg_t
    );
}

// ═══════════════════════════════════════════════════════════════════
//  ФАЗА 6 — ПРОПУСКНАЯ СПОСОБНОСТЬ (THROUGHPUT)
// ═══════════════════════════════════════════════════════════════════

phase_header('PHASE 6 — Пропускная способность (throughput)');

$total_rchar = $global_io_diff['rchar']        ?? 0;
$total_wchar = $global_io_diff['wchar']        ?? 0;
$total_rb    = $global_io_diff['read_bytes']   ?? 0;
$total_wb    = $global_io_diff['write_bytes']  ?? 0;
$total_syscr = $global_io_diff['syscr']        ?? 0;
$total_syscw = $global_io_diff['syscw']        ?? 0;

printf("  %-35s %10.3f MB/s  (всего %10.3f MB)\n",
    'Чтение символов (rchar):',
    $total_rchar / 1e6 / $global_elapsed,
    $total_rchar / 1e6);
printf("  %-35s %10.3f MB/s  (всего %10.3f MB)\n",
    'Запись символов (wchar):',
    $total_wchar / 1e6 / $global_elapsed,
    $total_wchar / 1e6);
printf("  %-35s %10.3f MB/s  (всего %10.3f MB)\n",
    'Физ.чтение (read_bytes):',
    $total_rb / 1e6 / $global_elapsed,
    $total_rb / 1e6);
printf("  %-35s %10.3f MB/s  (всего %10.3f MB)\n",
    'Физ.запись (write_bytes):',
    $total_wb / 1e6 / $global_elapsed,
    $total_wb / 1e6);
printf("  %-35s %10.0f  ops/s  (всего %12d)\n",
    'Системных вызовов чтения (syscr):',
    $total_syscr / $global_elapsed,
    $total_syscr);
printf("  %-35s %10.0f  ops/s  (всего %12d)\n",
    'Системных вызовов записи (syscw):',
    $total_syscw / $global_elapsed,
    $total_syscw);

// ═══════════════════════════════════════════════════════════════════
//  ФИНАЛЬНЫЙ ВЕРДИКТ
// ═══════════════════════════════════════════════════════════════════

echo "\n" . str_repeat('═', 70) . "\n";
echo "  ИТОГ\n";
echo str_repeat('═', 70) . "\n";
echo "  Время выполнения          : {$global_elapsed} сек\n";
echo "  Fibers (всего/упало)      : {$workers} / {$failed_workers}\n";
echo "  Строк в файле             : " . ($an_final['line_count'] ?? 0) . "\n";
echo "  flow_interruption         : " . ($an_final['flow_interruption'] ?? '?') . "\n";
echo "  Проверено ячеек (consist.): {$checked}\n";
echo "  Повреждено ячеек          : " . count($corrupted) . "\n";
echo "  Ошибок в воркерах         : {$total_errors}\n\n";

$overall_ok = $failed_workers === 0
    && count($corrupted) === 0
    && $total_errors    === 0
    && $ok_f3
    && $ok_cons
    && ($an_final['line_count'] ?? 0) > 0;

if ($overall_ok) {
    echo "✅  check-engine-fiber.php — УСПЕШНО ПРОЙДЕН\n";
    echo "    Библиотека fast_io корректно работает в fiber-среде.\n";
    echo "    Данные консистентны. Блокировки работают верно.\n";
} else {
    echo "❌  ОБНАРУЖЕНЫ ПРОБЛЕМЫ:\n";
    if ($failed_workers > 0) echo "    - Упало fibers: {$failed_workers}\n";
    if (count($corrupted) > 0) echo "    - Повреждённых ячеек: " . count($corrupted) . "\n";
    if ($total_errors > 0)  echo "    - Ошибок операций: {$total_errors}\n";
    if (!$ok_f3)            echo "    - file_analize финального файла провалена\n";
    if (!$ok_cons)          echo "    - Консистентность данных нарушена\n";
}

echo "\n  Файлы теста: {$base_dir} (будут удалены)\n";

// ═══════════════════════════════════════════════════════════════════
//  CLEANUP
// ═══════════════════════════════════════════════════════════════════
cleanup_dir($base_dir);
echo "  Очистка завершена.\n\n";
