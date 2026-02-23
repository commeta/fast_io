<?php
/**
 * Скрипт первичной установки.
 * Запустите ОДИН РАЗ: php data/install.php  (или откройте в браузере)
 * После создания admin-пользователя УДАЛИТЕ этот файл.
 */

define('ROOT_DIR', dirname(__DIR__));
define('CORE_DIR',  ROOT_DIR . '/core');
define('DATA_DIR',  ROOT_DIR . '/data');
define('TEMPLATES_DIR', ROOT_DIR . '/templates');
define('CHUNKS_DIR',    ROOT_DIR . '/chunks');
define('PAGES_DIR',     ROOT_DIR . '/pages');

require CORE_DIR . '/config.php';
require CORE_DIR . '/helpers.php';
require CORE_DIR . '/db.php';

if (!extension_loaded('fast_io')) {
    die("fast_io extension not loaded!\n");
}

if (!is_dir(DATA_DIR)) mkdir(DATA_DIR, 0755, true);

// Проверяем, нет ли уже аккаунтов
if (file_exists(DATA_DIR . '/users.dat')) {
    $existing = file_analize(DATA_DIR . '/users.dat');
    if (is_array($existing) && ($existing['line_count'] ?? 0) > 0) {
        die("Пользователи уже созданы. Удалите users.dat для переустановки.\n");
    }
}

// Создание admin-пользователя
$adminUser = 'admin';
$adminPass = 'admin123';   // ← СМЕНИТЕ ПОСЛЕ ВХОДА!

if (!DB::createUser($adminUser, $adminPass)) {
    die("Ошибка создания пользователя!\n");
}

echo "✅ Пользователь «{$adminUser}» создан. Пароль: {$adminPass}\n";

// Создание демо-страниц
$demoPages = [
    [
        'alias' => 'home', 'template' => 'main', 'published' => 1,
        'tv' => [
            'pagetitle' => 'Главная страница', 'longtitle' => 'Добро пожаловать!',
            'description' => 'Главная страница сайта на fast_io',
            'keywords' => 'fast_io, php, cms',
            'menu_title' => 'Главная', 'show_in_nav' => '1', 'nav_order' => '1',
            'og_image' => '', 'content' => '<p>Это содержимое главной страницы из базы данных fast_io.</p>',
        ],
    ],
    [
        'alias' => 'about', 'template' => 'main', 'published' => 1,
        'tv' => [
            'pagetitle' => 'О нас', 'longtitle' => 'Информация о проекте',
            'description' => 'Страница о нас', 'keywords' => 'о нас, контакты',
            'menu_title' => 'О нас', 'show_in_nav' => '1', 'nav_order' => '2',
            'og_image' => '', 'content' => '<p>Проект использует fast_io как файловую базу данных.</p>',
        ],
    ],
    [
        'alias' => 'contacts', 'template' => 'main', 'published' => 1,
        'tv' => [
            'pagetitle' => 'Контакты', 'longtitle' => 'Свяжитесь с нами',
            'description' => 'Страница контактов', 'keywords' => 'контакты',
            'menu_title' => 'Контакты', 'show_in_nav' => '1', 'nav_order' => '3',
            'og_image' => '', 'content' => '',
        ],
    ],
];

foreach ($demoPages as $pageData) {
    if (DB::savePage($pageData)) {
        echo "✅ Страница «{$pageData['alias']}» создана.\n";
    } else {
        echo "❌ Ошибка создания страницы «{$pageData['alias']}».\n";
    }
}

echo "\n✅ Установка завершена!\n";
echo "🔒 УДАЛИТЕ этот файл: rm data/install.php\n";
echo "🔑 Войдите в панель: /admin  (логин: {$adminUser}, пароль: {$adminPass})\n";
echo "⚠️  СМЕНИТЕ ПАРОЛЬ после первого входа!\n";
