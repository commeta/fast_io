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

			if(filectime($log_file) + $log_threshold < time()) {
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

    
if(flock($lock, LOCK_EX)) { // В этом месте функция ждет в очереди, пока параллельные процессы снимут блокировку

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
