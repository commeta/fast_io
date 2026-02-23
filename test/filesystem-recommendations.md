# Fast_IO — Рекомендации по файловым системам Linux

Данный документ содержит **архитектурный анализ** и **практические рекомендации** по выбору и настройке файловых систем Linux специально под нагрузку библиотеки `fast_io`.

---

## 1. Краткий вывод

**fast_io** активно использует:
- Много **случайных offset-based** операций (`fseek` + `fread`/`fwrite`)
- Частые `flock(LOCK_EX)`
- `ftruncate`, `rename()`, `fflush`
- Большой page cache и aligned блоки

**Лучшие ФС для fast_io (2026):**

| Приоритет | Файловая система | Сценарий использования                  | Общий балл |
|-----------|------------------|-----------------------------------------|------------|
| **1**     | **XFS**          | Высокая запись (update/defrag)          | **9.7**    |
| **2**     | **ext4**         | Универсальный production                | **9.5**    |
| **3**     | **F2FS**         | Чистый NVMe/SSD, максимальные IOPS      | **9.0**    |
| **4**     | **ZFS**          | Максимальная надёжность данных          | **8.0**    |
| —         | **tmpfs**        | Тесты и бенчмарки                       | **10**     |

**Не рекомендуется:** Btrfs, NFS, CephFS, GlusterFS.

---

## 2. Сравнительная таблица

| ФС          | Aligned I/O | Журналирование | flock() | rename() атомарность | ftruncate | Page Cache | Подходит для fast_io | Рекомендация |
|-------------|-------------|----------------|---------|----------------------|-----------|------------|----------------------|--------------|
| **XFS**     | Отлично     | Metadata only  | Отлично | Отлично              | Отлично   | Отличное   | Отлично              | **Лучший для write-heavy** |
| **ext4**    | Отлично     | writeback      | Отлично | Отлично              | Отлично   | Отличное   | Отлично              | **Универсальный выбор** |
| **F2FS**    | Отлично     | Roll-forward   | Хорошо  | Отлично              | Отлично   | Хорошее    | Отлично              | Для NVMe/SSD |
| **ZFS**     | Отлично     | CoW + ZIL      | Хорошо  | Отлично              | Хорошо    | Отличное   | Хорошо               | Когда нужна надёжность |
| **Btrfs**   | Хорошо      | CoW            | Хорошо  | Отлично              | Средне    | Среднее    | Плохо                | **Избегать** |
| **tmpfs**   | Идеально    | Нет            | Отлично | Отлично              | Идеально  | Идеально   | Идеально (тесты)     | Только тесты |

---

## 3. Рекомендуемые mount-опции и команды

### 3.1 ext4 (универсальный production)

```bash
# Рекомендуемые опции
mount -o noatime,nodiratime,data=writeback,commit=1,barrier=0,nobarrier \
      -t ext4 /dev/nvme0n1p2 /var/db/fast_io
```

**Постоянно в /etc/fstab:**
```
/dev/nvme0n1p2  /var/db/fast_io  ext4  noatime,nodiratime,data=writeback,commit=1,barrier=0,nobarrier  0 2
```

### 3.2 XFS (максимальная производительность записи)

```bash
mount -o noatime,nodiratime,attr2,inode64,swalloc,allocsize=64m \
      -t xfs /dev/nvme0n1p2 /var/db/fast_io
```

**/etc/fstab:**
```
/dev/nvme0n1p2  /var/db/fast_io  xfs  noatime,nodiratime,attr2,inode64,swalloc,allocsize=64m  0 2
```

### 3.3 F2FS (чисто NVMe/SSD)

```bash
mount -o noatime,nodiratime,background_gc=on,inline_data,inline_dentry \
      -t f2fs /dev/nvme0n1p2 /var/db/fast_io
```

### 3.4 tmpfs (для тестов)

```bash
mount -t tmpfs -o size=16G,mode=1777 tmpfs /var/db/fast_io_test
```

---

## 4. Сценарии использования и выбор ФС

| Сценарий                              | Рекомендуемая ФС | Почему |
|---------------------------------------|------------------|--------|
| **Высокая запись** (много `update_line`, `defrag_data`, `insert_line`) | **XFS** | Самая быстрая случайная запись + metadata journaling |
| **Смешанная нагрузка** (поиск + запись) | **ext4** | Лучший баланс стабильности и скорости |
| **Чистый NVMe, >50k IOPS**            | **F2FS** | Оптимизирована под flash-память |
| **Максимальная надёжность + много RAM** | **ZFS** | CoW + checksums + snapshots |
| **Тесты и бенчмарки** (`check-engine-*`) | **tmpfs** | Всё в RAM — максимальная скорость |
| **База > 100 ГБ**                     | **XFS** или **ext4** | Лучшая масштабируемость |
| **Логи / append-only**                | **ext4** с `data=writeback` | Режим `+100` (no-lock) |

---

## 5. Дополнительные рекомендации

### Для всех ФС:
```bash
# Отключить atime навсегда
echo "vm.vfs_cache_pressure=200" >> /etc/sysctl.d/99-fastio.conf

# Агрессивный writeback
echo "vm.dirty_ratio=5" >> /etc/sysctl.d/99-fastio.conf
echo "vm.dirty_background_ratio=2" >> /etc/sysctl.d/99-fastio.conf
```

### Для NVMe:
```bash
echo mq-deadline > /sys/block/nvme0n1/queue/scheduler
echo 8192 > /sys/block/nvme0n1/queue/read_ahead_kb
```

### TRIM / discard (для SSD):
```bash
# В fstab добавить:
,discard
```

---
