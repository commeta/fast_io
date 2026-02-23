<?php
$message = '';

if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    if (!csrf_verify()) { flash('error', 'CSRF ошибка.'); redirect(ADMIN_PREFIX . '/maintenance'); }

    $action = $_POST['action'] ?? '';
    if ($action === 'defrag') {
        DB::defrag();
        $message = 'Дефрагментация завершена.';
    }
    if ($action === 'clear_sessions') {
        // Очистить сессии кроме текущей
        $message = 'Не реализовано в базовой конфигурации PHP.';
    }
}
?>
<h1>Обслуживание</h1>

<?php if ($message): ?>
<div class="alert alert-success"><?= h($message) ?></div>
<?php endif; ?>

<div class="card">
    <h2>Дефрагментация файлов данных</h2>
    <p style="color:#666;margin-bottom:1rem">
        При обновлении и удалении страниц в файле данных накапливаются устаревшие записи.
        Дефрагментация очищает их, уменьшая размер файла.
    </p>
    <?php
    $info = file_exists(DATA_DIR . '/pages.dat') ? file_analize(DATA_DIR . '/pages.dat') : [];
    if ($info): ?>
    <table style="margin-bottom:1rem">
        <tr><td>Файл pages.dat</td><td><?= number_format($info['file_size'] ?? 0) ?> байт</td></tr>
        <tr><td>Строк в индексе</td><td><?= $info['line_count'] ?? '—' ?></td></tr>
        <tr><td>Активных страниц</td><td><?= count(DB::getAllPages()) ?></td></tr>
    </table>
    <?php endif; ?>
    <form method="post">
        <?= csrf_field() ?>
        <input type="hidden" name="action" value="defrag">
        <button type="submit" class="btn btn-primary">🔧 Запустить дефрагментацию</button>
    </form>
</div>

<div class="card">
    <h2>Информация о fast_io</h2>
    <table>
        <tr><td>Расширение загружено</td><td><?= extension_loaded('fast_io') ? '✅ Да' : '❌ Нет' ?></td></tr>
        <tr><td>fast_io.buffer_size</td><td><?= ini_get('fast_io.buffer_size') ?> байт</td></tr>
        <tr><td>data/ директория</td><td><?= is_writable(DATA_DIR) ? '✅ Доступна для записи' : '❌ Нет прав на запись' ?></td></tr>
    </table>
</div>
