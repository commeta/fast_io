<?php
declare(strict_types=1);

/**
 * DB — обёртка над fast_io.
 *
 * Страницы хранятся как JSON-значения, ключ = alias страницы.
 *   data/pages.dat       — бинарный блок данных (JSON)
 *   data/pages.dat.index — индекс: alias offset:size
 *
 * TV-поля встроены в JSON страницы под ключом «tv».
 *
 * Пользователи — фиксированная длина строки (users.dat).
 *   формат строки: «username hash»
 */
class DB
{
    const PAGES_FILE = DATA_DIR . '/pages.dat';
    const USERS_FILE = DATA_DIR . '/users.dat';

    // ───────────────── PAGES ─────────────────

    /** Получить страницу по alias */
    public static function getPage(string $alias): ?array
    {
        if (!file_exists(self::PAGES_FILE . '.index')) return null;
        $raw = file_search_data(self::PAGES_FILE, $alias);
        if ($raw === false) return null;
        return json_decode($raw, true) ?: null;
    }

    /** Получить все страницы */
    public static function getAllPages(): array
    {
        $indexFile = self::PAGES_FILE . '.index';
        if (!file_exists($indexFile)) return [];

        // Читаем ключи из индексного файла (mode 4 = только ключи)
        $keys = file_get_keys($indexFile, 0, 10000, 0, 4);
        if (!is_array($keys)) return [];

        $pages = [];
        foreach ($keys as $key) {
            // Пропускаем стёртые записи (начинаются с chr(127))
            if (!isset($key[0]) || ord($key[0]) === 127) continue;
            $raw = file_search_data(self::PAGES_FILE, $key);
            if ($raw === false) continue;
            $page = json_decode($raw, true);
            if ($page) $pages[] = $page;
        }

        // Сортировка по nav_order, затем по created_at
        usort($pages, fn($a, $b) =>
            ($a['tv']['nav_order'] ?? 0) <=> ($b['tv']['nav_order'] ?? 0)
            ?: ($a['created_at'] ?? 0) <=> ($b['created_at'] ?? 0)
        );

        return $pages;
    }

    /** Создать или обновить страницу */
    public static function savePage(array $data): bool
    {
        $alias = trim($data['alias'] ?? '');
        if ($alias === '') return false;

        $indexFile = self::PAGES_FILE . '.index';

        // Если запись уже есть — стираем старый индекс (orphaned data очистит defrag)
        if (file_exists($indexFile)) {
            $existing = file_search_data(self::PAGES_FILE, $alias);
            if ($existing !== false) {
                file_erase_line($indexFile, $alias . ' ');
            }
        }

        $now = time();
        if (!isset($data['created_at'])) $data['created_at'] = $now;
        $data['updated_at'] = $now;

        $json   = json_encode($data, JSON_UNESCAPED_UNICODE | JSON_UNESCAPED_SLASHES);
        $result = file_push_data(self::PAGES_FILE, $alias, $json);

        // Периодический defrag: когда индекс вырос на 20+ записей сверх реальных
        self::maybeDefrag();

        return $result >= 0;
    }

    /** Удалить страницу по alias */
    public static function deletePage(string $alias): bool
    {
        $indexFile = self::PAGES_FILE . '.index';
        if (!file_exists($indexFile)) return false;
        return file_erase_line($indexFile, $alias . ' ') >= 0;
    }

    /** Принудительная дефрагментация данных */
    public static function defrag(): void
    {
        if (file_exists(self::PAGES_FILE)) {
            file_defrag_data(self::PAGES_FILE);
        }
    }

    /** Авто-defrag: если файл индекса > threshold */
    private static function maybeDefrag(): void
    {
        $indexFile = self::PAGES_FILE . '.index';
        if (!file_exists($indexFile)) return;
        // Запускаем defrag каждые ~50 операций записи (примерно по размеру)
        $info = file_analize($indexFile);
        if (is_array($info) && ($info['line_count'] ?? 0) > 0) {
            $pages = self::getAllPages();
            $ratio = ($info['line_count'] + 1) / max(1, count($pages));
            if ($ratio > 1.5) {
                self::defrag();
            }
        }
    }

    // ───────────────── USERS ─────────────────

    /** Найти пользователя по имени */
    public static function getUser(string $username): ?array
    {
        if (!file_exists(self::USERS_FILE)) return null;
        $line = file_search_line(self::USERS_FILE, $username . ' ', 0, 1);
        if ($line === false || $line === null) return null;
        $parts = explode(' ', trim($line), 2);
        if (count($parts) < 2) return null;
        return ['username' => $parts[0], 'password_hash' => $parts[1]];
    }

    /** Создать пользователя */
    public static function createUser(string $username, string $password): bool
    {
        $hash = password_hash($password, PASSWORD_BCRYPT);
        $line = $username . ' ' . $hash;
        return file_insert_line(self::USERS_FILE, $line, 0, USERS_ALIGN) >= 0;
    }

    /** Сменить пароль */
    public static function changePassword(string $username, string $newPassword): bool
    {
        if (!file_exists(self::USERS_FILE)) return false;
        $hash    = password_hash($newPassword, PASSWORD_BCRYPT);
        $newLine = $username . ' ' . $hash;
        return file_replace_line(self::USERS_FILE, $username . ' ', $newLine) >= 0;
    }

    /** Статистика */
    public static function stats(): array
    {
        $pages     = self::getAllPages();
        $published = array_filter($pages, fn($p) => ($p['published'] ?? 0) == 1);
        $info      = file_exists(self::PAGES_FILE) ? file_analize(self::PAGES_FILE) : [];

        return [
            'total_pages'     => count($pages),
            'published_pages' => count($published),
            'draft_pages'     => count($pages) - count($published),
            'data_file_size'  => $info['file_size'] ?? 0,
        ];
    }
}
