# Проверка утечек памяти (Memory Leak Testing)

`fast_io` — это высокопроизводительное C-расширение для PHP 8, поэтому особенно важно гарантировать **нулевые утечки памяти** даже после миллионов операций с файлами.  

В репозитории поставляются два инструмента:

- **`check-memleaks.php`** — полноценный многофазный стресс-тест (ядро проверки)  
- **`check-memleaks.sh`** — удобная обёртка для запуска в один клик (native / Valgrind)

Оба инструмента входят в состав библиотеки и находятся в корне проекта.

---

### 1. `check-memleaks.php` — тест утечек памяти

**Назначение**  
Выполняет 6 фаз интенсивного тестирования **всех** функций `fast_io` и отслеживает:

- PHP `memory_get_usage()` / `memory_get_peak_usage()`
- Linux `/proc/self` метрики: **RSS**, **PSS**, **VmPeak**, `smaps_rollup`
- Тренды роста, аномалии, per-phase breakdown
- Автоматический парсинг лога Valgrind (если запущено под ним)

**Особенности**

- Автодетект режима Valgrind по `/proc/self/status` (Name=memcheck-amd64-)
- Умные пороги: под Valgrind RSS-дельта до **512 МБ** считается нормой (оверхед shadow memory)
- Прогресс-бары + детальная аналитика
- Сохранение всех proc-снимков в JSON
- Авто-разбор `definitely_lost` / `indirectly_lost` / `still reachable`
- Классификация утечек: `fast_io` vs PHP internals (zend/dlopen)

**Запуск**

```bash
# Обычный режим (быстро)
php check-memleaks.php 10000

# С сохранением логов
php check-memleaks.php 10000 keep-logs=1

# Явно Valgrind-режим
php check-memleaks.php 4000 valgrind=1

# Полный Valgrind с логом
valgrind --leak-check=full --log-file=vg.log \
    php check-memleaks.php 4000 valgrind=1 --vg-log=vg.log
```

**Что тестируется по фазам**

| Фаза | Функции | Итераций (по умолчанию) |
|------|---------|-------------------------|
| 0    | `file_insert_line` (подготовка) | 500 |
| 1    | `file_insert_line` + `file_select_line` | 10 000 |
| 2    | `file_update_line` + `file_update_array` | 2 000 |
| 3    | `file_search_*` + `file_get_keys` + `find_matches_pcre2` | 1 000 |
| 4    | `file_push_data` + `file_search_data` + `file_defrag_data` | 2 000 |
| 5    | `file_callback_line` + `file_pop_line` + `replicate_file` | 500 |

**Пример вывода (native)**

```
✅  УТЕЧЕК ПАМЯТИ НЕ ОБНАРУЖЕНО
    Рост PHP peak < 8 MB  |  RSS delta в норме
    fast_io корректно освобождает память во всех фазах.
```

**Пример вывода (Valgrind)**

```
PHP peak delta  : +2.00 MB  ✅
RSS delta       : +25.32 MB  ✅  [VG-оверхед учтён, порог 512 MB]

✅  УТЕЧЕК ПАМЯТИ НЕ ОБНАРУЖЕНО
    PHP peak delta < 8 MB  |  VG: 0 definitely/indirectly lost
```

---

### 2. `check-memleaks.sh` — обёртка с поддержкой valgrind

Два режима тестирования.

**Возможности**

- Автоматический поиск `valgrind-php.supp`
- Полный разбор Valgrind-лога с вердиктами
- Подсветка `fast_io` в стеках утечек
- Рекомендации по анализу
- Поддержка `--native`, `--keep-logs`, `--no-color`

**Запуск**

```bash
# Valgrind (рекомендуется для релиза)
./check-memleaks.sh                  # 4000 итераций
./check-memleaks.sh 40000            # больше нагрузки

# Только PHP (очень быстро)
./check-memleaks.sh 10000 --native

# С сохранением всех файлов
./check-memleaks.sh 4000 --keep-logs
```

**Пример финального вердикта Valgrind**

```
LEAK SUMMARY (Valgrind):
   definitely lost:            0 bytes   [OK]
   indirectly lost:            0 bytes   [OK]
   possibly lost:              0 bytes   [OK]
   still reachable:        77864 bytes   [ℹ PHP internals — норма]

  ✅  УТЕЧЕК ПАМЯТИ В fast_io НЕ ОБНАРУЖЕНО
```

---

### Как интерпретировать результаты

| Метрика                  | Норма (native) | Норма (Valgrind)          | Что значит |
|--------------------------|----------------|---------------------------|------------|
| **PHP peak delta**       | < 8 МБ         | < 8 МБ                    | Основной показатель |
| **RSS delta**            | < 8 МБ         | < 512 МБ                  | Учитывает overhead Valgrind |
| **definitely lost**      | 0 байт         | 0 байт                    | Реальная утечка |
| **indirectly lost**      | 0 байт         | 0 байт                    | Реальная утечка |
| **still reachable**      | —              | любые (PHP internals)     | Норма |
| **fast_io в стеке**      | —              | 0 упоминаний              | Критично |

**still reachable от `zend/dlopen`, `php_module_startup`** — это **нормально**. PHP не освобождает их при завершении скрипта, ОС делает это автоматически.

---

### Рекомендации по использованию в CI/CD

```yaml
# .github/workflows/memleak.yml
name: Memory Leak Test
on: [push, pull_request]

jobs:
  memleak:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - name: Install Valgrind
        run: sudo apt-get install -y valgrind
      - name: Build fast_io
        run: phpize && ./configure && make
      - name: Run leak test
        run: ./check-memleaks.sh 10000
```

---

**Эти инструменты — часть философии fast_io:**  
«Если тест на 40 000 итераций под Valgrind прошёл — библиотека безопасна для продакшена».

Все логи и снимки сохраняются в `/tmp/fast_io_memleak_<pid>` — для глубокого анализа.

