# Описание функции file_pop_line

## Назначение функции

Функция file_pop_line предназначена для извлечения и удаления последней строки из файла. Эта функция может быть полезна в ситуациях, когда необходимо работать с файлами в формате стека или журнала, где последняя запись должна быть обработана и затем удалена.

## Синтаксис

string file_pop_line(string $filename[, int $line_align = -1])


## Параметры

- **filename** *(string)*: Путь к файлу, из которого будет извлечена и удалена последняя строка.
- **line_align** *(int, optional)*: Индекс, начиная с которого будет производиться чтение строки. Если этот параметр не указан или указан как -1, функция будет читать файл с конца до начала, пока не найдет полную строку. Указание этого параметра может быть полезно для оптимизации производительности при работе с известным форматом данных.

## Возвращаемые значения

Функция возвращает **строку**, извлеченную из файла, если операция прошла успешно. В случае ошибки при работе с файлом (например, файл не найден, ошибка чтения и т.д.) функция вернет **FALSE** и сгенерирует предупреждение.

Строка возвращается без конечных пробелов и символа перевода строки.

## Примеры использования

### Пример 1. Извлечение последней строки из файла
```
$filename = "/path/to/your/file.txt";
$lastLine = file_pop_line($filename);

if ($lastLine !== false) {
    echo "Последняя строка файла: $lastLine";
} else {
    echo "Ошибка при чтении файла";
}
```

### Пример 2. Использование необязательного параметра index_align
```
$filename = "/path/to/your/file.txt";
// Предположим, что мы знаем, что строки в файле не превышают 100 байт
$line_align = 100;
$lastLine = file_pop_line($filename, $line_align);

if ($lastLine !== false) {
    echo "Последняя строка файла: $lastLine";
} else {
    echo "Ошибка при чтении файла";
}
```

Еще [пример](/test/readme.md): Тесты.

## Замечания

- Функция блокирует файл на время своей работы для предотвращения конкурентного доступа.
- Важно учитывать, что функция изменяет файл, удаляя из него последнюю строку.
- Функция может быть неэффективна для очень больших файлов, если не используется параметр line_align, так как ей придется читать файл с конца до начала для поиска последней строки.


## Стоимость вызова

- CPU Bound - Заполнение буфера и полнотекстовый поиск в начале каждой строки (без выравнивания).
- IO Bound - Чтение файла с конца (без выравнивания чтений больше), усечение файла. 

Низкое потребление, с выравниванием очень низкое, чтение файла с конца, усечение файла.