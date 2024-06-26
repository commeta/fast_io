# Описание функции file_select_array

Функция file_select_array предназначена для извлечения строк из текстового файла, осуществляет множественные запросы основываясь на данных косвенной адресации (смещение, размер).

Эта функция реализована на языке C и доступна в PHP через Fast_IO специальное расширение.


Функция file_select_array перебирает массив адресов переданный в параметрах вызова. Извлекает из массива смещение offset и размер size. 
Далее происходит чтение указанного блока из файла, и добавление в результирующий массив.


Страница описания прокта [Fast_IO Engine](https://github.com/commeta/fast_io).


## Синтаксис

array file_select_array(string $filename, array $query[, string $pattern][, int $mode = 0])

### Параметры

- **filename** (string): Путь к файлу, из которого необходимо извлечь ключи.
- **query** - Массив: [[offset, size],]
- **pattern** - регулярное выражение, по которому будет осуществляться выборка.
- **mode** (int, optional) - Режим выборки.



#### Режимы

##### Выборка всех строк
- 0: Возвращает ассоциативный массив: trim_line, trim_length, line_offset, line_length.
- 1: Возвращает ассоциативный массив: line, line_offset, line_length.
- 2: Возвращает массив: trim_line.

##### Выборка строк - по совпадению с подстрокой
- 3: Возвращает ассоциативный массив: line_count, found_count.
- 5: Возвращает ассоциативный массив: trim_line, trim_length, line_offset, line_length.
- 6: Возвращает ассоциативный массив: line, line_offset, line_length.
- 7: Возвращает массив: trim_line.

В режиме 5 и 6 функция вернет только выборки в которых присутствует подстрока pattern. В этих режимах pattern это обычная подстрока.

##### Выборка строк - по совпадению с регулярным выражением [PCRE2](https://pcre2project.github.io/pcre2/doc/html/index.html) в каждой строке
- 10: Возвращает ассоциативный массив: trim_line, trim_length, line_offset, line_length, line_count.
- 11: Возвращает ассоциативный массив: line, line_offset, line_length, line_count.
- 12: Возвращает массив: trim_line.
- 13: Возвращает ассоциативный массив: line_count, found_count.

##### Выборка строк и совпадений - по совпадению с регулярным выражением PCRE2
- 20: Возвращает ассоциативный массив: trim_line, trim_length, line_matches, line_offset, line_length, line_count.
- 21: Возвращает ассоциативный массив: line, line_matches, line_offset, line_length, line_count.
- 22: Возвращает ассоциативный массив: line_matches, line_match, match_offset, match_length, line_offset, line_length, line_count.
- 23: Возвращает массив: line_matches, line_match.


В режимах 20+ функция вернет только выборки в которых присутствует совпадение по регулярному выражению pattern.


##### Log mode
- +100 Log mode: Если добавить +100 к любому из вышеперечисленных режимов, функция пересчитает режим mode - 100 но не будет блокировать файл.

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


Вслучае, если произошла ошибка (например, файл не может быть прочитан), функция возвращает пустой массив или NULL.


## Особенности работы

При использовании функции чтения файла, будет использоваться внутренний буфер стандартной библиотеки ввода-вывода (stdio). 
Этот внутренний буфер называется "подстрочным буфером" и обычно применяется для оптимизации операций чтения и записи данных.

Когда вы вызываете функцию для чтения данных из файла, стандартная библиотека ввода-вывода может использовать внутренний буфер для кэширования прочитанных данных. 
Это позволяет сократить количество системных вызовов к файловой системе для операций чтения и улучшить производительность приложения.

Подстрочный буфер будет автоматически заполняться, когда данные считываются из файла, и автоматически сбрасываться при достижении определенного размера буфера или при вызове функции fflush. 
Также буфер будет сбрасываться при закрытии файла.

По умолчанию стандартная библиотека ввода-вывода обеспечивает автоматическую буферизацию для повышения производительности ввода-вывода.



## Примеры использования

```
for($i=0; $i <=500; $i++){
	print_r(
		file_insert_line(__DIR__ . '/fast_io1.dat', 'index_' . $i . ' file_insert_line_' . $i . ' ' . str_pad('', 92, '1234567890'), 0, 8192)
	);
}

$array=[
	[8192, 8192], // Адрес и размер строки 1 в файле
	[16384, 8192], // Адрес и размер строки 2 в файле
	[24576, 8192] // Адрес и размер строки 3 в файле
];
```

```
print_r(
	file_select_array(__DIR__ . '/fast_io1.dat', $array, 'index')
);

Array
(
    [0] => Array
        (
            [trim_line] => index_1 file_insert_line_1 12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012
            [trim_length] => 119
            [line_offset] => 8192
            [line_length] => 8192
        )

    [1] => Array
        (
            [trim_line] => index_2 file_insert_line_2 12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012
            [trim_length] => 119
            [line_offset] => 16384
            [line_length] => 8192
        )

    [2] => Array
        (
            [trim_line] => file_update_line mode 0
            [trim_length] => 23
            [line_offset] => 24576
            [line_length] => 8192
        )

)
```



```
print_r(
	file_select_array(__DIR__ . '/fast_io1.dat', $array, '\\w+_\\d+', 10)
);

Array
(
    [0] => Array
        (
            [trim_line] => index_1 file_insert_line_1 12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012
            [trim_length] => 119
            [line_offset] => 8192
            [line_length] => 8192
        )

    [1] => Array
        (
            [trim_line] => index_2 file_insert_line_2 12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012
            [trim_length] => 119
            [line_offset] => 16384
            [line_length] => 8192
        )

)
```



```
print_r([
	file_select_array(__DIR__ . '/fast_io1.dat', $array, 'index', 3),
	file_select_array(__DIR__ . '/fast_io1.dat', $array, '\\w+_\\d+', 13)
]);

Array
(
    [0] => Array
        (
            [line_count] => 3
            [found_count] => 3
        )

    [1] => Array
        (
            [line_count] => 3
            [found_count] => 3
        )

)
```


```
print_r(
	file_select_array(__DIR__ . '/fast_io1.dat', $array, '\\w+_\\d+', 20)
);

Array
(
    [0] => Array
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
        )

    [1] => Array
        (
            [trim_line] => index_2 file_insert_line_2 12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012
            [trim_length] => 119
            [line_matches] => Array
                (
                    [0] => index_2
                    [1] => file_insert_line_2
                )

            [line_offset] => 16384
            [line_length] => 8192
        )

    [2] => Array
        (
            [trim_line] => index_3 file_insert_line_3 12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012
            [trim_length] => 119
            [line_matches] => Array
                (
                    [0] => index_3
                    [1] => file_insert_line_3
                )

            [line_offset] => 24576
            [line_length] => 8192
        )

)
```



```
print_r(
	file_select_array(__DIR__ . '/fast_io1.dat', $array, '\\w+_\\d+', 22)
);

Array
(
    [0] => Array
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
                            [match_offset] => 8
                            [match_length] => 18
                        )

                )

            [line_offset] => 8192
            [line_length] => 8192
        )

    [1] => Array
        (
            [line_matches] => Array
                (
                    [0] => Array
                        (
                            [line_match] => index_2
                            [match_offset] => 0
                            [match_length] => 7
                        )

                    [1] => Array
                        (
                            [line_match] => file_insert_line_2
                            [match_offset] => 8
                            [match_length] => 18
                        )

                )

            [line_offset] => 16384
            [line_length] => 8192
        )

)

```



```
print_r(
	file_select_array(__DIR__ . '/fast_io1.dat', $array, '\\w+_\\d+', 23)
);

Array
(
    [0] => Array
        (
            [0] => index_1
            [1] => file_insert_line_1
        )

    [1] => Array
        (
            [0] => index_2
            [1] => file_insert_line_2
        )

    [2] => Array
        (
            [0] => index_3
            [1] => file_insert_line_3
        )

)
```


## Стоимость вызова

- CPU Bound - Регулярные выражения PCRE2.
- IO Bound - Посекторное чтение файла в буфер.

Среднее потребление, при линейном чтении если файл не помещается в буфер - низкое, если файл или окно файла в буфере - сверх низкое.

