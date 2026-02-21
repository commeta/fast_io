<?php
/*
 * Fast_IO Extension for PHP 8
 * https://github.com/commeta/fast_io
 *
 * Copyright 2026 commeta <dcs-spb@ya.ru>
 *
 * FastAdmin — полноценный интерфейс для fast_io
 * Все функции расширения: text + binary таблицы
 * PHP 8.1+ | Нативный JS | Без внешних зависимостей
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */


if (!function_exists('file_get_keys')) {
    die('<!DOCTYPE html><html><body style="background:#0a0c0f;color:#ff4444;font-family:monospace;padding:2rem"><h2>❌ Расширение fast_io не загружено</h2></body></html>');
}

// ==================== КОНФИГ ====================
$data_dir = __DIR__ . '/fast_io_data';
if (!is_dir($data_dir)) mkdir($data_dir, 0777, true);
define('DEFAULT_ALIGN', 4096);
ini_set('fast_io.buffer_size', 65536);

// ==================== УТИЛИТЫ ====================
function get_tables(): array {
    global $data_dir;
    $files = @scandir($data_dir) ?: [];
    $tables = [];
    foreach ($files as $f) {
        if (!str_ends_with($f, '.dat')) continue;
        $path = $data_dir . '/' . $f;
        $has_index = file_exists($path . '.index');
        $size = filesize($path);
        $tables[] = [
            'name' => $f,
            'type' => $has_index ? 'binary' : 'text',
            'path' => $path,
            'size' => $size,
        ];
    }
    return $tables;
}

function fmt_size(int $bytes): string {
    if ($bytes < 1024) return $bytes . ' B';
    if ($bytes < 1048576) return round($bytes/1024, 1) . ' KB';
    return round($bytes/1048576, 1) . ' MB';
}

function parse_kv(string $line): array {
    $pos = strpos($line, ' ');
    return $pos === false
        ? ['key' => $line, 'value' => '']
        : ['key' => substr($line, 0, $pos), 'value' => substr($line, $pos + 1)];
}

function json_resp(mixed $data): never {
    header('Content-Type: application/json');
    echo json_encode($data, JSON_UNESCAPED_UNICODE | JSON_UNESCAPED_SLASHES);
    exit;
}

// ==================== AJAX ====================
$is_ajax = !empty($_SERVER['HTTP_X_REQUESTED_WITH']) || (isset($_GET['ajax']) && $_GET['ajax'] === '1');

if ($is_ajax) {
    $op = $_POST['op'] ?? $_GET['op'] ?? '';
    $table = basename($_POST['table'] ?? $_GET['table'] ?? '');
    $tpath = $data_dir . '/' . $table;

    switch ($op) {

        // --- Список таблиц ---
        case 'list_tables':
            json_resp(['tables' => get_tables()]);

        // --- Создать таблицу ---
        case 'create_table': {
            $name = basename($_POST['name'] ?? '');
            $type = $_POST['type'] ?? 'text';
            if (!$name) json_resp(['error' => 'Имя не указано']);
            if (!str_ends_with($name, '.dat')) $name .= '.dat';
            $p = $data_dir . '/' . $name;
            if (file_exists($p)) json_resp(['error' => 'Таблица уже существует']);
            file_put_contents($p, '');
            if ($type === 'binary') file_put_contents($p . '.index', '');
            json_resp(['ok' => true, 'name' => $name, 'type' => $type]);
        }

        // --- Удалить таблицу ---
        case 'drop_table': {
            if (!file_exists($tpath)) json_resp(['error' => 'Не найдена']);
            unlink($tpath);
            if (file_exists($tpath . '.index')) unlink($tpath . '.index');
            json_resp(['ok' => true]);
        }

        // --- file_analize ---
        case 'analize': {
            $mode = (int)($_POST['mode'] ?? 0);
            $res = @file_analize($tpath, $mode);
            json_resp(['result' => $res]);
        }

        // --- file_get_keys (browse text) ---
        case 'get_keys': {
            $start = (int)($_POST['start'] ?? 0);
            $limit = (int)($_POST['limit'] ?? 25);
            $mode  = (int)($_POST['mode'] ?? 2);
            $pos   = (int)($_POST['position'] ?? 0);
            $rows  = @file_get_keys($tpath, $start, $limit, $pos, $mode) ?: [];
            $anal  = @file_analize($tpath) ?: [];
            json_resp(['rows' => $rows, 'analize' => $anal]);
        }

        // --- file_insert_line ---
        case 'insert_line': {
            $line   = $_POST['line'] ?? '';
            $align  = (int)($_POST['align'] ?? DEFAULT_ALIGN);
            $mode   = (int)($_POST['mode'] ?? 2);
            $res = file_insert_line($tpath, $line, $mode, $align);
            json_resp(['result' => $res, 'ok' => $res >= 0]);
        }

        // --- file_replace_line ---
        case 'replace_line': {
            $key    = $_POST['key'] ?? '';
            $newline = $_POST['newline'] ?? '';
            $mode   = (int)($_POST['mode'] ?? 0);
            $res = file_replace_line($tpath, $key, $newline, $mode);
            json_resp(['result' => $res, 'ok' => $res >= 0]);
        }

        // --- file_erase_line ---
        case 'erase_line': {
            $key  = $_POST['key'] ?? '';
            $pos  = (int)($_POST['position'] ?? 0);
            $mode = (int)($_POST['mode'] ?? 0);
            $res  = file_erase_line($tpath, $key, $pos, $mode);
            json_resp(['result' => $res, 'ok' => $res >= 0]);
        }

        // --- file_search_line ---
        case 'search_line': {
            $key  = $_POST['key'] ?? '';
            $pos  = (int)($_POST['position'] ?? 0);
            $mode = (int)($_POST['mode'] ?? 0);
            $res  = file_search_line($tpath, $key, $pos, $mode);
            json_resp(['result' => $res, 'found' => $res !== false]);
        }

        // --- file_search_array ---
        case 'search_array': {
            $key   = $_POST['key'] ?? '';
            $start = (int)($_POST['start'] ?? 0);
            $limit = (int)($_POST['limit'] ?? 50);
            $pos   = (int)($_POST['position'] ?? 0);
            $mode  = (int)($_POST['mode'] ?? 0);
            $res   = @file_search_array($tpath, $key, $start, $limit, $pos, $mode) ?: [];
            json_resp(['result' => $res, 'count' => count((array)$res)]);
        }

        // --- file_select_line ---
        case 'select_line': {
            $row   = (int)($_POST['row'] ?? 0);
            $align = (int)($_POST['align'] ?? DEFAULT_ALIGN);
            $mode  = (int)($_POST['mode'] ?? 0);
            $res   = file_select_line($tpath, $row, $align, $mode);
            json_resp(['result' => $res, 'found' => $res !== false]);
        }

        // --- file_select_array ---
        case 'select_array': {
            $raw     = $_POST['query'] ?? '[]';
            $query   = json_decode($raw, true) ?: [];
            $pattern = $_POST['pattern'] ?? '';
            $mode    = (int)($_POST['mode'] ?? 0);
            $res     = @file_select_array($tpath, $query, $pattern, $mode) ?: [];
            json_resp(['result' => $res]);
        }

        // --- file_update_line ---
        case 'update_line': {
            $line   = $_POST['line'] ?? '';
            $pos    = (int)($_POST['position'] ?? 0);
            $align  = (int)($_POST['align'] ?? DEFAULT_ALIGN);
            $mode   = (int)($_POST['mode'] ?? 0);
            $res    = file_update_line($tpath, $line, $pos, $align, $mode);
            json_resp(['result' => $res, 'ok' => $res >= 0]);
        }

        // --- file_update_array ---
        case 'update_array': {
            $raw  = $_POST['query'] ?? '[]';
            $query = json_decode($raw, true) ?: [];
            $mode = (int)($_POST['mode'] ?? 0);
            $res  = file_update_array($tpath, $query, $mode);
            json_resp(['result' => $res, 'ok' => $res >= 0]);
        }

        // --- file_pop_line ---
        case 'pop_line': {
            $offset = (int)($_POST['offset'] ?? -1);
            $mode   = (int)($_POST['mode'] ?? 0);
            $res    = file_pop_line($tpath, $offset, $mode);
            json_resp(['result' => $res, 'found' => $res !== false]);
        }

        // --- file_defrag_lines (text) ---
        case 'defrag_lines': {
            $key  = $_POST['key'] ?? '';
            $mode = (int)($_POST['mode'] ?? 0);
            $res  = file_defrag_lines($tpath, $key, $mode);
            json_resp(['result' => $res, 'ok' => $res >= 0]);
        }

        // --- file_callback_line ---
        case 'callback_line': {
            $pos  = (int)($_POST['position'] ?? 0);
            $mode = (int)($_POST['mode'] ?? 4);
            $collected = [];
            $limit = (int)($_POST['limit'] ?? 20);
            $count = 0;
            $ret = file_callback_line($tpath, function() use (&$collected, &$count, $limit) {
                $count++;
                $line    = func_get_arg(0);
                $fname   = func_num_args() > 1 ? func_get_arg(1) : '';
                $off     = func_num_args() > 2 ? func_get_arg(2) : 0;
                $len     = func_num_args() > 3 ? func_get_arg(3) : 0;
                $lcount  = func_num_args() > 4 ? func_get_arg(4) : 0;
                $collected[] = [
                    'line'        => rtrim($line),
                    'line_offset' => $off,
                    'line_length' => $len,
                    'line_count'  => $lcount,
                ];
                if ($count >= $limit) return false;
                return true;
            }, $pos, $mode);
            json_resp(['rows' => $collected, 'total' => $count]);
        }

        // --- file_push_data (binary) ---
        case 'push_data': {
            $key   = $_POST['key'] ?? '';
            $value = $_POST['value'] ?? '';
            $mode  = (int)($_POST['mode'] ?? 0);
            $res   = file_push_data($tpath, $key, $value, $mode);
            json_resp(['result' => $res, 'ok' => $res >= 0]);
        }

        // --- file_search_data (binary) ---
        case 'search_data': {
            $key  = $_POST['key'] ?? '';
            $pos  = (int)($_POST['position'] ?? 0);
            $mode = (int)($_POST['mode'] ?? 0);
            $res  = file_search_data($tpath, $key, $pos, $mode);
            json_resp(['result' => $res, 'found' => $res !== false]);
        }

        // --- file_defrag_data (binary) ---
        case 'defrag_data': {
            $key  = $_POST['key'] ?? '';
            $mode = (int)($_POST['mode'] ?? 0);
            $res  = file_defrag_data($tpath, $key, $mode);
            json_resp(['result' => $res, 'ok' => $res >= 0]);
        }

        // --- replicate_file ---
        case 'replicate': {
            $target = $data_dir . '/' . basename($_POST['target'] ?? '');
            $mode   = (int)($_POST['mode'] ?? 0);
            $res    = replicate_file($tpath, $target, $mode);
            json_resp(['result' => $res, 'ok' => $res >= 0]);
        }

        // --- find_matches_pcre2 ---
        case 'pcre2': {
            $pattern = $_POST['pattern'] ?? '';
            $subject = $_POST['subject'] ?? '';
            $mode    = (int)($_POST['mode'] ?? 0);
            $res     = find_matches_pcre2($pattern, $subject, $mode);
            json_resp(['result' => $res]);
        }

        // --- Прочитать index файл (binary browse) ---
        case 'browse_binary': {
            $start = (int)($_POST['start'] ?? 0);
            $limit = (int)($_POST['limit'] ?? 25);
            $idxpath = $tpath . '.index';
            $rows = @file_get_keys($idxpath, $start, $limit, 0, 2) ?: [];
            $anal = @file_analize($idxpath) ?: [];
            json_resp(['rows' => $rows, 'analize' => $anal]);
        }

        default:
            json_resp(['error' => 'Unknown op: ' . $op]);
    }
}
?>
<!DOCTYPE html>
<html lang="ru">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>FastAdmin — fast_io</title>
<link rel="preconnect" href="https://fonts.googleapis.com">
<link href="https://fonts.googleapis.com/css2?family=JetBrains+Mono:wght@300;400;500;700&family=Syne:wght@400;700;800&display=swap" rel="stylesheet">
<style>
:root {
    --bg: #070a0d;
    --bg2: #0d1117;
    --bg3: #131b24;
    --border: #1e2d3d;
    --border2: #253547;
    --accent: #00e5ff;
    --accent2: #00ff87;
    --accent3: #ff6b35;
    --accent4: #c084fc;
    --text: #c9d8e8;
    --text2: #6b8097;
    --text3: #3d5166;
    --danger: #ff4444;
    --warning: #ffb700;
    --success: #00ff87;
    --font-mono: 'JetBrains Mono', monospace;
    --font-display: 'Syne', sans-serif;
    --radius: 2px;
    --scan-speed: 8s;
}

*, *::before, *::after { box-sizing: border-box; margin: 0; padding: 0; }

html, body {
    height: 100%;
    background: var(--bg);
    color: var(--text);
    font-family: var(--font-mono);
    font-size: 13px;
    overflow: hidden;
}

/* ─── SCANLINE EFFECT ─── */
body::before {
    content: '';
    position: fixed;
    inset: 0;
    background: repeating-linear-gradient(
        0deg,
        transparent,
        transparent 2px,
        rgba(0,229,255,0.015) 2px,
        rgba(0,229,255,0.015) 4px
    );
    pointer-events: none;
    z-index: 9999;
}

body::after {
    content: '';
    position: fixed;
    inset: 0;
    background: radial-gradient(ellipse at 50% 0%, rgba(0,229,255,0.04) 0%, transparent 70%);
    pointer-events: none;
    z-index: 9998;
}

/* ─── LAYOUT ─── */
#app {
    display: grid;
    grid-template-columns: 260px 1fr;
    grid-template-rows: 48px 1fr;
    height: 100vh;
    overflow: hidden;
}

/* ─── TOPBAR ─── */
#topbar {
    grid-column: 1 / -1;
    background: var(--bg2);
    border-bottom: 1px solid var(--border);
    display: flex;
    align-items: center;
    padding: 0 20px;
    gap: 16px;
    position: relative;
}

#topbar::after {
    content: '';
    position: absolute;
    bottom: 0; left: 0; right: 0;
    height: 1px;
    background: linear-gradient(90deg, transparent, var(--accent), transparent);
    opacity: 0.4;
}

.logo {
    font-family: var(--font-display);
    font-size: 18px;
    font-weight: 800;
    letter-spacing: -0.5px;
    color: var(--accent);
    display: flex;
    align-items: center;
    gap: 8px;
    text-transform: uppercase;
}

.logo-dot { color: var(--accent2); }

.topbar-info {
    margin-left: auto;
    color: var(--text2);
    font-size: 11px;
    display: flex;
    gap: 16px;
}

.status-dot {
    display: inline-block;
    width: 6px; height: 6px;
    border-radius: 50%;
    background: var(--success);
    box-shadow: 0 0 6px var(--success);
    animation: pulse-dot 2s ease-in-out infinite;
}

@keyframes pulse-dot {
    0%, 100% { opacity: 1; }
    50% { opacity: 0.4; }
}

/* ─── SIDEBAR ─── */
#sidebar {
    background: var(--bg2);
    border-right: 1px solid var(--border);
    display: flex;
    flex-direction: column;
    overflow: hidden;
}

.sidebar-head {
    padding: 14px 16px 10px;
    border-bottom: 1px solid var(--border);
}

.sidebar-head h3 {
    font-family: var(--font-display);
    font-size: 10px;
    font-weight: 700;
    letter-spacing: 2px;
    text-transform: uppercase;
    color: var(--text2);
    margin-bottom: 10px;
}

.new-table-form {
    display: flex;
    flex-direction: column;
    gap: 6px;
}

.new-table-form input,
.new-table-form select {
    width: 100%;
    background: var(--bg3);
    border: 1px solid var(--border2);
    color: var(--text);
    font-family: var(--font-mono);
    font-size: 12px;
    padding: 6px 8px;
    border-radius: var(--radius);
    outline: none;
    transition: border-color 0.15s;
}

.new-table-form input:focus,
.new-table-form select:focus {
    border-color: var(--accent);
    box-shadow: 0 0 0 2px rgba(0,229,255,0.08);
}

.new-table-form select option { background: var(--bg3); }

#tables-list {
    flex: 1;
    overflow-y: auto;
    padding: 8px 0;
}

#tables-list::-webkit-scrollbar { width: 4px; }
#tables-list::-webkit-scrollbar-track { background: transparent; }
#tables-list::-webkit-scrollbar-thumb { background: var(--border2); border-radius: 2px; }

.table-item {
    display: flex;
    align-items: center;
    padding: 8px 16px;
    cursor: pointer;
    gap: 8px;
    border-left: 2px solid transparent;
    transition: all 0.12s;
    user-select: none;
}

.table-item:hover { background: var(--bg3); }
.table-item.active {
    background: rgba(0,229,255,0.06);
    border-left-color: var(--accent);
}

.table-item .ti-icon {
    font-size: 10px;
    opacity: 0.6;
    flex-shrink: 0;
}

.table-item .ti-name {
    flex: 1;
    overflow: hidden;
    text-overflow: ellipsis;
    white-space: nowrap;
    font-size: 12px;
    color: var(--text);
}

.table-item .ti-badge {
    font-size: 9px;
    padding: 2px 5px;
    border-radius: 2px;
    font-weight: 700;
    letter-spacing: 0.5px;
    flex-shrink: 0;
}

.badge-text { background: rgba(0,229,255,0.12); color: var(--accent); }
.badge-binary { background: rgba(192,132,252,0.12); color: var(--accent4); }

.ti-size {
    font-size: 10px;
    color: var(--text3);
    flex-shrink: 0;
}

/* ─── MAIN ─── */
#main {
    overflow: hidden;
    display: flex;
    flex-direction: column;
    background: var(--bg);
}

/* ─── WELCOME ─── */
#welcome {
    flex: 1;
    display: flex;
    flex-direction: column;
    align-items: center;
    justify-content: center;
    color: var(--text3);
    font-family: var(--font-display);
}

.welcome-art {
    font-size: 11px;
    font-family: var(--font-mono);
    line-height: 1.4;
    color: var(--text3);
    text-align: center;
    margin-bottom: 24px;
    opacity: 0.6;
}

.welcome-art span { color: var(--accent); opacity: 1; }

#welcome h2 {
    font-size: 28px;
    font-weight: 800;
    color: var(--text2);
    margin-bottom: 8px;
}

#welcome p { font-size: 12px; font-family: var(--font-mono); }

/* ─── TABLE VIEW ─── */
#table-view {
    display: none;
    flex: 1;
    flex-direction: column;
    overflow: hidden;
}

#table-view.visible { display: flex; }

/* ─── TABLE HEADER ─── */
.tv-header {
    padding: 14px 20px 0;
    border-bottom: 1px solid var(--border);
    background: var(--bg2);
}

.tv-title {
    display: flex;
    align-items: center;
    gap: 10px;
    margin-bottom: 12px;
}

.tv-title h2 {
    font-family: var(--font-display);
    font-size: 16px;
    font-weight: 800;
    color: var(--text);
}

.tv-title .tv-type {
    font-size: 9px;
    font-weight: 700;
    letter-spacing: 1px;
    padding: 3px 7px;
    border-radius: 2px;
}

.tv-title .del-btn {
    margin-left: auto;
    background: none;
    border: 1px solid rgba(255,68,68,0.3);
    color: var(--danger);
    font-family: var(--font-mono);
    font-size: 11px;
    padding: 4px 10px;
    cursor: pointer;
    border-radius: var(--radius);
    transition: all 0.15s;
}

.tv-title .del-btn:hover { background: rgba(255,68,68,0.08); border-color: var(--danger); }

/* ─── TABS ─── */
.tabs {
    display: flex;
    gap: 0;
}

.tab-btn {
    background: none;
    border: none;
    border-bottom: 2px solid transparent;
    color: var(--text2);
    font-family: var(--font-mono);
    font-size: 11px;
    font-weight: 500;
    padding: 8px 16px;
    cursor: pointer;
    letter-spacing: 0.5px;
    transition: all 0.12s;
    white-space: nowrap;
    text-transform: uppercase;
}

.tab-btn:hover { color: var(--text); }
.tab-btn.active { color: var(--accent); border-bottom-color: var(--accent); }

/* ─── TAB CONTENT ─── */
.tv-body {
    flex: 1;
    overflow: hidden;
    display: flex;
    flex-direction: column;
}

.tab-pane {
    display: none;
    flex: 1;
    overflow-y: auto;
    padding: 16px 20px;
}

.tab-pane::-webkit-scrollbar { width: 6px; }
.tab-pane::-webkit-scrollbar-track { background: transparent; }
.tab-pane::-webkit-scrollbar-thumb { background: var(--border2); border-radius: 2px; }

.tab-pane.active { display: block; }

/* ─── FORM ELEMENTS ─── */
.form-row {
    display: flex;
    gap: 8px;
    align-items: flex-end;
    flex-wrap: wrap;
    margin-bottom: 12px;
}

.form-group {
    display: flex;
    flex-direction: column;
    gap: 4px;
}

.form-group label {
    font-size: 10px;
    font-weight: 500;
    letter-spacing: 1px;
    text-transform: uppercase;
    color: var(--text2);
}

input[type="text"],
input[type="number"],
textarea,
select {
    background: var(--bg3);
    border: 1px solid var(--border2);
    color: var(--text);
    font-family: var(--font-mono);
    font-size: 12px;
    padding: 7px 10px;
    border-radius: var(--radius);
    outline: none;
    transition: border-color 0.15s;
}

input[type="text"]:focus,
input[type="number"]:focus,
textarea:focus,
select:focus {
    border-color: var(--accent);
    box-shadow: 0 0 0 2px rgba(0,229,255,0.06);
}

textarea { resize: vertical; min-height: 80px; width: 100%; font-size: 11px; }

select option { background: var(--bg3); }

.inp-sm { width: 120px; }
.inp-md { width: 220px; }
.inp-lg { width: 360px; }
.inp-full { width: 100%; }

/* ─── BUTTONS ─── */
.btn {
    display: inline-flex;
    align-items: center;
    gap: 6px;
    border: 1px solid var(--border2);
    background: var(--bg3);
    color: var(--text);
    font-family: var(--font-mono);
    font-size: 11px;
    font-weight: 500;
    padding: 7px 14px;
    cursor: pointer;
    border-radius: var(--radius);
    transition: all 0.12s;
    white-space: nowrap;
    letter-spacing: 0.3px;
}

.btn:hover { background: var(--bg2); border-color: var(--text2); }
.btn:active { transform: translateY(1px); }

.btn-primary {
    background: rgba(0,229,255,0.1);
    border-color: rgba(0,229,255,0.4);
    color: var(--accent);
}
.btn-primary:hover { background: rgba(0,229,255,0.16); border-color: var(--accent); }

.btn-success {
    background: rgba(0,255,135,0.08);
    border-color: rgba(0,255,135,0.3);
    color: var(--success);
}
.btn-success:hover { background: rgba(0,255,135,0.14); border-color: var(--success); }

.btn-danger {
    background: rgba(255,68,68,0.08);
    border-color: rgba(255,68,68,0.3);
    color: var(--danger);
}
.btn-danger:hover { background: rgba(255,68,68,0.14); border-color: var(--danger); }

.btn-warning {
    background: rgba(255,183,0,0.08);
    border-color: rgba(255,183,0,0.3);
    color: var(--warning);
}
.btn-warning:hover { background: rgba(255,183,0,0.14); border-color: var(--warning); }

.btn-purple {
    background: rgba(192,132,252,0.08);
    border-color: rgba(192,132,252,0.3);
    color: var(--accent4);
}
.btn-purple:hover { background: rgba(192,132,252,0.14); border-color: var(--accent4); }

.btn-sm { padding: 4px 10px; font-size: 10px; }
.btn-xs { padding: 2px 7px; font-size: 10px; }
.btn-icon { padding: 6px 8px; }

/* ─── NOTIFICATIONS ─── */
.notif {
    padding: 8px 14px;
    border-radius: var(--radius);
    margin-bottom: 12px;
    font-size: 12px;
    border-left: 3px solid;
    animation: slide-in 0.2s ease;
}

@keyframes slide-in {
    from { transform: translateX(-8px); opacity: 0; }
    to { transform: none; opacity: 1; }
}

.notif-ok { background: rgba(0,255,135,0.07); border-color: var(--success); color: var(--success); }
.notif-err { background: rgba(255,68,68,0.07); border-color: var(--danger); color: var(--danger); }
.notif-info { background: rgba(0,229,255,0.07); border-color: var(--accent); color: var(--accent); }
.notif-warn { background: rgba(255,183,0,0.07); border-color: var(--warning); color: var(--warning); }

/* ─── DATA TABLE ─── */
.data-table {
    width: 100%;
    border-collapse: collapse;
    font-size: 12px;
}

.data-table th {
    text-align: left;
    padding: 6px 10px;
    font-size: 10px;
    font-weight: 600;
    letter-spacing: 1px;
    text-transform: uppercase;
    color: var(--text2);
    border-bottom: 1px solid var(--border);
    white-space: nowrap;
}

.data-table td {
    padding: 6px 10px;
    border-bottom: 1px solid rgba(30,45,61,0.5);
    vertical-align: middle;
    color: var(--text);
}

.data-table tr:hover td { background: rgba(0,229,255,0.025); }

.cell-key { color: var(--accent); font-weight: 500; }
.cell-val {
    max-width: 400px;
    overflow: hidden;
    text-overflow: ellipsis;
    white-space: nowrap;
    color: var(--text2);
    font-size: 11px;
}
.cell-num { color: var(--text3); font-size: 11px; text-align: right; }
.cell-offset { color: var(--accent3); font-size: 11px; }

/* ─── PAGINATION ─── */
.pagination {
    display: flex;
    gap: 4px;
    align-items: center;
    margin-top: 12px;
    flex-wrap: wrap;
}

.page-btn {
    background: var(--bg3);
    border: 1px solid var(--border2);
    color: var(--text2);
    font-family: var(--font-mono);
    font-size: 11px;
    padding: 4px 10px;
    cursor: pointer;
    border-radius: var(--radius);
    transition: all 0.12s;
}

.page-btn:hover { border-color: var(--accent); color: var(--accent); }
.page-btn.current { background: rgba(0,229,255,0.1); border-color: var(--accent); color: var(--accent); }
.page-info { color: var(--text3); font-size: 11px; margin-left: 8px; }

/* ─── RESULT AREA ─── */
.result-area {
    background: var(--bg2);
    border: 1px solid var(--border);
    border-radius: var(--radius);
    padding: 12px 14px;
    margin-top: 12px;
    font-size: 12px;
    overflow: auto;
    max-height: 400px;
}

.result-area::-webkit-scrollbar { width: 6px; height: 6px; }
.result-area::-webkit-scrollbar-thumb { background: var(--border2); border-radius: 2px; }

.result-area pre {
    font-family: var(--font-mono);
    font-size: 11px;
    color: var(--text);
    white-space: pre-wrap;
    word-break: break-all;
}

/* ─── STAT GRID ─── */
.stat-grid {
    display: grid;
    grid-template-columns: repeat(auto-fill, minmax(180px, 1fr));
    gap: 8px;
    margin-bottom: 16px;
}

.stat-card {
    background: var(--bg2);
    border: 1px solid var(--border);
    border-radius: var(--radius);
    padding: 10px 12px;
}

.stat-card .sc-label {
    font-size: 10px;
    font-weight: 600;
    letter-spacing: 1px;
    text-transform: uppercase;
    color: var(--text2);
    margin-bottom: 4px;
}

.stat-card .sc-value {
    font-size: 18px;
    font-weight: 700;
    font-family: var(--font-display);
    color: var(--accent);
}

.stat-card .sc-value.green { color: var(--success); }
.stat-card .sc-value.orange { color: var(--accent3); }
.stat-card .sc-value.purple { color: var(--accent4); }

/* ─── SECTION TITLES ─── */
.section-title {
    font-size: 10px;
    font-weight: 700;
    letter-spacing: 2px;
    text-transform: uppercase;
    color: var(--text2);
    margin-bottom: 10px;
    padding-bottom: 6px;
    border-bottom: 1px solid var(--border);
    display: flex;
    align-items: center;
    gap: 8px;
}

.section-title .fn-name {
    font-family: var(--font-mono);
    color: var(--accent);
    font-size: 11px;
    font-weight: 400;
    background: rgba(0,229,255,0.08);
    padding: 1px 6px;
    border-radius: 2px;
}

/* ─── INLINE EDIT ─── */
.inline-edit {
    display: inline-flex;
    gap: 6px;
    align-items: center;
}

/* ─── OP CARDS ─── */
.op-grid {
    display: grid;
    grid-template-columns: repeat(auto-fill, minmax(300px, 1fr));
    gap: 12px;
    margin-bottom: 12px;
}

.op-card {
    background: var(--bg2);
    border: 1px solid var(--border);
    border-radius: var(--radius);
    padding: 14px;
}

.op-card .oc-title {
    font-family: var(--font-mono);
    font-size: 11px;
    font-weight: 600;
    color: var(--accent);
    margin-bottom: 10px;
    letter-spacing: 0.5px;
}

/* ─── CHECKBOX ─── */
.cb-row {
    display: flex;
    align-items: center;
    gap: 6px;
}

input[type="checkbox"] {
    accent-color: var(--accent);
    width: 13px;
    height: 13px;
    cursor: pointer;
}

/* ─── MODAL ─── */
.modal-overlay {
    display: none;
    position: fixed;
    inset: 0;
    background: rgba(7,10,13,0.85);
    z-index: 1000;
    align-items: center;
    justify-content: center;
    backdrop-filter: blur(4px);
}

.modal-overlay.open { display: flex; }

.modal-box {
    background: var(--bg2);
    border: 1px solid var(--border2);
    border-radius: var(--radius);
    min-width: 480px;
    max-width: 680px;
    width: 90%;
    max-height: 80vh;
    overflow-y: auto;
    box-shadow: 0 24px 80px rgba(0,0,0,0.6), 0 0 0 1px rgba(0,229,255,0.06);
    animation: modal-in 0.18s ease;
}

@keyframes modal-in {
    from { transform: scale(0.96) translateY(-8px); opacity: 0; }
    to { transform: none; opacity: 1; }
}

.modal-header {
    display: flex;
    align-items: center;
    justify-content: space-between;
    padding: 14px 18px;
    border-bottom: 1px solid var(--border);
}

.modal-header h3 {
    font-family: var(--font-display);
    font-size: 14px;
    font-weight: 800;
}

.modal-close {
    background: none;
    border: none;
    color: var(--text2);
    font-size: 18px;
    cursor: pointer;
    line-height: 1;
    padding: 2px 6px;
}

.modal-close:hover { color: var(--text); }

.modal-body { padding: 16px 18px; }
.modal-footer {
    padding: 12px 18px;
    border-top: 1px solid var(--border);
    display: flex;
    gap: 8px;
    justify-content: flex-end;
}

/* ─── LOADING ─── */
.loading {
    display: inline-flex;
    gap: 4px;
    align-items: center;
    color: var(--text2);
    font-size: 11px;
}

.loading::before {
    content: '';
    width: 10px; height: 10px;
    border: 1px solid var(--text3);
    border-top-color: var(--accent);
    border-radius: 50%;
    animation: spin 0.6s linear infinite;
}

@keyframes spin { to { transform: rotate(360deg); } }

/* ─── CODE ─── */
code {
    font-family: var(--font-mono);
    background: var(--bg3);
    border: 1px solid var(--border);
    padding: 1px 5px;
    border-radius: 2px;
    font-size: 11px;
    color: var(--accent2);
}

.sep { height: 1px; background: var(--border); margin: 16px 0; }

/* ─── COMPACT TABLE for select_array ─── */
.query-editor {
    font-family: var(--font-mono);
    font-size: 11px;
    background: var(--bg3);
    border: 1px solid var(--border2);
    color: var(--text);
    padding: 8px;
    width: 100%;
    min-height: 100px;
    resize: vertical;
    border-radius: var(--radius);
}

/* ─── EMPTY STATE ─── */
.empty-state {
    text-align: center;
    padding: 32px;
    color: var(--text3);
    font-size: 12px;
}

/* ─── SCROLLABLE TABS (mobile) ─── */
.tabs { overflow-x: auto; }
.tabs::-webkit-scrollbar { display: none; }

/* ─── ANIMATIONS ─── */
@keyframes fadeIn {
    from { opacity: 0; transform: translateY(4px); }
    to { opacity: 1; transform: none; }
}

.fade-in { animation: fadeIn 0.2s ease; }
</style>
</head>
<body>
<div id="app">

<!-- TOPBAR -->
<div id="topbar">
    <div class="logo">
        <span>Fast</span><span class="logo-dot">▸</span><span>Admin</span>
    </div>
    <div class="topbar-info">
        <span><span class="status-dot"></span> fast_io <?= phpversion('fast_io') ?: 'loaded' ?></span>
        <span>PHP <?= PHP_MAJOR_VERSION . '.' . PHP_MINOR_VERSION ?></span>
        <span style="color:var(--text3)">buf: <?= number_format((int)ini_get('fast_io.buffer_size') / 1024, 0) ?>KB</span>
    </div>
</div>

<!-- SIDEBAR -->
<div id="sidebar">
    <div class="sidebar-head">
        <h3>Таблицы</h3>
        <div class="new-table-form">
            <input type="text" id="new-table-name" placeholder="имя_таблицы.dat">
            <select id="new-table-type">
                <option value="text">TEXT (file_insert_line)</option>
                <option value="binary">BINARY (file_push_data)</option>
            </select>
            <button class="btn btn-success btn-sm" onclick="createTable()">+ Создать</button>
        </div>
    </div>
    <div id="tables-list">
        <div class="empty-state">Загрузка...</div>
    </div>
</div>

<!-- MAIN -->
<div id="main">

    <!-- WELCOME -->
    <div id="welcome">
        <div class="welcome-art">fast_io Engine Admin Panel</div>
        <h2>Выберите таблицу</h2>
        <p style="margin-top:8px">или создайте новую в сайдбаре</p>
    </div>

    <!-- TABLE VIEW -->
    <div id="table-view">

        <div class="tv-header">
            <div class="tv-title">
                <h2 id="tv-name">—</h2>
                <span id="tv-type-badge" class="tv-type">TEXT</span>
                <button class="del-btn btn-xs" onclick="dropTable()">✕ Удалить</button>
            </div>
            <div class="tabs" id="tabs-nav">
                <!-- injected by JS -->
            </div>
        </div>

        <div class="tv-body">
            <!-- injected by JS -->
        </div>

    </div><!-- #table-view -->
</div><!-- #main -->
</div><!-- #app -->

<!-- EDIT MODAL -->
<div class="modal-overlay" id="edit-modal">
    <div class="modal-box">
        <div class="modal-header">
            <h3 id="edit-modal-title">Редактировать строку</h3>
            <button class="modal-close" onclick="closeModal('edit-modal')">✕</button>
        </div>
        <div class="modal-body" id="edit-modal-body"></div>
        <div class="modal-footer" id="edit-modal-footer"></div>
    </div>
</div>

<script>
'use strict';

// ════════════════════════════════
//  STATE
// ════════════════════════════════
const state = {
    tables: [],
    current: null,
    tab: 'browse',
    browsePage: 0,
    browseLimit: 25,
};

// ════════════════════════════════
//  API
// ════════════════════════════════
async function api(op, data = {}, method = 'POST') {
    const fd = new FormData();
    fd.append('ajax', '1');
    fd.append('op', op);
    for (const [k, v] of Object.entries(data)) fd.append(k, v);
    const r = await fetch(location.pathname + '?ajax=1', { method, body: fd });
    return r.json();
}

// ════════════════════════════════
//  NOTIFICATION
// ════════════════════════════════
function notify(msg, type = 'ok', container = '#notif-area') {
    const el = document.querySelector(container);
    if (!el) return;
    const d = document.createElement('div');
    d.className = `notif notif-${type} fade-in`;
    d.textContent = msg;
    el.prepend(d);
    setTimeout(() => d.remove(), 5000);
}

// ═══════════════════════════════
//  MODAL
// ════════════════════════════════
function openModal(id) { document.getElementById(id).classList.add('open'); }
function closeModal(id) { document.getElementById(id).classList.remove('open'); }

// ════════════════════════════════
//  SIDEBAR
// ════════════════════════════════
async function loadTables() {
    const r = await api('list_tables');
    state.tables = r.tables || [];
    renderSidebar();
}

function renderSidebar() {
    const el = document.getElementById('tables-list');
    if (!state.tables.length) {
        el.innerHTML = '<div class="empty-state">Нет таблиц</div>';
        return;
    }
    el.innerHTML = state.tables.map(t => `
        <div class="table-item ${state.current?.name === t.name ? 'active' : ''}"
             onclick="selectTable(${escAttr(JSON.stringify(t.name))})">
            <span class="ti-icon">${t.type === 'binary' ? '◈' : '▤'}</span>
            <span class="ti-name" title="${t.name}">${t.name}</span>
            <span class="ti-badge ${t.type === 'binary' ? 'badge-binary' : 'badge-text'}">${t.type.toUpperCase()}</span>
            <span class="ti-size">${fmtSize(t.size)}</span>
        </div>
    `).join('');
}

function fmtSize(b) {
    if (b < 1024) return b + 'B';
    if (b < 1048576) return (b/1024).toFixed(1) + 'K';
    return (b/1048576).toFixed(1) + 'M';
}

// ════════════════════════════════
//  TABLE SELECTION
// ════════════════════════════════
function selectTable(name) {
    state.current = state.tables.find(t => t.name === name) || null;
    if (!state.current) return;
    state.browsePage = 0;
    renderSidebar();
    renderTableView();
}

function renderTableView() {
    const t = state.current;
    document.getElementById('welcome').style.display = 'none';
    const tv = document.getElementById('table-view');
    tv.classList.add('visible');

    document.getElementById('tv-name').textContent = t.name;
    const badge = document.getElementById('tv-type-badge');
    badge.textContent = t.type.toUpperCase();
    badge.style.cssText = t.type === 'binary'
        ? 'background:rgba(192,132,252,0.12);color:#c084fc'
        : 'background:rgba(0,229,255,0.12);color:#00e5ff';

    const tabs = t.type === 'text' ? [
        ['browse', '▤ Browse'],
        ['search', '⌕ Search'],
        ['insert', '+ Insert'],
        ['select', '⊡ Select'],
        ['update', '✎ Update'],
        ['ops', '⚙ Ops'],
        ['analize', '⊞ Analyze'],
    ] : [
        ['browse', '▤ Index'],
        ['search', '⌕ Search'],
        ['push', '+ Push'],
        ['ops', '⚙ Ops'],
        ['analize', '⊞ Analyze'],
    ];

    document.getElementById('tabs-nav').innerHTML = tabs.map(([id, label]) =>
        `<button class="tab-btn ${state.tab === id ? 'active' : ''}" onclick="switchTab('${id}')">${label}</button>`
    ).join('');

    renderTabBody();
}

function switchTab(id) {
    state.tab = id;
    document.querySelectorAll('.tab-btn').forEach(b => {
        b.classList.remove('active');
        if (b.getAttribute('onclick') === `switchTab('${id}')`) b.classList.add('active');
    });
    renderTabBody();
}

// ════════════════════════════════
//  TAB BODY RENDERER
// ════════════════════════════════
function renderTabBody() {
    const body = document.querySelector('.tv-body');
    if (!body) return;
    const t = state.current;
    let html = '<div class="tab-pane active" id="tab-content">';
    html += '<div id="notif-area"></div>';

    switch (state.tab) {
        case 'browse':  html += t.type === 'text' ? tplBrowseText() : tplBrowseBinary(); break;
        case 'search':  html += t.type === 'text' ? tplSearchText() : tplSearchBinary(); break;
        case 'insert':  html += tplInsert(); break;
        case 'push':    html += tplPush(); break;
        case 'select':  html += tplSelect(); break;
        case 'update':  html += tplUpdate(); break;
        case 'ops':     html += t.type === 'text' ? tplOpsText() : tplOpsBinary(); break;
        case 'analize': html += tplAnalize(); break;
    }

    html += '</div>';
    body.innerHTML = html;

    if (state.tab === 'browse') {
        loadBrowse();
    } else if (state.tab === 'analize') {
        loadAnalize();
    }
}

// ════════════════════════════════
//  BROWSE TEXT
// ═══════════════════════════════
function tplBrowseText() {
    return `
    <div class="section-title">Browse <span class="fn-name">file_get_keys</span></div>
    <div class="form-row" style="margin-bottom:8px">
        <div class="form-group">
            <label>Режим</label>
            <select id="browse-mode" onchange="loadBrowse()">
                <option value="0">0: key + meta</option>
                <option value="1">1: line + meta</option>
                <option value="2" selected>2: trim_line + meta</option>
                <option value="3">3: meta only</option>
                <option value="4">4: keys only</option>
                <option value="5">5: lines only</option>
            </select>
        </div>
        <div class="form-group">
            <label>Rows/page</label>
            <select id="browse-limit" onchange="state.browsePage=0;loadBrowse()">
                <option value="10">10</option>
                <option value="25" selected>25</option>
                <option value="50">50</option>
                <option value="100">100</option>
            </select>
        </div>
        <button class="btn btn-primary" style="margin-top:18px" onclick="loadBrowse()">↻ Обновить</button>
    </div>
    <div id="browse-area"><div class="loading">Загрузка</div></div>
    <div id="browse-pagination" class="pagination"></div>
    `;
}

async function loadBrowse() {
    const t = state.current;
    if (!t) return;

    if (t.type === 'binary') { loadBrowseBinary(); return; }

    const mode  = parseInt(document.getElementById('browse-mode')?.value || 2);
    const limit = parseInt(document.getElementById('browse-limit')?.value || 25);
    state.browseLimit = limit;

    const r = await api('get_keys', {
        table: t.name,
        start: state.browsePage * limit,
        limit,
        mode,
    });

    const area = document.getElementById('browse-area');
    const rows = r.rows || [];
    const anal = r.analize || {};
    const total = anal.line_count || 0;

    if (!rows.length) {
        area.innerHTML = '<div class="empty-state">Нет данных</div>';
        document.getElementById('browse-pagination').innerHTML = '';
        return;
    }

    let html = '';
    if (mode === 4 || mode === 5) {
        html = `<div class="result-area"><pre>${escHtml(JSON.stringify(rows, null, 2))}</pre></div>`;
    } else {
        html = `<table class="data-table"><thead><tr>`;
        const sample = rows[0];
        if (typeof sample === 'object') {
            const cols = Object.keys(sample);
            cols.forEach(c => { html += `<th>${c}</th>`; });
            html += `<th>Actions</th></tr></thead><tbody>`;
            rows.forEach(row => {
                html += '<tr>';
                cols.forEach(c => {
                    const v = row[c];
                    let cls = 'cell-num';
                    if (c === 'key' || c === 'trim_line' || c === 'line') cls = c === 'key' ? 'cell-key' : 'cell-val';
                    if (c === 'line_offset') cls = 'cell-offset';
                    const display = typeof v === 'string' && v.length > 120 ? v.slice(0, 120) + '…' : v;
                    html += `<td class="${cls}" title="${typeof v === 'string' ? escAttr(v) : ''}">${escHtml(String(display ?? ''))}</td>`;
                });
                const key = row.key || (row.trim_line ? row.trim_line.split(' ')[0] : '');
                const val = row.trim_line || row.line || '';
                const safeKey = JSON.stringify(key).replace(/'/g, "\\'");
                const safeVal = JSON.stringify(val).replace(/'/g, "\\'");
                html += `<td>
                    <div class="inline-edit">
                        <button class="btn btn-sm btn-primary btn-icon" title="Редактировать" onclick='openEditLine(${safeKey}, ${safeVal})'>✎</button>
                        <button class="btn btn-sm btn-danger btn-icon" title="Удалить" onclick='eraseLine(${safeKey})'>✕</button>
                    </div>
                </td>`;
                html += '</tr>';
            });
            html += '</tbody></table>';
        }
    }

    area.innerHTML = `<div class="fade-in">${html}</div>`;

    const pages = Math.ceil(total / limit);
    let pag = '';
    if (pages > 1) {
        const cur = state.browsePage;
        if (cur > 0) pag += `<button class="page-btn" onclick="goBrowsePage(${cur-1})">‹</button>`;
        for (let i = Math.max(0, cur-3); i <= Math.min(pages-1, cur+3); i++) {
            pag += `<button class="page-btn ${i===cur?'current':''}" onclick="goBrowsePage(${i})">${i+1}</button>`;
        }
        if (cur < pages-1) pag += `<button class="page-btn" onclick="goBrowsePage(${cur+1})">›</button>`;
        pag += `<span class="page-info">${total} строк</span>`;
    }
    document.getElementById('browse-pagination').innerHTML = pag;
}

function goBrowsePage(p) { state.browsePage = p; loadBrowse(); }

function openEditLine(key, val) {
    const t = state.current;
    document.getElementById('edit-modal-title').textContent = 'Редактировать: ' + key;
    document.getElementById('edit-modal-body').innerHTML = `
        <div class="form-group" style="margin-bottom:10px">
            <label>Ключ (line_key для поиска)</label>
            <input type="text" id="em-key" class="inp-full" value="${escAttr(key + ' ')}">
        </div>
        <div class="form-group">
            <label>Новая строка (полностью)</label>
            <textarea id="em-val" style="min-height:120px">${escHtml(val)}</textarea>
        </div>
        <div class="form-row" style="margin-top:10px">
            <div class="form-group">
                <label>Режим replace</label>
                <select id="em-mode">
                    <option value="0">0: chain mode</option>
                    <option value="1">1: rename mode</option>
                </select>
            </div>
        </div>
        <p style="font-size:11px;color:var(--text3);margin-top:8px">Использует <code>file_replace_line</code></p>
    `;
    document.getElementById('edit-modal-footer').innerHTML = `
        <button class="btn" onclick="closeModal('edit-modal')">Отмена</button>
        <button class="btn btn-primary" onclick="doReplaceLine()">Сохранить</button>
    `;
    openModal('edit-modal');
}

async function doReplaceLine() {
    const key  = document.getElementById('em-key').value;
    const val  = document.getElementById('em-val').value;
    const mode = document.getElementById('em-mode').value;
    const r = await api('replace_line', { table: state.current.name, key, newline: val, mode });
    closeModal('edit-modal');
    if (r.ok) { notify(`✓ Заменено (${r.result} строк)`, 'ok'); loadBrowse(); }
    else notify('Ошибка: ' + r.result, 'err');
}

async function eraseLine(key) {
    if (!confirm(`Стереть строку с ключом "${key}"?`)) return;
    const r = await api('erase_line', { table: state.current.name, key: key + ' ', position: 0, mode: 0 });
    if (r.ok) { notify(`✓ Стёрто (offset ${r.result})`, 'ok'); loadBrowse(); }
    else notify('Ошибка: ' + r.result, 'err');
}

// ════════════════════════════════
//  BROWSE BINARY
// ════════════════════════════════
function tplBrowseBinary() {
    return `
    <div class="section-title">Index Browser <span class="fn-name">*.index</span></div>
    <div class="form-row" style="margin-bottom:8px">
        <div class="form-group">
            <label>Rows/page</label>
            <select id="browse-limit" onchange="state.browsePage=0;loadBrowse()">
                <option value="10">10</option>
                <option value="25" selected>25</option>
                <option value="50">50</option>
            </select>
        </div>
        <button class="btn btn-primary" style="margin-top:18px" onclick="loadBrowse()">↻ Обновить</button>
    </div>
    <div id="browse-area"><div class="loading">Загрузка</div></div>
    <div id="browse-pagination" class="pagination"></div>
    `;
}

async function loadBrowseBinary() {
    const t = state.current;
    const limit = parseInt(document.getElementById('browse-limit')?.value || 25);
    const r = await api('browse_binary', {
        table: t.name,
        start: state.browsePage * limit,
        limit,
    });

    const area = document.getElementById('browse-area');
    const rows = r.rows || [];
    const anal = r.analize || {};
    const total = anal.line_count || 0;

    if (!rows.length) {
        area.innerHTML = '<div class="empty-state">Индекс пуст</div>';
        document.getElementById('browse-pagination').innerHTML = '';
        return;
    }

    let html = `<table class="data-table"><thead><tr>
        <th>#</th><th>Key</th><th>Offset</th><th>Size</th><th>Actions</th>
    </tr></thead><tbody>`;

    rows.forEach((row, i) => {
        const line = typeof row === 'string' ? row : (row.trim_line || '');
        const parts = line.split(' ');
        const key = parts[0] || '';
        const idx = parts[1] || '';
        const [off, sz] = idx.split(':');
        const absIdx = state.browsePage * limit + i;
        html += `<tr>
            <td class="cell-num">${absIdx}</td>
            <td class="cell-key">${escHtml(key)}</td>
            <td class="cell-offset">${escHtml(off || '')}</td>
            <td class="cell-num">${escHtml(sz || '')}</td>
            <td>
                <button class="btn btn-xs btn-primary" onclick="fetchBinaryVal(${escAttr(JSON.stringify(key))})">Fetch</button>
                <button class="btn btn-xs btn-danger" onclick="eraseIndexEntry(${escAttr(JSON.stringify(key + ' '))})">✕</button>
            </td>
        </tr>`;
    });
    html += '</tbody></table>';

    area.innerHTML = `<div class="fade-in">${html}</div>`;

    const pages = Math.ceil(total / limit);
    let pag = '';
    if (pages > 1) {
        const cur = state.browsePage;
        if (cur > 0) pag += `<button class="page-btn" onclick="goBrowsePage(${cur-1})">‹</button>`;
        for (let i = Math.max(0, cur-3); i <= Math.min(pages-1, cur+3); i++) {
            pag += `<button class="page-btn ${i===cur?'current':''}" onclick="goBrowsePage(${i})">${i+1}</button>`;
        }
        if (cur < pages-1) pag += `<button class="page-btn" onclick="goBrowsePage(${cur+1})">›</button>`;
        pag += `<span class="page-info">${total} записей</span>`;
    }
    document.getElementById('browse-pagination').innerHTML = pag;
}

async function fetchBinaryVal(key) {
    const r = await api('search_data', { table: state.current.name, key, position: 0, mode: 0 });
    if (r.found) {
        openResultModal('Binary Value: ' + key, r.result);
    } else {
        notify('Не найдено', 'err');
    }
}

async function eraseIndexEntry(key) {
    if (!confirm('Стереть запись из индекса?')) return;
    const t = state.current;
    const fd = new FormData();
    fd.append('ajax','1'); fd.append('op','erase_line');
    fd.append('table', t.name + '.index'); fd.append('key', key);
    fd.append('position','0'); fd.append('mode','0');
    const r = await (await fetch(location.pathname+'?ajax=1', {method:'POST',body:fd})).json();
    if (r.ok) { notify(`✓ Запись индекса стёрта`, 'ok'); loadBrowse(); }
    else notify('Ошибка: ' + r.result, 'err');
}

function openResultModal(title, content) {
    document.getElementById('edit-modal-title').textContent = title;
    document.getElementById('edit-modal-body').innerHTML = `
        <div class="result-area"><pre>${escHtml(String(content))}</pre></div>
    `;
    document.getElementById('edit-modal-footer').innerHTML = `
        <button class="btn" onclick="closeModal('edit-modal')">Закрыть</button>
    `;
    openModal('edit-modal');
}

// ════════════════════════════════
//  SEARCH TEXT
// ════════════════════════════════
function tplSearchText() {
    return `
    <div class="section-title">Поиск <span class="fn-name">file_search_line</span> <span class="fn-name">file_search_array</span></div>

    <div class="op-grid">
        <div class="op-card">
            <div class="oc-title">file_search_line — одна строка</div>
            <div class="form-group" style="margin-bottom:8px">
                <label>Ключ / подстрока</label>
                <input type="text" id="sl-key" class="inp-full" placeholder="my_key ">
            </div>
            <div class="form-row">
                <div class="form-group">
                    <label>Position</label>
                    <input type="number" id="sl-pos" value="0" class="inp-sm">
                </div>
                <div class="form-group">
                    <label>Mode</label>
                    <select id="sl-mode">
                        <option value="0">0: substr, raw</option>
                        <option value="1">1: substr, trim</option>
                        <option value="10">10: regex, raw</option>
                        <option value="11">11: regex, trim</option>
                    </select>
                </div>
                <button class="btn btn-primary" style="margin-top:18px" onclick="doSearchLine()">Найти</button>
            </div>
            <div id="sl-result" class="result-area" style="display:none;margin-top:8px"></div>
        </div>

        <div class="op-card">
            <div class="oc-title">file_search_array — массив результатов</div>
            <div class="form-group" style="margin-bottom:8px">
                <label>Ключ / PCRE2 pattern</label>
                <input type="text" id="sa-key" class="inp-full" placeholder="index_ или \\w+_\\d+">
            </div>
            <div class="form-row">
                <div class="form-group">
                    <label>Start</label>
                    <input type="number" id="sa-start" value="0" class="inp-sm">
                </div>
                <div class="form-group">
                    <label>Limit</label>
                    <input type="number" id="sa-limit" value="50" class="inp-sm">
                </div>
                <div class="form-group">
                    <label>Mode</label>
                    <select id="sa-mode">
                        <option value="0">0: substr, full meta</option>
                        <option value="1">1: substr, line+meta</option>
                        <option value="2">2: substr, lines</option>
                        <option value="3">3: substr, counts</option>
                        <option value="10">10: regex, full</option>
                        <option value="11">11: regex, line</option>
                        <option value="12">12: regex, lines</option>
                        <option value="13">13: regex, counts</option>
                        <option value="20">20: regex+matches, trim</option>
                        <option value="21">21: regex+matches, line</option>
                        <option value="22">22: regex+match detail</option>
                        <option value="23">23: regex matches only</option>
                    </select>
                </div>
                <button class="btn btn-primary" style="margin-top:18px" onclick="doSearchArray()">Найти</button>
            </div>
            <div id="sa-result" class="result-area" style="display:none;margin-top:8px;max-height:300px"></div>
        </div>
    </div>

    <div class="sep"></div>
    <div class="section-title">PCRE2 тест <span class="fn-name">find_matches_pcre2</span></div>
    <div class="op-card" style="max-width:600px">
        <div class="form-group" style="margin-bottom:8px">
            <label>Pattern (PCRE2)</label>
            <input type="text" id="pcre-pattern" class="inp-full" placeholder="\\w+_\\d+" value="\\w+_\\d+">
        </div>
        <div class="form-group" style="margin-bottom:8px">
            <label>Subject</label>
            <input type="text" id="pcre-subject" class="inp-full" value="index_42 file_insert_line_42 data">
        </div>
        <div class="form-row">
            <div class="form-group">
                <label>Mode</label>
                <select id="pcre-mode">
                    <option value="0">0: matches array</option>
                    <option value="1">1: detail (offset, length)</option>
                </select>
            </div>
            <button class="btn btn-purple" style="margin-top:18px" onclick="doPcre2()">Match</button>
        </div>
        <div id="pcre-result" class="result-area" style="display:none;margin-top:8px"></div>
    </div>
    `;
}

async function doSearchLine() {
    const r = await api('search_line', {
        table:    state.current.name,
        key:      document.getElementById('sl-key').value,
        position: document.getElementById('sl-pos').value,
        mode:     document.getElementById('sl-mode').value,
    });
    const el = document.getElementById('sl-result');
    el.style.display = 'block';
    el.innerHTML = r.found
        ? `<pre>${escHtml(r.result)}</pre>`
        : '<span style="color:var(--danger)">Не найдено</span>';
}

async function doSearchArray() {
    const r = await api('search_array', {
        table: state.current.name,
        key:   document.getElementById('sa-key').value,
        start: document.getElementById('sa-start').value,
        limit: document.getElementById('sa-limit').value,
        mode:  document.getElementById('sa-mode').value,
    });
    const el = document.getElementById('sa-result');
    el.style.display = 'block';
    el.innerHTML = `<pre>${escHtml(JSON.stringify(r.result, null, 2))}</pre>
        <div style="margin-top:6px;color:var(--text2);font-size:11px">Найдено: ${r.count}</div>`;
}

async function doPcre2() {
    const r = await api('pcre2', {
        pattern: document.getElementById('pcre-pattern').value,
        subject: document.getElementById('pcre-subject').value,
        mode:    document.getElementById('pcre-mode').value,
    });
    const el = document.getElementById('pcre-result');
    el.style.display = 'block';
    el.innerHTML = `<pre>${escHtml(JSON.stringify(r.result, null, 2))}</pre>`;
}

// ════════════════════════════════
//  SEARCH BINARY
// ════════════════════════════════
function tplSearchBinary() {
    return `
    <div class="section-title">Binary Search <span class="fn-name">file_search_data</span></div>
    <div class="op-card" style="max-width:500px">
        <div class="form-group" style="margin-bottom:8px">
            <label>Ключ</label>
            <input type="text" id="sd-key" class="inp-full" placeholder="mykey">
        </div>
        <div class="form-row">
            <div class="form-group">
                <label>Position (в .index)</label>
                <input type="number" id="sd-pos" value="0" class="inp-sm">
            </div>
            <div class="form-group">
                <label>Mode</label>
                <select id="sd-mode">
                    <option value="0">0: standard</option>
                    <option value="100">100: no lock</option>
                </select>
            </div>
            <button class="btn btn-primary" style="margin-top:18px" onclick="doSearchData()">Найти</button>
        </div>
        <div id="sd-result" class="result-area" style="display:none;margin-top:8px;max-height:300px"></div>
    </div>

    <div class="sep"></div>
    <div class="section-title">Index Search <span class="fn-name">file_search_line (on .index)</span></div>
    <div class="op-card" style="max-width:500px">
        <div class="form-group" style="margin-bottom:8px">
            <label>Ключ / PCRE2</label>
            <input type="text" id="idx-key" class="inp-full" placeholder="mykey ">
        </div>
        <div class="form-row">
            <div class="form-group">
                <label>Mode</label>
                <select id="idx-mode">
                    <option value="0">0: substr</option>
                    <option value="1">1: substr trim</option>
                    <option value="10">10: regex</option>
                    <option value="11">11: regex trim</option>
                </select>
            </div>
            <button class="btn btn-primary" style="margin-top:18px" onclick="doIdxSearch()">Найти в индексе</button>
        </div>
        <div id="idx-result" class="result-area" style="display:none;margin-top:8px"></div>
    </div>
    `;
}

async function doSearchData() {
    const r = await api('search_data', {
        table:    state.current.name,
        key:      document.getElementById('sd-key').value,
        position: document.getElementById('sd-pos').value,
        mode:     document.getElementById('sd-mode').value,
    });
    const el = document.getElementById('sd-result');
    el.style.display = 'block';
    el.innerHTML = r.found
        ? `<pre>${escHtml(r.result)}</pre>`
        : '<span style="color:var(--danger)">Не найдено</span>';
}

async function doIdxSearch() {
    const fd = new FormData();
    fd.append('ajax','1'); fd.append('op','search_line');
    fd.append('table', state.current.name + '.index');
    fd.append('key', document.getElementById('idx-key').value);
    fd.append('position','0');
    fd.append('mode', document.getElementById('idx-mode').value);
    const r = await (await fetch(location.pathname+'?ajax=1', {method:'POST',body:fd})).json();
    const el = document.getElementById('idx-result');
    el.style.display = 'block';
    el.innerHTML = r.found ? `<pre>${escHtml(r.result)}</pre>` : '<span style="color:var(--danger)">Не найдено</span>';
}

// ════════════════════════════════
//  INSERT (TEXT)
// ════════════════════════════════
function tplInsert() {
    return `
    <div class="section-title">Добавить строку <span class="fn-name">file_insert_line</span></div>
    <div class="op-card" style="max-width:600px">
        <div class="form-group" style="margin-bottom:10px">
            <label>Строка (key value ...)</label>
            <textarea id="ins-line" placeholder="mykey some value here" style="min-height:100px"></textarea>
        </div>
        <div class="form-row">
            <div class="form-group">
                <label>Align (size)</label>
                <input type="number" id="ins-align" value="4096" class="inp-sm">
            </div>
            <div class="form-group">
                <label>Mode</label>
                <select id="ins-mode">
                    <option value="2" selected>2: \\n + return offset</option>
                    <option value="0">0: \\n + return line#</option>
                    <option value="3">3: no \\n + return offset</option>
                    <option value="1">1: no \\n + return line#</option>
                </select>
            </div>
            <button class="btn btn-success" style="margin-top:18px" onclick="doInsert()">▶ file_insert_line</button>
        </div>
        <div id="ins-result" style="display:none;margin-top:10px"></div>
    </div>
    `;
}

async function doInsert() {
    const r = await api('insert_line', {
        table: state.current.name,
        line:  document.getElementById('ins-line').value,
        align: document.getElementById('ins-align').value,
        mode:  document.getElementById('ins-mode').value,
    });
    const el = document.getElementById('ins-result');
    el.style.display = 'block';
    if (r.ok) {
        el.innerHTML = `<div class="notif notif-ok">✓ Вставлено, result=${r.result}</div>`;
        loadTables();
    } else {
        el.innerHTML = `<div class="notif notif-err">✕ Ошибка: ${r.result}</div>`;
    }
}

// ════════════════════════════════
//  PUSH (BINARY)
// ════════════════════════════════
function tplPush() {
    return `
    <div class="section-title">Push data <span class="fn-name">file_push_data</span></div>
    <div class="op-card" style="max-width:600px">
        <div class="form-group" style="margin-bottom:10px">
            <label>Ключ</label>
            <input type="text" id="push-key" class="inp-full" placeholder="mykey">
        </div>
        <div class="form-group" style="margin-bottom:10px">
            <label>Значение (бинарные данные / строка)</label>
            <textarea id="push-val" style="min-height:120px" placeholder="any binary or text data"></textarea>
        </div>
        <div class="form-row">
            <div class="form-group">
                <label>Mode</label>
                <select id="push-mode">
                    <option value="0">0: standard</option>
                    <option value="100">100: no lock</option>
                </select>
            </div>
            <button class="btn btn-success" style="margin-top:18px" onclick="doPush()">▶ file_push_data</button>
        </div>
        <div id="push-result" style="display:none;margin-top:10px"></div>
    </div>
    `;
}

async function doPush() {
    const r = await api('push_data', {
        table: state.current.name,
        key:   document.getElementById('push-key').value,
        value: document.getElementById('push-val').value,
        mode:  document.getElementById('push-mode').value,
    });
    const el = document.getElementById('push-result');
    el.style.display = 'block';
    if (r.ok) {
        el.innerHTML = `<div class="notif notif-ok">✓ Записано, offset=${r.result}</div>`;
        loadTables();
    } else {
        el.innerHTML = `<div class="notif notif-err">✕ Ошибка: ${r.result}</div>`;
    }
}

// ════════════════════════════════
//  SELECT
// ════════════════════════════════
function tplSelect() {
    return `
    <div class="section-title">Direct Select <span class="fn-name">file_select_line</span> <span class="fn-name">file_select_array</span></div>

    <div class="op-grid">
        <div class="op-card">
            <div class="oc-title">file_select_line</div>
            <div class="form-row">
                <div class="form-group">
                    <label>Row / Offset</label>
                    <input type="number" id="sell-row" value="0" class="inp-sm">
                </div>
                <div class="form-group">
                    <label>Align</label>
                    <input type="number" id="sell-align" value="4096" class="inp-sm">
                </div>
                <div class="form-group">
                    <label>Mode</label>
                    <select id="sell-mode">
                        <option value="0">0: by row#, trim</option>
                        <option value="1">1: by offset, trim</option>
                        <option value="2">2: by row#, raw</option>
                        <option value="3">3: by offset, raw</option>
                    </select>
                </div>
                <button class="btn btn-primary" style="margin-top:18px" onclick="doSelectLine()">Select</button>
            </div>
            <div id="sell-result" class="result-area" style="display:none;margin-top:8px"></div>
        </div>

        <div class="op-card">
            <div class="oc-title">file_select_array — пакетная выборка по [offset, size]</div>
            <div class="form-group" style="margin-bottom:8px">
                <label>Query JSON [[offset, size], ...]</label>
                <textarea class="query-editor" id="sela-query">[[0, 4096], [4096, 4096]]</textarea>
            </div>
            <div class="form-row">
                <div class="form-group">
                    <label>Pattern (regex)</label>
                    <input type="text" id="sela-pattern" class="inp-md" placeholder="\\w+_\\d+">
                </div>
                <div class="form-group">
                    <label>Mode</label>
                    <select id="sela-mode">
                        <option value="0">0: trim+meta</option>
                        <option value="1">1: line+meta</option>
                        <option value="2">2: lines</option>
                        <option value="3">3: substr count</option>
                        <option value="5">5: substr filter+meta</option>
                        <option value="10">10: regex+meta</option>
                        <option value="13">13: regex count</option>
                        <option value="20">20: regex+matches</option>
                        <option value="22">22: regex match detail</option>
                        <option value="23">23: matches only</option>
                    </select>
                </div>
                <button class="btn btn-primary" style="margin-top:18px" onclick="doSelectArray()">Select</button>
            </div>
            <div id="sela-result" class="result-area" style="display:none;margin-top:8px;max-height:300px"></div>
        </div>
    </div>

    <div class="sep"></div>
    <div class="section-title">Callback <span class="fn-name">file_callback_line</span></div>
    <div class="op-card" style="max-width:580px">
        <div class="form-row">
            <div class="form-group">
                <label>Position</label>
                <input type="number" id="cb-pos" value="0" class="inp-sm">
            </div>
            <div class="form-group">
                <label>Mode (args count)</label>
                <input type="number" id="cb-mode" value="4" min="0" max="9" class="inp-sm">
            </div>
            <div class="form-group">
                <label>Limit строк</label>
                <input type="number" id="cb-limit" value="20" class="inp-sm">
            </div>
            <button class="btn btn-purple" style="margin-top:18px" onclick="doCallback()">▶ callback_line</button>
        </div>
        <p style="font-size:11px;color:var(--text3);margin:6px 0">
            Итерирует файл, собирает строки с метаданными (line, offset, length, count)
        </p>
        <div id="cb-result" class="result-area" style="display:none;margin-top:8px;max-height:300px"></div>
    </div>
    `;
}

async function doSelectLine() {
    const r = await api('select_line', {
        table: state.current.name,
        row:   document.getElementById('sell-row').value,
        align: document.getElementById('sell-align').value,
        mode:  document.getElementById('sell-mode').value,
    });
    const el = document.getElementById('sell-result');
    el.style.display = 'block';
    el.innerHTML = r.found ? `<pre>${escHtml(r.result)}</pre>` : '<span style="color:var(--danger)">Не найдено</span>';
}

async function doSelectArray() {
    let q;
    try { q = JSON.parse(document.getElementById('sela-query').value); } catch { notify('Некорректный JSON', 'err'); return; }
    const r = await api('select_array', {
        table:   state.current.name,
        query:   JSON.stringify(q),
        pattern: document.getElementById('sela-pattern').value,
        mode:    document.getElementById('sela-mode').value,
    });
    const el = document.getElementById('sela-result');
    el.style.display = 'block';
    el.innerHTML = `<pre>${escHtml(JSON.stringify(r.result, null, 2))}</pre>`;
}

async function doCallback() {
    const r = await api('callback_line', {
        table:    state.current.name,
        position: document.getElementById('cb-pos').value,
        mode:     document.getElementById('cb-mode').value,
        limit:    document.getElementById('cb-limit').value,
    });
    const el = document.getElementById('cb-result');
    el.style.display = 'block';
    if (!r.rows?.length) { el.innerHTML = '<span style="color:var(--text3)">Нет данных</span>'; return; }
    let html = `<table class="data-table"><thead><tr><th>#</th><th>line</th><th>offset</th><th>length</th></tr></thead><tbody>`;
    r.rows.forEach(row => {
        html += `<tr>
            <td class="cell-num">${row.line_count}</td>
            <td class="cell-val">${escHtml(row.line.slice(0,100))}</td>
            <td class="cell-offset">${row.line_offset}</td>
            <td class="cell-num">${row.line_length}</td>
        </tr>`;
    });
    html += `</tbody></table><div style="margin-top:6px;font-size:11px;color:var(--text2)">Итого: ${r.total}</div>`;
    el.innerHTML = html;
}

// ════════════════════════════════
//  UPDATE
// ════════════════════════════════
function tplUpdate() {
    return `
    <div class="section-title">Update <span class="fn-name">file_update_line</span> <span class="fn-name">file_update_array</span></div>

    <div class="op-grid">
        <div class="op-card">
            <div class="oc-title">file_update_line — одна запись</div>
            <div class="form-group" style="margin-bottom:8px">
                <label>Новая строка</label>
                <textarea id="ul-line" style="min-height:80px" placeholder="new content"></textarea>
            </div>
            <div class="form-row">
                <div class="form-group">
                    <label>Position (offset)</label>
                    <input type="number" id="ul-pos" value="0" class="inp-sm">
                </div>
                <div class="form-group">
                    <label>Align (size)</label>
                    <input type="number" id="ul-align" value="4096" class="inp-sm">
                </div>
                <div class="form-group">
                    <label>Mode</label>
                    <select id="ul-mode">
                        <option value="0">0: + \\n</option>
                        <option value="1">1: no \\n</option>
                    </select>
                </div>
                <button class="btn btn-warning" style="margin-top:18px" onclick="doUpdateLine()">Update</button>
            </div>
            <div id="ul-result" style="display:none;margin-top:8px"></div>
        </div>

        <div class="op-card">
            <div class="oc-title">file_update_array — пакетное обновление</div>
            <div class="form-group" style="margin-bottom:8px">
                <label>Query JSON [[line, offset, size], ...]</label>
                <textarea class="query-editor" id="ua-query">
[
  ["new content for row 0", 0, 4096],
  ["new content for row 1", 4096, 4096]
]</textarea>
            </div>
            <div class="form-row">
                <div class="form-group">
                    <label>Mode</label>
                    <select id="ua-mode">
                        <option value="0">0: + \\n</option>
                        <option value="1">1: no \\n</option>
                    </select>
                </div>
                <button class="btn btn-warning" style="margin-top:18px" onclick="doUpdateArray()">Update All</button>
            </div>
            <div id="ua-result" style="display:none;margin-top:8px"></div>
        </div>
    </div>
    `;
}

async function doUpdateLine() {
    const r = await api('update_line', {
        table:    state.current.name,
        line:     document.getElementById('ul-line').value,
        position: document.getElementById('ul-pos').value,
        align:    document.getElementById('ul-align').value,
        mode:     document.getElementById('ul-mode').value,
    });
    const el = document.getElementById('ul-result');
    el.style.display = 'block';
    el.innerHTML = r.ok
        ? `<div class="notif notif-ok">✓ Записано ${r.result} байт</div>`
        : `<div class="notif notif-err">✕ Ошибка: ${r.result}</div>`;
}

async function doUpdateArray() {
    let q;
    try { q = JSON.parse(document.getElementById('ua-query').value); } catch { notify('Некорректный JSON', 'err'); return; }
    const r = await api('update_array', {
        table: state.current.name,
        query: JSON.stringify(q),
        mode:  document.getElementById('ua-mode').value,
    });
    const el = document.getElementById('ua-result');
    el.style.display = 'block';
    el.innerHTML = r.ok
        ? `<div class="notif notif-ok">✓ Записано ${r.result} байт</div>`
        : `<div class="notif notif-err">✕ Ошибка: ${r.result}</div>`;
}

// ════════════════════════════════
//  OPS TEXT
// ════════════════════════════════
function tplOpsText() {
    return `
    <div class="section-title">Операции</div>
    <div class="op-grid">

        <div class="op-card">
            <div class="oc-title">file_pop_line — извлечь последнюю строку</div>
            <div class="form-row">
                <div class="form-group">
                    <label>Offset (-1=auto, N=bytes, -N=lines)</label>
                    <input type="number" id="pop-offset" value="-1" class="inp-sm">
                </div>
                <div class="form-group">
                    <label>Mode</label>
                    <select id="pop-mode">
                        <option value="0">0: trim + truncate</option>
                        <option value="1">1: raw + truncate</option>
                        <option value="2">2: trim, no trunc</option>
                        <option value="3">3: raw, no trunc</option>
                    </select>
                </div>
                <button class="btn btn-warning" style="margin-top:18px" onclick="doPopLine()">Pop</button>
            </div>
            <div id="pop-result" class="result-area" style="display:none;margin-top:8px"></div>
        </div>

        <div class="op-card">
            <div class="oc-title">file_defrag_lines — дефрагментация</div>
            <div class="form-group" style="margin-bottom:8px">
                <label>Ключ (пусто = все DEL-строки)</label>
                <input type="text" id="defrag-key" class="inp-full" placeholder="">
            </div>
            <div class="form-row">
                <div class="form-group">
                    <label>Mode</label>
                    <select id="defrag-mode">
                        <option value="0">0: copy back</option>
                        <option value="1">1: rename</option>
                    </select>
                </div>
                <button class="btn btn-danger" style="margin-top:18px" onclick="doDefragLines()">▶ Defrag</button>
            </div>
            <div id="defrag-result" style="display:none;margin-top:8px"></div>
        </div>

        <div class="op-card">
            <div class="oc-title">file_erase_line — стереть строку</div>
            <div class="form-group" style="margin-bottom:8px">
                <label>Ключ (include trailing space)</label>
                <input type="text" id="erase-key" class="inp-full" placeholder="mykey ">
            </div>
            <div class="form-row">
                <div class="form-group">
                    <label>Position</label>
                    <input type="number" id="erase-pos" value="0" class="inp-sm">
                </div>
                <div class="form-group">
                    <label>Mode</label>
                    <select id="erase-mode">
                        <option value="0">0: standard</option>
                        <option value="100">100: no lock</option>
                    </select>
                </div>
                <button class="btn btn-danger" style="margin-top:18px" onclick="doEraseLine()">Erase</button>
            </div>
            <div id="erase-result" style="display:none;margin-top:8px"></div>
        </div>

        <div class="op-card">
            <div class="oc-title">file_replace_line — заменить строку</div>
            <div class="form-group" style="margin-bottom:8px">
                <label>Ключ для поиска</label>
                <input type="text" id="repl-key" class="inp-full" placeholder="mykey ">
            </div>
            <div class="form-group" style="margin-bottom:8px">
                <label>Новая строка</label>
                <input type="text" id="repl-val" class="inp-full" placeholder="mykey new_value">
            </div>
            <div class="form-row">
                <div class="form-group">
                    <label>Mode</label>
                    <select id="repl-mode">
                        <option value="0">0: copy back</option>
                        <option value="1">1: rename</option>
                    </select>
                </div>
                <button class="btn btn-warning" style="margin-top:18px" onclick="doReplace()">Replace</button>
            </div>
            <div id="repl-result" style="display:none;margin-top:8px"></div>
        </div>

        <div class="op-card">
            <div class="oc-title">replicate_file — копировать таблицу</div>
            <div class="form-group" style="margin-bottom:8px">
                <label>Целевой файл (в том же каталоге)</label>
                <input type="text" id="rep-target" class="inp-full" placeholder="backup.dat">
            </div>
            <div class="form-row">
                <div class="form-group">
                    <label>Mode</label>
                    <select id="rep-mode">
                        <option value="0">0: data only</option>
                        <option value="1">1: data + index</option>
                    </select>
                </div>
                <button class="btn btn-purple" style="margin-top:18px" onclick="doReplicate()">Replicate</button>
            </div>
            <div id="rep-result" style="display:none;margin-top:8px"></div>
        </div>

    </div>
    `;
}

async function doPopLine() {
    const r = await api('pop_line', {
        table:  state.current.name,
        offset: document.getElementById('pop-offset').value,
        mode:   document.getElementById('pop-mode').value,
    });
    const el = document.getElementById('pop-result');
    el.style.display = 'block';
    el.innerHTML = r.found ? `<pre>${escHtml(String(r.result))}</pre>` : '<span style="color:var(--danger)">Файл пуст</span>';
    if (r.found) loadTables();
}

async function doDefragLines() {
    if (!confirm('Запустить дефрагментацию?')) return;
    const r = await api('defrag_lines', {
        table: state.current.name,
        key:   document.getElementById('defrag-key').value,
        mode:  document.getElementById('defrag-mode').value,
    });
    const el = document.getElementById('defrag-result');
    el.style.display = 'block';
    el.innerHTML = r.ok
        ? `<div class="notif notif-ok">✓ Удалено ${r.result} строк</div>`
        : `<div class="notif notif-err">✕ Ошибка: ${r.result}</div>`;
    loadTables();
}

async function doEraseLine() {
    const r = await api('erase_line', {
        table:    state.current.name,
        key:      document.getElementById('erase-key').value,
        position: document.getElementById('erase-pos').value,
        mode:     document.getElementById('erase-mode').value,
    });
    const el = document.getElementById('erase-result');
    el.style.display = 'block';
    el.innerHTML = r.ok
        ? `<div class="notif notif-ok">✓ Стёрто (offset ${r.result})</div>`
        : `<div class="notif notif-err">✕ Ошибка: ${r.result}</div>`;
}

async function doReplace() {
    const r = await api('replace_line', {
        table:   state.current.name,
        key:     document.getElementById('repl-key').value,
        newline: document.getElementById('repl-val').value,
        mode:    document.getElementById('repl-mode').value,
    });
    const el = document.getElementById('repl-result');
    el.style.display = 'block';
    el.innerHTML = r.ok
        ? `<div class="notif notif-ok">✓ Заменено (${r.result} строк)</div>`
        : `<div class="notif notif-err">✕ Ошибка: ${r.result}</div>`;
}

async function doReplicate() {
    const target = document.getElementById('rep-target').value;
    if (!target) { notify('Укажите целевой файл', 'warn'); return; }
    const r = await api('replicate', {
        table:  state.current.name,
        target: target,
        mode:   document.getElementById('rep-mode').value,
    });
    const el = document.getElementById('rep-result');
    el.style.display = 'block';
    el.innerHTML = r.ok
        ? `<div class="notif notif-ok">✓ Скопировано ${r.result} байт</div>`
        : `<div class="notif notif-err">✕ Ошибка: ${r.result}</div>`;
    loadTables();
}

// ════════════════════════════════
//  OPS BINARY
// ════════════════════════════════
function tplOpsBinary() {
    return `
    <div class="section-title">Binary Operations</div>
    <div class="op-grid">

        <div class="op-card">
            <div class="oc-title">file_defrag_data — дефрагментация</div>
            <div class="form-group" style="margin-bottom:8px">
                <label>Ключ для удаления (пусто = все DEL)</label>
                <input type="text" id="dfd-key" class="inp-full">
            </div>
            <div class="form-row">
                <div class="form-group">
                    <label>Mode</label>
                    <select id="dfd-mode">
                        <option value="0">0: copy back</option>
                        <option value="1">1: rename</option>
                    </select>
                </div>
                <button class="btn btn-danger" style="margin-top:18px" onclick="doDefragData()">▶ Defrag Data</button>
            </div>
            <div id="dfd-result" style="display:none;margin-top:8px"></div>
        </div>

        <div class="op-card">
            <div class="oc-title">replicate_file — копировать (data + index)</div>
            <div class="form-group" style="margin-bottom:8px">
                <label>Целевой файл</label>
                <input type="text" id="brep-target" class="inp-full" placeholder="backup.dat">
            </div>
            <div class="form-row">
                <div class="form-group">
                    <label>Mode</label>
                    <select id="brep-mode">
                        <option value="0">0: data only</option>
                        <option value="1" selected>1: data + index</option>
                    </select>
                </div>
                <button class="btn btn-purple" style="margin-top:18px" onclick="doBinaryReplicate()">Replicate</button>
            </div>
            <div id="brep-result" style="display:none;margin-top:8px"></div>
        </div>

        <div class="op-card">
            <div class="oc-title">Erase Index Entry <span style="font-size:10px;color:var(--text3)">file_erase_line на .index</span></div>
            <div class="form-group" style="margin-bottom:8px">
                <label>Ключ (include trailing space)</label>
                <input type="text" id="eidx-key" class="inp-full" placeholder="mykey ">
            </div>
            <button class="btn btn-danger" onclick="doEraseIndexOp()">Erase from Index</button>
            <div id="eidx-result" style="display:none;margin-top:8px"></div>
        </div>

    </div>
    `;
}

async function doDefragData() {
    if (!confirm('Запустить дефрагментацию бинарных данных?')) return;
    const r = await api('defrag_data', {
        table: state.current.name,
        key:   document.getElementById('dfd-key').value,
        mode:  document.getElementById('dfd-mode').value,
    });
    const el = document.getElementById('dfd-result');
    el.style.display = 'block';
    el.innerHTML = r.ok
        ? `<div class="notif notif-ok">✓ Дефраг завершён, удалено ${r.result}</div>`
        : `<div class="notif notif-err">✕ Ошибка: ${r.result}</div>`;
    loadTables();
}

async function doBinaryReplicate() {
    const target = document.getElementById('brep-target').value;
    if (!target) { notify('Укажите целевой файл', 'warn'); return; }
    const r = await api('replicate', {
        table:  state.current.name,
        target: target,
        mode:   document.getElementById('brep-mode').value,
    });
    const el = document.getElementById('brep-result');
    el.style.display = 'block';
    el.innerHTML = r.ok
        ? `<div class="notif notif-ok">✓ Скопировано ${r.result} байт</div>`
        : `<div class="notif notif-err">✕ Ошибка: ${r.result}</div>`;
    loadTables();
}

async function doEraseIndexOp() {
    const key = document.getElementById('eidx-key').value;
    const fd = new FormData();
    fd.append('ajax','1'); fd.append('op','erase_line');
    fd.append('table', state.current.name + '.index');
    fd.append('key', key); fd.append('position','0'); fd.append('mode','0');
    const r = await (await fetch(location.pathname+'?ajax=1', {method:'POST',body:fd})).json();
    const el = document.getElementById('eidx-result');
    el.style.display = 'block';
    el.innerHTML = r.ok
        ? `<div class="notif notif-ok">✓ Стёрто (offset ${r.result})</div>`
        : `<div class="notif notif-err">✕ Ошибка: ${r.result}</div>`;
}

// ════════════════════════════════
//  ANALYZE
// ════════════════════════════════
function tplAnalize() {
    const isBinary = state.current?.type === 'binary';
    return `
    <div class="section-title">Анализ <span class="fn-name">file_analize</span></div>
    <div class="form-row" style="margin-bottom:12px">
        <div class="form-group">
            <label>Mode</label>
            <select id="anal-mode" onchange="loadAnalize()">
                <option value="0">0: весь файл</option>
                <option value="1">1: первая строка</option>
            </select>
        </div>
        <button class="btn btn-primary" style="margin-top:18px" onclick="loadAnalize()">↻ Refresh</button>
    </div>
    <div id="anal-stats" class="stat-grid"></div>
    <div id="anal-raw" class="result-area" style="margin-top:8px"></div>

    ${isBinary ? `
    <div class="sep"></div>
    <div class="section-title">Index Analyze <span class="fn-name">file_analize(.index)</span></div>
    <div id="anal-idx-stats" class="stat-grid"></div>
    <div id="anal-idx-raw" class="result-area"></div>
    ` : ''}
    `;
}

async function loadAnalize() {
    const t = state.current;
    const mode = parseInt(document.getElementById('anal-mode')?.value || 0);
    const r = await api('analize', { table: t.name, mode });
    const res = r.result || {};

    const statsEl = document.getElementById('anal-stats');
    const rawEl   = document.getElementById('anal-raw');

    statsEl.innerHTML = [
        ['line_count', res.line_count, ''],
        ['file_size', fmtSize(res.file_size || 0), 'green'],
        ['min_length', res.min_length, ''],
        ['max_length', res.max_length, ''],
        ['avg_length', Math.round(res.avg_length || 0), 'purple'],
        ['flow_interruption', res.flow_interruption, res.flow_interruption > 0 ? 'orange' : ''],
        ['last_symbol', res.last_symbol + (res.last_symbol === 10 ? ' (\\n)' : ''), ''],
        ['total_chars', res.total_characters, ''],
    ].map(([label, val, cls]) => `
        <div class="stat-card">
            <div class="sc-label">${label}</div>
            <div class="sc-value ${cls}">${val ?? '—'}</div>
        </div>
    `).join('');

    rawEl.innerHTML = `<pre>${escHtml(JSON.stringify(res, null, 2))}</pre>`;

    if (t.type === 'binary') {
        const fd = new FormData();
        fd.append('ajax','1'); fd.append('op','analize');
        fd.append('table', t.name + '.index'); fd.append('mode', mode);
        const ri = await (await fetch(location.pathname+'?ajax=1',{method:'POST',body:fd})).json();
        const ri2 = ri.result || {};
        const idxStats = document.getElementById('anal-idx-stats');
        const idxRaw   = document.getElementById('anal-idx-raw');
        if (idxStats) idxStats.innerHTML = [
            ['index lines', ri2.line_count, ''],
            ['index size', fmtSize(ri2.file_size || 0), 'green'],
            ['avg_length', Math.round(ri2.avg_length || 0), 'purple'],
        ].map(([label, val, cls]) => `
            <div class="stat-card"><div class="sc-label">${label}</div><div class="sc-value ${cls}">${val ?? '—'}</div></div>
        `).join('');
        if (idxRaw) idxRaw.innerHTML = `<pre>${escHtml(JSON.stringify(ri2, null, 2))}</pre>`;
    }
}

// ════════════════════════════════
//  CREATE / DROP TABLE
// ════════════════════════════════
async function createTable() {
    const name = document.getElementById('new-table-name').value.trim();
    const type = document.getElementById('new-table-type').value;
    if (!name) return;
    const r = await api('create_table', { name, type });
    if (r.ok) {
        document.getElementById('new-table-name').value = '';
        await loadTables();
        selectTable(r.name);
    } else {
        alert('Ошибка: ' + r.error);
    }
}

async function dropTable() {
    const t = state.current;
    if (!t || !confirm(`Удалить таблицу "${t.name}"?`)) return;
    await api('drop_table', { table: t.name });
    state.current = null;
    document.getElementById('table-view').classList.remove('visible');
    document.getElementById('welcome').style.display = '';
    await loadTables();
}

// ════════════════════════════════
//  UTILS
// ════════════════════════════════
function escHtml(s) {
    return String(s)
        .replace(/&/g, '&amp;')
        .replace(/</g, '&lt;')
        .replace(/>/g, '&gt;')
        .replace(/"/g, '&quot;');
}

function escAttr(s) { return escHtml(s).replace(/'/g, '&#39;'); }

// ═══════════════════════════════
//  KEYBOARD
// ════════════════════════════════
document.addEventListener('keydown', e => {
    if (e.key === 'Escape') {
        document.querySelectorAll('.modal-overlay.open').forEach(m => m.classList.remove('open'));
    }
    if (e.key === 'Enter' && e.target.id === 'new-table-name') {
        createTable();
    }
});

// ════════════════════════════════
//  INIT
// ════════════════════════════════
loadTables();
</script>
</body>
</html>
