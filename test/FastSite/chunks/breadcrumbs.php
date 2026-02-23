<?php
$path  = parse_url($_SERVER['REQUEST_URI'] ?? '/', PHP_URL_PATH);
$parts = array_filter(explode('/', trim($path, '/')));
$crumbs = [['title' => 'Главная', 'url' => url('/')]];
$built = '';
foreach ($parts as $part) {
    $built .= '/' . $part;
    $pg = DB::getPage($part);
    $crumbs[] = [
        'title' => $pg['tv']['pagetitle'] ?? ucfirst($part),
        'url'   => url($built),
    ];
}
if (count($crumbs) <= 1) return; // только главная — не показываем
?>
<nav class="breadcrumbs" style="margin-bottom:1rem;font-size:.9rem;color:#666">
    <?php foreach ($crumbs as $i => $c): ?>
        <?= $i > 0 ? ' / ' : '' ?>
        <?php if ($i < count($crumbs) - 1): ?>
            <a href="<?= h($c['url']) ?>"><?= h($c['title']) ?></a>
        <?php else: ?>
            <span><?= h($c['title']) ?></span>
        <?php endif; ?>
    <?php endforeach; ?>
</nav>
