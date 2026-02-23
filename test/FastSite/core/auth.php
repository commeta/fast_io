<?php
declare(strict_types=1);

class Auth
{
    /** Проверить, залогинен ли пользователь */
    public static function check(): bool
    {
        return !empty($_SESSION['admin_user']);
    }

    /** Текущий пользователь */
    public static function user(): ?string
    {
        return $_SESSION['admin_user'] ?? null;
    }

    /** Попытка входа */
    public static function attempt(string $username, string $password): bool
    {
        $user = DB::getUser($username);
        if (!$user) return false;
        if (!password_verify($password, $user['password_hash'])) return false;

        session_regenerate_id(true);
        $_SESSION['admin_user'] = $user['username'];
        return true;
    }

    /** Выход */
    public static function logout(): void
    {
        $_SESSION = [];
        if (ini_get('session.use_cookies')) {
            $p = session_get_cookie_params();
            setcookie(session_name(), '', time() - 42000,
                $p['path'], $p['domain'], $p['secure'], $p['httponly']);
        }
        session_destroy();
    }

    /** Требовать авторизацию или редиректить */
    public static function require(): void
    {
        if (!self::check()) {
            redirect(ADMIN_PREFIX . '/login');
        }
    }
}
