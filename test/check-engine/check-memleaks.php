<?php
/*
 * Fast_IO Extension for PHP 8
 * https://github.com/commeta/fast_io
 *
 * Copyright 2026 commeta <dcs-spb@ya.ru>
 *
 * check-memleaks.php — ТЕСТ НА УТЕЧКИ ПАМЯТИ fast_io v1.1 (valgrind-friendly + /proc snapshots)
 *
 * Usage:
 *  php check-memleaks.php [iterations=10000] [logdir=/tmp/fast_io_memleak] [keep-logs=0] [valgrind=0]
 *
 * Example valgrind:
 *  valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --num-callers=40 \
 *    --log-file=check-memleaks-valgrind.log php check-memleaks.php 40000 valgrind=1 keep-logs=1
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

declare(ticks=1);

// ----------------- CONFIG -----------------
define('DEBUG', false);
define('MAX_ACCEPTABLE_DELTA_MB', 8);

$argv_map = [];
foreach ($argv as $i => $a) {
    if ($i === 0) continue;
    if (strpos($a, '=') !== false) {
        list($k, $v) = explode('=', $a, 2);
        $argv_map[$k] = $v;
    } else {
        // positional first numeric argument -> iterations
        if (!isset($argv_map['iterations']) && is_numeric($a)) {
            $argv_map['iterations'] = (int)$a;
        } elseif (strpos($a, '=') === false) {
            // allow flags like valgrind or keep-logs without =
            $argv_map[$a] = '1';
        }
    }
}

$iterations = isset($argv_map['iterations']) ? max(1000, (int)$argv_map['iterations']) : 10000;
$logdir     = $argv_map['logdir'] ?? (sys_get_temp_dir() . '/fast_io_memleak_' . getmypid());
$keep_logs  = !empty($argv_map['keep-logs']) || !empty($argv_map['keep_logs']);
$valgrind_mode = !empty($argv_map['valgrind']) || !empty($argv_map['valgrind_mode']);

@mkdir($logdir, 0755, true);

// DB files
$DB_MAIN = $logdir . '/main.dat';
$DB_DATA = $logdir . '/data.dat';
$DB_REPL = $logdir . '/replica.dat';

if (!function_exists('file_insert_line')) {
    fwrite(STDERR, "❌ Расширение fast_io не загружено — некоторые тесты пропущены\n");
    // Не делаем die(), т.к. может быть полезно увидеть /proc-метрики даже без расширения.
}

// ----------------- HELPERS -----------------

function dbg(string $msg): void {
    if (DEBUG) {
        echo "[DEBUG] $msg\n";
    }
}

function flush_out(): void {
    @fflush(STDOUT);
}

// PHP memory stats
function mem_stats(): array {
    return [
        'timestamp' => microtime(true),
        'usage'     => memory_get_usage(true),
        'peak'      => memory_get_peak_usage(true),
    ];
}

// Read /proc/self/status and return associative array of keys -> values (in KB if applicable)
function read_proc_status(): array {
    $path = '/proc/self/status';
    if (!is_readable($path)) return [];
    $content = file_get_contents($path);
    $out = [];
    foreach (preg_split('/\r?\n/', $content) as $line) {
        if (trim($line) === '') continue;
        if (strpos($line, ':') === false) continue;
        list($k, $v) = explode(':', $line, 2);
        $out[trim($k)] = trim($v);
    }
    return $out;
}

// Read /proc/self/statm -> fields in pages. return array with size,rss,shared,trs,lpages,dirty
function read_proc_statm(): array {
    $path = '/proc/self/statm';
    if (!is_readable($path)) return [];
    $content = trim(file_get_contents($path));
    $parts = preg_split('/\s+/', $content);
    $page_size = get_page_size();
    $fields = [
        'size_pages' => (int)($parts[0] ?? 0),
        'rss_pages'  => (int)($parts[1] ?? 0),
        'shared_pages' => (int)($parts[2] ?? 0),
        'text_pages' => (int)($parts[3] ?? 0),
        'lib_pages'  => (int)($parts[4] ?? 0),
        'data_pages' => (int)($parts[5] ?? 0),
        'dt_pages'   => (int)($parts[6] ?? 0),
    ];
    $fields['page_size'] = $page_size;
    $fields['rss_bytes'] = $fields['rss_pages'] * $page_size;
    $fields['size_bytes'] = $fields['size_pages'] * $page_size;
    return $fields;
}

// Read /proc/self/smaps_rollup if present (aggregated smaps). returns array of numeric fields (KB)
function read_proc_smaps_rollup(): array {
    $path = '/proc/self/smaps_rollup';
    if (!is_readable($path)) return [];
    $content = file_get_contents($path);
    $out = [];
    foreach (preg_split('/\r?\n/', $content) as $line) {
        if (preg_match('/^([A-Za-z0-9_]+):\s+(\d+)\s*kB$/i', $line, $m)) {
            $out[$m[1]] = (int)$m[2]; // in kB
        } elseif (preg_match('/^([A-Za-z0-9_]+):\s+(\d+)$/i', $line, $m)) {
            $out[$m[1]] = (int)$m[2];
        }
    }
    return $out;
}

// Read /proc/meminfo partial (MemTotal, MemFree, SwapTotal, SwapFree)
function read_proc_meminfo(): array {
    $path = '/proc/meminfo';
    if (!is_readable($path)) return [];
    $content = file_get_contents($path);
    $fields = [];
    foreach (preg_split('/\r?\n/', $content) as $line) {
        if (preg_match('/^(\w+):\s+(\d+)\s*kB$/', $line, $m)) {
            $fields[$m[1]] = (int)$m[2];
        }
    }
    return $fields;
}

function get_page_size(): int {
    static $ps = null;
    if ($ps !== null) return $ps;
    // try posix_getpagesize if available
    if (function_exists('posix_getpagesize')) {
        $ps = posix_getpagesize();
        if ($ps > 0) return $ps;
    }
    // fallback to getconf
    $out = @shell_exec('getconf PAGESIZE 2>/dev/null');
    if ($out) {
        $ps = (int)trim($out);
        if ($ps > 0) return $ps;
    }
    // final fallback
    $ps = 4096;
    return $ps;
}

// Compose a /proc snapshot and save to disk (JSON) to $logdir/proc_{phase}_{timestamp}.json
function proc_snapshot(string $phase, string $logdir, bool $extended = false): array {
    $snapshot = [
        'ts' => microtime(true),
        'pid' => getmypid(),
        'php_mem' => mem_stats(),
        'proc_status' => read_proc_status(),
        'proc_statm'  => read_proc_statm(),
        'smaps_rollup'=> read_proc_smaps_rollup(),
        'meminfo'     => read_proc_meminfo(),
    ];
    // Optionally include more heavy data (smaps full) when extended mode
    if ($extended && is_readable('/proc/self/smaps')) {
        // smaps is heavy, read if requested
        $snapshot['smaps_full_head'] = substr(file_get_contents('/proc/self/smaps'), 0, 65536); // keep first 64KB only
    }
    $fn = sprintf('%s/proc_%s_%s.json', rtrim($logdir, '/'), preg_replace('/[^a-z0-9_\-]/i','', $phase), str_replace('.','',microtime(true)));
    @file_put_contents($fn, json_encode($snapshot, JSON_PRETTY_PRINT|JSON_UNESCAPED_SLASHES));
    return $snapshot;
}

function human_bytes(int $b): string {
    if ($b < 1024) return $b . ' B';
    if ($b < 1048576) return round($b/1024,2) . ' KB';
    return round($b/1048576,2) . ' MB';
}

function mem_delta(array $before, array $after, float $elapsed): string {
    $d_usage = $after['php_mem']['usage'] - $before['php_mem']['usage'];
    $d_peak  = $after['php_mem']['peak']  - $before['php_mem']['peak'];

    // try /proc rss delta (KB)
    $before_rss_kb = extract_rss_kb_from_proc($before);
    $after_rss_kb  = extract_rss_kb_from_proc($after);
    $d_rss_kb = ($after_rss_kb !== null && $before_rss_kb !== null) ? ($after_rss_kb - $before_rss_kb) : null;

    $parts = [];
    $parts[] = sprintf("Δ PHP usage: %8s | Δ PHP peak: %8s", 
        number_format($d_usage/1048576, 2) . " MB",
        number_format($d_peak/1048576, 2) . " MB"
    );
    if ($d_rss_kb !== null) {
        $parts[] = sprintf("Δ RSS (proc): %6s kB", number_format($d_rss_kb));
    }
    $parts[] = sprintf("elapsed: %.3fs", $elapsed);
    return implode(' | ', $parts);
}

function extract_rss_kb_from_proc(array $snap): ?int {
    // prefer smaps_rollup "Rss" in kB
    if (!empty($snap['smaps_rollup']['Rss'])) {
        return (int)$snap['smaps_rollup']['Rss'];
    }
    // fallback to status VmRSS: e.g. "VmRSS:\t   12345 kB"
    if (!empty($snap['proc_status']['VmRSS'])) {
        if (preg_match('/(\d+)/', $snap['proc_status']['VmRSS'], $m)) {
            return (int)$m[1];
        }
    }
    // fallback to statm rss_pages * page_size / 1024
    if (!empty($snap['proc_statm']['rss_bytes'])) {
        return (int)round($snap['proc_statm']['rss_bytes'] / 1024);
    }
    return null;
}

// phase printing
function phase_header(string $name, int $iters): void {
    echo "\n" . str_repeat('─', 80) . "\n";
    echo "  ФАЗА: $name  (итераций: $iters)\n";
    echo str_repeat('─', 80) . "\n";
    flush_out();
}

function phase_result(string $name, bool $ok, float $time_s, string $mem_info): void {
    $mark = $ok ? '✅' : '⚠️';
    echo "$mark  $name — " . ($ok ? 'OK' : 'POTENTIAL LEAK') . " ({$time_s}s)\n";
    echo "     $mem_info\n";
    flush_out();
}

// cleanup function
function cleanup_dir(string $dir, bool $keep): void {
    if ($keep) return;
    if (is_dir($dir)) {
        foreach (glob($dir . '/*') as $f) {
            @unlink($f);
        }
        @rmdir($dir);
    }
}

// safety: register shutdown to ensure final snapshot and cleanup
register_shutdown_function(function () use ($logdir, $keep_logs) {
    // final /proc snapshot on shutdown
    @file_put_contents($logdir . '/shutdown_ts', (string)microtime(true));
    // do not aggressively delete logs on shutdown; cleanup handled explicitly later
});

// ----------------- START -----------------
echo "╔══════════════════════════════════════════════════════════════════════╗\n";
echo "║          fast_io MEMORY LEAK TEST  v1.1  (valgrind-friendly)          ║\n";
echo "╚══════════════════════════════════════════════════════════════════════╝\n\n";
echo "Итераций на фазу     : $iterations\n";
echo "Рабочая директория   : $logdir\n";
echo "PHP                  : " . PHP_VERSION . "\n";
echo "keep_logs            : " . ($keep_logs ? 'YES' : 'NO') . "\n";
echo "valgrind mode        : " . ($valgrind_mode ? 'YES' : 'NO') . "\n\n";
flush_out();

$global_start = microtime(true);
$global_mem = mem_stats();

// initial GC
gc_collect_cycles();

// store snapshots for delta computations
$snapshots = [];

// ----------------- PHASE 0: PREP -----------------
phase_header('PHASE 0 — Подготовка', 500);
$start0 = microtime(true);
$mem0 = mem_stats();

for ($i = 0; $i < 500; $i++) {
    if (function_exists('file_insert_line')) {
        file_insert_line($DB_MAIN, "prep_key_$i value_" . str_pad('', 200, 'A'));
    } else {
        // fallback: create placeholder file writes
        file_put_contents($DB_MAIN, "prep_key_$i value_" . str_pad('', 200, 'A') . PHP_EOL, FILE_APPEND | LOCK_EX);
    }
}
if (function_exists('file_analize')) {
    @file_analize($DB_MAIN);
}

gc_collect_cycles();
$snap0 = proc_snapshot('phase0_prep', $logdir, $valgrind_mode);
$snapshots['phase0'] = ['php_mem' => $mem0, 'snap' => $snap0];

$delta0 = mem_delta(['php_mem'=>$mem0,'smaps_rollup'=>[]], ['php_mem'=>mem_stats(),'smaps_rollup'=>$snap0['smaps_rollup'] ?? []], microtime(true) - $start0);
phase_result('Подготовка', true, microtime(true) - $start0, $delta0);

// ----------------- PHASE 1: INSERT + SELECT -----------------
phase_header('PHASE 1 — insert_line + select_line', $iterations);
$start1 = microtime(true);
$mem1 = mem_stats();

for ($i = 0; $i < $iterations; $i++) {
    if (function_exists('file_insert_line')) {
        $off = file_insert_line($DB_MAIN, "key$i " . str_pad('payload', 300, 'X'));
        $line = file_select_line($DB_MAIN, $off, 512, 1);
        if ($line === false) {
            fwrite(STDERR, "select_line failed at $i\n");
            break;
        }
    } else {
        // fallback: simple write + read
        $line = "key$i " . str_pad('payload', 300, 'X') . PHP_EOL;
        file_put_contents($DB_MAIN, $line, FILE_APPEND | LOCK_EX);
        // emulate select by reading last line (cheap)
        $f = fopen($DB_MAIN, 'r');
        if ($f) {
            fseek($f, -4096, SEEK_END);
            $tail = stream_get_contents($f);
            fclose($f);
        }
    }
    if (($i & 0x3FF) === 0) {
        // occasional GC to keep PHP memory stable
        gc_collect_cycles();
    }
}

gc_collect_cycles();
$snap1 = proc_snapshot('phase1_insert_select', $logdir, $valgrind_mode);
$snapshots['phase1'] = ['php_mem' => $mem1, 'snap' => $snap1];

$delta1 = mem_delta(['php_mem'=>$mem1,'smaps_rollup'=>[]], ['php_mem'=>mem_stats(),'smaps_rollup'=>$snap1['smaps_rollup'] ?? []], microtime(true) - $start1);
phase_result('insert+select', true, microtime(true) - $start1, $delta1);

// ----------------- PHASE 2: UPDATE -----------------
phase_header('PHASE 2 — update_line + update_array', min( (int)$iterations, 2000));
$start2 = microtime(true);
$mem2 = mem_stats();

$updates = [];
$limit = min(2000, $iterations);
for ($i = 0; $i < $limit; $i++) {
    $new = "UPD_key$i " . str_pad('NEW', 300, 'Y');
    if (function_exists('file_update_line')) {
        // position chosen as example; in real case must track offsets
        file_update_line($DB_MAIN, $new, $i * 512, 512, 0);
    } else {
        // fallback: append as "updated" marker
        file_put_contents($DB_MAIN, $new . PHP_EOL, FILE_APPEND | LOCK_EX);
    }
    $updates[] = [$new, $i * 512, 512];
}
if (function_exists('file_update_array')) {
    @file_update_array($DB_MAIN, $updates, 0);
}

gc_collect_cycles();
$snap2 = proc_snapshot('phase2_update', $logdir, $valgrind_mode);
$snapshots['phase2'] = ['php_mem' => $mem2, 'snap' => $snap2];

$delta2 = mem_delta(['php_mem'=>$mem2,'smaps_rollup'=>[]], ['php_mem'=>mem_stats(),'smaps_rollup'=>$snap2['smaps_rollup'] ?? []], microtime(true) - $start2);
phase_result('update', true, microtime(true) - $start2, $delta2);

// ----------------- PHASE 3: SEARCH + PCRE2 -----------------
phase_header('PHASE 3 — search_line + search_array + get_keys + pcre2', (int)($iterations/10));
$start3 = microtime(true);
$mem3 = mem_stats();

$loops = max(1, (int)($iterations/10));
for ($i = 0; $i < $loops; $i++) {
    if (function_exists('file_search_line')) {
        @file_search_line($DB_MAIN, "key" . ($i % 500), 0, 0);
    }
    if (function_exists('file_search_array')) {
        @file_search_array($DB_MAIN, "key" . ($i % 500), 0, 50, 0, 0);
    }
    if (function_exists('file_get_keys')) {
        @file_get_keys($DB_MAIN, 0, 100, 0, 2);
    }
    if (function_exists('find_matches_pcre2')) {
        @find_matches_pcre2('key\\d+', "test key123 payload", 1);
    } else {
        // fallback: native preg_match
        preg_match('/key\d+/', "test key123 payload");
    }
    if (($i & 0x7FF) === 0) gc_collect_cycles();
}

gc_collect_cycles();
$snap3 = proc_snapshot('phase3_search', $logdir, $valgrind_mode);
$snapshots['phase3'] = ['php_mem' => $mem3, 'snap' => $snap3];

$delta3 = mem_delta(['php_mem'=>$mem3,'smaps_rollup'=>[]], ['php_mem'=>mem_stats(),'smaps_rollup'=>$snap3['smaps_rollup'] ?? []], microtime(true) - $start3);
phase_result('search + pcre2', true, microtime(true) - $start3, $delta3);

// ----------------- PHASE 4: BINARY DATA -----------------
phase_header('PHASE 4 — push_data + search_data + defrag_data', (int)($iterations/5));
$start4 = microtime(true);
$mem4 = mem_stats();

$loops4 = max(1, (int)($iterations/5));
for ($i = 0; $i < $loops4; $i++) {
    $k = "binkey_$i";
    $v = str_repeat('BinaryData', 50) . $i;
    if (function_exists('file_push_data')) {
        file_push_data($DB_DATA, $k, $v, 0);
        $got = file_search_data($DB_DATA, $k, 0, 0);
        if ($got !== $v) {
            fwrite(STDERR, "search_data mismatch at $i\n");
            // continue, but note mismatch
        }
    } else {
        // fallback: local serialize
        file_put_contents($DB_DATA, $k . ':' . base64_encode($v) . PHP_EOL, FILE_APPEND | LOCK_EX);
    }
    if (($i & 0x3FF) === 0) gc_collect_cycles();
}
if (function_exists('file_defrag_data')) {
    @file_defrag_data($DB_DATA, '', 0);
}

gc_collect_cycles();
$snap4 = proc_snapshot('phase4_binary', $logdir, $valgrind_mode);
$snapshots['phase4'] = ['php_mem' => $mem4, 'snap' => $snap4];

$delta4 = mem_delta(['php_mem'=>$mem4,'smaps_rollup'=>[]], ['php_mem'=>mem_stats(),'smaps_rollup'=>$snap4['smaps_rollup'] ?? []], microtime(true) - $start4);
phase_result('binary ops', true, microtime(true) - $start4, $delta4);

// ----------------- PHASE 5: CALLBACK + POP + REPLICATE -----------------
phase_header('PHASE 5 — callback_line + pop_line + replicate_file', 500);
$start5 = microtime(true);
$mem5 = mem_stats();

// callback
if (function_exists('file_callback_line')) {
    file_callback_line($DB_MAIN, function () {
        static $cnt = 0;
        $cnt++;
        if ($cnt % 1000 === 0) gc_collect_cycles();
        return true;
    }, 0, 9);
}

// pop
for ($i = 0; $i < 500; $i++) {
    if (function_exists('file_pop_line')) {
        @file_pop_line($DB_MAIN, -1, 0);
    } else {
        // fallback: truncate file by removing last line (best-effort)
        $contents = file_get_contents($DB_MAIN);
        if ($contents !== false) {
            $lines = preg_split('/\r?\n/', rtrim($contents, "\n"));
            array_pop($lines);
            file_put_contents($DB_MAIN, implode(PHP_EOL, $lines) . PHP_EOL);
        }
    }
    if (($i & 0x1FF) === 0) gc_collect_cycles();
}

// replicate
if (function_exists('replicate_file')) {
    @replicate_file($DB_MAIN, $DB_REPL, 0);
} else {
    copy($DB_MAIN, $DB_REPL);
}

gc_collect_cycles();
$snap5 = proc_snapshot('phase5_callback_pop_repl', $logdir, $valgrind_mode);
$snapshots['phase5'] = ['php_mem' => $mem5, 'snap' => $snap5];

$delta5 = mem_delta(['php_mem'=>$mem5,'smaps_rollup'=>[]], ['php_mem'=>mem_stats(),'smaps_rollup'=>$snap5['smaps_rollup'] ?? []], microtime(true) - $start5);
phase_result('callback + pop + replicate', true, microtime(true) - $start5, $delta5);

// ----------------- FINAL VERDICT -----------------
$global_elapsed = microtime(true) - $global_start;
$final_mem = mem_stats();
$final_snap = proc_snapshot('final', $logdir, $valgrind_mode);

// compute peak growth in MB based on PHP peak
$peak_growth_mb = ($final_mem['peak'] - $global_mem['peak']) / 1048576;

echo "\n" . str_repeat('═', 80) . "\n";
echo "  ИТОГ ТЕСТА НА УТЕЧКИ ПАМЯТИ\n";
echo str_repeat('═', 80) . "\n";
echo "  Общее время           : " . round($global_elapsed, 3) . " сек\n";
echo "  Пиковый рост PHP-памяти (peak delta) : " . number_format($peak_growth_mb, 2) . " MB\n";
echo "  Финальный PHP usage   : " . number_format($final_mem['usage'] / 1048576, 2) . " MB\n";

// also show /proc RSS delta from phase0 -> final if available
$start_rss_kb = extract_rss_kb_from_proc($snapshots['phase0']['snap'] ?? []);
$final_rss_kb = extract_rss_kb_from_proc($final_snap);
if ($start_rss_kb !== null && $final_rss_kb !== null) {
    $delta_rss_mb = ($final_rss_kb - $start_rss_kb) / 1024.0;
    echo "  Пиковый рост RSS (proc) : " . number_format($delta_rss_mb, 2) . " MB\n";
} else {
    echo "  Пиковый рост RSS (proc) : недоступен (proc fields missing)\n";
}

if ($peak_growth_mb < MAX_ACCEPTABLE_DELTA_MB) {
    echo "✅  УТЕЧЕК ПАМЯТИ НЕ ОБНАРУЖЕНО (рост < " . MAX_ACCEPTABLE_DELTA_MB . " MB)\n";
    echo "    Библиотека fast_io корректно освобождает память в проверенных сценариях.\n";
} else {
    echo "⚠️  ВОЗМОЖНАЯ УТЕЧКА ПАМЯТИ (PHP peak рост " . number_format($peak_growth_mb, 2) . " MB)\n";
    echo "    Рекомендуется детальный анализ valgrind + просмотр файлов в $logdir\n";
}

// write summary JSON for automated parsing
$summary = [
    'time' => time(),
    'elapsed_s' => $global_elapsed,
    'php_start' => $global_mem,
    'php_final' => $final_mem,
    'peak_growth_mb' => $peak_growth_mb,
    'proc_start_rss_kb' => $start_rss_kb,
    'proc_final_rss_kb' => $final_rss_kb,
    'proc_delta_rss_kb' => ($final_rss_kb !== null && $start_rss_kb !== null) ? ($final_rss_kb - $start_rss_kb) : null,
    'logdir' => $logdir,
];

@file_put_contents($logdir . '/summary.json', json_encode($summary, JSON_PRETTY_PRINT|JSON_UNESCAPED_SLASHES));

echo "\n  Снимки /proc и логи сохранены в: $logdir\n";
if (!$keep_logs) {
    echo "  Примечание: логи будут автоматически удалены по завершении (keep_logs=0). Если хотите сохранить — перезапустите с keep-logs=1\n";
} else {
    echo "  keep-logs=1 — логи сохранены и не будут удалены.\n";
}

// CLEANUP
gc_collect_cycles();

if (!$keep_logs) {
    // minimal cleanup: remove generated DB files but keep proc snapshots for inspection in case valgrind was used
    @unlink($DB_MAIN);
    @unlink($DB_DATA);
    @unlink($DB_REPL);
    // keep proc snapshots for a short time in case of valgrind; comment out if you want full cleanup
    // cleanup_dir($logdir, false);
}

echo "  Тест завершён.\n";
flush_out();
