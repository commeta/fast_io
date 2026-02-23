<?php
// Уже залогинен — в дашборд
if (Auth::check()) redirect(ADMIN_PREFIX);

$error = '';

if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    if (!csrf_verify()) {
        $error = 'Неверный CSRF токен.';
    } else {
        $username = trim($_POST['username'] ?? '');
        $password = $_POST['password'] ?? '';

        if (Auth::attempt($username, $password)) {
            flash('success', 'Добро пожаловать, ' . $username . '!');
            redirect(ADMIN_PREFIX);
        } else {
            $error = 'Неверный логин или пароль.';
        }
    }
}
?>
<div class="card" style="max-width:360px;margin:3rem auto">
    <h1>Вход в панель</h1>
    <?php if ($error): ?>
        <div class="alert alert-error"><?= h($error) ?></div>
    <?php endif; ?>
    <form method="post">
        <?= csrf_field() ?>
        <div class="form-group">
            <label>Логин</label>
            <input type="text" name="username" value="<?= h($_POST['username'] ?? '') ?>" autofocus autocomplete="username">
        </div>
        <div class="form-group">
            <label>Пароль</label>
            <input type="password" name="password" autocomplete="current-password">
        </div>
        <button type="submit" class="btn btn-primary" style="width:100%">Войти</button>
    </form>
    <p style="margin-top:1rem;font-size:.85rem;color:#888">
        Нет аккаунта? Запустите <code>data/install.php</code>.
    </p>
</div>
