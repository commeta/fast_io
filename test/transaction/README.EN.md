# Implementing Transaction Algorithm in PHP Using File Locking

PHP provides functions for working with files, including file locking. File locking allows preventing simultaneous access to a file by different processes, which is important for ensuring data integrity and preventing race conditions.

PHP supports portable file locking with flock(), which is based on the UNIX locking mechanism. This mechanism allows processes to lock files at the operating system level.

Low-level extension functions like [Fast_IO](../../README.md) operate on the same principle, and there is a similar implementation example in [php cron requests events](https://github.com/commeta/php-cron-requests-events).


## Код примера
```
<?php
$data_file = __DIR__ . '/fast_io.dat'; // The data file is blocked during the FAST_IO function call.
$data_file_lock = $data_file . '.lock'; // Creating a separate lock file.
$log_file = $data_file . '.race_condition.log';
$log_threshold = 120; //Do not add a log entry for 2 minutes


if(file_exists($data_file_lock) && filesize($data_file_lock) > 0){ // Реализация IPC, журнал
	// Это условие сработает если параллельный процесс удерживает блокировку или вышел аварийно.
	$last_process_id = intval(file_get_contents($data_file_lock));
	$statFile = "/proc/$last_process_id/stat";
	$avg = sys_getloadavg();
	$log_array = [];
	

	if (file_exists($statFile)) {// Процесс с PID $last_process_id существует
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
				$statArray
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

## Description

The code presented above uses file locking mechanisms in Unix-like operating systems to implement transactions between different processes. Let's break it down step by step:

1. File Definitions:
   - $data_file - This is the data file used to store information.
   - $data_file_lock - This is a temporary lock file used for synchronizing access to the data.
   - $log_file - This is the log file where logs are written for system state analysis.

2. Checking the existence of the lock file and its size: file_exists($data_file_lock) && filesize($data_file_lock) > 0
   - If the lock file exists and is not empty, it means another process already holds the lock.
   - Extract the process ID (PID) holding the lock from the lock file content.
   - Check the existence of the /proc/$last_process_id/stat file, which contains process state information.
   - If the process exists, extract its user and system CPU time and the time spent by child processes. These data can be used for system performance analysis.

3. Attempting to acquire a lock: flock($lock, LOCK_EX | LOCK_NB)
   - The fopen function opens the lock file in "c+" mode (read/write, create if necessary).
   - The flock function attempts to acquire an exclusive lock (LOCK_EX) with the LOCK_NB flag, indicating that if the lock cannot be acquired immediately, the function should return without waiting.
   - If the lock is acquired, the $is_locked variable is set to false, indicating that the current process is the first to attempt to acquire the lock.
   - If the lock is not acquired, the $is_locked variable is set to true, indicating that another process already holds the lock.

4. Acquiring a lock with waiting: flock($lock, LOCK_EX)
   - If the previous code block fails to acquire the lock, this block attempts to acquire an exclusive lock (LOCK_EX). In this case, the flock function will wait until another process releases the lock.
   - After acquiring the lock, the current process ID (getmypid()) is written to the lock file. This is done so that other processes can determine which process holds the lock.
   - The transaction body is then executed, which involves working with other files.
   - Before completing the transaction, the lock file is truncated (ftruncate($lock, 0)) to remove the process ID, and the lock is released (flock($lock, LOCK_UN)).

5. Closing the lock file: fclose($lock);
   - The fclose function automatically releases the file lock, even if it was set using flock.

Therefore, the code uses file locking mechanisms to ensure data integrity when multiple processes access data simultaneously. Race conditions can occur when two or more processes attempt to acquire a lock simultaneously. However, by using the LOCK_NB flag, one process can detect that the lock is already held and avoid a race condition.
