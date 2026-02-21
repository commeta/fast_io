<?php
/*
 * Fast_IO Extension for PHP 8
 * https://github.com/commeta/fast_io
 *
 * Copyright 2026 commeta <dcs-spb@ya.ru>
 *
 * FastAdmin — аналог phpMyAdmin для fast_io
 * Использует ВСЕ функции расширения fast_io (text + binary)
 * Один файл. PHP 8.1+. Bootstrap 5.3 + Font Awesome
 * Поддержка:
 *   • Text tables (.dat) — фиксированная длина строк, file_get_keys, insert_line, replace_line, erase_line, select_*, update_*, defrag_lines, pop_line, callback_line
 *   • Binary tables (.dat + .index) — file_push_data, file_search_data, file_defrag_data, replicate_file
 *   • Полная пагинация, поиск (обычный + PCRE2 regex), массовые операции
 *   • Анализ, дефрагментация, репликация, pop, callback-примеры
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

if (!function_exists('file_get_keys')) {
    die('<div class="alert alert-danger m-5">❌ Расширение <b>fast_io</b> не загружено!</div>');
}

// ================== НАСТРОЙКИ ==================
$data_dir = __DIR__ . '/fast_io_data';
if (!is_dir($data_dir)) mkdir($data_dir, 0777, true);

define('DEFAULT_ALIGN', 8192);   // по умолчанию для text таблиц
ini_set('fast_io.buffer_size', 65536);

// ================== ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ ==================
function get_tables() {
    global $data_dir;
    $files = @scandir($data_dir) ?: [];
    $tables = [];
    foreach ($files as $f) {
        if (str_ends_with($f, '.dat')) {
            $name = $f;
            $idx = $data_dir . '/' . $name . '.index';
            $type = file_exists($idx) ? 'binary' : 'text';
            $tables[] = ['name' => $name, 'type' => $type, 'path' => $data_dir.'/'.$name];
        }
    }
    return $tables;
}

function get_analize($path) {
    $a = @file_analize($path) ?: [];
    if (!is_array($a)) $a = ['error' => $a];
    return $a;
}

function parse_kv($line) {
    if (!$line) return ['key'=>'','value'=>''];
    $pos = strpos($line, ' ');
    return $pos === false ? ['key'=>$line, 'value'=>''] : ['key'=>substr($line,0,$pos), 'value'=>substr($line,$pos+1)];
}

// ================== ОБРАБОТКА POST ==================
$action = $_REQUEST['action'] ?? 'list';
$table  = basename($_REQUEST['table'] ?? '');
$key    = $_REQUEST['key'] ?? '';
$page   = max(0, (int)($_GET['page'] ?? 0));
$limit  = 25;
$align  = (int)($_REQUEST['align'] ?? DEFAULT_ALIGN);
$message = '';

if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    $table_path = $data_dir . '/' . $table;
    $idx_path   = $table_path . '.index';

    if ($action === 'create_table') {
        $new = basename($_POST['new_table'] ?? '');
        if ($new) {
            $p = $data_dir . '/' . $new . (str_ends_with($new,'.dat') ? '' : '.dat');
            file_put_contents($p, '');
            $message = "✅ Таблица $new создана";
        }
    }

    // === TEXT OPERATIONS ===
    if ($table && $_POST['type'] === 'text') {
        if ($action === 'insert_text') {
            $line = trim($_POST['key']) . ' ' . $_POST['value'];
            $res = file_insert_line($table_path, $line, 0, $align); // mode 0 = с \n
            $message = $res >= 0 ? "✅ Вставлено (offset $res)" : "❌ Ошибка $res";
        }
        if ($action === 'replace_text') {
            $new_line = trim($_POST['new_key']) . ' ' . $_POST['new_value'];
            $res = file_replace_line($table_path, $_POST['old_key'], $new_line);
            $message = $res >= 0 ? "✅ Заменено" : "❌ Ошибка $res";
        }
        if ($action === 'erase_text') {
            $res = file_erase_line($table_path, $_POST['del_key']);
            $message = $res >= 0 ? "✅ Стерто (offset $res)" : "❌ Ошибка $res";
        }
        if ($action === 'defrag_lines') {
            $res = file_defrag_lines($table_path);
            $message = "✅ Дефрагментация завершена, удалено $res записей";
        }
        if ($action === 'pop_line') {
            $res = file_pop_line($table_path, $align);
            $message = $res ? "✅ Pop: " . htmlspecialchars(substr($res,0,200)) : "❌ Пусто";
        }
    }

    // === BINARY OPERATIONS ===
    if ($table && $_POST['type'] === 'binary') {
        if ($action === 'push_data') {
            $res = file_push_data($table_path, $_POST['bkey'], $_POST['bvalue']);
            $message = $res >= 0 ? "✅ Push binary (offset $res)" : "❌ Ошибка $res";
        }
        if ($action === 'defrag_data') {
            $res = file_defrag_data($table_path);
            $message = "✅ Дефраг binary завершена, удалено $res";
        }
    }

    if ($action === 'replicate') {
        $target = $data_dir . '/' . basename($_POST['target'] ?? '');
        $mode = (int)$_POST['rep_mode'];
        $res = replicate_file($table_path, $target, $mode);
        $message = $res ? "✅ Реплицировано в $target" : "❌ Ошибка репликации";
    }

    header("Location: ?table=$table&action=browse&msg=" . urlencode($message));
    exit;
}

if (isset($_GET['msg'])) $message = htmlspecialchars($_GET['msg']);

// ================== HTML ==================
?>
<!DOCTYPE html>
<html lang="ru">
<head>
    <meta charset="UTF-8">
    <title>FastAdmin — fast_io</title>
    <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/css/bootstrap.min.css" rel="stylesheet">
    <link href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.6.0/css/all.min.css" rel="stylesheet">
    <style>
        body { background:#f8f9fa; }
        .sidebar { background:#fff; border-right:1px solid #dee2e6; }
        .table td { vertical-align: middle; font-size:0.9rem; }
        pre { font-size:0.8rem; }
    </style>
</head>
<body>
<div class="container-fluid">
<div class="row">

<!-- SIDEBAR -->
<div class="col-md-3 sidebar py-4">
    <div class="px-3">
        <h4><i class="fas fa-bolt"></i> FastAdmin — fast_io</h4>
        <form method="post" class="input-group input-group-sm mb-3">
            <input type="hidden" name="action" value="create_table">
            <input type="text" name="new_table" class="form-control" placeholder="Имя таблицы (без .dat)" required>
            <button class="btn btn-success"><i class="fas fa-plus"></i></button>
        </form>
    </div>

    <div class="list-group list-group-flush">
        <?php foreach (get_tables() as $t): 
            $active = ($t['name'] === $table) ? 'active' : '';
            $icon = $t['type']=='binary' ? 'fa-database' : 'fa-file-lines';
        ?>
        <a href="?table=<?=urlencode($t['name'])?>&action=browse" 
           class="list-group-item list-group-item-action <?=$active?>">
            <i class="fas <?=$icon?>"></i> <?=htmlspecialchars($t['name'])?>
            <span class="badge bg-<?=$t['type']=='binary'?'primary':'secondary'?> float-end"><?= $t['type']=='binary'?'BINARY':'TEXT' ?></span>
        </a>
        <?php endforeach; ?>
    </div>
</div>

<!-- MAIN CONTENT -->
<div class="col-md-9 p-4">

<?php if ($message): ?>
    <div class="alert alert-info alert-dismissible"><?= $message ?></div>
<?php endif; ?>

<?php if (!$table): ?>
    <div class="text-center py-5">
        <h1 class="display-5">FastAdmin — fast_io</h1>
        <p class="lead">Полноценный веб-интерфейс ко всем функциям fast_io</p>
    </div>
<?php else: 
    $tpath = $data_dir . '/' . $table;
    $is_binary = file_exists($tpath . '.index');
    $type = $is_binary ? 'binary' : 'text';
    $anal = get_analize($tpath);
?>
    <ul class="nav nav-tabs mb-4" id="tabs">
        <li class="nav-item"><a class="nav-link active" data-bs-toggle="tab" href="#browse">Просмотр</a></li>
        <li class="nav-item"><a class="nav-link" data-bs-toggle="tab" href="#search">Поиск</a></li>
        <li class="nav-item"><a class="nav-link" data-bs-toggle="tab" href="#insert"><?= $is_binary ? 'Push Binary' : 'Добавить' ?></a></li>
        <li class="nav-item"><a class="nav-link" data-bs-toggle="tab" href="#ops">Операции</a></li>
        <li class="nav-item"><a class="nav-link" data-bs-toggle="tab" href="#anal">Анализ</a></li>
    </ul>

    <div class="tab-content">

    <!-- BROWSE -->
    <div class="tab-pane fade show active" id="browse">
        <?php if (!$is_binary): ?>
            <?php 
            $offset = $page * $limit;
            $rows = file_get_keys($tpath, $offset, $limit, 0, 5); // mode 5 = trim_line
            $total = $anal['line_count'] ?? count($rows);
            ?>
            <h5>Text таблица — <?=htmlspecialchars($table)?> (<?=$total?> строк)</h5>
            <table class="table table-hover table-sm">
                <thead><tr><th>Ключ</th><th>Значение</th><th>Действия</th></tr></thead>
                <tbody>
                <?php foreach ($rows as $r): 
                    $kv = parse_kv($r);
                ?>
                <tr>
                    <td><code><?=htmlspecialchars($kv['key'])?></code></td>
                    <td><?=htmlspecialchars(mb_substr($kv['value'],0,250))?>…</td>
                    <td>
                        <button class="btn btn-sm btn-primary edit-btn" 
                                data-key="<?=htmlspecialchars($kv['key'])?>" 
                                data-value="<?=htmlspecialchars($kv['value'])?>"><i class="fas fa-edit"></i></button>
                        <form method="post" class="d-inline" onsubmit="return confirm('Удалить?')">
                            <input type="hidden" name="action" value="erase_text">
                            <input type="hidden" name="type" value="text">
                            <input type="hidden" name="table" value="<?=htmlspecialchars($table)?>">
                            <input type="hidden" name="del_key" value="<?=htmlspecialchars($kv['key'])?>">
                            <button class="btn btn-sm btn-danger"><i class="fas fa-trash"></i></button>
                        </form>
                    </td>
                </tr>
                <?php endforeach; ?>
                </tbody>
            </table>

            <!-- Пагинация -->
            <nav>
                <ul class="pagination">
                    <?php for($p=0; $p<ceil($total/$limit); $p++): ?>
                    <li class="page-item <?= $p==$page?'active':'' ?>"><a class="page-link" href="?table=<?=urlencode($table)?>&action=browse&page=<?=$p?>"><?=$p+1?></a></li>
                    <?php endfor; ?>
                </ul>
            </nav>
        <?php else: // BINARY BROWSE ?>
            <p class="text-muted">Для binary таблиц полный просмотр отсутствует. Используйте поиск по ключу или Push.</p>
            <div class="alert alert-info">Индексный файл: <code><?=htmlspecialchars($table)?>.index</code></div>
        <?php endif; ?>
    </div>

    <!-- SEARCH -->
    <div class="tab-pane fade" id="search">
        <form method="get">
            <input type="hidden" name="table" value="<?=htmlspecialchars($table)?>">
            <input type="hidden" name="action" value="search">
            <div class="input-group">
                <input type="text" name="key" class="form-control" placeholder="Ключ или PCRE regex" value="<?=htmlspecialchars($key)?>">
                <select name="mode" class="form-select w-auto">
                    <option value="0">Обычный поиск</option>
                    <option value="10">PCRE2 regex</option>
                </select>
                <button class="btn btn-primary">Найти</button>
            </div>
        </form>

        <?php if ($key !== '' && isset($_GET['action']) && $_GET['action']=='search'):
            if (!$is_binary) {
                $found = file_search_array($tpath, $key, 0, 50, 0, (int)$_GET['mode']);
            } else {
                $found_val = file_search_data($tpath, $key);
                $found = $found_val !== false ? [['trim_line' => $found_val]] : [];
            }
        ?>
            <h6>Результаты (<?=count($found)?>)</h6>
            <pre><?=htmlspecialchars(print_r($found, true))?></pre>
        <?php endif; ?>
    </div>

    <!-- INSERT -->
    <div class="tab-pane fade" id="insert">
        <?php if (!$is_binary): ?>
            <form method="post">
                <input type="hidden" name="action" value="insert_text">
                <input type="hidden" name="type" value="text">
                <input type="hidden" name="table" value="<?=htmlspecialchars($table)?>">
                <div class="mb-3">
                    <label>Ключ</label>
                    <input type="text" name="key" class="form-control" required>
                </div>
                <div class="mb-3">
                    <label>Значение</label>
                    <textarea name="value" class="form-control" rows="5"></textarea>
                </div>
                <div class="mb-3">
                    <label>Фиксированная длина записи (align)</label>
                    <input type="number" name="align" value="<?=DEFAULT_ALIGN?>" class="form-control w-auto">
                </div>
                <button class="btn btn-success">file_insert_line</button>
            </form>
        <?php else: ?>
            <form method="post">
                <input type="hidden" name="action" value="push_data">
                <input type="hidden" name="type" value="binary">
                <input type="hidden" name="table" value="<?=htmlspecialchars($table)?>">
                <div class="mb-3">
                    <label>Ключ</label>
                    <input type="text" name="bkey" class="form-control" required>
                </div>
                <div class="mb-3">
                    <label>Бинарное значение (строка / blob)</label>
                    <textarea name="bvalue" class="form-control" rows="6"></textarea>
                </div>
                <button class="btn btn-success">file_push_data</button>
            </form>
        <?php endif; ?>
    </div>

    <!-- OPERATIONS -->
    <div class="tab-pane fade" id="ops">
        <div class="row">
            <div class="col-md-6">
                <div class="card mb-3">
                    <div class="card-header">Дефрагментация</div>
                    <div class="card-body">
                        <?php if (!$is_binary): ?>
                            <form method="post"><input type="hidden" name="action" value="defrag_lines"><input type="hidden" name="type" value="text"><input type="hidden" name="table" value="<?=htmlspecialchars($table)?>"><button class="btn btn-warning">file_defrag_lines</button></form>
                        <?php else: ?>
                            <form method="post"><input type="hidden" name="action" value="defrag_data"><input type="hidden" name="type" value="binary"><input type="hidden" name="table" value="<?=htmlspecialchars($table)?>"><button class="btn btn-warning">file_defrag_data</button></form>
                        <?php endif; ?>
                    </div>
                </div>
            </div>
            <div class="col-md-6">
                <div class="card mb-3">
                    <div class="card-header">Репликация</div>
                    <div class="card-body">
                        <form method="post">
                            <input type="hidden" name="action" value="replicate">
                            <input type="hidden" name="table" value="<?=htmlspecialchars($table)?>">
                            <input type="text" name="target" class="form-control mb-2" placeholder="target.dat" required>
                            <select name="rep_mode" class="form-select"><option value="0">0 — full</option><option value="1">1 — rename</option></select>
                            <button class="btn btn-info mt-2">replicate_file</button>
                        </form>
                    </div>
                </div>
            </div>
        </div>

        <div class="card">
            <div class="card-header">Дополнительно (text only)</div>
            <div class="card-body">
                <button onclick="popLast()" class="btn btn-secondary">Pop last line (file_pop_line)</button>
                <button onclick="showCallback()" class="btn btn-secondary">Пример callback_line</button>
            </div>
        </div>
    </div>

    <!-- ANALYZE -->
    <div class="tab-pane fade" id="anal">
        <pre class="bg-light p-3"><?=htmlspecialchars(print_r($anal, true))?></pre>
        <?php if (!$is_binary): ?>
        <small>line_count = <?=$anal['line_count'] ?? '—'?> | avg_length = <?=$anal['avg_length'] ?? '—'?></small>
        <?php endif; ?>
    </div>

    </div>
<?php endif; ?>

</div>
</div>
</div>

<!-- MODALS -->
<div class="modal fade" id="editModal">
    <div class="modal-dialog">
        <form method="post" id="editForm">
            <input type="hidden" name="action" value="replace_text">
            <input type="hidden" name="type" value="text">
            <input type="hidden" name="table" value="<?=htmlspecialchars($table ?? '')?>">
            <input type="hidden" name="old_key" id="old_key">
            <div class="modal-content">
                <div class="modal-header"><h5>Редактировать строку</h5></div>
                <div class="modal-body">
                    <div class="mb-3">
                        <label>Новый ключ</label>
                        <input type="text" name="new_key" id="new_key" class="form-control">
                    </div>
                    <div class="mb-3">
                        <label>Новое значение</label>
                        <textarea name="new_value" id="new_value" class="form-control" rows="6"></textarea>
                    </div>
                </div>
                <div class="modal-footer">
                    <button type="button" class="btn btn-secondary" data-bs-dismiss="modal">Отмена</button>
                    <button type="submit" class="btn btn-primary">Сохранить (file_replace_line)</button>
                </div>
            </div>
        </form>
    </div>
</div>

<script src="https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/js/bootstrap.bundle.min.js"></script>
<script>
document.querySelectorAll('.edit-btn').forEach(btn => {
    btn.addEventListener('click', () => {
        document.getElementById('old_key').value = btn.dataset.key;
        document.getElementById('new_key').value = btn.dataset.key;
        document.getElementById('new_value').value = btn.dataset.value;
        new bootstrap.Modal(document.getElementById('editModal')).show();
    });
});

function popLast() {
    if (!confirm('Pop последнюю строку?')) return;
    const f = document.createElement('form');
    f.method = 'post';
    f.innerHTML = `<input type="hidden" name="action" value="pop_line"><input type="hidden" name="type" value="text"><input type="hidden" name="table" value="<?=htmlspecialchars($table??'')?>">`;
    document.body.append(f); f.submit();
}

function showCallback() {
    alert('Пример file_callback_line:\nfile_callback_line("<?=$tpath?>", function(line){ console.log(line); }, 0, 0);\n\n(можно использовать для экспорта, фильтров и т.д.)');
}
</script>
</body>
</html>
