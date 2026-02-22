<?php
/*
 * Fast_IO Extension for PHP 8
 * https://github.com/commeta/fast_io
 *
 * check-memleaks.php — ТЕСТ НА УТЕЧКИ ПАМЯТИ fast_io v2.0
 *
 * Usage:
 *   php check-memleaks.php [iterations=10000] [logdir=/tmp/fast_io_memleak] [keep-logs=0] [valgrind=0]
 *
 * Valgrind:
 *   valgrind --leak-check=full --log-file=vg.log php check-memleaks.php 4000 valgrind=1
 *
 * Особенности:
 *   - Автодетект режима Valgrind по /proc/self/status (Name=memcheck-amd64-)
 *   - Умные пороги: под Valgrind RSS-дельта от Valgrind-оверхеда игнорируется
 *   - Прогресс-бары для длительных фаз
 *   - Детальная аналитика: тренды RSS/PSS, аномалии, per-phase breakdown
 *   - Авто-разбор Valgrind лога (если передан --vg-log=...)
 */

declare(ticks=1);

// ====================== CONFIG ======================
define('VERSION', '2.0');
define('MAX_PHP_DELTA_MB',  8.0);   // порог PHP peak delta (утечка)
define('MAX_RSS_DELTA_MB',  8.0);   // порог RSS delta (нативный режим)
define('MAX_RSS_DELTA_VG',  512.0); // порог RSS delta под Valgrind (его own overhead)
define('PROGRESS_WIDTH',    40);    // ширина прогресс-бара

// ====================== ARGS ======================
$argv_map = [];
foreach ($argv ?? [] as $i => $a) {
    if ($i === 0) continue;
    if (strpos($a, '=') !== false) {
        [$k, $v] = explode('=', $a, 2);
        $argv_map[$k] = $v;
    } elseif (is_numeric($a) && !isset($argv_map['iterations'])) {
        $argv_map['iterations'] = (int)$a;
    } else {
        $argv_map[$a] = '1';
    }
}

$iterations   = isset($argv_map['iterations']) ? max(1000, (int)$argv_map['iterations']) : 10000;
$logdir       = $argv_map['logdir'] ?? (sys_get_temp_dir() . '/fast_io_memleak_' . getmypid());
$keep_logs    = !empty($argv_map['keep-logs']) || !empty($argv_map['keep_logs']);
$vg_log_path  = $argv_map['vg-log'] ?? $argv_map['vg_log'] ?? null;

@mkdir($logdir, 0755, true);

$DB_MAIN = $logdir . '/main.dat';
$DB_DATA = $logdir . '/data.dat';
$DB_REPL = $logdir . '/replica.dat';

$fast_io_ok = function_exists('file_insert_line');

// ====================== VALGRIND AUTO-DETECT ======================
function detect_valgrind(): bool {
    // 1. Явный флаг из командной строки / env
    global $argv_map;
    if (!empty($argv_map['valgrind'])) return true;

    // 2. /proc/self/status: Name будет "memcheck-amd64-" вместо "php"
    $status_path = '/proc/self/status';
    if (is_readable($status_path)) {
        $content = file_get_contents($status_path);
        if (preg_match('/^Name:\s*(.+)$/m', $content, $m)) {
            $name = strtolower(trim($m[1]));
            if (str_contains($name, 'memcheck') || str_contains($name, 'valgrind')) {
                return true;
            }
        }
    }

    // 3. Переменные окружения Valgrind
    foreach (['VALGRIND_OPTS', 'VALGRIND_SCRIPT_DEBUG', 'VG_DEBUG'] as $env) {
        if (getenv($env) !== false) return true;
    }

    // 4. Аномально высокое RSS при старте (Valgrind добавляет >150 MB overhead)
    $statm = read_proc_statm();
    if (!empty($statm['rss_bytes']) && $statm['rss_bytes'] > 150 * 1024 * 1024) {
        return true;
    }

    return false;
}

$is_valgrind = detect_valgrind();
$max_rss_threshold = $is_valgrind ? MAX_RSS_DELTA_VG : MAX_RSS_DELTA_MB;

// ====================== HELPERS ======================

function fmt_mb(float $bytes): string {
    return number_format($bytes / 1048576, 2) . ' MB';
}
function fmt_kb_val(?int $kb): string {
    if ($kb === null) return '—';
    if (abs($kb) < 1024) return $kb . ' kB';
    return number_format($kb / 1024, 2) . ' MB';
}
function fmt_time(float $s): string {
    return number_format($s, 3) . 's';
}

function progress_bar(int $done, int $total, int $width = PROGRESS_WIDTH, string $prefix = ''): void {
    $pct    = $total > 0 ? min(1.0, $done / $total) : 0;
    $filled = (int)($pct * $width);
    $bar    = str_repeat('█', $filled) . str_repeat('░', $width - $filled);
    $pct_s  = sprintf('%3d%%', (int)($pct * 100));
    echo "\r  {$prefix}[{$bar}] {$pct_s} {$done}/{$total}   ";
    @ob_flush(); @flush();
}

function progress_done(int $total, string $label, float $elapsed): void {
    $bar = str_repeat('█', PROGRESS_WIDTH);
    echo "\r  [{$bar}] 100% {$total}/{$total}   ✓ " . $label . " (" . fmt_time($elapsed) . ")\n";
}

function get_process_io_stats(): array {
    $f = '/proc/' . getmypid() . '/io';
    if (!is_readable($f)) return [];
    $out = [];
    foreach (explode("\n", trim(file_get_contents($f))) as $line) {
        if (!trim($line) || !str_contains($line, ':')) continue;
        [$k, $v] = explode(':', $line, 2);
        $out[trim($k)] = (int)trim($v);
    }
    return $out;
}

function read_proc_status(): array {
    $f = '/proc/self/status';
    if (!is_readable($f)) return [];
    $out = [];
    foreach (preg_split('/\r?\n/', file_get_contents($f)) as $line) {
        if (!trim($line) || !str_contains($line, ':')) continue;
        [$k, $v] = explode(':', $line, 2);
        $out[trim($k)] = trim($v);
    }
    return $out;
}

function get_page_size(): int {
    static $ps = null;
    if ($ps !== null) return $ps;
    if (function_exists('posix_getpagesize')) { $ps = posix_getpagesize(); if ($ps > 0) return $ps; }
    $out = @shell_exec('getconf PAGESIZE 2>/dev/null');
    $ps = $out ? max((int)trim($out), 4096) : 4096;
    return $ps;
}

function read_proc_statm(): array {
    $f = '/proc/self/statm';
    if (!is_readable($f)) return [];
    $parts = preg_split('/\s+/', trim(file_get_contents($f)));
    $ps = get_page_size();
    return [
        'size_pages'   => (int)($parts[0] ?? 0),
        'rss_pages'    => (int)($parts[1] ?? 0),
        'shared_pages' => (int)($parts[2] ?? 0),
        'data_pages'   => (int)($parts[5] ?? 0),
        'page_size'    => $ps,
        'rss_bytes'    => ((int)($parts[1] ?? 0)) * $ps,
        'size_bytes'   => ((int)($parts[0] ?? 0)) * $ps,
    ];
}

function read_proc_smaps_rollup(): array {
    $f = '/proc/self/smaps_rollup';
    if (!is_readable($f)) return [];
    $out = [];
    foreach (preg_split('/\r?\n/', file_get_contents($f)) as $line) {
        if (preg_match('/^([A-Za-z0-9_]+):\s+(\d+)/i', $line, $m)) $out[$m[1]] = (int)$m[2];
    }
    return $out;
}

function mem_stats(): array {
    return ['ts' => microtime(true), 'usage' => memory_get_usage(true), 'peak' => memory_get_peak_usage(true)];
}

function get_proc_metric_kb(array $snap, string $metric): ?int {
    if (!empty($snap['smaps_rollup'][$metric])) return (int)$snap['smaps_rollup'][$metric];
    if (!empty($snap['proc_status'][$metric]) && preg_match('/(\d+)/', $snap['proc_status'][$metric], $m)) return (int)$m[1];
    if ($metric === 'Rss' && !empty($snap['proc_statm']['rss_bytes'])) return (int)round($snap['proc_statm']['rss_bytes'] / 1024);
    return null;
}

function proc_snapshot(string $phase, string $logdir, bool $extended = false): array {
    $snap = [
        'ts'           => microtime(true),
        'pid'          => getmypid(),
        'php_mem'      => mem_stats(),
        'proc_status'  => read_proc_status(),
        'proc_statm'   => read_proc_statm(),
        'smaps_rollup' => read_proc_smaps_rollup(),
    ];
    $safe_phase = preg_replace('/[^a-z0-9_\-]/i', '', $phase);
    $fn = sprintf('%s/proc_%s_%s.json', rtrim($logdir, '/'), $safe_phase, str_replace('.', '', (string)microtime(true)));
    @file_put_contents($fn, json_encode($snap, JSON_PRETTY_PRINT | JSON_UNESCAPED_SLASHES));
    return $snap;
}

function stats_summary(array $arr): array {
    $n = count($arr);
    if ($n === 0) return ['count' => 0, 'min' => null, 'max' => null, 'mean' => null, 'median' => null, 'stddev' => null, 'trend' => null];
    sort($arr, SORT_NUMERIC);
    $sum  = array_sum($arr);
    $mean = $sum / $n;
    $med  = $n % 2 === 1 ? $arr[(int)($n / 2)] : ($arr[$n / 2 - 1] + $arr[$n / 2]) / 2;
    $sq   = 0.0;
    foreach ($arr as $v) $sq += ($v - $mean) ** 2;
    // Простой линейный тренд (наклон, kB/снимок)
    $trend = null;
    if ($n > 1) {
        $xm = ($n - 1) / 2;
        $num = $den = 0;
        foreach ($arr as $i => $v) { $num += ($i - $xm) * ($v - $mean); $den += ($i - $xm) ** 2; }
        $trend = $den > 0 ? round($num / $den, 1) : 0;
    }
    return ['count' => $n, 'min' => $arr[0], 'max' => $arr[$n - 1], 'mean' => $mean, 'median' => $med,
            'stddev' => $n > 1 ? sqrt($sq / ($n - 1)) : 0.0, 'trend' => $trend];
}

// ====================== DISPLAY ======================

function phase_header(string $name, int $iters): void {
    $w = 80;
    echo "\n" . str_repeat('─', $w) . "\n";
    $label = "  ФАЗА: {$name}  (итераций: {$iters})";
    echo $label . "\n";
    echo str_repeat('─', $w) . "\n";
    @ob_flush(); @flush();
}

function phase_result(string $label, bool $ok, float $time_s, array $mem_before, array $mem_after,
                      ?int $rss_before_kb, ?int $rss_after_kb): void {
    $mark      = $ok ? '✅' : '⚠️ ';
    $status    = $ok ? 'OK' : 'ВНИМАНИЕ';
    $php_delta = $mem_after['usage'] - $mem_before['usage'];
    $php_peak  = $mem_after['peak'];
    $rss_delta = ($rss_before_kb !== null && $rss_after_kb !== null) ? ($rss_after_kb - $rss_before_kb) : null;

    echo "\n{$mark} {$label} — {$status}\n";
    printf("     Время: %-10s  PHP usage: %-10s  PHP peak: %-10s\n",
        fmt_time($time_s),
        fmt_mb($mem_after['usage']),
        fmt_mb($php_peak)
    );
    printf("     PHP Δ: %-10s  RSS: %-12s  RSS Δ: %s\n",
        ($php_delta >= 0 ? '+' : '') . fmt_mb($php_delta),
        $rss_after_kb !== null ? fmt_kb_val($rss_after_kb) : '—',
        $rss_delta !== null ? (($rss_delta >= 0 ? '+' : '') . fmt_kb_val($rss_delta)) : '—'
    );
    @ob_flush(); @flush();
}

// ====================== VALGRIND LOG PARSER ======================

function parse_valgrind_log(string $path): ?array {
    if (!is_readable($path)) return null;
    $content = file_get_contents($path);

    $result = [
        'path'             => $path,
        'pid'              => null,
        'definitely_lost'  => 0,
        'definitely_blocks'=> 0,
        'indirectly_lost'  => 0,
        'indirectly_blocks'=> 0,
        'possibly_lost'    => 0,
        'possibly_blocks'  => 0,
        'still_reachable'  => 0,
        'still_blocks'     => 0,
        'suppressed'       => 0,
        'error_count'      => 0,
        'total_allocs'     => 0,
        'total_frees'      => 0,
        'total_bytes_alloc'=> 0,
        'heap_in_use_exit' => 0,
        'heap_in_use_blocks'=> 0,
        'fast_io_leaks'    => [],
        'php_intern_leaks' => [],
        'raw_summary'      => '',
    ];

    // PID
    if (preg_match('/==(\d+)==\s*Memcheck/', $content, $m)) $result['pid'] = (int)$m[1];

    // HEAP SUMMARY
    if (preg_match('/in use at exit:\s*([\d,]+)\s*bytes in\s*([\d,]+)\s*blocks/i', $content, $m)) {
        $result['heap_in_use_exit']   = (int)str_replace(',', '', $m[1]);
        $result['heap_in_use_blocks'] = (int)str_replace(',', '', $m[2]);
    }
    if (preg_match('/total heap usage:\s*([\d,]+)\s*allocs,\s*([\d,]+)\s*frees,\s*([\d,]+)\s*bytes/i', $content, $m)) {
        $result['total_allocs'] = (int)str_replace(',', '', $m[1]);
        $result['total_frees']  = (int)str_replace(',', '', $m[2]);
        $result['total_bytes_alloc'] = (int)str_replace(',', '', $m[3]);
    }

    // LEAK SUMMARY
    $patterns = [
        'definitely_lost'   => '/definitely lost:\s*([\d,]+)\s*bytes in\s*([\d,]+)/i',
        'indirectly_lost'   => '/indirectly lost:\s*([\d,]+)\s*bytes in\s*([\d,]+)/i',
        'possibly_lost'     => '/possibly lost:\s*([\d,]+)\s*bytes in\s*([\d,]+)/i',
        'still_reachable'   => '/still reachable:\s*([\d,]+)\s*bytes in\s*([\d,]+)/i',
        'suppressed'        => '/suppressed:\s*([\d,]+)\s*bytes/i',
    ];
    foreach ($patterns as $key => $pat) {
        if (preg_match($pat, $content, $m)) {
            $result[$key] = (int)str_replace(',', '', $m[1]);
            if (isset($m[2])) $result[$key . '_blocks'] = (int)str_replace(',', '', $m[2]);
        }
    }

    // ERROR SUMMARY
    if (preg_match('/ERROR SUMMARY:\s*(\d+)\s*errors/i', $content, $m)) $result['error_count'] = (int)$m[1];

    // Сырая сводка
    if (preg_match('/LEAK SUMMARY:(.*?)(?:ERROR SUMMARY:|$)/si', $content, $m)) {
        $result['raw_summary'] = trim(preg_replace('/==\d+==/m', '', $m[1]));
    }

    // Классификация loss records: fast_io vs PHP internals
    preg_match_all('/==\d+==\s*[\d,]+\s*bytes.*?loss record\s*\d+.*?\n((?:==\d+==.*?\n)*)/si', $content, $records);
    foreach ($records[0] as $rec) {
        $is_fast_io = preg_match('/fast_io|PHP_FUNCTION|file_insert|file_search|file_pop|file_push|file_defrag|file_replace|file_erase|file_callback|file_select|file_update|file_get_keys|replicate_file|find_matches/i', $rec);
        $is_php     = preg_match('/zend_register_ini|dlopen|dl-open|dl-close|php_module_startup|php_load_extension/i', $rec);
        if ($is_fast_io && !$is_php) {
            $result['fast_io_leaks'][] = substr($rec, 0, 200);
        } elseif ($is_php) {
            $result['php_intern_leaks'][] = 'PHP internals (zend/dlopen)';
        }
    }
    $result['php_intern_leaks'] = array_unique($result['php_intern_leaks']);

    return $result;
}

function print_valgrind_analysis(array $vg): void {
    $w = 80;
    echo "\n" . str_repeat('═', $w) . "\n";
    echo "  АНАЛИЗ VALGRIND ЛОГА: " . basename($vg['path']) . "\n";
    echo str_repeat('═', $w) . "\n";

    printf("  PID: %s\n", $vg['pid'] ?? '?');
    printf("  Heap at exit  : %s bytes в %d блоках\n",
        number_format($vg['heap_in_use_exit']), $vg['heap_in_use_blocks']);
    printf("  Allocs/Frees  : %s / %s  (разница: %+d)\n",
        number_format($vg['total_allocs']),
        number_format($vg['total_frees']),
        $vg['total_allocs'] - $vg['total_frees']
    );
    printf("  Всего выделено: %s\n", fmt_mb($vg['total_bytes_alloc']));

    echo str_repeat('─', $w) . "\n";
    echo "  СВОДКА УТЕЧЕК:\n";

    $categories = [
        ['key' => 'definitely_lost',  'label' => 'Точно потеряно',    'critical' => true],
        ['key' => 'indirectly_lost',  'label' => 'Косвенно потеряно', 'critical' => true],
        ['key' => 'possibly_lost',    'label' => 'Возможно потеряно', 'critical' => false],
        ['key' => 'still_reachable',  'label' => 'Всё ещё достижимо','critical' => false],
        ['key' => 'suppressed',       'label' => 'Подавлено',         'critical' => false],
    ];
    foreach ($categories as $cat) {
        $bytes  = $vg[$cat['key']] ?? 0;
        $blocks = $vg[$cat['key'] . '_blocks'] ?? null;
        $icon   = $cat['critical'] ? ($bytes > 0 ? '❌' : '✅') : ($bytes > 0 ? '⚠️ ' : '✅');
        $blocks_s = $blocks !== null ? " в {$blocks} блоках" : '';
        printf("  %s %-28s : %s%s\n", $icon, $cat['label'], number_format($bytes) . ' байт', $blocks_s);
    }

    // Классификация "still reachable"
    if ($vg['still_reachable'] > 0) {
        echo str_repeat('─', $w) . "\n";
        echo "  КЛАССИФИКАЦИЯ 'still reachable':\n";
        if (count($vg['fast_io_leaks']) > 0) {
            echo "  ❌ fast_io блоки: " . count($vg['fast_io_leaks']) . " записей — ТРЕБУЕТ АНАЛИЗА!\n";
        } else {
            echo "  ✅ Блоков fast_io не найдено\n";
        }
        if (count($vg['php_intern_leaks']) > 0) {
            echo "  ℹ️  PHP internals (zend/dlopen): нормально, PHP не освобождает при завершении\n";
        }
    }

    echo str_repeat('─', $w) . "\n";
    $no_real_leak = ($vg['definitely_lost'] === 0 && $vg['indirectly_lost'] === 0);
    if ($no_real_leak && count($vg['fast_io_leaks']) === 0) {
        echo "  ✅ УТЕЧЕК ПАМЯТИ В fast_io НЕ ОБНАРУЖЕНО\n";
        echo "     'still reachable' = PHP-рантайм, освобождается ОС при выходе (нормально)\n";
    } else {
        echo "  ❌ ОБНАРУЖЕНЫ УТЕЧКИ ПАМЯТИ — требуется детальный анализ!\n";
        echo "     Изучите: " . $vg['path'] . "\n";
    }
    echo "  Ошибок Valgrind: {$vg['error_count']}\n";
}

// ====================== BANNER ======================

$w = 80;
echo "╔" . str_repeat('═', $w - 2) . "╗\n";
$title = sprintf("fast_io MEMORY LEAK TEST v%s%s", VERSION, $is_valgrind ? " [VALGRIND MODE]" : "");
$pad   = (int)(($w - 2 - strlen($title)) / 2);
echo "║" . str_repeat(' ', $pad) . $title . str_repeat(' ', $w - 2 - $pad - strlen($title)) . "║\n";
echo "╚" . str_repeat('═', $w - 2) . "╝\n\n";

printf("  Итераций на фазу   : %d\n", $iterations);
printf("  Рабочая директория : %s\n", $logdir);
printf("  PHP                : %s\n", PHP_VERSION);
printf("  fast_io загружен   : %s\n", $fast_io_ok ? 'ДА' : 'НЕТ (fallback mode)');
printf("  Режим Valgrind     : %s\n", $is_valgrind ? '✅ ДА (автодетект)' : 'нет');
printf("  keep_logs          : %s\n", $keep_logs ? 'ДА' : 'нет');
if ($is_valgrind) {
    printf("  Порог RSS-дельты   : %s (VG-режим, оверхед Valgrind ~150-250 MB игнорируется)\n",
        fmt_mb(MAX_RSS_DELTA_VG * 1048576));
} else {
    printf("  Порог RSS-дельты   : %s\n", fmt_mb(MAX_RSS_DELTA_MB * 1048576));
}
echo "\n";

if ($is_valgrind) {
    echo "  ℹ️  Valgrind добавляет 150-250 MB RSS overhead (shadow memory).\n";
    echo "     Это ожидаемо и НЕ является утечкой fast_io.\n";
    echo "     Ориентируемся на: PHP peak delta + 'definitely/indirectly lost' в VG-логе.\n\n";
}

// ====================== RUNTIME ======================

$global_start = microtime(true);
$global_mem   = mem_stats();
gc_collect_cycles();

$snapshots = [];
$phase_io_stats = [];

// ====================== PHASE 0: Подготовка ======================
$PREP_N = 500;
phase_header('PHASE 0 — Подготовка', $PREP_N);
$t0 = microtime(true);
$m0 = mem_stats();
$io_start = get_process_io_stats();
$rss_p0_before = get_proc_metric_kb(proc_snapshot('measure', $logdir), 'Rss');

for ($i = 0; $i < $PREP_N; $i++) {
    if ($fast_io_ok) {
        file_insert_line($DB_MAIN, "prep_key_$i value_" . str_pad('', 200, 'A'));
    } else {
        file_put_contents($DB_MAIN, "prep_key_$i value_" . str_pad('', 200, 'A') . PHP_EOL, FILE_APPEND | LOCK_EX);
    }
    if ($i % 50 === 0) progress_bar($i + 1, $PREP_N, PROGRESS_WIDTH, 'Prep ');
}
progress_done($PREP_N, 'insert_line', microtime(true) - $t0);
if ($fast_io_ok) @file_analize($DB_MAIN);
gc_collect_cycles();
$snap0 = proc_snapshot('phase0_prep', $logdir);
$io_end = get_process_io_stats();

$phase_io_stats['phase0'] = array_map(fn($k, $v) => $v - ($io_start[$k] ?? 0), array_keys($io_end), $io_end);
$snapshots[] = ['name' => 'phase0', 'php_mem' => $m0, 'snap' => $snap0];
$rss_p0_after = get_proc_metric_kb($snap0, 'Rss');
phase_result('Подготовка', true, microtime(true) - $t0, $m0, mem_stats(), $rss_p0_before, $rss_p0_after);

// ====================== PHASE 1: insert_line + select_line ======================
phase_header('PHASE 1 — insert_line + select_line', $iterations);
$t1 = microtime(true);
$m1 = mem_stats();
$rss_p1_before = get_proc_metric_kb($snap0, 'Rss');
$mem_track = [];
$err_count = 0;

for ($i = 0; $i < $iterations; $i++) {
    if ($fast_io_ok) {
        $off  = file_insert_line($DB_MAIN, "key$i " . str_pad('payload', 300, 'X'));
        $line = file_select_line($DB_MAIN, $off, 512, 1);
        if ($line === false) { $err_count++; }
    } else {
        $line = "key$i payload\n";
        file_put_contents($DB_MAIN, $line, FILE_APPEND | LOCK_EX);
    }

    if ($i % 500 === 0) {
        gc_collect_cycles();
        $mem_track[] = memory_get_usage(true);
        progress_bar($i + 1, $iterations, PROGRESS_WIDTH, 'P1 ');
    }
}
progress_done($iterations, 'insert+select', microtime(true) - $t1);
gc_collect_cycles();
$snap1 = proc_snapshot('phase1_insert_select', $logdir);
$snapshots[] = ['name' => 'phase1', 'php_mem' => $m1, 'snap' => $snap1];
$rss_p1_after = get_proc_metric_kb($snap1, 'Rss');

$mem_growth = count($mem_track) > 1 ? end($mem_track) - $mem_track[0] : 0;
phase_result('insert+select', $err_count === 0, microtime(true) - $t1, $m1, mem_stats(), $rss_p1_before, $rss_p1_after);
if ($err_count > 0) printf("     ⚠️  Ошибок в фазе: %d\n", $err_count);
printf("     Рост памяти внутри фазы: %+s\n", fmt_mb($mem_growth));

// ====================== PHASE 2: update_line + update_array ======================
$UPD_N = min(2000, $iterations);
phase_header('PHASE 2 — update_line + update_array', $UPD_N);
$t2 = microtime(true);
$m2 = mem_stats();
$rss_p2_before = get_proc_metric_kb($snap1, 'Rss');
$updates = [];

for ($i = 0; $i < $UPD_N; $i++) {
    $new = "UPD_key$i " . str_pad('NEW', 300, 'Y');
    if ($fast_io_ok) {
        file_update_line($DB_MAIN, $new, $i * 512, 512, 0);
    } else {
        file_put_contents($DB_MAIN, $new . PHP_EOL, FILE_APPEND | LOCK_EX);
    }
    $updates[] = [$new, $i * 512, 512];
    if ($i % 200 === 0) progress_bar($i + 1, $UPD_N, PROGRESS_WIDTH, 'P2 ');
}
progress_done($UPD_N, 'update_line', microtime(true) - $t2);
if ($fast_io_ok) @file_update_array($DB_MAIN, $updates, 0);
gc_collect_cycles();
$snap2 = proc_snapshot('phase2_update', $logdir);
$snapshots[] = ['name' => 'phase2', 'php_mem' => $m2, 'snap' => $snap2];
$rss_p2_after = get_proc_metric_kb($snap2, 'Rss');
phase_result('update', true, microtime(true) - $t2, $m2, mem_stats(), $rss_p2_before, $rss_p2_after);
printf("     Batch update_array: %d строк\n", count($updates));

// ====================== PHASE 3: search + pcre2 ======================
$SRCH_N = max(1, (int)($iterations / 10));
phase_header('PHASE 3 — search_line + search_array + get_keys + pcre2', $SRCH_N);
$t3 = microtime(true);
$m3 = mem_stats();
$rss_p3_before = get_proc_metric_kb($snap2, 'Rss');
$search_mem = [];

for ($i = 0; $i < $SRCH_N; $i++) {
    $key = "key" . ($i % 500);
    if ($fast_io_ok) {
        @file_search_line($DB_MAIN, $key, 0, 0);
        @file_search_array($DB_MAIN, $key, 0, 50, 0, 0);
        @file_get_keys($DB_MAIN, 0, 100, 0, 2);
        @find_matches_pcre2('key\\d+', "test key123 payload", 1);
    } else {
        preg_match('/key\d+/', "test key123 payload");
    }

    if ($i % max(1, (int)($SRCH_N / 20)) === 0) {
        gc_collect_cycles();
        $search_mem[] = memory_get_usage(true);
        progress_bar($i + 1, $SRCH_N, PROGRESS_WIDTH, 'P3 ');
    }
}
progress_done($SRCH_N, 'search+pcre2', microtime(true) - $t3);
gc_collect_cycles();
$snap3 = proc_snapshot('phase3_search', $logdir);
$snapshots[] = ['name' => 'phase3', 'php_mem' => $m3, 'snap' => $snap3];
$rss_p3_after = get_proc_metric_kb($snap3, 'Rss');

$search_mem_growth = count($search_mem) > 1 ? end($search_mem) - $search_mem[0] : 0;
phase_result('search + pcre2', true, microtime(true) - $t3, $m3, mem_stats(), $rss_p3_before, $rss_p3_after);
printf("     4 операции/итер, %d итераций — рост памяти внутри: %+s\n", $SRCH_N, fmt_mb($search_mem_growth));

// ====================== PHASE 4: push_data + search_data + defrag ======================
$BIN_N = max(1, (int)($iterations / 5));
phase_header('PHASE 4 — push_data + search_data + defrag_data', $BIN_N);
$t4 = microtime(true);
$m4 = mem_stats();
$rss_p4_before = get_proc_metric_kb($snap3, 'Rss');
$bin_errors = 0;

for ($i = 0; $i < $BIN_N; $i++) {
    $k = "binkey_$i";
    $v = str_repeat('BinaryData', 50) . $i;
    if ($fast_io_ok) {
        file_push_data($DB_DATA, $k, $v, 0);
        $got = @file_search_data($DB_DATA, $k, 0, 0);
        if ($got !== $v) $bin_errors++;
    } else {
        file_put_contents($DB_DATA, $k . ':' . base64_encode($v) . PHP_EOL, FILE_APPEND | LOCK_EX);
    }
    if ($i % max(1, (int)($BIN_N / 20)) === 0) {
        gc_collect_cycles();
        progress_bar($i + 1, $BIN_N, PROGRESS_WIDTH, 'P4 ');
    }
}
progress_done($BIN_N, 'push+search_data', microtime(true) - $t4);
if ($fast_io_ok) @file_defrag_data($DB_DATA, '', 0);
gc_collect_cycles();
$snap4 = proc_snapshot('phase4_binary', $logdir);
$snapshots[] = ['name' => 'phase4', 'php_mem' => $m4, 'snap' => $snap4];
$rss_p4_after = get_proc_metric_kb($snap4, 'Rss');
phase_result('binary ops', $bin_errors === 0, microtime(true) - $t4, $m4, mem_stats(), $rss_p4_before, $rss_p4_after);
if ($bin_errors) printf("     Несовпадений данных: %d\n", $bin_errors);

// ====================== PHASE 5: callback_line + pop_line + replicate ======================
$CB_N = 500;
phase_header('PHASE 5 — callback_line + pop_line + replicate_file', $CB_N);
$t5 = microtime(true);
$m5 = mem_stats();
$rss_p5_before = get_proc_metric_kb($snap4, 'Rss');

if ($fast_io_ok) {
    $cb_count = 0;
    file_callback_line($DB_MAIN, function () use (&$cb_count) {
        $cb_count++;
        if ($cb_count % 1000 === 0) gc_collect_cycles();
        return true;
    }, 0, 0);
    printf("  Callback вызовов: %d\n", $cb_count);
}

for ($i = 0; $i < $CB_N; $i++) {
    if ($fast_io_ok) @file_pop_line($DB_MAIN, -1, 0);
    else {
        $c = @file_get_contents($DB_MAIN);
        if ($c !== false) {
            $lines = preg_split('/\r?\n/', rtrim($c, "\n"));
            array_pop($lines);
            @file_put_contents($DB_MAIN, implode(PHP_EOL, $lines) . PHP_EOL);
        }
    }
    if ($i % 50 === 0) progress_bar($i + 1, $CB_N, PROGRESS_WIDTH, 'P5 ');
}
progress_done($CB_N, 'pop_line', microtime(true) - $t5);

if ($fast_io_ok) @replicate_file($DB_MAIN, $DB_REPL, 0);
else @copy($DB_MAIN, $DB_REPL);
gc_collect_cycles();
$snap5 = proc_snapshot('phase5_callback_pop_repl', $logdir);
$snapshots[] = ['name' => 'phase5', 'php_mem' => $m5, 'snap' => $snap5];
$rss_p5_after = get_proc_metric_kb($snap5, 'Rss');
phase_result('callback + pop + replicate', true, microtime(true) - $t5, $m5, mem_stats(), $rss_p5_before, $rss_p5_after);

// ====================== ФИНАЛЬНЫЙ СНИМОК ======================
$global_elapsed = microtime(true) - $global_start;
$final_mem  = mem_stats();
$final_snap = proc_snapshot('final', $logdir, true);
$snapshots[] = ['name' => 'final', 'php_mem' => $final_mem, 'snap' => $final_snap];

// ====================== ТАБЛИЦА СНИМКОВ ======================

$series = [];
foreach ($snapshots as $s) {
    $snap = $s['snap'];
    $rss  = get_proc_metric_kb($snap, 'Rss');
    $pss  = get_proc_metric_kb($snap, 'Pss');
    $vmpeak_kb = null;
    if (!empty($snap['proc_status']['VmPeak']) && preg_match('/(\d+)/', $snap['proc_status']['VmPeak'], $m2)) $vmpeak_kb = (int)$m2[1];
    $series[] = [
        'name'            => $s['name'],
        'ts'              => $snap['ts'] ?? null,
        'php_usage_bytes' => $s['php_mem']['usage'],
        'php_peak_bytes'  => $s['php_mem']['peak'],
        'rss_kb'          => $rss,
        'pss_kb'          => $pss,
        'vmpeak_kb'       => $vmpeak_kb,
    ];
}

$rss_arr = array_filter(array_column($series, 'rss_kb'), fn($v) => $v !== null);
$pss_arr = array_filter(array_column($series, 'pss_kb'), fn($v) => $v !== null);
$rss_stats = stats_summary(array_values($rss_arr));
$pss_stats = stats_summary(array_values($pss_arr));

echo "\n" . str_repeat('─', $w) . "\n";
echo "  СНИМКИ ПО ФАЗАМ (PHP / proc)\n";
echo str_repeat('─', $w) . "\n";
printf("  %-10s %-10s %-12s %-12s %-12s %-10s %-8s\n",
    'Фаза', 'Время', 'PHP usage', 'PHP peak', 'RSS', 'PSS', 'VmPeak');
$prev_rss = null;
foreach ($series as $it) {
    $t_s   = $it['ts'] ? date('H:i:s', (int)$it['ts']) : '-';
    $rss_s = fmt_kb_val($it['rss_kb']);
    $pss_s = fmt_kb_val($it['pss_kb']);
    $vmp_s = $it['vmpeak_kb'] ? fmt_kb_val($it['vmpeak_kb']) : '—';
    $rss_delta_arrow = '';
    if ($prev_rss !== null && $it['rss_kb'] !== null) {
        $d = $it['rss_kb'] - $prev_rss;
        $rss_delta_arrow = $d > 0 ? sprintf(' (+%s)', fmt_kb_val($d)) : '';
    }
    if ($it['rss_kb'] !== null) $prev_rss = $it['rss_kb'];
    printf("  %-10s %-10s %-12s %-12s %-12s %-10s %-8s\n",
        $it['name'], $t_s,
        fmt_mb($it['php_usage_bytes']),
        fmt_mb($it['php_peak_bytes']),
        $rss_s . $rss_delta_arrow,
        $pss_s,
        $vmp_s
    );
}

// ====================== АНАЛИТИКА ======================

echo str_repeat('─', $w) . "\n";
echo "  АНАЛИТИКА МЕТРИК\n";
echo str_repeat('─', $w) . "\n";

if ($rss_stats['count'] > 0) {
    printf("  RSS  | min: %-10s max: %-10s mean: %-10s median: %-10s\n",
        fmt_kb_val($rss_stats['min']), fmt_kb_val($rss_stats['max']),
        fmt_kb_val((int)$rss_stats['mean']), fmt_kb_val((int)$rss_stats['median']));
    printf("       | stddev: %-8s тренд: %+.1f kB/снимок\n",
        fmt_kb_val((int)$rss_stats['stddev']), $rss_stats['trend'] ?? 0);
}
if ($pss_stats['count'] > 0) {
    printf("  PSS  | min: %-10s max: %-10s mean: %-10s median: %-10s\n",
        fmt_kb_val($pss_stats['min']), fmt_kb_val($pss_stats['max']),
        fmt_kb_val((int)$pss_stats['mean']), fmt_kb_val((int)$pss_stats['median']));
}

// Обнаружение аномалий
$rss_values = array_values($rss_arr);
$anomalies = [];
for ($i = 1; $i < count($rss_values); $i++) {
    $delta = $rss_values[$i] - $rss_values[$i - 1];
    if ($delta > 10240) { // > 10 MB скачок между фазами
        $anomalies[] = sprintf("Фаза %d→%d: RSS вырос на %s", $i, $i + 1, fmt_kb_val($delta));
    }
}
if ($anomalies) {
    echo "  ⚠️  Аномалии RSS:\n";
    foreach ($anomalies as $a) echo "     • $a\n";
}

// ====================== ДЕЛЬТЫ ======================

$start_rss = $series[0]['rss_kb'] ?? null;
$final_rss  = end($series)['rss_kb'] ?? null;
$delta_rss  = ($start_rss && $final_rss) ? $final_rss - $start_rss : null;
$peak_growth_mb = ($final_mem['peak'] - $global_mem['peak']) / 1048576;
$proc_growth_mb = $delta_rss !== null ? $delta_rss / 1024.0 : null;

echo str_repeat('─', $w) . "\n";
echo "  ДЕЛЬТЫ start → final:\n";
printf("  PHP peak delta : %+s\n", fmt_mb($peak_growth_mb * 1048576));
printf("  PHP usage final: %s\n", fmt_mb($final_mem['usage']));
if ($delta_rss !== null) {
    printf("  RSS delta      : %+s\n", fmt_kb_val($delta_rss));
    if ($is_valgrind && abs($delta_rss) > 1024) {
        $vg_note = $delta_rss > 0
            ? "  ℹ️  RSS рост под Valgrind ожидаем (shadow memory + dlopen overhead)\n"
            : "";
        echo $vg_note;
    }
}

// ====================== VALGRIND LOG ANALYSIS ======================

$vg_analysis = null;
if ($vg_log_path) {
    $vg_analysis = parse_valgrind_log($vg_log_path);
    if ($vg_analysis) print_valgrind_analysis($vg_analysis);
    else echo "\n  ⚠️  Valgrind-лог не найден: $vg_log_path\n";
}

// ====================== ИТОГОВЫЙ ВЕРДИКТ ======================

echo "\n" . str_repeat('═', $w) . "\n";
echo "  ИТОГ ТЕСТА НА УТЕЧКИ ПАМЯТИ\n";
echo str_repeat('═', $w) . "\n";
printf("  Общее время     : %s\n", fmt_time($global_elapsed));
printf("  PHP peak delta  : %+.2f MB  %s\n",
    $peak_growth_mb, $peak_growth_mb >= MAX_PHP_DELTA_MB ? '❌' : '✅');
printf("  PHP usage final : %.2f MB\n", $final_mem['usage'] / 1048576);
if ($delta_rss !== null) {
    printf("  RSS delta (proc): %+.2f MB  %s",
        $proc_growth_mb,
        abs($proc_growth_mb) >= $max_rss_threshold ? '❌' : '✅');
    if ($is_valgrind) echo "  [VG-оверхед учтён, порог {$max_rss_threshold} MB]";
    echo "\n";
}

// Вердикт
$php_ok  = $peak_growth_mb < MAX_PHP_DELTA_MB;
$rss_ok  = $proc_growth_mb === null || abs($proc_growth_mb) < $max_rss_threshold;
$vg_ok   = $vg_analysis === null || ($vg_analysis['definitely_lost'] === 0 && $vg_analysis['indirectly_lost'] === 0 && count($vg_analysis['fast_io_leaks']) === 0);
$all_ok  = $php_ok && $rss_ok && $vg_ok;

echo "\n";
if ($all_ok) {
    echo "✅  УТЕЧЕК ПАМЯТИ НЕ ОБНАРУЖЕНО\n";
    if ($is_valgrind) {
        echo "    PHP peak delta < " . MAX_PHP_DELTA_MB . " MB  |  VG: 0 definitely/indirectly lost\n";
        echo "    'still reachable' = PHP-рантайм (zend/dlopen), ОС освобождает при выходе\n";
    } else {
        echo "    Рост PHP peak < " . MAX_PHP_DELTA_MB . " MB  |  RSS delta в норме\n";
        echo "    fast_io корректно освобождает память во всех фазах.\n";
    }
} else {
    echo "⚠️  ВОЗМОЖНАЯ ПРОБЛЕМА С ПАМЯТЬЮ\n";
    if (!$php_ok)  printf("    PHP peak delta %.2f MB >= порог %.1f MB\n", $peak_growth_mb, MAX_PHP_DELTA_MB);
    if (!$rss_ok)  printf("    RSS delta %.2f MB >= порог %.1f MB\n", $proc_growth_mb, $max_rss_threshold);
    if (!$vg_ok)   echo  "    Valgrind: найдены потери в fast_io!\n";
    echo "    Рекомендуется: запустить с valgrind=1 и изучить " . $logdir . "\n";
    if ($is_valgrind && !$php_ok) {
        echo "    Под Valgrind RSS-overhead нормален; критично только 'definitely/indirectly lost'\n";
    }
}

// ====================== СОХРАНЕНИЕ SUMMARY ======================

$summary = [
    'version'         => VERSION,
    'time'            => time(),
    'elapsed_s'       => $global_elapsed,
    'is_valgrind'     => $is_valgrind,
    'fast_io_loaded'  => $fast_io_ok,
    'verdict'         => $all_ok ? 'PASS' : 'WARN',
    'thresholds'      => ['php_mb' => MAX_PHP_DELTA_MB, 'rss_mb' => $max_rss_threshold],
    'php_start'       => $global_mem,
    'php_final'       => $final_mem,
    'peak_growth_mb'  => $peak_growth_mb,
    'proc_series'     => $series,
    'proc_stats'      => ['rss' => $rss_stats, 'pss' => $pss_stats],
    'proc_delta_rss_kb' => $delta_rss,
    'anomalies'       => $anomalies,
    'vg_analysis'     => $vg_analysis,
    'logdir'          => $logdir,
];
@file_put_contents($logdir . '/summary.json', json_encode($summary, JSON_PRETTY_PRINT | JSON_UNESCAPED_SLASHES));

printf("\n  Снимки и логи: %s\n", $logdir);
if (!$keep_logs) {
    echo "  DB-файлы будут удалены. Запустите с keep-logs=1 для сохранения.\n";
}
echo "  Тест завершён.\n\n";
@ob_flush(); @flush();

// ====================== CLEANUP ======================
gc_collect_cycles();
if (!$keep_logs) {
    foreach ([$DB_MAIN, $DB_DATA, $DB_REPL] as $f) @unlink($f);
}
