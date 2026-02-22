#!/bin/bash
# =============================================================================
# Fast_IO Extension for PHP 8
# https://github.com/commeta/fast_io
#
# Copyright 2026 commeta <dcs-spb@ya.ru>
#
# check-memleaks.sh v2.0 — Умная обёртка для проверки fast_io на утечки памяти
#
# Использование:
#   ./check-memleaks.sh                  # Valgrind, 4000 итераций
#   ./check-memleaks.sh 40000            # Valgrind, 40000 итераций
#   ./check-memleaks.sh 10000 --native   # Только PHP (без Valgrind)
#   ./check-memleaks.sh 4000 --keep-logs # Сохранить все промежуточные файлы
#   ./check-memleaks.sh --help           # Показать справку
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
# =============================================================================

set -euo pipefail

# --- Цвета ---
if [[ -t 1 ]]; then
    RED='\033[0;31m'; GREEN='\033[0;32m'; YELLOW='\033[1;33m'
    CYAN='\033[0;36m'; BOLD='\033[1m'; DIM='\033[2m'; RESET='\033[0m'
else
    RED=''; GREEN=''; YELLOW=''; CYAN=''; BOLD=''; DIM=''; RESET=''
fi

# --- Функции ---

# Извлечь суммарное число байт для метрики из лога Valgrind
extract_bytes() {
    local pattern="$1"
    grep -E "$pattern" "$LOGFILE" 2>/dev/null \
        | grep -oP '[0-9,]+(?= bytes)' \
        | tr -d ',' \
        | paste -sd+ \
        | bc 2>/dev/null \
        || echo "0"
}

# Вердикт утечки
verdict_leak() {
    if [[ "${1:-0}" -eq 0 ]]; then printf "${GREEN}OK${RESET}"
    else printf "${RED}УТЕЧКА!${RESET}"; fi
}

# Вердикт предупреждения
verdict_warn() {
    if [[ "${1:-0}" -eq 0 ]]; then printf "${GREEN}OK${RESET}"
    else printf "${YELLOW}ПОДОЗРЕНИЕ${RESET}"; fi
}

# --- Аргументы ---
ITERATIONS=4000
NATIVE_MODE=false
KEEP_LOGS=false

for arg in "$@"; do
    case "$arg" in
        --help|-h)
            cat <<'EOF'
Использование: ./check-memleaks.sh [ITERATIONS] [OPTIONS]

  ITERATIONS        Количество итераций (по умолчанию: 4000)
  --native          Запустить только PHP без Valgrind (быстро, ~5 сек)
  --keep-logs       Сохранить все промежуточные DB-файлы и proc-снимки
  --no-color        Отключить цветной вывод (для CI/логов)
  --help            Показать эту справку

Интерпретация результатов Valgrind:
  definitely lost  = УТЕЧКА (bad!) — память выделена и недостижима
  indirectly lost  = УТЕЧКА (bad!) — указатели потеряны из-за другой утечки
  possibly lost    = ПОДОЗРЕНИЕ — Valgrind не уверен
  still reachable  = НЕ УТЕЧКА — PHP освободит при завершении (НОРМАЛЬНО!)

  RSS-рост под Valgrind (150-250 MB) — это оверхед самого Valgrind
  (shadow memory + dlopen instrumentation). Это НЕ утечка fast_io!
EOF
            exit 0
            ;;
        --native)    NATIVE_MODE=true ;;
        --keep-logs) KEEP_LOGS=true ;;
        --no-color)  RED=''; GREEN=''; YELLOW=''; CYAN=''; BOLD=''; DIM=''; RESET='' ;;
        [0-9]*)      ITERATIONS="$arg" ;;
        *)           echo -e "${YELLOW}Неизвестный аргумент: $arg${RESET}" ;;
    esac
done

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
TEST_SCRIPT="$SCRIPT_DIR/check-memleaks.php"
LOGFILE="$SCRIPT_DIR/check-memleaks-valgrind.log"
KEEP_FLAG=""
[[ "$KEEP_LOGS" == "true" ]] && KEEP_FLAG="keep-logs=1"

# --- Заголовок ---
echo -e "${BOLD}╔══════════════════════════════════════════════════════════════════════╗${RESET}"
echo -e "${BOLD}║          fast_io MEMORY LEAK WRAPPER v2.0                           ║${RESET}"
echo -e "${BOLD}╚══════════════════════════════════════════════════════════════════════╝${RESET}"
echo -e "${CYAN}Режим     :${RESET} $([[ "$NATIVE_MODE" == "true" ]] && echo 'Native PHP (без Valgrind)' || echo 'Valgrind')"
echo -e "${CYAN}Итераций  :${RESET} $ITERATIONS"
echo -e "${CYAN}Скрипт    :${RESET} $TEST_SCRIPT"
[[ "$NATIVE_MODE" != "true" ]] && echo -e "${CYAN}Лог       :${RESET} $LOGFILE"
echo -e "${CYAN}Keep logs :${RESET} $([[ "$KEEP_LOGS" == "true" ]] && echo 'YES' || echo 'NO')"
echo

# --- Проверка скрипта ---
if [[ ! -f "$TEST_SCRIPT" ]]; then
    echo -e "${RED}❌ Скрипт не найден: $TEST_SCRIPT${RESET}"; exit 1
fi

# --- Проверка fast_io ---
echo -n "Проверка fast_io... "
if php -r "exit(extension_loaded('fast_io')?0:1);" 2>/dev/null; then
    echo -e "${GREEN}✓ загружено${RESET}"
else
    echo -e "${YELLOW}⚠ не загружено — fast_io-функции будут пропущены${RESET}"
fi
echo

# ======= НАТИВНЫЙ РЕЖИМ =======
if [[ "$NATIVE_MODE" == "true" ]]; then
    echo -e "${BOLD}Запускаем PHP нативно...${RESET}"; echo
    php "$TEST_SCRIPT" "$ITERATIONS" $KEEP_FLAG
    EXIT_CODE=$?
    echo
    if [[ $EXIT_CODE -eq 0 ]]; then
        echo -e "${GREEN}${BOLD}✅ Нативный тест завершён успешно${RESET}"
    else
        echo -e "${RED}${BOLD}❌ Нативный тест завершён с ошибкой (код: $EXIT_CODE)${RESET}"
    fi
    exit $EXIT_CODE
fi

# ======= РЕЖИМ VALGRIND =======
if ! command -v valgrind &>/dev/null; then
    echo -e "${RED}❌ Valgrind не найден. Установите: sudo apt install valgrind${RESET}"
    echo "Для запуска без Valgrind: ./check-memleaks.sh $ITERATIONS --native"
    exit 1
fi

echo -e "${CYAN}Valgrind  :${RESET} $(valgrind --version 2>&1 | head -1)"; echo

# Поиск suppression-файла
SUPP_OPTION=""
for path in \
    /usr/lib/php/valgrind-php.supp \
    /usr/lib/php/*/valgrind-php.supp \
    /usr/share/php/valgrind-php.supp \
    /usr/local/lib/php/valgrind-php.supp; do
    if [[ -f "$path" ]]; then
        echo -e "${GREEN}✓ Suppression file: $path${RESET}"
        SUPP_OPTION="--suppressions=$path"
        break
    fi
done
if [[ -z "$SUPP_OPTION" ]]; then
    echo -e "${DIM}ℹ  valgrind-php.supp не найден — PHP internals могут отображаться как 'still reachable'${RESET}"
fi
echo

echo -e "${DIM}Что ожидать под Valgrind:${RESET}"
echo -e "${DIM}  • RSS вырастет на 150-250 MB (Valgrind shadow memory) — это НЕ утечка${RESET}"
echo -e "${DIM}  • Скорость в 20-50x медленнее нативного PHP${RESET}"
echo -e "${DIM}  • 'still reachable' от zend/dlopen — PHP internals, норма${RESET}"
echo -e "${DIM}  • Реальные утечки: только 'definitely lost' и 'indirectly lost'${RESET}"
echo

echo -e "${BOLD}Запускаем Valgrind ${YELLOW}(30-120 секунд)${RESET}${BOLD}...${RESET}"; echo
START_TIME=$(date +%s)

valgrind \
    --leak-check=full \
    --show-leak-kinds=all \
    --track-origins=yes \
    --num-callers=50 \
    --trace-children=no \
    --child-silent-after-fork=yes \
    --log-file="$LOGFILE" \
    $SUPP_OPTION \
    php "$TEST_SCRIPT" "$ITERATIONS" "valgrind=1" "--vg-log=$LOGFILE" $KEEP_FLAG

VALGRIND_EXIT=$?
ELAPSED=$(( $(date +%s) - START_TIME ))

echo
echo -e "${BOLD}══════════════════════════════════════════════════════════════════════════${RESET}"
echo -e "${BOLD}  Анализ лога Valgrind${RESET}"
echo -e "${BOLD}══════════════════════════════════════════════════════════════════════════${RESET}"
echo -e "${CYAN}Время выполнения :${RESET} ${ELAPSED}s"
echo -e "${CYAN}Лог              :${RESET} $LOGFILE"
echo

if [[ ! -f "$LOGFILE" ]]; then
    echo -e "${RED}❌ Лог-файл не создан: $LOGFILE${RESET}"; exit 1
fi

# Разбор лога
DEFINITELY=$(extract_bytes "definitely lost:")
INDIRECTLY=$(extract_bytes "indirectly lost:")
POSSIBLY=$(  extract_bytes "possibly lost:")
STILL_REACH=$(extract_bytes "still reachable:")

ERROR_LINE=$(grep "ERROR SUMMARY:" "$LOGFILE" 2>/dev/null | tail -1 || echo "")
ERROR_COUNT=0
if [[ -n "$ERROR_LINE" ]]; then
    ERROR_COUNT=$(echo "$ERROR_LINE" | grep -oP '\d+(?= errors)' 2>/dev/null || echo "0")
fi

# Вывод LEAK SUMMARY
echo -e "${BOLD}LEAK SUMMARY (Valgrind):${RESET}"
printf "   %-20s %14s   [%s]\n" "definitely lost:" "$DEFINITELY bytes" "$(verdict_leak "$DEFINITELY")"
printf "   %-20s %14s   [%s]\n" "indirectly lost:" "$INDIRECTLY bytes" "$(verdict_leak "$INDIRECTLY")"
printf "   %-20s %14s   [%s]\n" "possibly lost:"   "$POSSIBLY bytes"   "$(verdict_warn "$POSSIBLY")"
printf "   %-20s %14s   ${DIM}[ℹ PHP internals — норма]${RESET}\n" "still reachable:" "$STILL_REACH bytes"
echo

# Ошибки доступа к памяти
echo -e "${BOLD}Ошибки памяти:${RESET}"
if [[ "$ERROR_COUNT" -eq 0 ]]; then
    echo -e "   ${GREEN}✅ 0 ошибок (buffer overflows, use-after-free, uninitialized reads)${RESET}"
else
    echo -e "   ${RED}❌ $ERROR_COUNT ошибок — подробности: less $LOGFILE${RESET}"
fi
echo

# Анализ still reachable
if [[ "$STILL_REACH" -gt 0 ]]; then
    echo -e "${BOLD}Анализ 'still reachable':${RESET}"
    FAST_IO_LINES=$(grep -c "fast_io" "$LOGFILE" 2>/dev/null || echo "0")
    ZEND_LINES=$(grep -cE "zend_register|dlopen|dlclose|php_" "$LOGFILE" 2>/dev/null || echo "0")

    if [[ "$FAST_IO_LINES" -eq 0 ]]; then
        echo -e "   ${GREEN}✅ fast_io не упоминается в стеках утечек${RESET}"
    else
        echo -e "   ${YELLOW}⚠  fast_io упоминается в $FAST_IO_LINES строках — проверьте вручную:${RESET}"
        echo -e "      grep -B2 -A20 'fast_io' $LOGFILE | head -80"
    fi
    [[ "$ZEND_LINES" -gt 0 ]] && \
        echo -e "   ${DIM}ℹ  PHP/Zend internals в $ZEND_LINES строках — это PHP lifecycle, не fast_io${RESET}"
    echo
fi

# Объяснение RSS-роста
echo -e "${DIM}Почему RSS растёт под Valgrind:${RESET}"
echo -e "${DIM}  Valgrind выделяет ~8 байт shadow memory на каждый байт программы.${RESET}"
echo -e "${DIM}  Загрузка fast_io.so + shared libs добавляет 100-250 MB RSS.${RESET}"
echo -e "${DIM}  Это оверхед Valgrind, а не утечка fast_io.${RESET}"
echo

# --- ФИНАЛЬНЫЙ ВЕРДИКТ ---
echo -e "${BOLD}══════════════════════════════════════════════════════════════════════════${RESET}"
echo -e "${BOLD}  ФИНАЛЬНЫЙ ВЕРДИКТ${RESET}"
echo -e "${BOLD}══════════════════════════════════════════════════════════════════════════${RESET}"

HAS_REAL_LEAKS=false
[[ "$DEFINITELY" -gt 0 ]] && HAS_REAL_LEAKS=true
[[ "$INDIRECTLY" -gt 0 ]] && HAS_REAL_LEAKS=true

if [[ "$HAS_REAL_LEAKS" == "false" ]] && [[ "$ERROR_COUNT" -eq 0 ]]; then
    echo -e "${GREEN}${BOLD}"
    echo    "  ✅  УТЕЧЕК ПАМЯТИ В fast_io НЕ ОБНАРУЖЕНО"
    echo -e "${RESET}${GREEN}"
    echo    "  definitely lost = 0, indirectly lost = 0, errors = 0"
    echo    "  Библиотека fast_io корректно освобождает всю выделенную память."
    echo -e "${RESET}"
    [[ "$POSSIBLY"    -gt 0 ]] && echo -e "${YELLOW}  ℹ  possibly lost = $POSSIBLY bytes — обычно ложное срабатывание${RESET}" && echo
    [[ "$STILL_REACH" -gt 0 ]] && echo -e "${DIM}  ℹ  still reachable = $STILL_REACH bytes — PHP runtime, освободится при выходе${RESET}" && echo
elif [[ "$HAS_REAL_LEAKS" == "true" ]]; then
    echo -e "${RED}${BOLD}"
    echo    "  ❌  ОБНАРУЖЕНЫ РЕАЛЬНЫЕ УТЕЧКИ ПАМЯТИ"
    echo -e "${RESET}${RED}"
    [[ "$DEFINITELY" -gt 0 ]] && echo "  definitely lost = $DEFINITELY bytes"
    [[ "$INDIRECTLY" -gt 0 ]] && echo "  indirectly lost = $INDIRECTLY bytes"
    echo -e "${RESET}"
    echo    "  Для анализа:"
    echo    "    grep -A 30 'definitely lost' $LOGFILE"
    echo    "    grep -B2 -A20 'fast_io' $LOGFILE"
    echo
else
    echo -e "${YELLOW}${BOLD}"
    echo    "  ⚠  УТЕЧЕК НЕТ, НО ЕСТЬ ОШИБКИ ДОСТУПА К ПАМЯТИ ($ERROR_COUNT)"
    echo -e "${RESET}"
    echo    "  Подробности: less $LOGFILE"
    echo
fi

# --- Полезные команды ---
echo -e "${BOLD}Команды для анализа:${RESET}"
echo -e "  ${CYAN}Полный лог:${RESET}           less $LOGFILE"
echo -e "  ${CYAN}Реальные утечки:${RESET}      grep -E 'definitely|indirectly' $LOGFILE"
echo -e "  ${CYAN}LEAK SUMMARY:${RESET}         grep -A10 'LEAK SUMMARY' $LOGFILE"
echo -e "  ${CYAN}Стеки fast_io:${RESET}        grep -B2 -A20 'fast_io' $LOGFILE | head -100"
echo -e "  ${CYAN}Ошибки памяти:${RESET}        grep 'ERROR SUMMARY' $LOGFILE"
echo

if [[ "$HAS_REAL_LEAKS" == "true" ]] || [[ "$ERROR_COUNT" -gt 0 ]]; then
    exit 1
else
    exit 0
fi
