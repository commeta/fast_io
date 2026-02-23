<?php
declare(strict_types=1);

define('SITE_NAME',    'My Fast Site');
define('SITE_URL',     '');           // e.g. https://example.com — без слеша на конце
define('ADMIN_PREFIX', '/admin');
define('SESSION_NAME', 'fast_admin_sess');
define('CSRF_SECRET',  'CHANGE_THIS_SECRET_32CHARS_MIN!!');

// fast_io I/O buffer
ini_set('fast_io.buffer_size', '8192');

// Users: fixed-line format. 64 (username) + 1 (space) + 255 (bcrypt) + padding + \n
define('USERS_ALIGN', 384);

// Available templates (alias => label)
define('TEMPLATES', [
    'main' => 'Main (1 column)',
    'wide' => 'Wide (sidebar)',
]);

// TV-поля на шаблон (аналог TV в MODx)
// key => [label, type, default]
define('TV_FIELDS', [
    'main' => [
        'pagetitle'   => ['label' => 'Page Title',         'type' => 'text',     'default' => ''],
        'longtitle'   => ['label' => 'Long Title',         'type' => 'text',     'default' => ''],
        'description' => ['label' => 'Meta Description',   'type' => 'textarea', 'default' => ''],
        'keywords'    => ['label' => 'Meta Keywords',      'type' => 'text',     'default' => ''],
        'og_image'    => ['label' => 'OG Image URL',       'type' => 'text',     'default' => ''],
        'menu_title'  => ['label' => 'Menu Title',         'type' => 'text',     'default' => ''],
        'show_in_nav' => ['label' => 'Show in Nav',        'type' => 'checkbox', 'default' => '1'],
        'nav_order'   => ['label' => 'Nav Order',          'type' => 'number',   'default' => '0'],
        'content'     => ['label' => 'Page Content',       'type' => 'richtext', 'default' => ''],
    ],
    'wide' => [
        'pagetitle'   => ['label' => 'Page Title',         'type' => 'text',     'default' => ''],
        'longtitle'   => ['label' => 'Long Title',         'type' => 'text',     'default' => ''],
        'description' => ['label' => 'Meta Description',   'type' => 'textarea', 'default' => ''],
        'keywords'    => ['label' => 'Meta Keywords',      'type' => 'text',     'default' => ''],
        'menu_title'  => ['label' => 'Menu Title',         'type' => 'text',     'default' => ''],
        'show_in_nav' => ['label' => 'Show in Nav',        'type' => 'checkbox', 'default' => '1'],
        'nav_order'   => ['label' => 'Nav Order',          'type' => 'number',   'default' => '0'],
        'content'     => ['label' => 'Main Content',       'type' => 'richtext', 'default' => ''],
        'sidebar'     => ['label' => 'Sidebar Content',    'type' => 'richtext', 'default' => ''],
    ],
]);
