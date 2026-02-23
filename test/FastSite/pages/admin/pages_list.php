<?php
// Обработка удаления
if ($_SERVER['REQUEST_METHOD'] === 'POST' && ($_POST['action'] ?? '') === 'delete') {
    if (!csrf_verify()) { flash('error', 'CSRF ошибка.'); redirect(ADMIN_PREFIX . '/pages'); }
    $alias = trim($_POST['alias'] ?? '');
    if ($alias && DB::deletePage($alias)) {
        flash('success', 'Страница «' . $alias . '» удалена.');
    } else {
        flash('error', 'Не удалось удалить страницу.');
    }
    redirect(ADMIN_PREFIX . '/pages');
}

$pages = DB::getAllPages();

// Поиск
$q = trim($_GET['q'] ?? '');
if ($q !== '') {
    $pages = array_filter($pages, fn($p) =>
        str_contains($p['alias'], $q) ||
        str_contains($p['tv']['pagetitle'] ?? '', $q)
    );
}
?>
<div style="display:flex;justify-content:space-between;align-items:center;margin-bottom:1rem">
    <h1 style="margin:0">Страницы (<?= count($pages) ?>)</h1>
    <a href="<?= url(ADMIN_PREFIX . '/pages/edit') ?>" class="btn btn-primary">+ Новая</a>
</div>

<div class="card" style="margin-bottom:1rem">
    <form method="get" style="display:flex;gap:.5rem">
        <input type="text" name="q" value="<?= h($q) ?>" placeholder="Поиск по alias или заголовку…" style="flex:1">
        <button type="submit" class="btn btn-secondary">Найти</button>
        <?php if ($q): ?><a href="<?= url(ADMIN_PREFIX . '/pages') ?>" class="btn btn-secondary">✕</a><?php endif; ?>
    </form>
</div>

<div class="card">
<?php if (empty($pages)): ?>
    <p style="color:#888">Страниц нет. <?= $q ? 'Попробуйте другой запрос.' : '' ?></p>
<?php else: ?>
    <table>
        <thead>
            <tr><th>Alias</th><th>Заголовок</th><th>Шаблон</th><th>Нав.</th><th>Статус</th><th>Изменён</th><th>Действия</th></tr>
        </thead>
        <tbody>
        <?php foreach ($pages as $p):
            $alias  = $p['alias'];
            $pub    = ($p['published'] ?? 0);
            $inNav  = ($p['tv']['show_in_nav'] ?? '1') === '1';
        ?>
        <tr>
            <td><code><?= h($alias) ?></code></td>
            <td><?= h($p['tv']['pagetitle'] ?? '—') ?></td>
            <td><?= h($p['template'] ?? 'main') ?></td>
            <td><?= $inNav ? '✅' : '—' ?></td>
            <td><span class="badge <?= $pub ? 'badge-green' : 'badge-gray' ?>"><?= $pub ? 'Опубл.' : 'Черновик' ?></span></td>
            <td style="white-space:nowrap"><?= date('d.m.Y', $p['updated_at'] ?? 0) ?></td>
            <td style="white-space:nowrap">
                <a href="<?= url(ADMIN_PREFIX . '/pages/edit?alias=' . urlencode($alias)) ?>" class="btn btn-secondary">✏️</a>
                <a href="<?= url($alias) ?>" target="_blank" class="btn btn-secondary">↗</a>
                <form method="post" style="display:inline" onsubmit="return confirm('Удалить «<?= h($alias) ?>»?')">
                    <?= csrf_field() ?>
                    <input type="hidden" name="action" value="delete">
                    <input type="hidden" name="alias"  value="<?= h($alias) ?>">
                    <button type="submit" class="btn btn-danger">🗑</button>
                </form>
            </td>
        </tr>
        <?php endforeach; ?>
        </tbody>
    </table>
<?php endif; ?>
</div>
