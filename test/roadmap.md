# Roadmap (C-прототипы + PHP-сигнатуры)  

Ниже — исчерпывающий каталог предложенных функций, сгруппированных по назначению. Для каждой функции: короткое назначение, C-прототип (как в расширении PHP), PHP-сигнатура (удобная для использования из PHP), поведение, входные параметры, возвращаемые значения, заметки по блокировкам/консистентности и алгоритмические примечания. Функции спроектированы так, чтобы работать независимо от внешней логики — подойдут любой CMS/библиотеке, где данные хранятся в текстово/байтовых файлах в формате «строка = запись».

---

# 1. Управление индексами (вторичные индексы)

Индексы — ключ к быстрым WHERE/ORDER/JOIN. Форматы: `hash` (bucket → список offset) и `sorted` (список (key, offset, size) отсортированный по key).

---

## 1.1 `file_create_index`

**Назначение:** создать вторичный индекс для указанного поля файла.

**C-прототип**

```c
PHP_FUNCTION(file_create_index);
```

**PHP-сигнатура**

```php
bool file_create_index(string $data_filename, int|string $field, string $index_filename = null, int $options = 0);
```

**Параметры**

* `$data_filename` — путь к исходному файлу данных.
* `$field` — позиция поля (int, нумерация с 0) либо имя поля (string), если используется формат с именованными полями (JSON/CSV с заголовком).
* `$index_filename` — (опционально) имя файла индекса; по умолчанию `<data_filename>.idx.<field>`.
* `$options` — битовое поле опций: `0` = hash, `1` = sorted, `2` = case_insensitive, `4` = numeric_keys.

**Возвращает**

* `true` — индекс создан успешно.
* `false` — ошибка (файл не найден, парсинг, диск).

**Поведение/заметки**

* Создаёт на диске индекс в выбранном формате.
* Во время построения берёт shared/exclusive lock на data file (flock), обеспечивает консистентность.
* Реализует потоковую обработку (не требует загрузки всего файла в память).
* Индекс должен содержать минимальную метаинформацию: версия формата, поле, тип ключа.

---

## 1.2 `file_rebuild_index`

**Назначение:** перегенерировать/обновить существующий индекс (полная перестройка).

**C-прототип**

```c
PHP_FUNCTION(file_rebuild_index);
```

**PHP-сигнатура**

```php
bool file_rebuild_index(string $data_filename, string $index_filename, int $options = 0);
```

**Поведение**

* Безопасно перестраивает индекс: создаёт временный `.tmp`, заполняет, затем атомарно переименовывает.
* Возвращает `true` при успехе.

---

## 1.3 `file_index_info`

**Назначение:** получить метаданные по индексу.

**C-прототип**

```c
PHP_FUNCTION(file_index_info);
```

**PHP-сигнатура**

```php
array|false file_index_info(string $index_filename);
```

**Возвращает**

* Ассоциативный массив: `['format' => 'hash|sorted', 'field' => int|string, 'entries' => int, 'size' => int, 'created' => timestamp]` или `false` при ошибке.

---

## 1.4 `file_index_lookup`

**Назначение:** искать записи по ключу, используя индекс (низкоуровневое API).

**C-прототип**

```c
PHP_FUNCTION(file_index_lookup);
```

**PHP-сигнатура**

```php
array|false file_index_lookup(string $index_filename, string $key, int $limit = 0, int $offset = 0);
```

**Возвращает**

* Список пар `[['offset'=>int,'size'=>int], ...]`, найденных по ключу, либо пустой массив, либо `false` при ошибке.

**Замечание**

* Используется как building block для `file_select` и `file_join`.

---

# 2. Выборка / проекция / WHERE / LIMIT / OFFSET / ORDER BY

---

## 2.1 `file_select`

**Назначение:** универсальная выборка из файла с поддержкой WHERE, ORDER BY, LIMIT/OFFSET и проекции полей.

**C-прототип**

```c
PHP_FUNCTION(file_select);
```

**PHP-сигнатура**

```php
array|false file_select(string $data_filename, array $options = []);
```

**Опции (`$options`)**

* `where` — строка (PCRE) или callback; поддерживается:

  * `string` — PCRE (например `'/"published":1/'`) — быстрый путь;
  * `array ['field'=>int|string, 'op'=> '=', 'value'=>mixed]` — простые условия;
  * `callable ($line, $parsed) : bool` — PHP callback (медленно, но гибко).
* `order_by` — int|string (позиция поля или имя).
* `direction` — 1 (ASC) или -1 (DESC).
* `limit` — int (0 = без лимита).
* `offset` — int.
* `fields` — array позиций/имен (проекция).
* `index` — путь к индексу (если указано, будет использован).
* `mode` — флаги (например +100 = log mode).

**Возвращает**

* Массив строк (каждая — ассоциативный массив полей или массив проекции), либо `false` при ошибке.

**Алгоритм**

1. Попытка использовать индекс/индексы (если WHERE по индексированному полю или ORDER_BY по индексированному полю).
2. Если индекса нет и `order_by` указан — либо in-memory сортировка (если fit in memory), либо external sort (через `file_sort`).
3. Применяет `offset/limit` максимально рано (чтобы не держать ненужные результаты).
4. Для `where` в формате простого `field op value` выполняется сравнение в C — быстрее, чем PCRE.

**Блокировки**

* Для чтения используется shared lock; если использован индекс — может потребоваться кратковременный exclusive lock при его обновлении.

---

## 2.2 `file_select_stream` (callback-вариант)

**Назначение:** стриминговая выборка с передачей каждой строки в callback (меньше памяти).

**C-прототип**

```c
PHP_FUNCTION(file_select_stream);
```

**PHP-сигнатура**

```php
int|false file_select_stream(string $data_filename, array $options = [], callable $callback);
```

**Поведение**

* Для больших выборок позволяет обрабатывать данные по мере чтения.
* Callback получает `(string $line, array $parsed_fields)`; если callback возвращает `false` — остановка (ранний выход).
* Возвращает количество обработанных строк или `false` при ошибке.

---

# 3. Сортировка (ORDER BY) и внешняя сортировка

---

## 3.1 `file_sort`

**Назначение:** отсортировать файл по ключу; поддерживает in-memory и external-merge.

**C-прототип**

```c
PHP_FUNCTION(file_sort);
```

**PHP-сигнатура**

```php
bool file_sort(string $source_filename, string $dest_filename = null, int|string $key_field, int $direction = 1, int $options = 0);
```

**Опции**

* `options` может включать `0` = auto (memory estimate), `1` = force_external, `2` = stable_sort.

**Поведение**

* Если `dest_filename` = null — выполняет in-place через временный файл + atomic rename.
* Для больших файлов разбивает на чанки, сортирует чанки, затем сливает (external merge).
* Ключи извлекаются по позиции/имени (при необходимости поддерживается JSON/CSV парсинг).

**Возвращает**

* `true` при успехе, `false` при ошибке.

---

## 3.2 `file_sort_info`

**Назначение:** метаданные/статус текущей сортировки (для мониторинга).

**PHP-сигнатура**

```php
array|false file_sort_info(string $job_id_or_dest_filename);
```

---

# 4. Группировка и агрегаты (GROUP BY / HAVING)

---

## 4.1 `file_group_by`

**Назначение:** выполнить GROUP BY с агрегатами (COUNT, SUM, AVG, MIN, MAX) и опциональным HAVING.

**C-прототип**

```c
PHP_FUNCTION(file_group_by);
```

**PHP-сигнатура**

```php
array|false file_group_by(string $data_filename, int|string $group_field, array $agg_spec = [], array $options = []);
```

**Параметры**

* `$group_field` — позиция/имя поля для группировки.
* `$agg_spec` — массив агрегатов: примеры:

  * `['count' => true]`
  * `['sum' => ['field' => 4]]`
  * `['avg' => ['field' => 5]]`
  * поддерживаются несколько агрегатов одновременно: `['count'=>true,'sum'=>['field'=>4]]`.
* `$options` может содержать `having` (PCRE или callable), `memory_limit` (байты), `external` (force external group-by).

**Возвращает**

* Ассоциативный массив: `group_key => ['count'=>..., 'sum'=>..., ...]`.

**Алгоритм**

* По умолчанию — хэш-агрегирование в памяти.
* Если число уникальных групп велико и превысит `memory_limit` — переключается на external: сначала `file_sort` по `group_field`, затем последовательное редуцирование.

---

# 5. JOIN (INNER, LEFT, RIGHT, FULL)

---

## 5.1 `file_join`

**Назначение:** выполнить JOIN двух файлов по ключу (hash-join / merge-join в зависимости от наличия индексов/размеров).

**C-прототип**

```c
PHP_FUNCTION(file_join);
```

**PHP-сигнатура**

```php
array|false file_join(string $left_file, string $right_file, int|string $left_key, int|string $right_key, int $join_type = 0, array $options = []);
```

**Параметры**

* `$left_file`, `$right_file` — файлы с данными.
* `$left_key`, `$right_key` — позиции/имена ключевых полей.
* `$join_type` — `0`=INNER, `1`=LEFT, `2`=RIGHT, `3`=FULL.
* `$options`:

  * `left_index`, `right_index` — пути к индексам (если есть).
  * `select` — массив полей для проекции: `['left'=>[0,1], 'right'=>[0,2]]`.
  * `memory_limit` — байты для hash table.
  * `callback` — callable для обработки каждой joined row (стрим).

**Возвращает**

* При без callback — массив объединённых строк.
* При наличии `callback` — число обработанных строк.

**Алгоритм**

1. Если один из файлов значительно меньше и умещается в `memory_limit` — строим hash table по меньшему файлу и пробуем читать больший, делая быстрые O(1) lookup.
2. Если оба файла отсортированы по ключу (или есть sorted index) — используем merge-join (линейное).
3. Если ни одно условие не выполнено — пытаемся использовать индекс (если есть) или выполняем блочный подход (разбиваем на чанки).
4. Для LEFT/RIGHT/FULL соблюдаем семантику — при отсутствии совпадения возвращаем `NULL` для полей примыкаемой стороны.

**Блокировки**

* Чтение — shared locks; если используется index update — может потребоваться кратковременный exclusive lock.

---

## 5.2 `file_join_stream` (callback-вариант)

**PHP-сигнатура**

```php
int|false file_join_stream(string $left_file, string $right_file, int|string $left_key, int|string $right_key, int $join_type = 0, array $options = [], callable $callback);
```

* Обрабатывает joined строки по мере генерации, снижая потребление памяти для больших джоинов.

---

# 6. Транзакции и атомарные операции

> Замечание: полных ACID-транзакций на файловой структуре ждать не стоит, но можно обеспечить атомарность и упрощённую транзакционную оболочку (журналирование, atomic rename).

---

## 6.1 `file_upsert`

**Назначение:** атомарно вставить или обновить запись по ключу (insert-or-update).

**C-прототип**

```c
PHP_FUNCTION(file_upsert);
```

**PHP-сигнатура**

```php
bool file_upsert(string $data_filename, int|string $key_field, string $key_value, string $line_or_serialized, int $options = 0);
```

**Поведение**

* Если найдено совпадение по ключу — обновляет строку (atomic replace via temp file / or append + tombstone strategy), иначе вставляет.
* Обновляет/поддерживает индекс (если присутствует).

---

## 6.2 `file_transaction_begin`, `file_transaction_commit`, `file_transaction_rollback`

**Назначение:** эмуляция транзакции с журналом.

**PHP-сигнатуры**

```php
bool file_transaction_begin(string $data_filename, string $journal_name = null);
bool file_transaction_commit(string $data_filename, string $journal_name = null);
bool file_transaction_rollback(string $data_filename, string $journal_name = null);
```

**Поведение**

* `begin` — создаёт journal (.txn), ставит exclusive lock.
* Все `upsert/update/delete` в рамках txn логируются в journal.
* `commit` — применяет journal atomically (rename/temp), снимает lock и удаляет journal.
* `rollback` — удаляет journal и откатывает незавершённые изменения.

**Ограничения**

* Не даёт распределённой транзакции; пригодно для одиночного файла и коротких операций.

---

# 7. Утилиты / пакетные операции

---

## 7.1 `file_batch_get`

**Назначение:** получить множество записей по списку ключей быстрее, чем N отдельных `file_index_lookup`.

**PHP-сигнатура**

```php
array|false file_batch_get(string $data_filename, array $keys, array $options = []);
```

**Поведение**

* Использует индекс (если есть) и читает блоками, минимизируя системные вызовы.

---

## 7.2 `file_multi_search`

**Назначение:** искать шаблон/условие сразу по списку файлов и объединить результаты.

**PHP-сигнатура**

```php
array|false file_multi_search(array $data_filenames, array $options = []);
```

**Опции**

* `where`, `order_by`, `limit_per_file`, `global_limit`, `callback`.

---

## 7.3 `file_compact` / `file_compact_range`

**Назначение:** дефрагментация и компактизация файла (удаление tombstone, реорганизация).

**PHP-сигнатуры**

```php
bool file_compact(string $data_filename, array $options = []);
bool file_compact_range(string $data_filename, int $from_offset, int $to_offset, array $options = []);
```

**Поведение**

* Перезаписывает файл в компактную форму, обновляет индексы атомарно.

---

## 7.4 `file_defrag_index`

**Назначение:** оптимизация/перегенерация индексного файла (удаление удалённых записей, перестройка buckets).

**PHP-сигнатура**

```php
bool file_defrag_index(string $index_filename, array $options = []);
```

---

# 8. Стриминг / Callback-ориентированные API

Стриминг критичен для больших наборов данных, чтобы не держать всё в памяти.

---

## 8.1 `file_map` / `file_foreach`

**Сигнатура**

```php
int|false file_map(string $data_filename, callable $callback, array $options = []);
```

**Поведение**

* Проходит файл построчно, вызывает `$callback($line, $parsed_fields)`.
* Возвращает число обработанных строк.

---

## 8.2 `file_join_callback`

**Сигнатура**

```php
int|false file_join_callback(string $left_file, string $right_file, int|string $left_key, int|string $right_key, int $join_type, array $options, callable $callback);
```

**Поведение**

* Генерирует joined строки и передаёт их callback по мере вычисления.

---

# 9. Диагностика и мониторинг

---

## 9.1 `file_stats`

**Сигнатура**

```php
array|false file_stats(string $data_filename);
```

**Возвращает**

* `['rows'=>int,'size'=>int,'avg_row_size'=>float,'indexes'=>array, 'last_modified'=>timestamp]`.

---

## 9.2 `file_benchmark`

**Сигнатура**

```php
array|false file_benchmark(string $data_filename, array $scenario);
```

**Поведение**

* Выполняет набор тестов: поиск по ключу, скан + фильтр, сортировка, join с малой таблицей; возвращает времена и I/O метрики.

---

# 10. Составные/высокоуровневые помощники (можно реализовать на PHP на базе нижележащих C-функций)

Эти функции не обязательно реализовывать в C, но их удобно иметь как API-обёртки.

---

## 10.1 `file_query_simple` (SQL-like)

**PHP-сигнатура**

```php
array|false file_query_simple(string $sql_like, array $options = []);
```

**Назначение**

* Парсер очень простого SQL (SELECT … FROM … WHERE … ORDER BY … LIMIT …).
* Переводит в последовательность вызовов `file_select`/`file_sort`/`file_group_by`/`file_join`.

**Ограничение**

* Поддержка ограничена (no subqueries, no complex expressions); удобно как PoC.

---

## 10.2 `file_cache_load` / `file_cache_write`

**Назначина:** кэширование результатов тяжёлых операций на диск/файловую структуру (TTL, invalidation).

**PHP-сигнатура**

```php
mixed file_cache_load(string $cache_key, int $ttl = 0);
bool  file_cache_write(string $cache_key, mixed $value, int $ttl = 0);
bool  file_cache_invalidate(string $cache_key);
```

---

# 11. Форматы/парсинг строк — соглашения

Для всех функций нужен общий набор парсеров строк в fast_io:

* `split_by_delim($line, $delim)` — быстрый C-split.
* JSON-mode: извлекаем поле по JSON path (поддержка быстрых поисков, оптимизация).
* CSV-mode: учитывать quoting/escape.
* Generic: raw line + PCRE2.

API должны позволять выбрать mode в параметрах (`format` => `raw|csv|json|kv`).

---

# 12. Замечания по блокировкам и консистентности

* Чтение: shared lock (flock LOCK_SH).
* Запись/перестройка: exclusive lock (LOCK_EX).
* Для `file_upsert`/`file_compact`/`file_rebuild_index` — использовать временные файлы и `rename()` для атомичности.
* Journal (txn) также хранить в отдельном файле `.txn` и применять через atomic rename.
* При репликации/распределении: явно документировать, что flock не надёжна по NFS; рекомендовать локальные диски.

---

# 13. Примеры PHP-вызовов (кратко)

```php
// создать индекс
file_create_index('/data/posts.dat', 2, null, 1); // sorted по полю 2

// простая выборка
$res = file_select('/data/posts.dat', [
  'where' => ['field'=>3,'op'=>'=','value'=>'published'],
  'order_by' => 0,
  'direction'=>-1,
  'limit' => 50,
  'offset'=>100,
  'fields'=>[0,1,3]
]);

// группировка
$groups = file_group_by('/data/posts.dat', 4, ['count'=>true,'sum'=>['field'=>5]]);

// join с callback
file_join_callback('/data/posts.dat','/data/tags.dat',0,1,0, ['left_index'=>null,'right_index'=>'/data/tags.idx'],
  function($joined){ /* обработка */ return true; });
```

---

# 14. Резюме — какие функции следует реализовать в первую очередь 

1. `file_create_index`, `file_rebuild_index`, `file_index_lookup`, `file_index_info` — индексы.
2. `file_select` и `file_select_stream` — универсальная выборка с WHERE/LIMIT/OFFSET/ORDER.
3. `file_sort` — in-memory + external merge-sort.
4. `file_group_by` — хэш/streaming и external reduce.
5. `file_join` и `file_join_stream` — hash-join + merge-join.
6. `file_upsert`, транзакционное API (`file_transaction_*`) — атомарные изменения.
7. `file_batch_get`, `file_multi_search`, `file_compact`, `file_defrag_index` — утилиты обслуживания.
8. `file_map/file_foreach`, `file_join_callback` — стриминговые интерфейсы.
9. Диагностика: `file_stats`, `file_benchmark`, `file_sort_info`.

---


