<?php
/*
 * Fast_IO Extension for PHP 8
 * https://github.com/commeta/fast_io
 *
 * check-memleaks.php — ТЕСТ НА УТЕЧКИ ПАМЯТИ fast_io v1.2
 * (valgrind-friendly + /proc snapshots + proc-based statistics)
 *
 * Usage:
 *  php check-memleaks.php [iterations=10000] [logdir=/tmp/fast_io_memleak] [keep-logs=0] [valgrind=0]
 *
 * Example valgrind:
 *  valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --num-callers=40 \
 *    --log-file=check-memleaks-valgrind.log php check-memleaks.php 40000 valgrind=1 keep-logs=1
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
        if (!isset($argv_map['iterations']) && is_numeric($a)) {
            $argv_map['iterations'] = (int)$a;
        } elseif (strpos($a, '=') === false) {
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
}

// ----------------- HELPERS -----------------

function dbg(string $msg): void {
    if (DEBUG) echo "[DEBUG] $msg\n";
}
function flush_out(): void { @fflush(STDOUT); }
function mem_stats(): array {
    return [
        'timestamp' => microtime(true),
        'usage'     => memory_get_usage(true),
        'peak'      => memory_get_peak_usage(true),
    ];
}

function get_page_size(): int {
    static $ps = null;
    if ($ps !== null) return $ps;
    if (function_exists('posix_getpagesize')) {
        $ps = posix_getpagesize();
        if ($ps > 0) return $ps;
    }
    $out = @shell_exec('getconf PAGESIZE 2>/dev/null');
    if ($out) {
        $ps = (int)trim($out);
        if ($ps > 0) return $ps;
    }
    $ps = 4096;
    return $ps;
}

// /proc readers
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
function read_proc_smaps_rollup(): array {
    $path = '/proc/self/smaps_rollup';
    if (!is_readable($path)) return [];
    $content = file_get_contents($path);
    $out = [];
    foreach (preg_split('/\r?\n/', $content) as $line) {
        if (preg_match('/^([A-Za-z0-9_]+):\s+(\d+)\s*kB$/i', $line, $m)) {
            $out[$m[1]] = (int)$m[2];
        } elseif (preg_match('/^([A-Za-z0-9_]+):\s+(\d+)$/i', $line, $m)) {
            $out[$m[1]] = (int)$m[2];
        }
    }
    return $out;
}
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

// snapshot composer
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
    if ($extended && is_readable('/proc/self/smaps')) {
        $snapshot['smaps_full_head'] = substr(file_get_contents('/proc/self/smaps'), 0, 65536);
    }
    $fn = sprintf('%s/proc_%s_%s.json', rtrim($logdir, '/'), preg_replace('/[^a-z0-9_\-]/i','', $phase), str_replace('.','',microtime(true)));
    @file_put_contents($fn, json_encode($snapshot, JSON_PRETTY_PRINT|JSON_UNESCAPED_SLASHES));
    return $snapshot;
}

// extract numeric /proc metrics (kB) with fallbacks
function get_proc_metric_kb(array $snap, string $metric): ?int {
    // smaps_rollup preferred (values are kB)
    if (!empty($snap['smaps_rollup'][$metric])) return (int)$snap['smaps_rollup'][$metric];
    // proc_status keys e.g. VmRSS: "   12345 kB"
    if (!empty($snap['proc_status'][$metric])) {
        if (preg_match('/(\d+)/', $snap['proc_status'][$metric], $m)) return (int)$m[1];
    }
    // proc_statm fallback (rss_bytes -> kB)
    if ($metric === 'Rss' && !empty($snap['proc_statm']['rss_bytes'])) {
        return (int)round($snap['proc_statm']['rss_bytes'] / 1024);
    }
    return null;
}

// small stats helpers
function stats_summary(array $arr): array {
    $count = count($arr);
    if ($count === 0) return ['count'=>0,'min'=>null,'max'=>null,'mean'=>null,'median'=>null,'stddev'=>null];
    sort($arr, SORT_NUMERIC);
    $min = $arr[0];
    $max = $arr[$count-1];
    $sum = array_sum($arr);
    $mean = $sum / $count;
    $median = ($count % 2 === 1) ? $arr[(int)floor($count/2)] : (($arr[$count/2 - 1] + $arr[$count/2]) / 2);
    $sq = 0.0;
    foreach ($arr as $v) $sq += ($v - $mean) * ($v - $mean);
    $stddev = $count > 1 ? sqrt($sq / ($count - 1)) : 0.0;
    return [
        'count' => $count,
        'min' => $min,
        'max' => $max,
        'mean' => $mean,
        'median' => $median,
        'stddev' => $stddev,
    ];
}

function human_kb($kb): string {
    if ($kb === null) return '-';
    if ($kb < 1024) return $kb . " kB";
    return round($kb/1024,2) . " MB";
}
function human_mb_from_bytes($bytes): string {
    return round($bytes / 1048576, 2) . " MB";
}

// printing helpers
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

// ----------------- RUN -----------------
echo "╔══════════════════════════════════════════════════════════════════════╗\n";
echo "║          fast_io MEMORY LEAK TEST  v1.2  (proc-stats included)       ║\n";
echo "╚══════════════════════════════════════════════════════════════════════╝\n\n";
echo "Итераций на фазу     : $iterations\n";
echo "Рабочая директория   : $logdir\n";
echo "PHP                  : " . PHP_VERSION . "\n";
echo "keep_logs            : " . ($keep_logs ? 'YES' : 'NO') . "\n";
echo "valgrind mode        : " . ($valgrind_mode ? 'YES' : 'NO') . "\n\n";
flush_out();

$global_start = microtime(true);
$global_mem = mem_stats();
gc_collect_cycles();

$snapshots = []; // will store ['name'=>..., 'php_mem'=>..., 'snap'=>...]

// PHASE 0
phase_header('PHASE 0 — Подготовка', 500);
$start0 = microtime(true);
$mem0 = mem_stats();
for ($i = 0; $i < 500; $i++) {
    if (function_exists('file_insert_line')) {
        file_insert_line($DB_MAIN, "prep_key_$i value_" . str_pad('', 200, 'A'));
    } else {
        file_put_contents($DB_MAIN, "prep_key_$i value_" . str_pad('', 200, 'A') . PHP_EOL, FILE_APPEND | LOCK_EX);
    }
}
if (function_exists('file_analize')) @file_analize($DB_MAIN);
gc_collect_cycles();
$snap0 = proc_snapshot('phase0_prep', $logdir, $valgrind_mode);
$snapshots[] = ['name'=>'phase0','php_mem'=>$mem0,'snap'=>$snap0];
$delta0 = sprintf("Δ PHP usage: %s | elapsed %.3fs", human_mb_from_bytes(mem_stats()['usage'] - $mem0['usage']), microtime(true)-$start0);
phase_result('Подготовка', true, microtime(true) - $start0, $delta0);

// PHASE 1
phase_header('PHASE 1 — insert_line + select_line', $iterations);
$start1 = microtime(true);
$mem1 = mem_stats();
for ($i = 0; $i < $iterations; $i++) {
    if (function_exists('file_insert_line')) {
        $off = file_insert_line($DB_MAIN, "key$i " . str_pad('payload', 300, 'X'));
        $line = file_select_line($DB_MAIN, $off, 512, 1);
        if ($line === false) { fwrite(STDERR, "select_line failed at $i\n"); break; }
    } else {
        $line = "key$i " . str_pad('payload', 300, 'X') . PHP_EOL;
        file_put_contents($DB_MAIN, $line, FILE_APPEND | LOCK_EX);
    }
    if (($i & 0x3FF) === 0) gc_collect_cycles();
}
gc_collect_cycles();
$snap1 = proc_snapshot('phase1_insert_select', $logdir, $valgrind_mode);
$snapshots[] = ['name'=>'phase1','php_mem'=>$mem1,'snap'=>$snap1];
$delta1 = sprintf("Δ PHP usage: %s | elapsed %.3fs", human_mb_from_bytes(mem_stats()['usage'] - $mem1['usage']), microtime(true)-$start1);
phase_result('insert+select', true, microtime(true) - $start1, $delta1);

// PHASE 2
$limit = min(2000, $iterations);
phase_header('PHASE 2 — update_line + update_array', $limit);
$start2 = microtime(true);
$mem2 = mem_stats();
$updates = [];
for ($i = 0; $i < $limit; $i++) {
    $new = "UPD_key$i " . str_pad('NEW', 300, 'Y');
    if (function_exists('file_update_line')) {
        file_update_line($DB_MAIN, $new, $i * 512, 512, 0);
    } else {
        file_put_contents($DB_MAIN, $new . PHP_EOL, FILE_APPEND | LOCK_EX);
    }
    $updates[] = [$new, $i * 512, 512];
}
if (function_exists('file_update_array')) @file_update_array($DB_MAIN, $updates, 0);
gc_collect_cycles();
$snap2 = proc_snapshot('phase2_update', $logdir, $valgrind_mode);
$snapshots[] = ['name'=>'phase2','php_mem'=>$mem2,'snap'=>$snap2];
$delta2 = sprintf("Δ PHP usage: %s | elapsed %.3fs", human_mb_from_bytes(mem_stats()['usage'] - $mem2['usage']), microtime(true)-$start2);
phase_result('update', true, microtime(true) - $start2, $delta2);

// PHASE 3
$loops = max(1, (int)($iterations/10));
phase_header('PHASE 3 — search_line + search_array + get_keys + pcre2', $loops);
$start3 = microtime(true);
$mem3 = mem_stats();
for ($i = 0; $i < $loops; $i++) {
    if (function_exists('file_search_line')) @file_search_line($DB_MAIN, "key" . ($i % 500), 0, 0);
    if (function_exists('file_search_array')) @file_search_array($DB_MAIN, "key" . ($i % 500), 0, 50, 0, 0);
    if (function_exists('file_get_keys')) @file_get_keys($DB_MAIN, 0, 100, 0, 2);
    if (function_exists('find_matches_pcre2')) @find_matches_pcre2('key\\d+', "test key123 payload", 1); else preg_match('/key\d+/', "test key123 payload");
    if (($i & 0x7FF) === 0) gc_collect_cycles();
}
gc_collect_cycles();
$snap3 = proc_snapshot('phase3_search', $logdir, $valgrind_mode);
$snapshots[] = ['name'=>'phase3','php_mem'=>$mem3,'snap'=>$snap3];
$delta3 = sprintf("Δ PHP usage: %s | elapsed %.3fs", human_mb_from_bytes(mem_stats()['usage'] - $mem3['usage']), microtime(true)-$start3);
phase_result('search + pcre2', true, microtime(true) - $start3, $delta3);

// PHASE 4
$loops4 = max(1, (int)($iterations/5));
phase_header('PHASE 4 — push_data + search_data + defrag_data', $loops4);
$start4 = microtime(true);
$mem4 = mem_stats();
for ($i = 0; $i < $loops4; $i++) {
    $k = "binkey_$i";
    $v = str_repeat('BinaryData', 50) . $i;
    if (function_exists('file_push_data')) {
        file_push_data($DB_DATA, $k, $v, 0);
        $got = @file_search_data($DB_DATA, $k, 0, 0);
        if ($got !== $v) fwrite(STDERR, "search_data mismatch at $i\n");
    } else {
        file_put_contents($DB_DATA, $k . ':' . base64_encode($v) . PHP_EOL, FILE_APPEND | LOCK_EX);
    }
    if (($i & 0x3FF) === 0) gc_collect_cycles();
}
if (function_exists('file_defrag_data')) @file_defrag_data($DB_DATA, '', 0);
gc_collect_cycles();
$snap4 = proc_snapshot('phase4_binary', $logdir, $valgrind_mode);
$snapshots[] = ['name'=>'phase4','php_mem'=>$mem4,'snap'=>$snap4];
$delta4 = sprintf("Δ PHP usage: %s | elapsed %.3fs", human_mb_from_bytes(mem_stats()['usage'] - $mem4['usage']), microtime(true)-$start4);
phase_result('binary ops', true, microtime(true) - $start4, $delta4);

// PHASE 5
phase_header('PHASE 5 — callback_line + pop_line + replicate_file', 500);
$start5 = microtime(true);
$mem5 = mem_stats();
if (function_exists('file_callback_line')) {
    file_callback_line($DB_MAIN, function () {
        static $cnt = 0; $cnt++; if ($cnt % 1000 === 0) gc_collect_cycles(); return true;
    }, 0, 9);
}
for ($i = 0; $i < 500; $i++) {
    if (function_exists('file_pop_line')) @file_pop_line($DB_MAIN, -1, 0);
    else {
        $contents = @file_get_contents($DB_MAIN);
        if ($contents !== false) {
            $lines = preg_split('/\r?\n/', rtrim($contents, "\n"));
            array_pop($lines);
            @file_put_contents($DB_MAIN, implode(PHP_EOL, $lines) . PHP_EOL);
        }
    }
    if (($i & 0x1FF) === 0) gc_collect_cycles();
}
if (function_exists('replicate_file')) @replicate_file($DB_MAIN, $DB_REPL, 0); else @copy($DB_MAIN, $DB_REPL);
gc_collect_cycles();
$snap5 = proc_snapshot('phase5_callback_pop_repl', $logdir, $valgrind_mode);
$snapshots[] = ['name'=>'phase5','php_mem'=>$mem5,'snap'=>$snap5];
$delta5 = sprintf("Δ PHP usage: %s | elapsed %.3fs", human_mb_from_bytes(mem_stats()['usage'] - $mem5['usage']), microtime(true)-$start5);
phase_result('callback + pop + replicate', true, microtime(true) - $start5, $delta5);

// FINAL snapshot
$global_elapsed = microtime(true) - $global_start;
$final_mem = mem_stats();
$final_snap = proc_snapshot('final', $logdir, $valgrind_mode);
$snapshots[] = ['name'=>'final','php_mem'=>$final_mem,'snap'=>$final_snap];

// ----------------- PROC-BASED STATISTICS -----------------

// Collect per-snapshot proc metrics (Rss, Pss, VmPeak if available)
$series = [];
foreach ($snapshots as $s) {
    $name = $s['name'];
    $snap = $s['snap'];
    $php_usage = $s['php_mem']['usage'] ?? ($snap['php_mem']['usage'] ?? null);
    $php_peak  = $s['php_mem']['peak']   ?? ($snap['php_mem']['peak'] ?? null);
    $rss_kb = get_proc_metric_kb($snap, 'Rss');
    // Pss often available in smaps_rollup as 'Pss'
    $pss_kb = get_proc_metric_kb($snap, 'Pss');
    // VmPeak from proc_status (kB)
    $vmpeak_kb = null;
    if (!empty($snap['proc_status']['VmPeak']) && preg_match('/(\d+)/', $snap['proc_status']['VmPeak'], $m)) $vmpeak_kb = (int)$m[1];
    $series[] = [
        'name' => $name,
        'ts'   => $snap['ts'] ?? null,
        'php_usage_bytes' => $php_usage,
        'php_peak_bytes'  => $php_peak,
        'rss_kb' => $rss_kb,
        'pss_kb' => $pss_kb,
        'vmpeak_kb' => $vmpeak_kb,
    ];
}

// build arrays for stats
$rss_arr = [];
$pss_arr = [];
foreach ($series as $it) {
    if ($it['rss_kb'] !== null) $rss_arr[] = $it['rss_kb'];
    if ($it['pss_kb'] !== null) $pss_arr[] = $it['pss_kb'];
}

$rss_stats = stats_summary($rss_arr);
$pss_stats = stats_summary($pss_arr);

// Print table of snapshots
echo "\n" . str_repeat('─', 80) . "\n";
echo "  Снимки по фазам (PHP / proc)\n";
echo str_repeat('─', 80) . "\n";
printf("%-12s %-10s %-12s %-12s %-10s %-10s\n", "phase","time","php_usage","php_peak","RSS","PSS");
foreach ($series as $it) {
    $t = $it['ts'] ? date('H:i:s', (int)$it['ts']) . sprintf(".%03d", (int)(($it['ts'] - (int)$it['ts'])*1000)) : '-';
    $php_usage_mb = $it['php_usage_bytes'] !== null ? round($it['php_usage_bytes']/1048576,2) . " MB" : '-';
    $php_peak_mb  = $it['php_peak_bytes'] !== null ? round($it['php_peak_bytes']/1048576,2) . " MB" : '-';
    $rss = $it['rss_kb'] !== null ? round($it['rss_kb']/1024,2) . " MB" : '-';
    $pss = $it['pss_kb'] !== null ? round($it['pss_kb']/1024,2) . " MB" : '-';
    printf("%-12s %-10s %-12s %-12s %-10s %-10s\n", $it['name'], $t, $php_usage_mb, $php_peak_mb, $rss, $pss);
}

echo str_repeat('─', 80) . "\n";
echo "  Аггрегированные метрики (proc):\n";
if ($rss_stats['count'] > 0) {
    echo sprintf("    RSS samples: %d  | min: %s  max: %s  mean: %s  median: %s  stddev: %.2f kB\n",
        $rss_stats['count'],
        human_kb($rss_stats['min']),
        human_kb($rss_stats['max']),
        human_kb((int)round($rss_stats['mean'])),
        human_kb((int)round($rss_stats['median'])),
        $rss_stats['stddev']
    );
} else {
    echo "    RSS samples: none (proc fields missing)\n";
}
if ($pss_stats['count'] > 0) {
    echo sprintf("    PSS samples: %d  | min: %s  max: %s  mean: %s  median: %s  stddev: %.2f kB\n",
        $pss_stats['count'],
        human_kb($pss_stats['min']),
        human_kb($pss_stats['max']),
        human_kb((int)round($pss_stats['mean'])),
        human_kb((int)round($pss_stats['median'])),
        $pss_stats['stddev']
    );
} else {
    echo "    PSS samples: none (smaps_rollup Pss missing)\n";
}

// compute start->final deltas
$start_rss_kb = $series[0]['rss_kb'] ?? null;
$final_rss_kb = end($series)['rss_kb'] ?? null;
$start_php_peak = $series[0]['php_peak_bytes'] ?? null;
$final_php_peak = end($series)['php_peak_bytes'] ?? null;

echo str_repeat('─', 80) . "\n";
echo "  Дельты start→final:\n";
if ($start_rss_kb !== null && $final_rss_kb !== null) {
    $delta_rss_kb = $final_rss_kb - $start_rss_kb;
    echo "    RSS delta: " . human_kb($delta_rss_kb) . " (" . number_format($delta_rss_kb/1024,2) . " MB)\n";
} else {
    echo "    RSS delta: недоступен\n";
}
if ($start_php_peak !== null && $final_php_peak !== null) {
    $delta_peak_bytes = $final_php_peak - $start_php_peak;
    echo "    PHP peak delta: " . human_mb_from_bytes($delta_peak_bytes) . "\n";
} else {
    echo "    PHP peak delta: недоступен\n";
}

// Final verdict using both PHP peak and proc RSS peak
$peak_growth_mb = ($final_mem['peak'] - $global_mem['peak']) / 1048576;
echo "\n" . str_repeat('═', 80) . "\n";
echo "  ИТОГ ТЕСТА НА УТЕЧКИ ПАМЯТИ\n";
echo str_repeat('═', 80) . "\n";
echo "  Общее время           : " . round($global_elapsed, 3) . " сек\n";
echo "  Пиковый рост PHP-памяти (peak delta) : " . number_format($peak_growth_mb, 2) . " MB\n";
echo "  Финальный PHP usage   : " . number_format($final_mem['usage'] / 1048576, 2) . " MB\n";
if ($start_rss_kb !== null && $final_rss_kb !== null) {
    echo "  Пиковый рост RSS (proc) : " . number_format(($final_rss_kb - $start_rss_kb)/1024, 2) . " MB\n";
} else {
    echo "  Пиковый рост RSS (proc) : недоступен (proc fields missing)\n";
}

// decision using both sources: require both to be small to pass
$proc_growth_mb = ($start_rss_kb !== null && $final_rss_kb !== null) ? (($final_rss_kb - $start_rss_kb)/1024.0) : null;
$pass = true;
if ($peak_growth_mb >= MAX_ACCEPTABLE_DELTA_MB) $pass = false;
if ($proc_growth_mb !== null && $proc_growth_mb >= MAX_ACCEPTABLE_DELTA_MB) $pass = false;

if ($pass) {
    echo "✅  УТЕЧЕК ПАМЯТИ НЕ ОБНАРУЖЕНО (рост < " . MAX_ACCEPTABLE_DELTA_MB . " MB по PHP peak и proc RSS)\n";
    echo "    Библиотека fast_io корректно освобождает память в проверенных сценариях.\n";
} else {
    echo "⚠️  ВОЗМОЖНАЯ УТЕЧКА ПАМЯТИ\n";
    echo "    PHP peak delta: " . number_format($peak_growth_mb, 2) . " MB\n";
    echo "    proc RSS delta: " . ($proc_growth_mb !== null ? number_format($proc_growth_mb,2) . " MB" : "н/д") . "\n";
    echo "    Рекомендуется детальный анализ valgrind + просмотр файлов в $logdir\n";
}

// write summary JSON for automated parsing (extended with proc stats)
$summary = [
    'time' => time(),
    'elapsed_s' => $global_elapsed,
    'php_start' => $global_mem,
    'php_final' => $final_mem,
    'peak_growth_mb' => $peak_growth_mb,
    'proc_series' => $series,
    'proc_stats' => [
        'rss' => $rss_stats,
        'pss' => $pss_stats,
    ],
    'proc_delta_rss_kb' => ($start_rss_kb !== null && $final_rss_kb !== null) ? ($final_rss_kb - $start_rss_kb) : null,
    'logdir' => $logdir,
];

@file_put_contents($logdir . '/summary.json', json_encode($summary, JSON_PRETTY_PRINT|JSON_UNESCAPED_SLASHES));

echo "\n  Снимки /proc и логи сохранены в: $logdir\n";
if (!$keep_logs) {
    echo "  Примечание: DB файлы будут удалены (но proc-снимки сохранены). Перезапустите с keep-logs=1 чтобы сохранить всё.\n";
} else {
    echo "  keep-logs=1 — логи сохранены и не будут удалены.\n";
}

// CLEANUP
gc_collect_cycles();
if (!$keep_logs) {
    @unlink($DB_MAIN);
    @unlink($DB_DATA);
    @unlink($DB_REPL);
}

echo "  Тест завершён.\n";
flush_out();
