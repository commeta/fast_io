<?php
// Читаем страницы с show_in_nav=1, отсортированные по nav_order
$nav_pages = array_filter(
    DB::getAllPages(),
    fn($p) => ($p['published'] ?? 0) && ($p['tv']['show_in_nav'] ?? '1') === '1'
);
usort($nav_pages, fn($a, $b) => ($a['tv']['nav_order'] ?? 0) <=> ($b['tv']['nav_order'] ?? 0));

$current = $GLOBALS['current_route'] ?? [];
$cur_alias = $current['page_data']['alias'] ?? '';
?>
<nav>
    <a href="<?= url('/') ?>"<?= $cur_alias === 'home' ? ' class="active"' : '' ?>>Главная</a>
    <?php foreach ($nav_pages as $np): ?>
    <a href="<?= url($np['alias']) ?>"
       <?= $cur_alias === $np['alias'] ? 'class="active"' : '' ?>>
        <?= h($np['tv']['menu_title'] ?: $np['tv']['pagetitle'] ?: $np['alias']) ?>
    </a>
    <?php endforeach; ?>
</nav>
