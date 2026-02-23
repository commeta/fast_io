<?php
$stats = DB::stats();
$pages = DB::getAllPages();
$recent = array_slice(
    array_reverse(
        usort($pages, fn($a,$b) => ($a['updated_at']??0)<=>($b['updated_at']??0)) ? $pages : $pages
    ), 0, 5
);
?>
<div style="display:grid;grid-template-columns:repeat(3,1fr);gap:1rem;margin-bottom:1.5rem">
    <?php foreach ([
        ['📄', 'Всего страниц',     $stats['total_pages']],
        ['✅', 'Опубликовано',      $stats['published_pages']],
        ['📝', 'Черновиков',        $stats['draft_pages']],
    ] as [$icon, $label, $val]): ?>
    <div class="card" style="text-align:center">
        <div style="font-size:2rem"><?= $icon ?></div>
        <div style="font-size:2rem;font-weight:700"><?= $val ?></div>
        <div style="color:#666;font-size:.9rem"><?= $label ?></div>
    </div>
    <?php endforeach; ?>
</div>

<div class="card">
    <div style="display:flex;justify-content:space-between;align-items:center;margin-bottom:1rem">
        <h2 style="margin:0">Последние страницы</h2>
        <a href="<?= url(ADMIN_PREFIX . '/pages/edit') ?>" class="btn btn-primary">+ Новая страница</a>
    </div>
    <?php $allPages = DB::getAllPages(); ?>
    <?php if (empty($allPages)): ?>
        <p style="color:#888">Страниц ещё нет. <a href="<?= url(ADMIN_PREFIX . '/pages/edit') ?>">Создайте первую</a>.</p>
    <?php else: ?>
    <table>
        <thead><tr><th>Alias</th><th>Заголовок</th><th>Шаблон</th><th>Статус</th><th>Изменён</th></tr></thead>
        <tbody>
        <?php foreach (array_slice(array_reverse($allPages), 0, 5) as $p): ?>
        <tr>
            <td><a href="<?= url(ADMIN_PREFIX . '/pages/edit?alias=' . urlencode($p['alias'])) ?>"><?= h($p['alias']) ?></a></td>
            <td><?= h($p['tv']['pagetitle'] ?? '—') ?></td>
            <td><?= h($p['template'] ?? 'main') ?></td>
            <td><?php $pub = ($p['published'] ?? 0); ?>
                <span class="badge <?= $pub ? 'badge-green' : 'badge-gray' ?>"><?= $pub ? 'Опубликовано' : 'Черновик' ?></span>
            </td>
            <td><?= date('d.m.Y H:i', $p['updated_at'] ?? 0) ?></td>
        </tr>
        <?php endforeach; ?>
        </tbody>
    </table>
    <?php endif; ?>
</div>

<div class="card">
    <h2>Системная информация</h2>
    <table>
        <tr><td>PHP</td><td><?= phpversion() ?></td></tr>
        <tr><td>fast_io</td><td><?= extension_loaded('fast_io') ? '✅ загружен' : '❌ не загружен' ?></td></tr>
        <tr><td>Размер data/pages.dat</td><td><?= number_format($stats['data_file_size']) ?> байт</td></tr>
        <tr><td>fast_io.buffer_size</td><td><?= ini_get('fast_io.buffer_size') ?></td></tr>
    </table>
</div>
