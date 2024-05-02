# Описание функции file_search_array

Функция file_search_array осуществляет поиск значения по ключу в файле данных. 
Она выполняет посекторный низкоуровневый поиск и возвращает найденное значение. 
Файл читается порциями, что делает функцию эффективной для работы с большими файлами. 


Страница описания прокта [Fast_IO Engine](https://github.com/commeta/fast_io).


## Синтаксис

string file_search_array(string $filename, string $line_key[, int mode = 0][, int search_start = 0][, int search_length = 1][, int position = 0])


### Параметры

- **filename** - Путь к файлу, в котором будет производиться поиск.
- **line_key** - Подстрока, по которой осуществляется поиск строки.
- **mode** (int, optional) - Режим поиска.
- **search_start** (int, optional) - Стартовая строка начала выборки.
- **search_limit** (int, optional) - Ограничение массива выборки.
- **position** (int, optional) - Позиция начала поиска в файле.

Чтение по смещению позиции position позволяет избежать лишних чтений файла при выборке с окном пагинации.
Чтение файла идет всегда с нулевой позиции position = 0, и если search_start больше нуля то функция просто пропускает строки у которых номер меньше search_start.


#### Режимы поиска

##### Поиск line_key в каждой строке
- 0: Возвращает ассоциативный массив: trim_line, trim_length, line_offset, line_length, line_count.
- 1: Возвращает ассоциативный массив: line, line_offset, line_length, line_count.
- 3: Возвращает ассоциативный массив: line_count, found_count.

##### Поиск по регулярному выражению [PCRE2](https://pcre2project.github.io/pcre2/doc/html/index.html) в каждой строке
- 10: Возвращает ассоциативный массив: trim_line, trim_length, line_offset, line_length, line_count.
- 11: Возвращает ассоциативный массив: line, line_offset, line_length, line_count.
- 13: Возвращает ассоциативный массив: line_count, found_count.

##### Поиск по регулярному выражению 
- 20: Возвращает ассоциативный массив: line_matches, line_offset, line_length, line_count.
- 21: Возвращает ассоциативный массив: trim_line, trim_length, line_matches, line_offset, line_length, line_count.
- 22: Возвращает ассоциативный массив: line, line_matches, line_match, match_offset, match_length, line_offset, line_length, line_count.
- 23: Возвращает ассоциативный массив: line_matches, line_match, match_offset, match_length, line_offset, line_length, line_count.

##### Log mode
- +100 Log mode: Если добавить +100 к любому из вышеперечисленных режимов, функция пересчитает режим mode -= 100 но не будет блокировать файл.

Режимы +100 Log mode подходят для работы с файлами журналов. Подробнее: [алгоритм реализации транзакции с помощью блокировки файла](/test/transaction/README.md).



### Возвращаемые значения

Функция возвращает ассоциативный массив

- line - Строка.
- trim_line - Строка без пробелов справа и символа перевода строки.
- trim_length - Длина обрезанной строки.
- line_matches - Подстроки совпавшие с регулярным выражением (в режиме 20).

- line_match - Подстрока совпавшая с регулярным выражением.
- match_offset - Смещение подстроки от начала строки
- match_length - Длина подстроки

- line_offset - Смещение строки (с учетом смещения начала поиска в файле).
- line_length - Длина строки.
- line_count - Счетчик строк от начала поиска.

или

- line_count - Количество строк в файле
- found_count - Количество строк найденных совпадений

Функция возвращает массив строк, содержащий значения, ассоциированное с найденной строкой.
Вслучае, если ключ не найден или произошла ошибка (например, файл не может быть прочитан), функция возвращает пустой массив или NULL.
Строки возвращается без конечных пробелов и символа перевода строки.

### Примеры возвращаемых массивов

```
for($i=0; $i <=500; $i++){
	print_r(
		file_insert_line(__DIR__ . '/fast_io1.dat', 'index_' . $i . ' insert_line_' . $i . ' ' . str_pad('', 8, '1234567890'), 8192)
	);
}
```


```
print_r([
	file_search_array(__DIR__ . '/fast_io1.dat', 'index_3', 0, 0, 2),
]);

Array
(
    [0] => Array
        (
            [0] => Array
                (
                    [trim_line] => index_3 file_insert_line_3 12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012
                    [trim_length] => 119
                    [line_offset] => 24576
                    [line_length] => 8192
                    [line_count] => 4
                )

            [1] => Array
                (
                    [trim_line] => index_30 file_insert_line_30 12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012
                    [trim_length] => 121
                    [line_offset] => 245760
                    [line_length] => 8192
                    [line_count] => 31
                )
        )
)

```


```
print_r([
	file_search_array(__DIR__ . '/fast_io1.dat', '\\w+_\\d+', 10, 0, 2)
]);

Array
(
    [0] => Array
        (
            [0] => Array
                (
                    [trim_line] => index_0 file_insert_line_0 12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012
                    [trim_length] => 8192
                    [line_offset] => 0
                    [line_length] => 8192
                    [line_count] => 1
                )

            [1] => Array
                (
                    [trim_line] => index_1 file_insert_line_1 12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012
                    [trim_length] => 8192
                    [line_offset] => 8192
                    [line_length] => 8192
                    [line_count] => 2
                )

        )

)
```

```
print_r([
	file_search_array(__DIR__ . '/fast_io1.dat', '\\w+', 13),
	file_search_array(__DIR__ . '/fast_io1.dat', 'index_3', 3),
]);

Array
(
    [0] => Array
        (
            [0] => Array
                (
                    [line_count] => 501
                    [found_count] => 501
                )

        )

    [1] => Array
        (
            [0] => Array
                (
                    [line_count] => 501
                    [found_count] => 111
                )

        )

)

```


```
print_r([
	file_search_array(__DIR__ . '/fast_io1.dat', '\\w+_\\d+', 20, 0, 2),
]);

Array
(
    [0] => Array
        (
            [0] => Array
                (
                    [trim_line] => index_0 file_insert_line_0 12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012
                    [trim_length] => 119
                    [line_matches] => Array
                        (
                            [0] => index_0
                            [1] => file_insert_line_0
                        )

                    [line_offset] => 0
                    [line_length] => 8192
                    [line_count] => 1
                )

            [1] => Array
                (
                    [trim_line] => index_1 file_insert_line_1 12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012
                    [trim_length] => 119
                    [line_matches] => Array
                        (
                            [0] => index_1
                            [1] => file_insert_line_1
                        )

                    [line_offset] => 8192
                    [line_length] => 8192
                    [line_count] => 2
                )
        )
)
```

```
print_r([
	file_search_array(__DIR__ . '/fast_io1.dat', '\\w+_\\d+', 22, 0, 2),
]);

Array
(
    [0] => Array
        (
            [0] => Array
                (
                    [line_matches] => Array
                        (
                            [0] => Array
                                (
                                    [line_match] => index_0
                                    [match_offset] => 0
                                    [match_length] => 7
                                )
                            [1] => Array
                                (
                                    [line_match] => file_insert_line_0
                                    [match_offset] => 7
                                    [match_length] => 18
                                )
                        )
                    [line_offset] => 0
                    [line_length] => 8192
                    [line_count] => 1
                )
            [1] => Array
                (
                    [line_matches] => Array
                        (
                            [0] => Array
                                (
                                    [line_match] => index_1
                                    [match_offset] => 0
                                    [match_length] => 7
                                )
                            [1] => Array
                                (
                                    [line_match] => file_insert_line_1
                                    [match_offset] => 7
                                    [match_length] => 18
                                )
                        )
                    [line_offset] => 8192
                    [line_length] => 8192
                    [line_count] => 2
                )
        )
)
```





## Пример использования

В этом примере мы ищем значение по ключу "user_id" в файле "data.txt".
```
<?php
$filename = 'path/to/data.txt';
$key = 'user_id';

$value = file_search_array($filename, $key);

if ($value !== NULL) {
    print_r(["Найденное значение: ", $value]);
} else {
    echo "Значение не найдено или произошла ошибка";
}
?>
```

Еще [пример](/test/readme.md): Тесты.

### Формат файла

Функция предполагает, что файл содержит данные в формате "ключ значение", где каждая пара разделена новой строкой. Например:

```
index_0 data_write_key_value_pair_0
index_1 data_write_key_value_pair_1
index_2 data_write_key_value_pair_2
```


## Ограничения

- Файл должен быть доступен для чтения процессом PHP.
- Функция читает файл порциями, размер которых определяется константой BUFFER_SIZE.

## Стоимость вызова

- CPU Bound - Заполнение буфера и полнотекстовый поиск в каждой строке, регулярные выражения PCRE2.
- IO Bound - Посекторное чтение файла в буфер.

Среднее потребление, чтение всего файла в режимах.
