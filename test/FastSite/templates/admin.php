<?php declare(strict_types=1); ?>
<!DOCTYPE html>
<html lang="ru">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Admin — <?= h(SITE_NAME) ?></title>
<style>
*{box-sizing:border-box;margin:0;padding:0}
body{font:14px/1.5 system-ui,sans-serif;background:#f4f5f7;color:#333}
.adm-layout{display:flex;min-height:100vh}
.adm-sidebar{width:220px;background:#1e2533;color:#cdd5e0;padding:1rem;flex-shrink:0}
.adm-sidebar a{color:#a3b3c8;text-decoration:none;display:block;padding:.4rem .6rem;border-radius:4px}
.adm-sidebar a:hover,.adm-sidebar a.active{background:#2d3748;color:#fff}
.adm-sidebar .logo{font-size:1.1rem;font-weight:700;color:#fff;padding:.5rem 0 1rem}
.adm-content{flex:1;padding:1.5rem;overflow-x:auto}
.adm-topbar{display:flex;justify-content:space-between;align-items:center;margin-bottom:1.5rem}
.card{background:#fff;border-radius:8px;padding:1.5rem;margin-bottom:1rem;box-shadow:0 1px 3px rgba(0,0,0,.08)}
h1,h2{font-size:1.3rem;font-weight:600;margin-bottom:1rem}
.btn{display:inline-block;padding:.45rem .9rem;border-radius:5px;font-size:.9rem;cursor:pointer;border:none;text-decoration:none}
.btn-primary{background:#4f6ef7;color:#fff}.btn-primary:hover{background:#3a57e8}
.btn-danger{background:#e53e3e;color:#fff}.btn-danger:hover{background:#c53030}
.btn-secondary{background:#e2e8f0;color:#333}.btn-secondary:hover{background:#cbd5e0}
table{width:100%;border-collapse:collapse}th,td{text-align:left;padding:.6rem .8rem;border-bottom:1px solid #e2e8f0}
th{background:#f7f9fc;font-weight:600}tr:hover td{background:#f7f9fc}
.form-group{margin-bottom:1rem}label{display:block;font-weight:600;margin-bottom:.3rem;font-size:.85rem}
input[type=text],input[type=password],input[type=number],input[type=email],select,textarea
{width:100%;padding:.5rem .7rem;border:1px solid #cbd5e0;border-radius:5px;font-size:.9rem}
textarea{min-height:200px;resize:vertical}input:focus,textarea:focus,select:focus{outline:none;border-color:#4f6ef7}
.alert{padding:.7rem 1rem;border-radius:5px;margin-bottom:1rem}
.alert-success{background:#c6f6d5;color:#276749}.alert-error{background:#fed7d7;color:#9b2c2c}
.badge{display:inline-block;padding:.15rem .5rem;border-radius:10px;font-size:.75rem;font-weight:600}
.badge-green{background:#c6f6d5;color:#276749}.badge-gray{background:#e2e8f0;color:#666}
</style>
</head>
<body>
<div class="adm-layout">
    <?php echo chunk('admin_nav'); ?>
    <div class="adm-content">
        <div class="adm-topbar">
            <h1><?= h(SITE_NAME) ?> — Admin</h1>
            <?php if (Auth::check()): ?>
                <span>👤 <?= h(Auth::user() ?? '') ?>
                &nbsp;<a href="<?= url(ADMIN_PREFIX . '/logout') ?>" class="btn btn-secondary">Выйти</a></span>
            <?php endif; ?>
        </div>
        <?php
        if ($msg = flash('success')) echo '<div class="alert alert-success">' . h($msg) . '</div>';
        if ($msg = flash('error'))   echo '<div class="alert alert-error">'   . h($msg) . '</div>';
        ?>
        <?php include $page_content_file; ?>
    </div>
</div>
</body>
</html>
