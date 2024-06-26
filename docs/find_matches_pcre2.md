
# Краткий обзор лучших и быстрых типов регулярных выражений в PHP8


В Fast_IO Engine Версия на момент разработки PCRE2 10.42
Страница описания прокта [Fast_IO Engine](https://github.com/commeta/fast_io)

В Fast_IO Engine реализована функция find_matches_pcre2, во всех функциях чтения файла подсистема PCRE2 инициализируется однократно при массовых операциях, это позволяет сэкономить системные ресурсы на инициализацию и компиляцию паттернов 
- [Стоимость вызова find_matches_pcre2](#стоимость-вызова)
- [Описание функции find_matches_pcre2](#описание-функции-find_matches_pcre2)




## Регулярные выражения
В PHP8 доступны различные типы регулярных выражений, каждый из которых имеет свои особенности и преимущества. Вот краткий обзор лучших и быстрых типов регулярных выражений в PHP8:

1.  **POSIX Regular Expressions** :
   - Преимущества:
     - Простота и понятность синтаксиса.
     - Поддержка большинства стандартных операций, таких как поиск подстроки, соответствие шаблону и т.д.
     - Хорошая поддержка в большинстве систем Unix/Linux.
   - Недостатки:
     - Ограниченная функциональность по сравнению с другими типами регулярных выражений.
     - Некоторые операции могут быть медленнее, чем в других типах.

2.  **Perl Compatible Regular Expressions (PCRE)** :
   - Преимущества:
     - Мощный и гибкий синтаксис, наследованный от Perl.
     - Поддержка обратной ссылки (backreference), захвата групп и других продвинутых возможностей.
     - Быстрый и эффективный движок регулярных выражений.
   - Недостатки:
     - Может быть сложнее для понимания новичками.
     - Требует больше ресурсов для выполнения сложных операций.

3.  **PCRE2 (PHP 8.1 и выше)** :
   - Преимущества:
     - Улучшенная производительность по сравнению с PCRE.
     - Поддержка UTF-8 и многобайтовых кодировок по умолчанию.
     - Возможность использования более сложных и оптимизированных алгоритмов.
   - Недостатки:
     - Не полностью совместим с PCRE, что может вызвать проблемы при миграции кода.
     - Может потребоваться переписывание некоторых регулярных выражений для работы с PCRE2.

 **Разница между POSIX и Perl Compatible Regular Expressions (PCRE)** :

-  **Синтаксис** : POSIX регулярные выражения имеют более простой и ограниченный синтаксис по сравнению с PCRE, который наследует синтаксис из Perl.
-  **Поддержка** : POSIX регулярные выражения хорошо поддерживаются в системах Unix/Linux, в то время как PCRE предоставляет более широкую поддержку и большую гибкость.
-  **Производительность** : PCRE обычно быстрее и эффективнее, особенно при выполнении сложных операций.
-  **Функциональность** : PCRE предлагает больше возможностей, таких как обратные ссылки, вложенные захваты и сложные условия.

 **Преимущества PCRE2** :

- Улучшенная производительность: PCRE2 оптимизирован для работы с большими объемами данных и сложными регулярными выражениями, что делает его быстрее, чем PCRE.
- Поддержка UTF-8: PCRE2 по умолчанию поддерживает UTF-8, что позволяет работать с многобайтовыми символами без необходимости дополнительных настроек.
- Совместимость: PCRE2 стремится быть совместимым с PCRE, что упрощает переход на новую версию.
- Безопасность: PCRE2 включает улучшения безопасности, такие как защита от некоторых видов атак.

Важно отметить, что выбор типа регулярного выражения зависит от конкретных требований вашего приложения и условий его использования.




## Описание функции find_matches_pcre2

#### Назначение
Функция find_matches_pcre2 предназначена для поиска всех совпадений в строке с использованием регулярного выражения, основанного на библиотеке [PCRE2](https://pcre2project.github.io/pcre2/doc/html/index.html). Это расширение PHP позволяет выполнить поиск по заданному шаблону и вернуть все найденные совпадения в виде массива.

#### Использование
mixed find_matches_pcre2(string $pattern, string $subject[, int $mode = 0])


- **$pattern** - регулярное выражение, по которому будет осуществляться поиск.
- **$subject** - строка, в которой будет производиться поиск.
- **$mode** - Режим работы, 0 - массив: matched, 1 - ассоциативный массив: line_match, match_offset, match_length.


#### Возвращаемые значения
Функция возвращает массив, содержащий все найденные совпадения. Если совпадений не найдено, возвращается пустой массив. В случае ошибки при компиляции регулярного выражения или другой ошибки выполнения функция возвращает FALSE.

#### Пример
```
for($i=0; $i <=500; $i++){
	print_r(
		file_insert_line(__DIR__ . '/fast_io1.dat', 'index_' . $i . ' file_insert_line_' . $i . ' ' . str_pad('', 92, '1234567890'), 8192) . ', '
	);
}

print_r([
	file_select_line(__DIR__ . '/fast_io1.dat', 0, 1391, 0),
	find_matches_pcre2('\\w+_', file_select_line(__DIR__ . '/fast_io1.dat', 0, 8192, 1), 0),
	find_matches_pcre2('\\w+_', file_select_line(__DIR__ . '/fast_io1.dat', 0, 8192, 1), 1),
]);

Array
(
    [0] => index_0 file_insert_line_0 12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012
    [1] => Array
        (
            [0] => index_
            [1] => file_insert_line_
        )
    [2] => Array
        (
            [0] => Array
                (
                    [line_match] => index_
                    [match_offset] => 0
                    [match_length] => 6
                )
            [1] => Array
                (
                    [line_match] => file_insert_line_
                    [match_offset] => 6
                    [match_length] => 17
                )
        )
)

```

#### Ошибки и предупреждения
Если произойдет ошибка при компиляции регулярного выражения, функция выведет предупреждение с описанием ошибки и позицией, на которой она произошла. При других ошибках выполнения также генерируются предупреждения.

#### Замечания
- Функция использует библиотеку PCRE2 для обработки регулярных выражений, что обеспечивает высокую производительность и совместимость с широким спектром регулярных выражений.



#### Стоимость вызова
Инициализация регулярного выражения в PHP происходит с использованием библиотеки PCRE2 (Perl Compatible Regular Expressions). 
В приведенном коде функции find_matches_pcre2 выполняются следующие шаги инициализации:

- Проверка параметров функции: в функцию передаются шаблон (pattern), строка для поиска (subject), а также дополнительный параметр mode, который по умолчанию равен 0.
- Компиляция шаблона: вызов функции pcre2_compile с параметрами pattern. Если компиляция успешна, то функция возвращает скомпилированное регулярное выражение, иначе устанавливаются код ошибки.
- Создание объекта для хранения результатов поиска: вызов функции pcre2_match_data_create_from_pattern. Функция возвращает объект match_data, который будет использоваться для хранения результатов поиска.

##### В процессе инициализации вызываются следующие функции:

- pcre2_compile: выполняет компиляцию шаблона регулярного выражения. Стоимость вызова функции зависит от сложности шаблона и может варьироваться от быстрого выполнения для простых шаблонов до более долгого выполнения для сложных шаблонов.
- pcre2_match_data_create_from_pattern: выделяет память для объекта match_data. Стоимость вызова функции зависит от размера объекта match_data и может быть незначительной.

Стоимость выполнения инициализации зависит от сложности шаблона и размера строки для поиска. В целом, инициализация регулярного выражения требует ресурсов процессора для компиляции шаблона и выделения памяти для объекта match_data.

##### В момент поиска в строке выполняются следующие шаги:

- Инициализация переменных для хранения результатов поиска: создается массив который будет содержать найденные совпадения.
- Поиск совпадений: выполняется цикл, в котором вызывается функция pcre2_match с параметрами subject и match_data. Если функция возвращает количество найденных совпадений  больше 0, то выполняется обработка результатов поиска. Если функция возвращает код ошибки или PCRE2_ERROR_NOMATCH, то выполняется обработка ошибки.
- Обработка результатов поиска: выполняется цикл, в котором обрабатываются результаты поиска.
- Обработка ошибок: если функция pcre2_match возвращает код ошибки, то выполняется обработка ошибки.
- Освобождение ресурсов: вызываются функции pcre2_match_data_free и pcre2_code_free для освобождения памяти, выделенной под объект match_data и скомпилированное регулярное выражение.

Стоимость выполнения поиска зависит от количества совпадений и сложности шаблона. В целом, поиск требует ресурсов процессора для выполнения функции pcre2_match и обработки результатов поиска.

Вызов функции pcre2_compile и pcre2_match требует ресурсов процессора для компиляции шаблона и выполнения поиска. Стоимость вызова этих функций зависит от сложности шаблона и размера строки для поиска. В целом, регулярные выражения являются достаточно ресурсоемкими, и их использование может привести к снижению производительности при работе с большими данными.
