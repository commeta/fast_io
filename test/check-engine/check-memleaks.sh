#!/bin/bash
# =============================================================================
# * Fast_IO Extension for PHP 8
# * https://github.com/commeta/fast_io
# *
# * Copyright 2026 commeta <dcs-spb@ya.ru>
# *
# * check-memleaks.sh — Удобная обёртка для проверки fast_io на утечки памяти
# *
# * Использование:
# *   ./check-memleaks.sh              # 40 000 итераций (рекомендуется)
# *   ./check-memleaks.sh 80000        # больше нагрузки
# *   ./check-memleaks.sh 20000        # быстрее
# *
# * This program is free software; you can redistribute it and/or modify
# * it under the terms of the GNU General Public License as published by
# * the Free Software Foundation; either version 2 of the License, or
# * (at your option) any later version.
# =============================================================================



set -euo pipefail

ITERATIONS=${1:-40000}
TEST_SCRIPT="check-memleaks.php"
LOGFILE="check-memleaks.log"

echo "╔══════════════════════════════════════════════════════════════════════╗"
echo "║               fast_io VALGRIND MEMORY LEAK WRAPPER                   ║"
echo "╚══════════════════════════════════════════════════════════════════════╝"
echo "Итераций          : $ITERATIONS"
echo "Тестовый скрипт   : $TEST_SCRIPT"
echo "Лог-файл          : $LOGFILE"
echo

# ─── Автоматический поиск suppression-файла PHP ─────────────────────────────
SUPP_FILE=""

for path in \
    /usr/lib/php/valgrind-php.supp \
    /usr/lib/php/*/valgrind-php.supp \
    /usr/share/php/valgrind-php.supp \
    /usr/local/lib/php/valgrind-php.supp \
    /usr/local/share/php/valgrind-php.supp; do

    if [[ -f "$path" ]]; then
        SUPP_FILE="$path"
        break
    fi
done

if [[ -n "$SUPP_FILE" ]]; then
    echo "✓ Suppression file найден: $SUPP_FILE"
    SUPP_OPTION="--suppressions=$SUPP_FILE"
else
    echo "⚠  Suppression file valgrind-php.supp не найден — запускаем без него (это нормально)"
    SUPP_OPTION=""
fi

echo

# ─── Запуск Valgrind с оптимальными параметрами ─────────────────────────────
echo "Запускаем Valgrind (это может занять 30–90 секунд)..."

valgrind \
    --leak-check=full \
    --show-leak-kinds=all \
    --track-origins=yes \
    --num-callers=50 \
    --trace-children=no \
    --child-silent-after-fork=yes \
    --log-file="$LOGFILE" \
    $SUPP_OPTION \
    php "$TEST_SCRIPT" "$ITERATIONS"

echo
echo "══════════════════════════════════════════════════════════════════════════"
echo "✓ Valgrind завершён!"
echo "   Лог сохранён в: $LOGFILE"
echo
echo "Полезные команды для анализа:"
echo "   less $LOGFILE"
echo "   grep -E 'definitely lost|indirectly lost|possibly lost' $LOGFILE"
echo "   grep -A 30 'LEAK SUMMARY' $LOGFILE"
echo

# Быстрый вывод итоговой сводки
if [[ -f "$LOGFILE" ]]; then
    echo "Краткая сводка утечек:"
    grep -E "definitely lost|indirectly lost|possibly lost|still reachable" "$LOGFILE" | tail -10 || true
fi

echo
echo "Если в отчёте нет строк 'definitely lost' или 'indirectly lost' — утечек в fast_io нет."
