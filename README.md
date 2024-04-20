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
- **Binary-Safe Index Functions**: The index_ prefixed functions support unlimited data volume storage with binary safety.
- **Data File Storage**: Non-indexed functions store strings - key:value pairs (string, json, serialized) in data files with a maximum pair size of 4096 bytes.

## Function List

- [write_key_value_pair](/docs/write_key_value_pair.md): Writes a key-value pair to a text file.
- [pop_key_value_pair](/docs/pop_key_value_pair.md): Extracts and deletes the last line from a file.
- [rebuild_data_file](/docs/rebuild_data_file.md): Reconstructs a data file and its corresponding index file.
- [indexed_write_key_value_pair](/docs/indexed_write_key_value_pair.md): Writes a key-value pair to a text data file and its corresponding index file.
- [indexed_find_value_by_key](/docs/indexed_find_value_by_key.md): Searches for a value by key in a large text file using an index file.
- [hide_key_value_pair](/docs/hide_key_value_pair.md): Hides keys in files storing data in key-value format.
- [find_value_by_key](/docs/find_value_by_key.md): Searches for a value by key in a data file, return string.
- [find_array_by_key](/docs/find_array_by_key.md): Searches for a value by key in a data file, return array.
- [delete_key_value_pair](/docs/delete_key_value_pair.md): Deletes a key-value pair from a data file.
- [get_index_keys](/docs/get_index_keys.md): Extracts unique keys from a text file.
- [update_key_value_pair](/docs/update_key_value_pair.md): Updates a value by the given key.
- [insert_key_value](/docs/insert_key_value.md): Adds lines with specific alignment to a file.
- [select_key_value](/docs/select_key_value.md): Selects a line by the specified line number and alignment from a file.
- [update_key_value](/docs/update_key_value.md): Updates a value by key in the file.
- [detect_align_size](/docs/detect_align_size.md) - Returns the maximum length of a line in a file.

## Implementation Highlights

- All functions utilize UNIX portable file locking on write operations to ensure data integrity during concurrent access.
- Concurrent function instances wait in queue for file release, ensuring orderly access and operation.
- To prevent memory overflow, functions read data in portions without loading the entire file into memory.
- The extension is developed without reliance on external frameworks or libraries, ensuring high performance and compatibility with PHP 8.
- The cost of calling low-consumption functions in case of hitting the cache is comparable to the speed of cache memory access.

## Getting Started

Each function within the Fast_IO extension is documented in detail on separate pages with PHP examples. These resources will help developers quickly get started with the extension and efficiently utilize its capabilities in their projects.

For more information on how to install, configure, and use Fast_IO in your PHP 8 environment, please refer to the detailed documentation provided with the extension.

## Parameters

buffer_size is a configuration parameter that determines the buffer size for read and write operations. Specifying the optimal buffer size can significantly improve performance when working with large volumes of data.

#### Setting via php.ini

To configure the buffer size in the global PHP configuration, add or modify the following line in the php.ini file:

fast_io.buffer_size = 4096

The value is specified in bytes. By default, the buffer size is set to 4096 bytes (4 KB).

#### Using in PHP code

You can get the current buffer_size value or set a new value at the beginning of your PHP script:
```
// Get the current buffer size
$currentBufferSize = ini_get('fast_io.buffer_size');

// Set a new buffer size, before calling functions!
ini_set('fast_io.buffer_size', 8192); // 8 KB
```

### Notes

- Changing the buffer size during script execution can affect the performance of I/O operations performed after this change.
- Choosing the optimal buffer size depends on specific tasks and working conditions of the application. It is recommended to perform testing with different values to find the best option.
- Specify the buffer size based on the size of the data chunk; the default value of 4096 is sufficient for working with strings not larger than 4096 bytes.
- With a large buffer size, unnecessary file reads are possible, for example, during a full-text search, when the value can be found at the beginning of the file, and it will be read by the size of the buffer.
- The buffer value must be a multiple of the 4096 byte cache memory page.
- In string search operations, a dynamic_buffer is created.

dynamic_buffer is used to store parts of a file that are read into memory to search for a specific key or perform a mapping with a regular expression. dynamic_buffer is based on the following principles:

### How dynamic_buffer Works

1. **Initialization and Size**: The buffer is initialized with the initial size defined by the value fast_io.buffer_size.

2. **Reading from a File**: Data is read from the file in blocks of size fast_io.buffer_size and added to dynamic_buffer. In string search mode, if the string does not fit into the current buffer size, dynamic_buffer increases by the size of fast_io.buffer_size at each iteration of reading from the file.

3. **Size Increase**: dynamic_buffer size increases by ini_buffer_size. The increase occurs using the erealloc function, which tries to change the size of the already allocated memory, while preserving existing data.

### Using the System Buffer

When you work with file operations, such as reading from a file using buffer functions, a system buffer is utilized. This buffer serves as an intermediary storage for data between the physical file on the disk and the computer's RAM. The system buffer allows for optimization of input/output by minimizing the number of disk accesses, which significantly enhances the performance of read and write operations.

During operation, the system buffer is used for the temporary storage of data read from a file before it is placed into the user buffer (dynamic_buffer). This enables efficient reading of data in large blocks, reducing the number of disk accesses.

### Advantages of Using the System Buffer

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

```
write_key_value_pair: 0.11487889289856 (0.00001149)
find_value_by_key: 1.1644461154938 (0.00011644)
find_value_by_key repeat: 0.059406042098999 (0.00000594)
delete_key_value_pair: 10.655761003494 (0.00106558)
indexed_write_key_value_pair: 0.21924114227295 (0.00002192)
indexed_find_value_by_key: 0.91544914245605 (0.00009154)
indexed_find_value_by_key repeat: 0.11506295204163 (0.00001151)
pop_key_value_pair: 0.20079398155212 (0.00002008)
```

## Call Costs

The table of function call costs in ascending order:

- select_key_value Very low consumption, sector-based reading of a file segment.
- update_key_value Very low consumption, sector-based writing of a file segment.
- pop_key_value_pair Low consumption, with very low alignment, reading from the end of the file, truncating the file.
- write_key_value_pair Very low consumption, writing a line at the end of the file.
- insert_key_value Very low consumption, writing a line at the end of the file.
- indexed_write_key_value_pair Low consumption, writing a line at the end of the index file and a block at the end of the data file.
- find_value_by_key Medium consumption, reading the entire file.
- find_array_by_key Medium consumption, reading the entire file.
- detect_align_size Medium consumption, reading the entire file.
- get_index_keys Medium consumption, reading the entire file.
- hide_key_value_pair Medium consumption, reading the entire file, writing a line to the file.
- indexed_find_value_by_key Medium consumption, reading the entire index file and a block of the data file.
- delete_key_value_pair High consumption, full reading/writing the entire file.
- update_key_value_pair High consumption, full reading/writing of the entire file.
- rebuild_data_file Very high consumption, full reading/writing of index and data files.


## Function Overview

Each function within the Fast_IO extension has been meticulously documented on separate pages, complete with PHP examples to guide you through their usage. This ensures that you have all the necessary information to effectively utilize these functions in your projects. Here are some of the key features provided by Fast_IO:

- Efficient key-value pair manipulation
- Advanced indexing for quick data retrieval
- Portable file locking for data integrity
- Buffered reading for performance optimization

For detailed information about each function and how to use them, please refer to their respective documentation pages.

## Getting Started

Now that you have installed the Fast_IO extension, you can begin optimizing your PHP applications for better performance in file operations. Whether you're managing large datasets or require fast and reliable data access, Fast_IO provides the tools you need to succeed.

We hope this guide has been helpful in setting up the Fast_IO extension for PHP 8. For further assistance or more detailed examples, please consult the individual function documentation pages.

Happy coding!
