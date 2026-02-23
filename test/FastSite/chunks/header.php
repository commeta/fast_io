<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title><?= h(tv('pagetitle', SITE_NAME)) ?></title>
<meta name="description" content="<?= h(tv('description')) ?>">
<meta name="keywords"    content="<?= h(tv('keywords')) ?>">
<?php if (tv('og_image')): ?>
<meta property="og:image" content="<?= h(tv('og_image')) ?>">
<?php endif; ?>
<style>
*{box-sizing:border-box;margin:0;padding:0}
body{font:16px/1.6 system-ui,sans-serif;color:#333;background:#fff}
.container{max-width:1100px;margin:0 auto;padding:0 1.5rem}
.container--flex{display:flex;gap:2rem}
.site-header{background:#1e2533;color:#fff;padding:1rem 0}
.site-header .logo{color:#fff;text-decoration:none;font-size:1.3rem;font-weight:700}
nav a{color:#a3b3c8;text-decoration:none;margin-left:1.2rem;font-size:.95rem}
nav a:hover,nav a.active{color:#fff}
.site-main{padding:2.5rem 0}
.content{flex:1;min-width:0}
.sidebar{width:260px;flex-shrink:0}
footer{background:#f4f5f7;border-top:1px solid #e2e8f0;padding:1.5rem 0;text-align:center;color:#666;font-size:.9rem}
h1{font-size:2rem;margin-bottom:1rem}h2{font-size:1.4rem;margin-bottom:.8rem}
p{margin-bottom:1rem}.btn{display:inline-block;padding:.5rem 1.2rem;border-radius:5px;text-decoration:none;cursor:pointer;border:none}
.btn-primary{background:#4f6ef7;color:#fff}.btn-primary:hover{background:#3a57e8}
</style>
