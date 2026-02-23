<?php
// Загружаем данные из БД если они есть, иначе — статика
$dbPage = DB::getPage('home');
if ($dbPage) {
    global $tv;
    $tv = $dbPage['tv'] ?? $tv;
}
?>
<section class="hero" style="text-align:center;padding:3rem 0">
    <h1><?= h(tv('longtitle', 'Добро пожаловать на ' . SITE_NAME)) ?></h1>
    <p style="font-size:1.2rem;color:#666;max-width:600px;margin:1rem auto">
        <?= h(tv('description', 'Быстрый файловый движок на основе fast_io')) ?>
    </p>
    <a href="<?= url('about') ?>" class="btn btn-primary" style="margin-top:1rem">Подробнее</a>
</section>

<?php if ($content = tv('content')): ?>
<section class="content-block"><?= $content ?></section>
<?php else: ?>
<section style="display:grid;grid-template-columns:repeat(3,1fr);gap:1.5rem;margin-top:2rem">
    <?php foreach ([['⚡','Быстро','Хранилище fast_io без SQL'],
                    ['🔒','Безопасно','Блокировки файлов, CSRF'],
                    ['📦','Просто','Без фреймворков и composer']] as [$icon,$title,$desc]): ?>
    <div style="background:#f4f5f7;border-radius:8px;padding:1.5rem;text-align:center">
        <div style="font-size:2rem"><?= $icon ?></div>
        <h2 style="margin:.5rem 0"><?= $title ?></h2>
        <p style="color:#666;font-size:.9rem"><?= $desc ?></p>
    </div>
    <?php endforeach; ?>
</section>
<?php endif; ?>
