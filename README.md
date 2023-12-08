# File Scanner and Word Frequency Counter

## About
This project is a file scanner and word frequency counter implemented in C for a university course on operating systems. The program scans text files, counts the frequency of each word excluding specified stop words, and provides an interface for querying word frequencies.

The main functionality of the program is implemented in the `main.c` file, and the necessary declarations are provided in the `main.h` file. The program uses multithreading to concurrently scan multiple files for word frequencies, and a hash map is employed to efficiently store and retrieve word counts. Additionally, stop words can be defined to exclude them from the word count.

## Features
- Multithreaded Scanning: The program supports concurrent scanning of multiple files using pthreads.

- Word Frequency Counter: Words are counted using a hash map data structure for efficient retrieval.

- Stop Words: Certain words can be designated as stop words and are excluded from the word count.

- Dynamic File Monitoring: The program monitors files for changes and updates word frequencies accordingly.

## Installation
The project can be compiled using a standard C compiler. Ensure that the necessary dependencies, such as the pthread library, are available on your system.

Makefile is included so it can be compiled with the following command:

```
make
```

To run the program, execute the compiled binary with the desired command-line arguments.

```
./file_scanner stop_words.txt
```

Replace stop_words.txt with the path to a text file containing stop words.

## Command-Line Interface
The program provides a simple command-line interface for interacting with the file scanner. You can issue commands to scan files, stop the scanning process, and query word frequencies.

## Commands
### Scan File:

```
_count_ filename.txt
```
Initiates the scanning process for the specified file.

### Stop Scanning:

```
_stop_
```
Stops the scanning process and terminates the program.

### Query Word Frequency:

```
arduino
word
```

Retrieves the frequency of the specified word.

## File Structure

- `main.c`: Contains the main program logic, including file scanning, word counting, and user interface.
- `main.h`: Header file declaring data structures, function prototypes, and constants used in the program.
