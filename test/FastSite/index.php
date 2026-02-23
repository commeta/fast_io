<?php
declare(strict_types=1);

define('ROOT_DIR', __DIR__);
define('CORE_DIR',      ROOT_DIR . '/core');
define('DATA_DIR',      ROOT_DIR . '/data');
define('TEMPLATES_DIR', ROOT_DIR . '/templates');
define('CHUNKS_DIR',    ROOT_DIR . '/chunks');
define('PAGES_DIR',     ROOT_DIR . '/pages');

require CORE_DIR . '/config.php';
require CORE_DIR . '/helpers.php';
require CORE_DIR . '/db.php';
require CORE_DIR . '/auth.php';
require CORE_DIR . '/router.php';

if (!extension_loaded('fast_io')) {
    die('<h2>fast_io PHP extension required.</h2><p>See <a href="https://github.com/commeta/fast_io">github.com/commeta/fast_io</a></p>');
}

if (!is_dir(DATA_DIR)) {
    mkdir(DATA_DIR, 0755, true);
}

// Redirect to install if no users exist
if (!file_exists(DATA_DIR . '/users.dat') && !str_contains($_SERVER['REQUEST_URI'] ?? '', 'install.php')) {
    // Allow install.php to run freely
}

session_name(SESSION_NAME);
session_start();

$router = new Router();

require CORE_DIR . '/routes.php';

$router->dispatch();
