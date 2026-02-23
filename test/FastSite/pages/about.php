<?php
$dbPage = DB::getPage('about');
if ($dbPage) { global $tv; $tv = $dbPage['tv'] ?? $tv; }
?>
<h1><?= h(tv('pagetitle', 'О нас')) ?></h1>
<?php if ($c = tv('content')): ?>
    <?= $c ?>
<?php else: ?>
<p>Мы используем <strong>fast_io</strong> — PHP-расширение для низкоуровневой
работы с файлами как с базой данных. Никакого SQL, минимальные зависимости.</p>
<p>Движок поддерживает шаблоны, чанки и TV-поля по аналогии с MODx Revolution 2.9.</p>
<?php endif; ?>
