# fast_io
PHP extension, fast DB with indexes

Интеграция C-функций в модуль PHP через API Zend. 
Наши функции deleteKeyValuePair, writeKeyValuePair и findValueByKey

## Установка

### Шаг 1: Создание каркаса расширения

Для начала, вам нужно создать каркас вашего расширения. Это можно сделать вручную или с помощью инструмента ext_skel в исходниках PHP. Например:

phpize
./configure
make
make test


Это создаст базовую структуру для вашего расширения.


### Шаг 2: Компиляция и тестирование

Вам нужно скомпилировать расширение и протестировать его. Используйте phpize, ./configure, make и make install для компиляции и установки вашего расширения. После установки не забудьте добавить строку extension=fast_io.so в ваш php.ini, чтобы активировать расширение.

Теперь вы можете вызывать findValueByKey, writeKeyValuePair и deleteKeyValuePair из PHP как обычные функции.
