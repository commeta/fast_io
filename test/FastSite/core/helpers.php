<?php
declare(strict_types=1);

/** HTML-escape */
function h(mixed $v): string {
    return htmlspecialchars((string)$v, ENT_QUOTES | ENT_SUBSTITUTE, 'UTF-8');
}

/** Absolute URL */
function url(string $path = ''): string {
    return SITE_URL . '/' . ltrim($path, '/');
}

/** Render a chunk file, returns output string */
function chunk(string $name, array $vars = []): string {
    $file = CHUNKS_DIR . '/' . $name . '.php';
    if (!file_exists($file)) return "<!-- chunk '{$name}' not found -->";
    ob_start();
    extract($vars, EXTR_SKIP);
    include $file;
    return ob_get_clean();
}

/** CSRF token (generate once per session) */
function csrf_token(): string {
    if (empty($_SESSION['csrf_token'])) {
        $_SESSION['csrf_token'] = bin2hex(random_bytes(32));
    }
    return $_SESSION['csrf_token'];
}

/** CSRF hidden input */
function csrf_field(): string {
    return '<input type="hidden" name="_csrf" value="' . h(csrf_token()) . '">';
}

/** Verify CSRF */
function csrf_verify(): bool {
    $token = $_POST['_csrf'] ?? '';
    return !empty($token) && hash_equals(csrf_token(), $token);
}

/** Flash message: set or get+clear */
function flash(string $key, string $message = ''): string {
    if ($message !== '') {
        $_SESSION['flash'][$key] = $message;
        return '';
    }
    $msg = $_SESSION['flash'][$key] ?? '';
    unset($_SESSION['flash'][$key]);
    return $msg;
}

/** Redirect helper */
function redirect(string $path, int $code = 302): never {
    header('Location: ' . SITE_URL . '/' . ltrim($path, '/'), true, $code);
    exit;
}

/** Get TV value with fallback */
function tv(string $field, string $default = ''): string {
    global $tv;
    return $tv[$field] ?? $default;
}
