<?php
$dbPage = DB::getPage('contacts');
if ($dbPage) { global $tv; $tv = $dbPage['tv'] ?? $tv; }

$errors  = [];
$success = false;

if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    if (!csrf_verify()) {
        $errors[] = 'Неверный CSRF токен.';
    } else {
        $name    = trim($_POST['name']    ?? '');
        $email   = trim($_POST['email']   ?? '');
        $message = trim($_POST['message'] ?? '');

        if (strlen($name) < 2)             $errors[] = 'Введите имя.';
        if (!filter_var($email, FILTER_VALIDATE_EMAIL)) $errors[] = 'Неверный e-mail.';
        if (strlen($message) < 10)         $errors[] = 'Сообщение слишком короткое.';

        if (!$errors) {
            // В реальном проекте: mail() или запись в fast_io лог
            // file_insert_line(DATA_DIR.'/contacts.log', date('c').' '.$email.' '.addslashes($message));
            $success = true;
        }
    }
}
?>
<h1><?= h(tv('pagetitle', 'Контакты')) ?></h1>

<?php if ($success): ?>
    <p style="color:green;padding:1rem;background:#e6ffed;border-radius:5px">
        ✅ Сообщение отправлено. Мы свяжемся с вами в ближайшее время.
    </p>
<?php else: ?>
    <?php foreach ($errors as $e): ?>
        <p style="color:red"><?= h($e) ?></p>
    <?php endforeach; ?>
    <form method="post" style="max-width:500px">
        <?= csrf_field() ?>
        <p><label>Имя<br><input type="text" name="name" value="<?= h($_POST['name'] ?? '') ?>"></label></p>
        <p style="margin:.8rem 0"><label>E-mail<br><input type="email" name="email" value="<?= h($_POST['email'] ?? '') ?>"></label></p>
        <p style="margin:.8rem 0"><label>Сообщение<br><textarea name="message" rows="5"><?= h($_POST['message'] ?? '') ?></textarea></label></p>
        <button type="submit" class="btn btn-primary">Отправить</button>
    </form>
<?php endif; ?>
