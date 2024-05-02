# Implementing Transaction Algorithm in PHP Using File Locking

PHP provides functions for working with files, including file locking. File locking allows preventing simultaneous access to a file by different processes, which is important for ensuring data integrity and preventing race conditions.

PHP supports portable file locking with flock(), which is based on the UNIX locking mechanism. This mechanism allows processes to lock files at the operating system level.

Low-level extension functions like [Fast_IO](../../README.md) operate on the same principle, and there is a similar implementation example in [php cron requests events](https://github.com/commeta/php-cron-requests-events).


The product description page [Fast_IO Engine](https://github.com/commeta/fast_io).


## example.php
```
<?php
$data_file = __DIR__ . '/fast_io.dat'; // The data file is blocked during the FAST_IO function call.
$data_file_lock = $data_file . '.lock'; // Creating a separate lock file.
$log_file = $data_file . '.race_condition.log';
$log_threshold = 120; //Do not add a log entry for 2 minutes


if(file_exists($data_file_lock) && filesize($data_file_lock) > 0){ // IPC implementation, journal
	// This condition will work if the parallel process is holding a lock or has crashed.
	$last_process_id = intval(file_get_contents($data_file_lock));
	$statFile = "/proc/$last_process_id/stat";
	$avg = sys_getloadavg();
	$log_array = [];
	

	if ($last_process_id > 0 && file_exists($statFile)) {// A process with PID $last_process_id exists
		$statData = file_get_contents($statFile);
	
		// Splitting the core statistics data into an array
		$statArray = explode(" ", $statData);
		
		if (count($statArray) > 16) {// Getting the values from the array
			$utime = intval($statArray[13]); // user time
			$stime = intval($statArray[14]); // system time
			$cutime = intval($statArray[15]); // user time of child processes
			$cstime = intval($statArray[16]); // system time child processes
		
			// Calculate the total time
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
	// This condition will work without waiting for the lock to be released if a parallel process holds the lock
	$is_locked = false;
} else {
	$is_locked = true; // A sign that the lock is being held by a parallel process
}

    
if(flock($lock, LOCK_EX)) {// At this point, the function waits in the queue until parallel processes release the lock

	// IPC Implementation
	ftruncate($lock, 0); // Truncating the file
	fwrite($lock, strval(getmypid())); // Id of the running process, for IPC implementation
	fflush($lock);




	// The body of the transaction, there is a resource-intensive work with several files.



	// Exit
	ftruncate($lock, 0);
	flock($lock, LOCK_UN); // Removes the lock
}

fclose($lock); // Removes the lock too
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
