<?php
$data_file = __DIR__ . '/fast_io.dat';
$data_file_lock = $data_file . '.lock';
$align = 64; // line_number - длина 12 байт, 52 байта под данные.


if(file_exists($data_file_lock) && filesize($data_file_lock) > 0){ // Реализация IPC
	// Это условие сработает если параллельный процесс удерживает блокировку или вышел аварийно.
	$last_process_id = intval(file_get_contents($data_file_lock));
	$statFile = "/proc/$last_process_id/stat";

	if (file_exists($statFile)) {// Процесс с PID $last_process_id существует
		$statData = file_get_contents($statFile);
	
		// Разбиваем данные статистики ядра на массив
		$statArray = explode(" ", $statData);

		if (count($statArray) > 21) {// Получаем значения из массива
			$utime = intval($statArray[13]); // user time
			$stime = intval($statArray[14]); // system time
			$cutime = intval($statArray[15]); // user time дочерних процессов
			$cstime = intval($statArray[16]); // system time дочерних процессов
			$starttime = intval($statArray[21]); // время старта процесса
		
			// Вычисляем общее время
			$total_time = $utime + $stime;
			if ($cutime > 0 || $cstime > 0) {
				$total_time += $cutime + $cstime;
			}
		
			$avg = sys_getloadavg();
			file_put_contents(
				$data_file . 'race_condition.log',
				print_r([
					time(),
					$total_time,
					sys_getloadavg(),
					$statArray
				], true)
			);

			//if($avg[0] > 1 && $total_time > 20) exec("kill $last_process_id"); // Убить процесс
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

    
if(flock($lock, LOCK_EX)) { 
	// В этом месте функция ждет в очереди, пока параллельные процессы снимут блокировку

	// Реализация IPC
	ftruncate($lock, 0); // Усекаем файл
	fwrite($lock, strval(getmypid())); // Id запущенного процесса, для реализации IPC
	fflush($lock);

	// Данные с выравниванием
	$last_line_number = 0;
	if(file_exists($data_file) && filesize($data_file) > 0){
		$last_line_number = filesize($data_file) / ($align + 1);
	}

	$new_line_number = insert_key_value($data_file, 'insert_key_value_' . $last_line_number, $align); // Добавить строку в файл с выравниванием
	$str = select_key_value($data_file, $new_line_number, $align); // Получить строку из файла по номеру строки



	// Даннае без выравнивания
	$last_offset = 0;
	if(file_exists($data_file . '.dat') && filesize($data_file) > 0){
		$last_offset = filesize($data_file . '.dat');
	}

	$new_offset = write_key_value_pair($data_file . '.dat', "write_key_value_pair_" . $last_offset); // Добавить строку в файл без выравнивания
	$new_str = select_key_value($data_file . '.dat', $new_offset, mb_strlen($str), 1); // Получить строку из файла по смещению


	// Выход
	ftruncate($lock, 0);
	flock($lock, LOCK_UN); // Снимает блокировку
}

fclose($lock); // Тоже снимает блокировку  


print_r([$last_line_number, $new_line_number, $str]);
print_r([$last_offset, $new_offset, $new_str]);
