<?php declare(strict_types=1); ?>
<!DOCTYPE html>
<html lang="ru">
<head>
<?php echo chunk('header'); ?>
</head>
<body>
<header class="site-header">
    <div class="container">
        <a class="logo" href="<?= url() ?>"><?= h(SITE_NAME) ?></a>
        <?php echo chunk('nav'); ?>
    </div>
</header>

<main class="site-main site-main--wide">
    <div class="container container--flex">
        <article class="content">
            <?php echo chunk('breadcrumbs'); ?>
            <?php include $page_content_file; ?>
        </article>
        <aside class="sidebar">
            <?php echo tv('sidebar'); ?>
        </aside>
    </div>
</main>

<?php echo chunk('footer'); ?>
</body>
</html>
