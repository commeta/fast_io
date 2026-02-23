<?php http_response_code(404); ?>
<div style="text-align:center;padding:4rem 0">
    <h1 style="font-size:5rem;color:#e2e8f0">404</h1>
    <h2>Страница не найдена</h2>
    <p style="color:#666;margin:1rem 0">Запрошенный URL не существует.</p>
    <a href="<?= url('/') ?>" class="btn btn-primary">На главную</a>
</div>
