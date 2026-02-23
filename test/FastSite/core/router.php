<?php
declare(strict_types=1);

class Router
{
    private array  $routes     = [];
    private string $currentUrl = '/';

    /** Зарегистрировать маршрут */
    public function add(string $url, array $config): void
    {
        $this->routes[$url] = $config;
    }

    /** Диспетчеризация */
    public function dispatch(): void
    {
        $this->currentUrl = $this->parsePath();

        // 1. Точное совпадение статических маршрутов
        if (isset($this->routes[$this->currentUrl])) {
            $this->render($this->routes[$this->currentUrl]);
            return;
        }

        // 2. Динамические страницы из БД (не /admin/*)
        if (!str_starts_with($this->currentUrl, ADMIN_PREFIX)) {
            $alias = ltrim($this->currentUrl, '/') ?: 'home';
            $page  = DB::getPage($alias);
            if ($page && ($page['published'] ?? 0)) {
                $this->render([
                    'template'  => $page['template'] ?? 'main',
                    'page'      => 'dynamic',
                    'page_data' => $page,
                ]);
                return;
            }
        }

        // 3. 404
        http_response_code(404);
        $this->render(['template' => 'main', 'page' => '404']);
    }

    private function parsePath(): string
    {
        $uri  = $_SERVER['REQUEST_URI'] ?? '/';
        $path = parse_url($uri, PHP_URL_PATH) ?: '/';
        return rtrim($path, '/') ?: '/';
    }

    public function getUrl(): string { return $this->currentUrl; }

    /** Рендер: собирает переменные → шаблон → страница */
    private function render(array $route): void
    {
        // Глобальные переменные контекста
        global $current_route, $page_data, $tv;

        $current_route = $route;

        // Защита admin-маршрутов
        if (!empty($route['auth'])) {
            Auth::require();
        }

        // Данные страницы и TV-поля
        $page_data = $route['page_data'] ?? [];
        $tv        = $page_data['tv'] ?? [];

        // Файл страницы
        $page_file = PAGES_DIR . '/' . ltrim($route['page'], '/') . '.php';
        if (!file_exists($page_file)) {
            $page_file = PAGES_DIR . '/404.php';
        }

        // Шаблон
        $tpl_name = $route['template'] ?? 'main';
        $tpl_file = TEMPLATES_DIR . '/' . $tpl_name . '.php';
        if (!file_exists($tpl_file)) {
            $tpl_file = TEMPLATES_DIR . '/main.php';
        }

        // Переменная, которую используют шаблоны для подключения страницы
        $page_content_file = $page_file;

        include $tpl_file; // шаблон делает include $page_content_file
    }
}
