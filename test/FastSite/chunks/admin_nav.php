<?php
$cur = parse_url($_SERVER['REQUEST_URI'] ?? '/', PHP_URL_PATH);
$is  = fn(string $p) => str_starts_with($cur, ADMIN_PREFIX . $p) ? 'active' : '';
?>
<aside class="adm-sidebar">
    <div class="logo">⚡ Admin</div>
    <?php if (Auth::check()): ?>
    <a href="<?= url(ADMIN_PREFIX) ?>" class="<?= $is('/') ?>">🏠 Дашборд</a>
    <a href="<?= url(ADMIN_PREFIX . '/pages') ?>" class="<?= $is('/pages') ?>">📄 Страницы</a>
    <a href="<?= url(ADMIN_PREFIX . '/maintenance') ?>" class="<?= $is('/maintenance') ?>">🔧 Обслуживание</a>
    <hr style="border-color:#2d3748;margin:1rem 0">
    <a href="<?= url('/') ?>" target="_blank">↗ Сайт</a>
    <?php else: ?>
    <a href="<?= url(ADMIN_PREFIX . '/login') ?>">🔑 Войти</a>
    <?php endif; ?>
</aside>
