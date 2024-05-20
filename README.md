# Fast_IO Extension for PHP 8 (BETA)

[Описание на русском](README.RU.md)

## Overview

Fast_IO is a high-performance PHP 8 extension designed for efficient data file management, focusing on key-value pairs. It offers a suite of functions for reading, writing, and deleting data using low-level sector access and portable file locking to synchronize access across concurrent instances. This extension facilitates the storage and maintenance of arbitrarily large database files, with buffered functions reading files in chunks of 4096 bytes (1 OS kernel-cached page), ensuring high efficiency and performance.

## Features

- **High-Performance Data Handling**: Fast_IO is built for speed, allowing rapid manipulation of large data files.
- **Key-Value Pair Operations**: Comprehensive support for creating, reading, updating, and deleting key-value pairs.
- **Index Support**: Enhances search operations through the use of index files, making data retrieval fast and efficient.
- **Portable File Locking**: Ensures data integrity by using UNIX portable file locking, allowing safe concurrent access.
- **Buffered Reading**: Functions read data in chunks, preventing memory overflow and ensuring efficient data processing.
- **Framework-Free**: Developed without third-party frameworks or libraries for maximum performance and compatibility with PHP 8.
- **Binary-Safe Index Functions**: Functions with the data postfix ensure safe operation with binary data.

## Function List

- [file_callback_line](/docs/file_callback_line.md) - Line-by-line reading of the file with the callback function.
- [file_insert_line](/docs/file_insert_line.md): Inserting lines with alignment into the file.
- [file_pop_line](/docs/file_pop_line.md): Extracting and deleting the last line from the file.
- [file_defrag_data](/docs/file_defrag_data.md): Defragmenting the data file and its corresponding index file.
- [file_push_data](/docs/file_push_data.md): Adding a portion of binary data to the data file and its corresponding index file.
- [file_search_data](/docs/file_search_data.md): Searching for a portion of binary data by key, using the index file.
- [file_erase_line](/docs/file_erase_line.md): Erasing a line in the data file.
- [file_search_line](/docs/file_search_line.md): Searching for a line by key in the data file, returning the line.
- [file_search_array](/docs/file_search_array.md): Searching for lines by key in the data file, returning an array.
- [file_defrag_lines](/docs/file_defrag_lines.md): Removing lines from the data file.
- [file_get_keys](/docs/file_get_keys.md): Extracting unique keys from a text file.
- [file_replace_line](/docs/file_replace_line.md): Replacing a line by key.
- [file_select_line](/docs/file_select_line.md): Selecting a line from the file, based on a specified number or offset.
- [file_select_array](/docs/file_select_array.md) - Bulk selection of lines from a file, according to the specified offset and size.
- [file_update_line](/docs/file_update_line.md): Updating a line in the file.
- [file_update_array](/docs/file_update_array.md) - Massive updating of lines in the file.
- [file_analize](/docs/file_analize.md): - Returns statistics for data analysis and file diagnostics.
- [replicate_file](/docs/replicate_file.md): - Data file replication


## Implementation Highlights

- All functions utilize UNIX portable file locking on write operations to ensure data integrity during concurrent access [an algorithm for implementing a transaction using file locking](test/transaction/README.EN.md).
- Concurrent function instances wait in queue for file release, ensuring orderly access and operation.
- To prevent memory overflow, functions read data in portions without loading the entire file into memory.
- The extension is developed without reliance on external frameworks or libraries, ensuring high performance and compatibility with PHP 8.
- The cost of calling low-consumption functions in case of hitting the cache is comparable to the speed of cache memory access.


### Structure of the file
The line is divided into substrings: line_key, line_value.

The line_key can be any printable character, reserved symbols:
- ASCII Code 32 space (space) - substring delimiter.
- ASCII Code 127 delete (non-printable) - empty frame marker, placed at the beginning of a string filled with spaces.
- ASCII Code 10 line feed (enter) - line separator.
- ASCII Code 0 null character (null) - substring separator.

Functions: file_defrag_data, file_defrag_lines, file_erase_line, file_get_keys, file_push_data, file_replace_line, file_search_data - when indexing lines, they analyze the first substring, considering any printable character as the line key. The key must always end with a space.

### Support for ACID transactions

ACID is a set of rules that a database must follow to ensure the correctness of transaction execution.

The ACID acronym is explained as follows:
- Atomicity - each transaction is indivisible and is either completed or not performed at all.
- Consistency - a transaction should not violate the database integrity constraints.
- Isolation - transactions should not affect each other.
- Durability - after a successful transaction completion, all changes must be saved, even in case of failure.


Fast_IO uses a UNIX portable file lock mechanism using the flock function - for synchronizing access between parallel operations.

The flock function in Linux is designed to lock files at the kernel level of the operating system. This function provides the possibility for a process to establish a lock on a file that will prevent it from being changed by other processes until the current process removes this lock.

If a Fast_IO function tries to read or write to a file with an established flock LOCK_EX lock, the operation will wait for the release of the lock by parallel process.

The flock lock is in effect until the process that set it calls flock LOCK_UN to release the lock or closes the file. If a process terminates without releasing the lock, the lock is automatically released.

It is important to note that the flock lock itself is not transactional, i.e., it does not support ACID transactions. If the process that established the lock terminates with an error, data may remain inconsistent. Therefore, flock is usually used for implementing simple synchronization mechanisms rather than ensuring data integrity.


### Ensuring data integrity
- file_push_data, file_insert_line - always cancel the last record and exit with an error.
- file_replace_line, file_defrag_data, file_defrag_line - if there is an error during writeback, it renames temporary files and data remains intact. If a parallel copy of the Fast_IO function is waiting for the file lock to be released, it will fail with a lock error.
- file_erase_line - checks the number of written bytes; if there is an error writing a file (-3), this operation cannot be undone!
- file_update_line, file_update_array - checks the number of written bytes; if there is an error writing a file (-4), this operation cannot be undone!

An error when writing to the file_update_line or file_update_array function can occur when updating the last sectors of the file if there is no more space on the disk and the length of the string exceeds the file size.

An error when writing to the file_erase_line function can only indicate hardware failure.


### PCRE2 Regular Expressions

PCRE2 Version at the Time of Development: 10.42

In the Fast_IO Engine, the function find_matches_pcre2 is implemented, and in all file reading functions, the PCRE2 subsystem is initialized once during mass operations, which saves system resources for initialization and pattern compilation.

- [A brief overview of the best and fastest regular expression types in PHP8](/docs/find_matches_pcre2.md)
- [Perl-compatible Regular Expressions (revised API: PCRE2)](https://pcre2project.github.io/pcre2/doc/html/index.html)



## Getting Started

Each function within the Fast_IO extension is documented in detail on separate pages with PHP examples. These resources will help developers quickly get started with the extension and efficiently utilize its capabilities in their projects.

For more information on how to install, configure, and use Fast_IO in your PHP 8 environment, please refer to the detailed documentation provided with the extension.

## Parameters

buffer_size is a configuration parameter that determines the buffer size for read\write operations. Specifying the optimal buffer size can significantly improve performance when working with large volumes of data.

#### Setting via php.ini

To configure the buffer size in the global PHP configuration, add or modify the following line in the php.ini file:

fast_io.buffer_size = 4096 // From 16 bytes

The value is specified in bytes. By default, the buffer size is set to 4096 bytes (4 KB).

#### Using in PHP code

You can get the current buffer_size value or set a new value at the beginning of your PHP script:
```
// Get the current buffer size
$currentBufferSize = ini_get('fast_io.buffer_size');

// Set a new buffer size, before calling functions!
ini_set('fast_io.buffer_size', 8192); // 8 KB
```

#### Initialization

Initialization of parameters in PHP occurs during server startup or script execution. In the provided code example, initialization of the "fast_io.buffer_size" parameter is done during the initialization of the fast_io module in PHP.


### Notes

- Changing the buffer size during script execution can affect the performance of I/O operations performed after this change.
- Choosing the optimal buffer size depends on specific tasks and working conditions of the application. It is recommended to perform testing with different values to find the best option.
- Specify the buffer size based on the size of the data portion, the default value of 4096 is enough to work with strings of 4096 bytes.
- With a large buffer size, unnecessary file reads are possible, for example, during a full-text search, when the value can be found at the beginning of the file, and it will be read by the size of the buffer.
- At a very low buffer size, the number of read/write requests can significantly increase, which will create additional load.
- The buffer value must be a multiple of the 4096 byte cache memory page.
- In string search operations, a dynamic dynamic_buffer buffer is created based on the dynamic_buffer_size += buffer_size principle. The size of the dynamic buffer will be set to the value of the maximum spike in the line length.
- If the file size is less than buffer_size, then buffer_size is reduced to the file size.

dynamic_buffer is used to store parts of a file that are read into memory to search for a specific key or perform a mapping with a regular expression. dynamic_buffer is based on the following principles:

### How dynamic_buffer Works

1. **Initialization and Size**: The buffer is initialized with the initial size defined by the value fast_io.buffer_size.

2. **Reading from a File**: Data is read from the file in blocks of size fast_io.buffer_size and added to dynamic_buffer. In string search mode, if the string does not fit into the current buffer size, dynamic_buffer increases by the size of fast_io.buffer_size at each iteration of reading from the file.

3. **Size Increase**: dynamic_buffer size increases by ini_buffer_size. The increase occurs using the erealloc function, which tries to change the size of the already allocated memory, while preserving existing data.

### Using the system buffer

When you work with file operations, such as reading from a file using buffer functions, a system buffer is utilized. This buffer serves as an intermediary storage for data between the physical file on the disk and the computer's RAM. The system buffer allows for optimization of input/output by minimizing the number of disk accesses, which significantly enhances the performance of read and write operations.

During operation, the system buffer is used for the temporary storage of data read from a file before it is placed into the user buffer (dynamic_buffer). This enables efficient reading of data in large blocks, reducing the number of disk accesses.

System reading and writing functions use an internal buffer to optimize input/output operations. Even if you request reading or writing of just one byte, these functions usually read or write data in blocks.

When you request reading of 1 byte, the system function may read an entire block of data from the file and store it in the internal buffer. On subsequent calls, the data is read from this buffer, which reduces the number of requests to the operating system kernel and speeds up the reading process.

Similarly, when you request writing of 1 byte, the data is first placed in the internal buffer. When the buffer is full or when the file is closed, the data from the buffer is written to the file.

The size of a block (blocksize) depends on the system and can be different. In many systems, the block size is 4096 bytes (4 KB) or 8192 bytes (8 KB), but this is not a fixed value and may differ depending on the file system and the specific operating system.

### Advantages of using the System Buffer

1. Efficiency: Reading data in large blocks and their temporary storage in the system buffer reduces the load on the disk system and enhances the overall performance of input/output operations.

2. Reduced Latency: Minimizing disk accesses decreases delays associated with the mechanical characteristics of hard drives and the features of solid-state drives (SSD).

### Utilizing the Linux Kernel System Cache

When a function requests data from a file, the Linux kernel system cache plays a crucial role in optimizing performance when accessing disk data.

### How the Linux Kernel System Cache Works:

1. Reading Data: When a function attempts to read data from a file, the operating system first checks if these data are in the system cache (kernel page cache). If the data are already in the cache, they can be immediately provided to the process without needing to access the physical disk.

2. Caching Data: If the requested data are not in the cache, the kernel loads these data from the disk into the system cache before providing them to the process. In this process, data are loaded in blocks, which enhances the efficiency of subsequent accesses to the same data.

3. Writing Data: When writing data to a file, the data are first placed in the system cache, and only then, depending on the caching policy and system activity, they may be written to the physical medium. This reduces the number of write operations to the disk, positively affecting the disk's lifespan and system performance.

### Advantages of Using the System Cache:

- Reduced Latency: Accessing data from the cache is significantly faster than accessing data from the physical disk, which reduces the time it takes to perform read/write operations.
- Disk Work Optimization: Caching allows for reducing the number of disk accesses due to prefetching and delayed writing, which enhances overall system performance.
- Increased Throughput: The system cache enables handling more I/O operations per unit of time thanks to reducing the number of actual disk accesses.

Utilizing the Linux kernel system cache helps speed up the process of searching for values in a file, especially if the file is frequently used or its size exceeds the size of RAM. This makes data reading more efficient and reduces the overall execution time of the function.


### Read-Ahead

Read-ahead is a method used by operating systems and file systems to increase the performance of reading data from a disk. The OS reads more data from the disk in advance than was requested by the application, anticipating that this data will soon be needed. This reduces the number of disk accesses and increases the speed of data reading.

When reading data, read-ahead can be implicitly used through the mechanisms of the OS and file system when you make a request to read blocks of data from a file. The OS can preload data into the system buffer, accelerating access to subsequent blocks of data.

### Disk Write Optimization

Write-Back Caching: When writing data, it is first placed in a buffer (cache) in memory, rather than being immediately written to disk. Writing to the physical medium occurs later, at a more convenient time. This reduces the number of write operations to the disk, positively affecting both performance and the lifespan of the disk.


## Conclusion

Fast_IO represents a significant advancement in PHP data file management, offering unparalleled speed, efficiency, and reliability for handling large volumes of key-value pairs. Its comprehensive feature set makes it an ideal choice for developers seeking to optimize their data-driven applications.

---

## Installation

### Step 1: Creating the Extension Skeleton

Firstly, you need to prepare the skeleton of your extension. This can be achieved manually or by using the ext_skel tool available in the PHP source. Begin by copying config.m4, fast_io.c, and fast_io.h into your project directory. Then, execute the following commands:

```
phpize
./configure
make
make
```

These commands will create the basic structure for your Fast_IO extension.

### Step 2: Compilation and Testing

Next, compile and test your extension using phpize, ./configure, make, and make test. After successful compilation and testing, remember to add the line extension=fast_io.so to your php.ini file to activate the extension. Now, you can use the new functions provided by Fast_IO just like any other PHP function.

## Performance Test Results

The Fast_IO extension was rigorously tested on Ubuntu 24.04, with a Ryzen 12 Cores CPU, 16GB RAM, and a SATA 3 SSD. Here are the results for some of the key functions when executed in a loop of 10,000 iterations, with linear index incrementation (to avoid cache hits) and repeated searches for the same index:


**test/test.php**

```
file_insert_line: 0.10697793960571 (0.00001070)
file_search_line: 2.9135210514069 (0.00029135)
file_search_line repeat: 0.084739208221436 (0.00000847)
file_defrag_lines: 0.65570092201233 (0.00006557)
file_push_data: 0.21720695495605 (0.00002172)
file_search_data: 2.353935956955 (0.00023539)
file_search_data repeat: 0.15796780586243 (0.00001580)
file_pop_line: 0.24059700965881 (0.00002406)
```


**test/auto_test.php**


The test of functions is a complex check of their work with various input data and modes. Here is a brief description of the test with analysis:

Characteristics of the test:
- Purpose: Check the correctness of the function's operation under various conditions.
- Methodology: Using a cycle to repeatedly test the function with different parameters.
- Test parameters: Buffer size, number of line insertions, alignment, and function modes.

Test process:
- Deletion of an existing file before each test.
- Generation of random values for function parameters.
- Inserting lines into a file using the file_insert_line function.
- Checking the results of the function's operation in different modes.

Results:
- Success: If all checks are passed, the test is considered successful.
- Failure: Any discrepancy in the function's operation result leads to test failure.

Analysis of results:
- Test duration and input/output statistics are measured to assess performance and throughput.
- Results are output to the console indicating the test duration and passage status.

Convergence analysis:

The test checks the convergence of data extracted by the functions from the file with the data that was inserted into the file. Convergence means that the extracted data exactly matches the inserted data without errors.
For convergence analysis, various function modes are used, each of which checks certain aspects of the data (for example, line counter, line offset, line length, and the line itself).

Purpose of convergence analysis:

The purpose is to ensure that the functions reliably operate under various conditions and correctly process data, which is critical for the stability and reliability of the system using this function.

```
Check file_insert_line: time: 1.9269127845764 - PASS
rchar: 674020973 (349.79 millions per sec)
wchar: 174840700 (90.74 millions per sec)
syscr: 71455 (37,082.63 per sec)
syscw: 10495 (5,446.54 per sec)
read_bytes: 0 (0.00 millions per sec)
write_bytes: 175058944 (90.85 millions per sec)
cancelled_write_bytes: 168718336 (87.56 millions per sec)

Check file_analize: time: 4.6234958171844 - PASS
rchar: 7684914475 (1,662.14 millions per sec)
wchar: 211675884 (45.78 millions per sec)
syscr: 600218 (129,819.09 per sec)
syscw: 11690 (2,528.39 per sec)
read_bytes: 0 (0.00 millions per sec)
write_bytes: 211869696 (45.82 millions per sec)
cancelled_write_bytes: 216678400 (46.86 millions per sec)

Check file_get_keys: time: 2.1775228977203 - PASS
rchar: 1179394176 (541.62 millions per sec)
wchar: 196357512 (90.17 millions per sec)
syscr: 119811 (55,021.69 per sec)
syscw: 11056 (5,077.33 per sec)
read_bytes: 0 (0.00 millions per sec)
write_bytes: 196562944 (90.27 millions per sec)
cancelled_write_bytes: 194015232 (89.10 millions per sec)

Check file_search_array: time: 2.6541838645935 - PASS
rchar: 2158531044 (813.26 millions per sec)
wchar: 179663771 (67.69 millions per sec)
syscr: 211077 (79,526.14 per sec)
syscw: 10118 (3,812.09 per sec)
read_bytes: 0 (0.00 millions per sec)
write_bytes: 179859456 (67.76 millions per sec)
cancelled_write_bytes: 180285440 (67.93 millions per sec)

Check file_select_array: time: 2.4541449546814 - PASS
rchar: 2465994254 (1,004.83 millions per sec)
wchar: 164208502 (66.91 millions per sec)
syscr: 148473 (60,498.87 per sec)
syscw: 10405 (4,239.77 per sec)
read_bytes: 0 (0.00 millions per sec)
write_bytes: 164421632 (67.00 millions per sec)
cancelled_write_bytes: 161804288 (65.93 millions per sec)

Check file_search_line: time: 5.820433139801 - PASS
rchar: 12680275214 (2,178.58 millions per sec)
wchar: 177966239 (30.58 millions per sec)
syscr: 1344481 (230,993.29 per sec)
syscw: 11131 (1,912.40 per sec)
read_bytes: 0 (0.00 millions per sec)
write_bytes: 178176000 (30.61 millions per sec)
cancelled_write_bytes: 180559872 (31.02 millions per sec)

Check file_select_line: time: 1.6426420211792 - PASS
rchar: 702575103 (427.71 millions per sec)
wchar: 164970973 (100.43 millions per sec)
syscr: 79011 (48,099.95 per sec)
syscw: 10222 (6,222.90 per sec)
read_bytes: 0 (0.00 millions per sec)
write_bytes: 165183488 (100.56 millions per sec)
cancelled_write_bytes: 163766272 (99.70 millions per sec)

Check file_pop_line: time: 5.3802130222321 - PASS
rchar: 1289819041 (239.73 millions per sec)
wchar: 773095295 (143.69 millions per sec)
syscr: 165699 (30,797.85 per sec)
syscw: 46892 (8,715.64 per sec)
read_bytes: 0 (0.00 millions per sec)
write_bytes: 773881856 (143.84 millions per sec)
cancelled_write_bytes: 774709248 (143.99 millions per sec)

Check file_callback_line: time: 1.8096261024475 - PASS
rchar: 578088413 (319.45 millions per sec)
wchar: 153451752 (84.80 millions per sec)
syscr: 55345 (30,583.67 per sec)
syscw: 10048 (5,552.53 per sec)
read_bytes: 0 (0.00 millions per sec)
write_bytes: 153649152 (84.91 millions per sec)
cancelled_write_bytes: 154738688 (85.51 millions per sec)

```


What the test shows:

If the test passes, it shows that the function works correctly.
Test duration and input/output statistics allow you to assess the performance and overall throughput of the functions.
If the test fails, an error message is displayed, indicating problems in the function's operation.

Thus, the test not only checks the correctness of the function's operation but also provides information about its performance, which can be used for optimization and code improvement.

Conclusions:
The test shows that the functions work correctly under various conditions and settings.

For a deeper analysis and performance optimization, additional profiling and testing in various execution environments may be required. This will help identify potential bottlenecks and optimize the function's operation for different types of loads and data sizes.


For more information, see: [Auto database test](/test/auto_test.md)



## Call Costs

The table of function call costs in ascending order:

- file_select_line Very low consumption, sector-based reading of a file segment.
- file_update_line Very low consumption, sector-based writing of a file segment.
- file_insert_line Very low consumption, writing a line at the end of the file.
- file_select_array Average consumption, low when reading linearly or if the file window is in the buffer.
- file_update_array Average consumption, low when writing linearly or if the file window is in the buffer.
- file_pop_line Low consumption, with very low alignment, reading from the end of the file, truncating the file.
- file_push_data Low consumption, writing a line at the end of the index file and a block at the end of the data file.
- file_search_line Medium consumption, reading the entire file.
- file_search_array Medium consumption, reading the entire file.
- file_analize Medium consumption, reading the entire file.
- file_get_keys Medium consumption, reading the entire file.
- file_erase_line Medium consumption, reading the entire file, writing a line to the file.
- file_search_data Medium consumption, reading the entire index file and a block of the data file.
- file_defrag_lines Very high consumption, full reading/writing the entire file.
- file_replace_line Very high consumption, full reading/writing of the entire file.
- file_defrag_data Very high consumption, full reading/writing of index and data files.


## Function Overview

Each function within the Fast_IO extension has been meticulously documented on separate pages, complete with PHP examples to guide you through their usage. This ensures that you have all the necessary information to effectively utilize these functions in your projects. Here are some of the key features provided by Fast_IO:

- Efficient key-value pair manipulation
- Advanced indexing for quick data retrieval
- Portable file locking for data integrity
- Buffered reading for performance optimization

For detailed information about each function and how to use them, please refer to their respective documentation pages.


#### Examples

- [Tests](test/readme.md)
- [Example of an SQL adapter](test/mysql-adapter/readme.md)


## Getting Started

Now that you have installed the Fast_IO extension, you can begin optimizing your PHP applications for better performance in file operations. Whether you're managing large datasets or require fast and reliable data access, Fast_IO provides the tools you need to succeed.

We hope this guide has been helpful in setting up the Fast_IO extension for PHP 8. For further assistance or more detailed examples, please consult the individual function documentation pages.

Happy coding!


## Principles of LIFO Stack

A LIFO (Last In, First Out) stack is a data structure where the last added element is the first one to be removed. The main operations on a stack include:
- push: adding an element to the end of the stack.
- pop: removing and retrieving the last added element.

### Example of Using a LIFO Stack
```
// Adding a line to the file (simulating push)
file_insert_line('stack.data', "new line");

// Retrieving the last line from the file (pop)
$lastLine = file_pop_line('stack.data');
echo "Retrieved line: " . $lastLine;
```

## Applications of LIFO Stack
The LIFO stack is widely used in various fields:
- Algorithms: recursive algorithms, Reverse Polish Notation (RPN).
- Parsers: syntax analyzers for expressions.
- Memory management systems: managing the function call stack.
- Browser history: navigating back/forward.

### Inter-Process Communication (IPC)

For inter-process communication (IPC), a file can be used as a shared resource accessible by multiple processes. Each process can add lines to the file (push) or retrieve lines from the file (pop).

Inter-process communication (IPC) in the context of PHP and the file system represents a method of data exchange between processes running on one or multiple machines. In PHP, despite the lack of built-in IPC mechanisms, various approaches can be used to implement interaction between processes. One such approach is using the file system as an intermediary for data transfer.


### Key Concepts of IPC in PHP

1. **File System as a Shared Resource**: Files can serve as a shared resource for multiple processes. Processes can read from and write to files, allowing them to exchange information.

2. **Access Synchronization**: To prevent data races and ensure data integrity, access to files needs to be synchronized. This can be achieved using file locking.

3. **Buffering and Caching**: It is important to consider the levels of caching and buffering provided by the operating system to ensure correct and efficient interaction between processes.

### Linux Kernel Caching Levels

1. **I/O Buffering**: Linux uses buffering to reduce the number of direct I/O operations. Data is first written to memory buffers and then asynchronously written to disk.

2. **Page Cache**: This mechanism caches the contents of files in RAM to speed up access. When reading a file, data is first searched in the page cache, avoiding slow disk read operations.

3. **Filesystem Journaling**: Some file systems, like ext4, use journaling to ensure data integrity. Journaling helps restore the filesystem state after failures.

### Implementing a LIFO Stack and the file_pop_line Function

To implement a LIFO stack using the file system, a file can be used as a container for stack data. The file_pop_line function allows extracting the last line from the file and deleting it, corresponding to the pop operation in a LIFO stack.

#### Technological Aspects:

1. **Writing Data (Push)**:
   - A process adds a line to the end of the file.
   - The file_insert_line function is used, automatically adding a newline character.
   - File locking (flock) is used to prevent data races.

2. **Extracting Data (Pop)**:
   - The file_pop_line function opens the file and reads its contents.
   - The last line is extracted and removed from the file.
   - Buffering may be used for reading efficiency.
   - After extracting the line, the file is truncated by the size of the last line.
   - File locking can also be used to prevent simultaneous access.

3. **Synchronization and Atomicity**:
   - File locking (flock) ensures atomicity of write and read operations.
   - It is important to handle lock errors correctly and retry if necessary.

4. **Caching and Buffering**:
   - When writing data to a file, the operating system uses buffers for temporarily storing data before writing it to disk.
   - Reading data first occurs from the page cache, speeding up access.

### Advantages and Disadvantages of the Approach

Advantages:
- Simplicity of Implementation: Using the file system does not require additional libraries or extensions.
- Portability: The solution works on all UNIX-like systems that support PHP and the file system.

Disadvantages:
- Performance: Disk I/O operations can be slow compared to other IPC methods (e.g., sockets or shared memory).
- Synchronization Complexity: Careful handling of locks is required to prevent data races.

## Optimization at All Levels of Caching

### Linux Kernel-Level Caching
Using system calls allows efficient file operations by leveraging Linux kernel-level caching. This ensures high performance when working with files.

### Buffering
The function uses buffering to read the file in blocks, reducing the number of system calls and increasing performance. Buffering also helps minimize memory usage.

### Memory Optimization
Using dynamic memory allocation allows efficient memory management and avoids excessive resource consumption.

## Performance of the LIFO Stack Function

The file_pop_line function is optimized to work with files of any size. The use of buffering and efficient memory management ensures high-speed execution of operations.

## System Advantages
- Low Memory Consumption: Thanks to dynamic memory allocation.
- High Performance: Due to kernel-level caching and buffering.
- Flexibility: Ability to work with files of any size.
- Reliability: Error handling and protection against memory leaks.


## Application Scope of LIFO Stack for Implementing Data Chunk Multiplexing/Demultiplexing Algorithms Using file_pop_line

The LIFO (Last-In-First-Out) stack can be useful in various scenarios where data multiplexing and demultiplexing are required. Multiplexing means combining multiple data streams into one, while demultiplexing is the reverse process, splitting one data stream into several.

### Examples of Application:

1. **Real-time Log and Event Processing**:
   - **Multiplexing**: Different processes or services can write events to a common file. Each new log or event is added to the end of the file.
   - **Demultiplexing**: Multiple processes can simultaneously process these events by extracting them from the end of the file (using file_pop_line), allowing for real-time response to the latest events.

2. **Task Queue Systems**:
   - **Multiplexing**: Tasks from different sources are added to the end of a common queue file.
   - **Demultiplexing**: Multiple workers extract tasks from the end of the file and process them, distributing the load and speeding up task completion.

3. **Data Processing and ETL (Extract, Transform, Load) Systems**:
   - **Multiplexing**: Data from various sources (e.g., databases, files) is written to a common file for subsequent processing.
   - **Demultiplexing**: Multiple processes simultaneously extract data from the file to perform ETL operations such as cleaning, transforming, and loading data into the target system.

4. **Caching and Buffering Systems**:
   - **Multiplexing**: New data or computation results are added to the end of a cache file.
   - **Demultiplexing**: When reading data from the cache, processes extract the most recently added elements, which can be useful in scenarios where the latest data is more relevant.

5. **Monitoring and Alerting Systems**:
   - **Multiplexing**: Different monitoring agents write metrics and events to a common file.
   - **Demultiplexing**: Alert processing systems extract the latest metrics and events for analysis and generating notifications.

### Important Aspects:

1. **Synchronization of Access**:
   - To correctly work with the file, access synchronization using locking mechanisms (e.g., flock) is necessary to avoid concurrent access issues and potential errors.

2. **Atomicity of Operations**:
   - Reading and removing a line from the file should be atomic operations to prevent race conditions.

3. **Efficiency and Performance**:
   - Using multiple processes allows for load distribution and efficient utilization of system resources (CPU and I/O).

4. **Error Handling**:
   - Proper error handling should be implemented for possible issues when working with the file (e.g., read/write errors, locking problems).

Using a LIFO stack in such scenarios allows for efficient management of data flow, ensuring the relevance of processed information and load distribution among multiple processes.


## Example Implementation of a Worker Logic for Loading URLs

A software worker is a process that performs a specific task, such as processing data from a LIFO stack. In this case, the worker will extract URLs from the stack, download content from these URLs, and perform complex analysis or parsing of the content.

The example is inspired by the queue_address_manager function in the project [php cron requests events](https://github.com/commeta/php-cron-requests-events), file cron.php.


### Worker Logic:

1. Initialization: The worker starts up and prepares for operation.
2. Processing Loop:
   - The worker calls the file_pop_line function to extract the last URL from the file.
   - If a URL is successfully extracted, the worker downloads the content from that URL.
   - After downloading the content, the worker performs analysis or parsing of the content.
   - The worker repeats the cycle as long as there are URLs left in the file for processing.

### Cooperative Work of Multiple Parallel Processes with a LIFO Queue

#### Scenario:

1. Queue Creation Process:
   - One process (process_id: 1) creates a long queue of URLs for downloading.
   - The process adds many lines to the end of the file, each containing a URL.

2. Processing Processes:
   - Several processes (process_id: 2, 3, 4, 5) work in parallel.
   - Each process in a loop calls the file_pop_line function to extract the last element of the stack.
   - The process downloads the content from the extracted URL.
   - The process performs complex analysis or parsing of the content.
   - Thus, the work is distributed across multiple CPU cores (for CPU-BOUND tasks) or queued with long wait cycles for data download (IO-BOUND).


## /proc/locks in Linux

The /proc/locks file in Linux contains information about all file locks established in the system. This file is part of the virtual file system /proc, which provides an interface for interacting with the kernel and obtaining information about the system's state.

Each line in /proc/locks describes one lock and contains several fields separated by spaces. Fields on a sample line:

```
$db_file = __DIR__ . '/fast_io.dat';
file_insert_line($db_file, ' ');
file_callback_line(
	$db_file,
		function () {
			$locks = explode("\n", file_get_contents("/proc/locks"));
			$p_id = getmypid();

			foreach($locks as $lock){
				$records = explode(" ", $lock);
				if(isset($records[6]) && $records[6] == $p_id) {
					print_r($records);
				}
			}

			return false;
		}
);

Array
(
    [0] => 31:
    [1] => FLOCK
    [2] => 
    [3] => ADVISORY
    [4] => 
    [5] => WRITE
    [6] => 1542032
    [7] => fc:01:4760679
    [8] => 0
    [9] => EOF
)
```

### Records в /proc/locks

0. 31: Number of lock. This is a unique identifier for each lock.
1. FLOCK: Type of lock. Possible values:
   - FLOCK: Lock set using the system call flock.
   - POSIX: Lock set using the system call fcntl (POSIX-compatible).
   - LEASE: Means lease lock (lease lock).
2. ADVISORY: Lock mode. Possible values:
   - ADVISORY: Advisory lock (optional), which depends on coordination between processes.
   - MANDATORY: Mandatory lock, which is enforced on all I/O operations.
3. WRITE: Access type. Possible values:
   - READ: Lock for reading.
   - WRITE: Lock for writing.
4. 1542032: Process ID that set the lock.
5. fc:01:4760679: Identifier of the file that is locked. This identifier consists of three parts:
   - fc: Major device number.
   - 01: Minor device number.
   - 4760679: Inode number of the file.
6. 0: Start of the lock range (offset in bytes from the beginning of the file).
7. EOF: End of the lock range (EOF indicates the end of the file).


For a deeper understanding of file lock implementation in the Linux kernel, consider the source code from the locks.c file on GitHub at the link [fs/locks.c](https://github.com/torvalds/linux/blob/master/fs/locks.c).

