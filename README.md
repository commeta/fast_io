# Fast_IO Extension for PHP 8
PHP extension, fast DB with indexes

Интеграция C-функций в модуль PHP через API Zend. 

Fast_IO - это расширение для PHP 8, предназначенное для эффективной работы с файлами данных, содержащими пары ключ-значение. Оно предоставляет функции для чтения, записи и удаления данных с использованием низкоуровневого посекторного доступа и портируемой блокировки файла, чтобы обеспечить синхронизацию доступа между параллельными экземплярами.

Позволяет хранить и поддерживать файлы баз данных произвольного размера, чтение файлов осуществляется порциями по 4096 байт (1 кэшируемая ядром ОС страница).

## Функции расширения

### write_key_value_pair

Функция write_key_value_pair предназначена для записи пары ключ-значение в файл данных. 
Во время записи устанавливается портируемая блокировка на файл, чтобы другие экземпляры ожидали освобождения файла для чтения или записи.

write_key_value_pair($filename, $index_key, $index_val);

**Параметры:**
- $filename - Путь к файлу базы данных.
- $index_key - ключ, который будет использоваться для идентификации записи.
- $index_val - значение, связанное с ключом.

---

### indexed_write_key_value_pair

Функция indexed_write_key_value_pair предназначена для записи пары ключ-значение в файловое хранилище с созданием или обновлением соответствующего индексного файла для быстрого доступа.

indexed_write_key_value_pair($filename, $index_key, $index_val);

**Параметры:**
- filename: Имя файла данных (без расширения .index), в который будет произведена запись.
- index_key: Ключ, под которым значение будет сохранено.
- index_val: Значение, которое нужно сохранить.

**Описание работы функции:**

1. Функция формирует имя индексного файла, добавляя к имени основного файла суффикс .index.
2. Открывает файл данных и индексный файл в режиме чтения/записи, создает их при необходимости. Если открыть файлы не удается, функция завершает работу.
3. Производит блокировку обоих файлов для безопасной записи в многопоточной среде.
4. Определяет текущее смещение в конце файла данных, куда будет произведена запись значения.
5. Записывает значение (index_val) в файл данных.
6. Создает запись индекса (IndexRecord), содержащую смещение и размер записанного блока данных.
7. Записывает ключ (index_key) и информацию об индексе в индексный файл.
8. Освобождает блокировки и закрывает файлы.

**Пример использования:**

`indexed_write_key_value_pair("data", "myKey", "Значение, которое нужно сохранить");`

Этот вызов функции сохранит строку "Значение, которое нужно сохранить" в файл "data", а соответствующий ключ "myKey" вместе с информацией о смещении и размере данных будет записан в файл индекса "data.index" для быстрого поиска.

**Примечание:** Функция не возвращает значение, однако она обеспечивает корректное закрытие файлов и освобождение ресурсов даже в случае возникновения ошибок в процессе выполнения.

---

### find_value_by_key

Функция find_value_by_key осуществляет поиск значения по ключу в файле данных. Она выполняет посекторный низкоуровневый поиск и возвращает найденное значение. Во время работы функция не блокирует файл на чтение, позволяя другим экземплярам выполнять параллельный доступ.

**Пример использования:**

$value = find_value_by_key($filename, $index_key);


**Параметры:**

- $filename - Путь к файлу базы данных
- $index_key - ключ, который будет использоваться для идентификации записи.

---

### indexed_find_value_by_key

Функция indexed_find_value_by_key предназначена для поиска и извлечения значения по ключу из файлового хранилища с использованием индексного файла для ускорения поиска.

**Пример использования:**

$value = find_value_by_key($filename, $index_key);


**Параметры:**

- filename: Имя файла данных (без расширения .index), в котором будет производиться поиск.
- index_key: Ключ, значение которого необходимо найти.

**Описание работы функции:**

1. Функция формирует имя индексного файла, добавляя к имени основного файла суффикс .index.
2. Открывает индексный файл и файл данных только для чтения.
3. Производит блокировку обоих файлов для безопасного чтения в многопоточной среде.
4. Читает индексный файл блоками фиксированного размера, ищет совпадение заданного ключа с ключами в индексном файле.
5. При нахождении совпадения извлекает информацию о смещении и размере данных в файле данных.
6. Считывает данные указанного размера из файла данных, начиная с найденного смещения.
7. Добавляет нуль-терминатор к полученным данным и возвращает указатель на строку с результатом.
8. Освобождает блокировки и закрывает файлы.

---

### delete_key_value_pair

Функция delete_key_value_pair удаляет пару ключ-значение из файла данных. Во время удаления устанавливается портируемая блокировка на файл, чтобы другие экземпляры ожидали освобождения файла. Данные копируются порциями во временный файл с последующим переименованием, что позволяет избежать загрузки всего файла в память.

**Пример использования:**

delete_key_value_pair($filename, $index_key);


**Параметры:**

- $filename - Путь к файлу базы данных.
- $index_key - ключ, который будет использоваться для идентификации записи.

---

## Особенности реализации

- Все функции используют UNIX портируемую блокировку файла на запись и чтение.
- Параллельные экземпляры функций ожидают в очереди освобождения файла.
- Для предотвращения перегрузки памяти функция удаления работает с копированием данных построчно без загрузки всего файла в память.
- Расширение разработано без использования сторонних фреймворков и библиотек, что обеспечивает его высокую производительность и совместимость с PHP 8.
- Функции с префиксом index_ бинарно безопасны, и позволяют хранить неограниченный объем в файле даных.
- Функции без префикса index_ позволяют хранить пары ключ:значение (string, json, serialized) в файле данных. Размер пары не должен превышать 4096 байт.

---

## Установка

### Шаг 1: Создание каркаса расширения
Скопируйте файлы: `config.m4`, `fast_io.c`, `fast_io.h` в текущий каталог проекта.

Для начала, вам нужно создать каркас вашего расширения. Это можно сделать вручную или с помощью инструмента ext_skel в исходниках PHP. Например:
```
phpize
./configure
make
make test
```

Это создаст базовую структуру для вашего расширения.



### Шаг 2: Компиляция и тестирование

Вам нужно скомпилировать расширение и протестировать его. Используйте phpize, ./configure, make и make test для компиляции и установки вашего расширения.

После установки не забудьте добавить строку `extension=fast_io.so` в ваш `php.ini`, чтобы активировать расширение.

Теперь вы можете вызывать `find_value_by_key`, `indexed_find_value_by_key`, `write_key_value_pair`, `indexed_write_key_value_pair`, `delete_key_value_pair` из PHP как обычные функции.


