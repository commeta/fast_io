<?php
// Страница полностью из БД
global $page_data, $tv;

if (empty($page_data)) {
    http_response_code(404);
    include PAGES_DIR . '/404.php';
    return;
}

$tv = $page_data['tv'] ?? [];
?>
<h1><?= h(tv('pagetitle', $page_data['alias'] ?? '')) ?></h1>
<?php if (tv('longtitle')): ?>
<p style="font-size:1.1rem;color:#555;margin-bottom:1.5rem"><?= h(tv('longtitle')) ?></p>
<?php endif; ?>
<?php echo tv('content', '<p>Содержимое страницы не заполнено.</p>'); ?>
