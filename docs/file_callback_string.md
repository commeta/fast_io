# Описание функции file_callback_string

Функция file_callback_string позволяет пользователю читать файл и обрабатывать его содержимое построчно, используя заданную callback-функцию.


### Синтаксис

array file_callback_string(string $filename, $callback[, int $mode = 0])


#### Параметры

- **$filename**: Путь к текстовому файлу, который необходимо проанализировать.
- **$callback** Callback-функция, которая будет вызвана для каждой строки файла.
- **$mode** (int, optional): Режим анализа.


#### Возвращаемое значение:
- В случае успеха возвращает последнее значение, возвращенное callback-функцией.
- В случае ошибки возвращает `FALSE`.

- Если callback-функция возвращает строку, эта строка сохраняется и возвращается в конце работы функции.
- Если callback-функция возвращает целое число (int), функция пытается переместиться на эту позицию в файле.
- Если callback-функция возвращает `TRUE`, чтение файла прекращается.
- Если callback-функция возвращает `FALSE`, чтение файла продолжается.


#### Описание работы функции:
1. Функция начинает работу с разбора параметров, переданных в неё.
2. Открывает указанный файл для чтения.
3. Если указана позиция, функция пытается переместиться на эту позицию в файле.
4. Читает файл построчно и вызывает callback-функцию для каждой строки.

Callback-функция в PHP, вызванная из расширения Fast_IO, работает в контексте глобального пространства имен PHP. 
Это означает, что ей доступны все глобальные переменные, определенные в скрипте PHP, в котором она вызывается. 
Однако область видимости переменных внутри самой callback-функции зависит от того, как эта функция определена в PHP-скрипте.

Когда вы вызываете callback-функцию с помощью call_user_function, PHP создает новый контекст исполнения для этого вызова. В этом контексте:

- Глобальные переменные: Callback-функция имеет доступ ко всем глобальным переменным, которые были определены в глобальной области видимости PHP-скрипта до момента вызова функции.
- Локальные переменные: Локальные переменные, определенные внутри callback-функции, будут доступны только внутри этой функции.
- Статические переменные: Если в callback-функции определены статические переменные, они сохранят свое значение между вызовами, но будут недоступны за пределами функции.
- Суперглобальные переменные: Суперглобальные переменные, такие как $_GET, $_POST, $_SESSION и другие, будут доступны внутри callback-функции, так как они доступны в любом месте скрипта.

Важно отметить, что если callback-функция определена как анонимная функция (замыкание), она может использовать переменные из родительской области видимости с помощью ключевого слова use. 
Это позволяет передавать в замыкание значения переменных, доступных в момент его создания.


#### Пример использования:
```
$db_file = __DIR__ . '/fast_io.dat';

for($i=0; $i <=1; $i++){
	$str = 'index_' . $i . ' file_insert_line_' . $i . ' ' . str_pad('', 8192, '1234567890_' . $i . '_');
	file_insert_line($db_file, $str, 2, 8192);
}

print_r([
	file_callback_string(
		$db_file,
		function () {
			$mode = func_num_args(); // Режим

			$line = ''; // Текущая строка в файле
			$line_offset = 0; // Смещение начала строки в файле
			$line_length = 0; // Длина строки в файле
			$line_count = 0; // Количество прочитанных строк 
			$position = 0; // Позиция начала поиска строк в файле
			$return_line = ''; // Строка для возврата из функции
			$current_size = 0; // Текущий размер порции данных в динамическом буфере
			$dynamic_buffer_size = 0; // Текущий размер динамического буфера
			$dynamic_buffer = ''; // Динамический буфер

			if ($mode > 0){
				$line  = func_get_arg(0);
				if($mode > 1) $line_offset = func_get_arg(1);
				if($mode > 2) $line_length = func_get_arg(2);
				if($mode > 3) $line_count = func_get_arg(3);
				if($mode > 4) $position = func_get_arg(4);
				if($mode > 5) $return_line = func_get_arg(5);
				if($mode > 6) $current_size = func_get_arg(6);
				if($mode > 7) $dynamic_buffer_size = func_get_arg(7);
				if($mode > 8) $dynamic_buffer = func_get_arg(8);
			}

			print_r([
				$mode,
				mb_strlen($line), 
				$line_offset,
				$line_length,
				$line_count,
				$position,
				mb_strlen($return_line),
				$current_size,
				$dynamic_buffer_size,
				mb_strlen($dynamic_buffer)
			]);


			//return true; // Закончить поиск
			//return false; // Продолжить поиск со следующей строки
			//return 0; // Переместится на начало файла
			
			return $return_line . $line_count . ', '; // Вернуть составную строку
		}, 9
	)
]);
```

Результат:
```
Array
(
    [0] => 9
    [1] => 8191
    [2] => 0
    [3] => 8192
    [4] => 0
    [5] => 0
    [6] => 0
    [7] => 8192
    [8] => 8192
    [9] => 8191
)
Array
(
    [0] => 9
    [1] => 8191
    [2] => 8192
    [3] => 8192
    [4] => 1
    [5] => 0
    [6] => 3
    [7] => 8192
    [8] => 8192
    [9] => 8191
)
Array
(
    [0] => 0, 1, 
)
```
