<?php
declare(strict_types=1);

/**
 * Статические маршруты.
 * Ключ       = URL-путь
 * template   = файл в templates/
 * page       = файл в pages/ (без .php)
 * auth       = true → требует авторизации
 * page_data  = предзаполненные данные страницы (опционально)
 */

$router->add('/', [
    'template' => 'main',
    'page'     => 'home',
    'page_data'=> ['alias' => 'home'],
]);

$router->add('/about', [
    'template' => 'main',
    'page'     => 'about',
    'page_data'=> ['alias' => 'about'],
]);

$router->add('/contacts', [
    'template' => 'main',
    'page'     => 'contacts',
    'page_data'=> ['alias' => 'contacts'],
]);

// ── Admin ────────────────────────────────────────

$router->add(ADMIN_PREFIX . '/login', [
    'template' => 'admin',
    'page'     => 'admin/login',
]);

$router->add(ADMIN_PREFIX . '/logout', [
    'template' => 'admin',
    'page'     => 'admin/logout',
]);

$router->add(ADMIN_PREFIX, [
    'template' => 'admin',
    'page'     => 'admin/dashboard',
    'auth'     => true,
]);

$router->add(ADMIN_PREFIX . '/pages', [
    'template' => 'admin',
    'page'     => 'admin/pages_list',
    'auth'     => true,
]);

$router->add(ADMIN_PREFIX . '/pages/edit', [
    'template' => 'admin',
    'page'     => 'admin/pages_edit',
    'auth'     => true,
]);

$router->add(ADMIN_PREFIX . '/maintenance', [
    'template' => 'admin',
    'page'     => 'admin/maintenance',
    'auth'     => true,
]);
