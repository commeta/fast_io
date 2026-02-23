# Fast CMS (file-based на fast_io)

## Руководство разработчика по скелетону Fast CMS (file-based на fast_io)

Это полностью готовый минималистичный CMS-скелет, который работает **без MySQL/PostgreSQL** — вся база данных лежит в двух файлах (`data/pages.dat` + `data/pages.dat.index` и `data/users.dat`).

### 1. Что это такое

![FastSite Admin Dashboard](https://raw.githubusercontent.com/commeta/fast_io/refs/heads/main/test/FastSite/img/Admin-Dashboard.png "FastSite Admin Dashboard")

- **Ядро** — чистый PHP 8.1+ + расширение **fast_io** (C-расширение для сверхбыстрой работы с файлами как с БД).
- **Хранилище страниц** — ключ `alias` → JSON-объект (все TV-поля внутри).
- **Шаблоны** — как в MODx Revolution (main / wide + TV-поля).
- **Админка** — встроенная, с CRUD страницами, дефрагментацией и статистикой.
- **Безопасность** — CSRF, сессии, bcrypt, file-locks от fast_io.
- **Производительность** — десятки тысяч страниц читаются за миллисекунды.

### 2. Требования

- PHP 8.1+
- Расширение **fast_io** — обязательно!
- Права на запись в папку `data/`
- Apache с mod_rewrite (или nginx + аналог .htaccess)

### 3. Установка за 2 минуты

```bash
# 1. Запустить установку
php data/install.php
# → создаст пользователя admin / admin123 + демо-страницы

# 2. УДАЛИТЬ install.php !!!
rm data/install.php
```
После этого сайт уже работает по адресу `/`.

### 4. Структура проекта

```
/
├── .htaccess
├── index.php                 ← единственная точка входа
├── core/
│   ├── .htaccess
│   ├── config.php
│   ├── helpers.php
│   ├── db.php
│   ├── auth.php
│   ├── router.php
│   └── routes.php
├── data/                     ← здесь всё хранится
│   ├── .htaccess
│   ├── pages.dat
│   ├── pages.dat.index
│   └── users.dat
├── templates/
│   ├── .htaccess
│   ├── main.php
│   ├── wide.php
│   └── admin.php
├── chunks/
│   ├── .htaccess
│   ├── header.php
│   ├── footer.php
│   ├── nav.php
│   ├── breadcrumbs.php
│   └── admin_nav.php
└── pages/
    ├── .htaccess
    ├── home.php
    ├── about.php
    ├── contacts.php
    ├── dynamic.php
    ├── 404.php
    └── admin/...
```

### 5. Конфигурация (core/config.php)

```php
define('SITE_NAME',    'My Fast Site');
define('SITE_URL',     'https://example.com');   // без слеша в конце!
define('ADMIN_PREFIX', '/admin');
define('SESSION_NAME', 'fast_admin_sess');
define('CSRF_SECRET',  'CHANGE_THIS_32CHARS_MIN!!');

define('TEMPLATES', [
    'main' => 'Main (1 колонка)',
    'wide' => 'Wide (с сайдбаром)',
]);

define('TV_FIELDS', [ ... ]);   // все поля шаблонов
```

### 6. Как добавить новую статическую страницу

1. Создай файл `pages/my-page.php`
2. Добавь маршрут в `core/routes.php`:

```php
$router->add('/my-page', [
    'template' => 'main',
    'page'     => 'my-page',
]);
```

Готово. Теперь `/my-page` работает.

### 7. Динамические страницы (из БД)

![FastSite Admin Pages](https://raw.githubusercontent.com/commeta/fast_io/refs/heads/main/test/FastSite/img/Admin-Pages.png "FastSite Admin Pages")

Все страницы с алиасом, которых нет в статических маршрутах, автоматически ищутся в `DB::getPage($alias)`.

Шаблон по умолчанию — `dynamic.php`.

![FastSite Admin Edit](https://raw.githubusercontent.com/commeta/fast_io/refs/heads/main/test/FastSite/img/Admin-Edit.png "FastSite Admin Edit")


### 8. TV-поля (Template Variables)

В `config.php` описываются поля для каждого шаблона:

```php
'main' => [
    'pagetitle'   => ['label' => 'Page Title', 'type' => 'text'],
    'content'     => ['label' => 'Page Content', 'type' => 'richtext'],
    // ...
]
```

В любой странице/чанке/шаблоне:

```php
<?= h(tv('pagetitle')) ?>
<?= tv('content') ?>          <!-- HTML без экранирования -->
```

### 9. Чанки (chunks)

```php
<?= chunk('header') ?>
<?= chunk('nav', ['extra' => 'value']) ?>
```

### 10. Админка (/admin)

![FastSite Admin Login](https://raw.githubusercontent.com/commeta/fast_io/refs/heads/main/test/FastSite/img/Admin-Login.png "FastSite Admin Login")

- `/admin` — дашборд
- `/admin/pages` — список + поиск + удаление
- `/admin/pages/edit?alias=...` — редактирование/создание
- `/admin/maintenance` — дефрагментация

Логин/пароль после установки: `admin` / `admin123` (сразу смените!).

### 11. Класс DB — как работать с данными

```php
// Получить страницу
$page = DB::getPage('about');

// Все страницы (отсортированы по nav_order)
$all = DB::getAllPages();

// Сохранить/обновить
DB::savePage([
    'alias'     => 'new-page',
    'template'  => 'main',
    'published' => 1,
    'tv'        => [
        'pagetitle' => 'Новая страница',
        'content'   => '<p>Hello world</p>',
    ]
]);

// Удалить
DB::deletePage('old-page');
```

### 12. Полезные хелперы

```php
h($str)          // htmlspecialchars
url('/about')    // SITE_URL + путь
redirect('/admin')
flash('success', 'Готово!')   // один раз показывается в админке
csrf_field()     // <input name="_csrf" ...>
csrf_verify()
tv('field', 'default')
```

### 13. Расширение скелетона

**Добавить новый шаблон**
1. Создать `templates/newtpl.php`
2. Добавить в `TEMPLATES` и `TV_FIELDS`
3. Готово.

**Добавить новое TV-поле**
Просто добавь в массив `TV_FIELDS['main']` или нужного шаблона.

**Добавить маршрут с авторизацией**
```php
$router->add('/secret', [
    'template' => 'main',
    'page'     => 'secret',
    'auth'     => true,
]);
```

### 14. Обслуживание (Maintenance)

В админке → Обслуживание → «Запустить дефрагментацию»  
Рекомендуется запускать после большого количества удалений/изменений алиасов.

![FastSite Admin Service](https://raw.githubusercontent.com/commeta/fast_io/refs/heads/main/test/FastSite/img/Admin-Service.png "FastSite Admin Service")

### 15. Безопасность и производительность

- Все операции с файлами блокируются `flock(LOCK_EX)`
- CSRF на всех POST-формах
- Пароли — bcrypt
- Нет SQL-инъекций (нет SQL вообще)
- При 10 000 страницах `DB::getAllPages()` работает быстрее любой ORM на MySQL

### 16. Что дальше?

Хочешь:
- TinyMCE / Quill в админке?
- Многоязычность?
- Кэширование?
- API?

Просто добавляй новые файлы в `pages/admin/` и новые маршруты — всё остальное уже работает.

![FastSite](https://raw.githubusercontent.com/commeta/fast_io/refs/heads/main/test/FastSite/img/Fast-Site.png "FastSite")


### 17. Дополнительные меры безопасности (ОБЯЗАТЕЛЬНО после установки!)

Хотя корневой `.htaccess` уже запрещает листинг директорий (`Options -Indexes`), этого **недостаточно**.  
Чтобы полностью закрыть доступ к исходному коду и данным сайта, **в каждый важный подкаталог** нужно положить свой защитный `.htaccess`.

#### Рекомендуемый защитный .htaccess (старый стиль, работает везде)

Создайте файл `.htaccess` в следующих папках со **строго** таким содержимым:

```apache
# === ЗАПРЕТ ПРЯМОГО ДОСТУПА К СИСТЕМНЫМ ПАПКАМ ===
Order Deny,Allow
Deny from all
```

#### Современный вариант (Apache 2.4+ — рекомендуется)

```apache
# === ЗАПРЕТ ПРЯМОГО ДОСТУПА К СИСТЕМНЫМ ПАПКАМ ===
<Files "*">
    Require all denied
</Files>
```

#### Какие папки **обязательно** защитить

| Папка              | Критичность | Почему важно |
|--------------------|-------------|--------------|
| `data/`            | ★★★★★      | Здесь `users.dat` (хэши паролей) и весь контент сайта |
| `core/`            | ★★★★★      | Ядро CMS, пароли, маршруты, конфиг |
| `chunks/`          | ★★★★       | Чанки могут содержать логику |
| `pages/admin/`     | ★★★★       | Админ-панель |
| `pages/`           | ★★★        | Статические страницы |
| `templates/`       | ★★★        | Шаблоны (можно открыть, если хотите) |

#### Проверка

После создания откройте в браузере, например:
- `https://ваш-сайт.ru/data/`
- `https://ваш-сайт.ru/core/`
- `https://ваш-сайт.ru/pages/admin/`

Должна быть **ошибка 403 Forbidden**.

---
