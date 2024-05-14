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


### Behavior of the Fast_IO function in case of transaction abort
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
Check file_insert_line: time: 2.3249299526215 - PASS
rchar: 734991037 (316.13 millions per sec)
wchar: 197211103 (84.82 millions per sec)
syscr: 82689 (35,566.23 per sec)
syscw: 10742 (4,620.35 per sec)
read_bytes: 0 (0.00 millions per sec)
write_bytes: 197427200 (84.92 millions per sec)
cancelled_write_bytes: 196390912 (84.47 millions per sec)

Check file_analize: time: 6.8016469478607 - PASS
rchar: 7602825353 (1,117.79 millions per sec)
wchar: 215786744 (31.73 millions per sec)
syscr: 768615 (113,004.25 per sec)
syscw: 11951 (1,757.07 per sec)
read_bytes: 0 (0.00 millions per sec)
write_bytes: 216023040 (31.76 millions per sec)
cancelled_write_bytes: 216506368 (31.83 millions per sec)

Check file_get_keys: time: 1.927946805954 - PASS
rchar: 1077162792 (558.71 millions per sec)
wchar: 179332885 (93.02 millions per sec)
syscr: 92253 (47,850.39 per sec)
syscw: 10206 (5,293.71 per sec)
read_bytes: 0 (0.00 millions per sec)
write_bytes: 179548160 (93.13 millions per sec)
cancelled_write_bytes: 176144384 (91.36 millions per sec)

Check file_search_array: time: 2.6650650501251 - PASS
rchar: 2204118948 (827.04 millions per sec)
wchar: 183480195 (68.85 millions per sec)
syscr: 195569 (73,382.45 per sec)
syscw: 10288 (3,860.32 per sec)
read_bytes: 0 (0.00 millions per sec)
write_bytes: 183689216 (68.92 millions per sec)
cancelled_write_bytes: 182661120 (68.54 millions per sec)

Check file_select_array: time: 3.3443758487701 - PASS
rchar: 3093490454 (924.98 millions per sec)
wchar: 206018159 (61.60 millions per sec)
syscr: 182073 (54,441.55 per sec)
syscw: 12269 (3,668.55 per sec)
read_bytes: 0 (0.00 millions per sec)
write_bytes: 206213120 (61.66 millions per sec)
cancelled_write_bytes: 207093760 (61.92 millions per sec)

Check file_search_line: time: 7.2499101161957 - PASS
rchar: 16776156810 (2,313.98 millions per sec)
wchar: 216642676 (29.88 millions per sec)
syscr: 1770967 (244,274.34 per sec)
syscw: 11680 (1,611.05 per sec)
read_bytes: 0 (0.00 millions per sec)
write_bytes: 216854528 (29.91 millions per sec)
cancelled_write_bytes: 206196736 (28.44 millions per sec)

Check file_select_line: time: 1.8261711597443 - PASS
rchar: 785417171 (430.09 millions per sec)
wchar: 186901077 (102.35 millions per sec)
syscr: 85439 (46,785.87 per sec)
syscw: 10949 (5,995.60 per sec)
read_bytes: 0 (0.00 millions per sec)
write_bytes: 187133952 (102.47 millions per sec)
cancelled_write_bytes: 186793984 (102.29 millions per sec)

Check file_pop_line: time: 5.9825229644775 - PASS
rchar: 1425209869 (238.23 millions per sec)
wchar: 901270799 (150.65 millions per sec)
syscr: 154466 (25,819.54 per sec)
syscw: 46904 (7,840.17 per sec)
read_bytes: 0 (0.00 millions per sec)
write_bytes: 902180864 (150.80 millions per sec)
cancelled_write_bytes: 897216512 (149.97 millions per sec)

Check file_callback_line: time: 2.1984159946442 - PASS
rchar: 674857333 (306.97 millions per sec)
wchar: 185078722 (84.19 millions per sec)
syscr: 71290 (32,427.89 per sec)
syscw: 11074 (5,037.26 per sec)
read_bytes: 0 (0.00 millions per sec)
write_bytes: 185290752 (84.28 millions per sec)
cancelled_write_bytes: 186834944 (84.99 millions per sec)


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
