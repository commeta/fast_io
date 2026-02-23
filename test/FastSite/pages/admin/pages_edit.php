<?php
$alias    = trim($_GET['alias'] ?? '');
$isEdit   = $alias !== '';
$page     = $isEdit ? DB::getPage($alias) : null;
$errors   = [];

// Текущий шаблон (для TV-полей)
$tplKey   = $_POST['template'] ?? $page['template'] ?? 'main';
$tvDefs   = TV_FIELDS[$tplKey] ?? TV_FIELDS['main'];

if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    if (!csrf_verify()) { $errors[] = 'CSRF ошибка'; }

    $newAlias = trim($_POST['alias'] ?? '');
    $template = trim($_POST['template'] ?? 'main');
    $published = (int)($_POST['published'] ?? 0);
    $tplKey   = $template;
    $tvDefs   = TV_FIELDS[$tplKey] ?? TV_FIELDS['main'];

    // Валидация
    if (!preg_match('/^[a-z0-9_\-]+$/i', $newAlias)) {
        $errors[] = 'Alias: только латиница, цифры, _ и -';
    }
    if (!isset(TEMPLATES[$template])) {
        $errors[] = 'Неверный шаблон.';
    }

    // Если создаём новую страницу и alias занят
    if (!$isEdit && $newAlias && DB::getPage($newAlias)) {
        $errors[] = 'Страница с alias «' . $newAlias . '» уже существует.';
    }

    if (!$errors) {
        // Собираем TV-поля из POST
        $tvData = [];
        foreach ($tvDefs as $field => $def) {
            if ($def['type'] === 'checkbox') {
                $tvData[$field] = isset($_POST['tv'][$field]) ? '1' : '0';
            } else {
                $tvData[$field] = $_POST['tv'][$field] ?? $def['default'];
            }
        }

        // Если меняем alias при редактировании — удаляем старый
        if ($isEdit && $newAlias !== $alias) {
            DB::deletePage($alias);
        }

        $saveData = [
            'alias'     => $newAlias,
            'template'  => $template,
            'published' => $published,
            'tv'        => $tvData,
        ];
        if ($isEdit && $page) {
            $saveData['created_at'] = $page['created_at'] ?? time();
        }

        if (DB::savePage($saveData)) {
            flash('success', ($isEdit ? 'Страница обновлена.' : 'Страница создана.'));
            redirect(ADMIN_PREFIX . '/pages/edit?alias=' . urlencode($newAlias));
        } else {
            $errors[] = 'Ошибка при сохранении. Проверьте права на data/';
        }
    }

    // После ошибки — заполняем $alias из POST
    if (!$isEdit) $alias = $_POST['alias'] ?? '';
}

// Текущие значения (из $page или из POST при ошибке)
$cur = fn(string $field) =>
    $_POST['tv'][$field]
    ?? $page['tv'][$field]
    ?? (TV_FIELDS[$tplKey][$field]['default'] ?? '');
?>

<div style="display:flex;justify-content:space-between;align-items:center;margin-bottom:1rem">
    <h1 style="margin:0"><?= $isEdit ? 'Редактировать: ' . h($alias) : 'Новая страница' ?></h1>
    <div>
        <?php if ($isEdit): ?>
        <a href="<?= url($alias) ?>" target="_blank" class="btn btn-secondary">↗ Просмотр</a>
        <?php endif; ?>
        <a href="<?= url(ADMIN_PREFIX . '/pages') ?>" class="btn btn-secondary">← Назад</a>
    </div>
</div>

<?php foreach ($errors as $e): ?>
    <div class="alert alert-error"><?= h($e) ?></div>
<?php endforeach; ?>

<form method="post">
    <?= csrf_field() ?>

    <div style="display:grid;grid-template-columns:2fr 1fr;gap:1rem">
        <!-- LEFT: основные поля и TV -->
        <div>
            <!-- Системные поля -->
            <div class="card">
                <h2>Основные параметры</h2>
                <div class="form-group">
                    <label>Alias (URL-путь) *</label>
                    <input type="text" name="alias" value="<?= h($_POST['alias'] ?? $alias) ?>"
                           placeholder="my-page" pattern="[a-zA-Z0-9_\-]+" required
                           <?= $isEdit ? '' : '' ?>>
                    <small style="color:#888">Только латиница, цифры, _ и -. Пример: about-us</small>
                </div>
                <div class="form-group">
                    <label>Шаблон</label>
                    <select name="template" onchange="this.form.submit()">
                        <?php foreach (TEMPLATES as $tKey => $tLabel): ?>
                        <option value="<?= h($tKey) ?>" <?= ($tplKey === $tKey) ? 'selected' : '' ?>><?= h($tLabel) ?></option>
                        <?php endforeach; ?>
                    </select>
                </div>
            </div>

            <!-- TV-поля шаблона -->
            <div class="card">
                <h2>Поля шаблона «<?= h(TEMPLATES[$tplKey] ?? $tplKey) ?>» (TV)</h2>
                <?php foreach ($tvDefs as $field => $def):
                    if ($def['type'] === 'richtext' || $def['type'] === 'textarea') continue;
                ?>
                <div class="form-group">
                    <label><?= h($def['label']) ?> <small style="color:#888;font-weight:400">[<?= h($field) ?>]</small></label>
                    <?php if ($def['type'] === 'checkbox'): ?>
                        <label style="font-weight:normal;display:flex;align-items:center;gap:.4rem">
                            <input type="checkbox" name="tv[<?= h($field) ?>]" value="1"
                                <?= ($cur($field) === '1') ? 'checked' : '' ?>>
                            Включено
                        </label>
                    <?php elseif ($def['type'] === 'number'): ?>
                        <input type="number" name="tv[<?= h($field) ?>]" value="<?= h($cur($field)) ?>">
                    <?php else: ?>
                        <input type="text"   name="tv[<?= h($field) ?>]" value="<?= h($cur($field)) ?>">
                    <?php endif; ?>
                </div>
                <?php endforeach; ?>

                <!-- Textarea TV-поля -->
                <?php foreach ($tvDefs as $field => $def):
                    if (!in_array($def['type'], ['richtext', 'textarea'])) continue;
                ?>
                <div class="form-group">
                    <label><?= h($def['label']) ?> <small style="color:#888;font-weight:400">[<?= h($field) ?>]</small></label>
                    <textarea name="tv[<?= h($field) ?>]"
                              rows="<?= $def['type'] === 'richtext' ? 12 : 4 ?>"><?= h($cur($field)) ?></textarea>
                    <?php if ($def['type'] === 'richtext'): ?>
                    <small style="color:#888">HTML разрешён. Подключите TinyMCE/Quill самостоятельно.</small>
                    <?php endif; ?>
                </div>
                <?php endforeach; ?>
            </div>
        </div>

        <!-- RIGHT: публикация -->
        <div>
            <div class="card">
                <h2>Публикация</h2>
                <div class="form-group">
                    <label>Статус</label>
                    <select name="published">
                        <option value="0" <?= (($_POST['published'] ?? $page['published'] ?? 0) == 0) ? 'selected' : '' ?>>Черновик</option>
                        <option value="1" <?= (($_POST['published'] ?? $page['published'] ?? 0) == 1) ? 'selected' : '' ?>>Опубликовано</option>
                    </select>
                </div>
                <?php if ($isEdit && $page): ?>
                <p style="font-size:.8rem;color:#888;margin-top:1rem">
                    Создано: <?= date('d.m.Y H:i', $page['created_at'] ?? 0) ?><br>
                    Изменено: <?= date('d.m.Y H:i', $page['updated_at'] ?? 0) ?>
                </p>
                <?php endif; ?>
                <button type="submit" name="_save" class="btn btn-primary" style="width:100%;margin-top:1rem">
                    💾 Сохранить
                </button>
            </div>
        </div>
    </div>
</form>
