<?php
Auth::logout();
flash('success', 'Вы вышли из панели управления.');
redirect(ADMIN_PREFIX . '/login');
