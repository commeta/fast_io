# Алгоритм реализации транзакции на PHP с помощью блокировки файла

PHP предоставляет функции для работы с файлами, включая их блокировку. Блокировка файлов позволяет предотвратить одновременный доступ к файлу со стороны разных процессов, что важно для обеспечения целостности данных и предотвращения состояния гонки.

PHP поддерживает портируемую блокировку файлов flock(), которая основана на механизме блокировок UNIX. Этот механизм позволяет процессам блокировать файлы на уровне операционной системы.

По такому же принципу работают низкоуровневые функции расширения [Fast_IO](../../README.md), есть похожий пример реализации [php cron requests events](https://github.com/commeta/php-cron-requests-events)


Страница описания прокта [Fast_IO Engine](https://github.com/commeta/fast_io).


## Код примера example.php
```
<?php
$data_file = __DIR__ . '/fast_io.dat'; // Файл данных блокируется во время вызова FAST_IO функций.
$data_file_lock = $data_file . '.lock'; // Создаем отдельный файл блокировки.
$log_file = $data_file . '.race_condition.log';
$log_threshold = 120; // Не добавлять запись в журнал 2 минуты


if(file_exists($data_file_lock) && filesize($data_file_lock) > 0){ // Реализация IPC, журнал
	// Это условие сработает если параллельный процесс удерживает блокировку или вышел аварийно.
	$last_process_id = intval(file_get_contents($data_file_lock));
	$statFile = "/proc/$last_process_id/stat";
	$avg = sys_getloadavg();
	$log_array = [];
	

	if ($last_process_id > 0 && file_exists($statFile)) {// Процесс с PID $last_process_id существует
		$statData = file_get_contents($statFile);
	
		// Разбиваем данные статистики ядра на массив
		$statArray = explode(" ", $statData);
		
		if (count($statArray) > 16) {// Получаем значения из массива
			$utime = intval($statArray[13]); // user time
			$stime = intval($statArray[14]); // system time
			$cutime = intval($statArray[15]); // user time дочерних процессов
			$cstime = intval($statArray[16]); // system time дочерних процессов
		
			// Вычисляем общее время
			$total_time = $utime + $stime;
			if ($cutime > 0 || $cstime > 0) {
				$total_time += $cutime + $cstime;
			}
			
			$log_array[] = [
				time(),
				$total_time,
				$avg,
				$statArray,
				$last_process_id
			];

			if(filectime($log_file) + $log_threshold  < time()) {
				file_put_contents($log_file, print_r($log_array, true), FILE_APPEND);
			}
		}
	}
}


$lock= fopen($data_file_lock, "c+");

if(flock($lock, LOCK_EX | LOCK_NB)) { 
	// Это условие сработает без ожидания снятия блокировки, если параллельный процесс удерживает блокировку 
	$is_locked = false;
} else {
	$is_locked = true; // Признак удержания блокировки параллельным процессом
}

    
if(flock($lock, LOCK_EX)) {// В этом месте функция ждет в очереди, пока параллельные процессы снимут блокировку

	// Реализация IPC
	ftruncate($lock, 0); // Усекаем файл
	fwrite($lock, strval(getmypid())); // Id запущенного процесса, для реализации IPC
	fflush($lock);




	// Тело транзакции, здесь идет ресурсоемкая работа с несколькими файлами.



	// Выход
	ftruncate($lock, 0);
	flock($lock, LOCK_UN); // Снимает блокировку
}

fclose($lock); // Тоже снимает блокировку  
```

## Описание

Код, представленный выше, использует механизмы блокировки файлов в Unix-подобных операционных системах для реализации транзакций между различными процессами. Давайте разберем его по шагам:

1. Определение файлов:
   - $data_file - это файл данных, который будет использоваться для хранения информации.
   - $data_file_lock - это временный файл блокировки, который используется для синхронизации доступа к данным.
   - $log_file - это файл журнала, в который записываются логи для анализа состояния системы.

2. Проверка существования файла блокировки и его размера: file_exists($data_file_lock) && filesize($data_file_lock) > 0
   - Если файл блокировки существует и он не пустой, значит другой процесс уже удерживает блокировку.
   - Извлекается идентификатор процесса (PID), который удерживает блокировку, из содержимого файла блокировки.
   - Проверяется существование файла /proc/$last_process_id/stat, который содержит информацию о состоянии процесса.
   - Если процесс существует, то извлекаются его пользовательское и системное время работы, а также время работы дочерних процессов. Эти данные могут быть использованы для анализа производительности системы.

3. Попытка получения блокировки: flock($lock, LOCK_EX | LOCK_NB)
   - Функция fopen открывает файл блокировки в режиме "c+" (чтение/запись, создание при необходимости).
   - Функция flock пытается получить эксклюзивную блокировку (LOCK_EX) с флагом LOCK_NB, который указывает на то, что если блокировка не может быть получена немедленно, функция должна вернуть управление без ожидания.
   - Если блокировка получена, переменная $is_locked устанавливается в false, что означает, что текущий процесс является первым, кто попытался получить блокировку.
   - Если блокировка не получена, переменная $is_locked устанавливается в true, что означает, что другой процесс уже удерживает блокировку.

4. Получение блокировки с ожиданием: flock($lock, LOCK_EX)
   - Если предыдущий блок кода не смог получить блокировку, этот блок кода пытается получить эксклюзивную блокировку (LOCK_EX). В этом случае функция flock будет ждать, пока другой процесс не снимет блокировку.
   - После получения блокировки, в файл блокировки записывается идентификатор текущего процесса (getmypid()). Это делается для того, чтобы другие процессы могли определить, какой процесс удерживает блокировку.
   - Затем выполняется тело транзакции, которое включает в себя работу с другими файлами.
   - Перед завершением транзакции, файл блокировки очищается (ftruncate($lock, 0)), чтобы удалить идентификатор процесса, и блокировка снимается (flock($lock, LOCK_UN)).

5. Закрытие файла блокировки: fclose($lock);
   - Функция fclose автоматически снимает блокировку файла, даже если она была установлена функцией flock.

Таким образом, код использует механизмы блокировки файлов для обеспечения целостности данных при одновременном доступе нескольких процессов. Возможность гонки возникает, когда два или более процессов пытаются одновременно получить блокировку. Однако, благодаря использованию флага LOCK_NB, один из процессов может обнаружить, что блокировка уже занята, и избежать состояния гонки.
