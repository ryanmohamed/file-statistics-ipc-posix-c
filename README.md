# File Statistics with Interprocess Communication in C

This program calculates various statistics for text files, including newline count, word count, character count, and maximum line length.

## Usage

To use the program, follow these steps:

1. Compile the program using a C compiler:

   ```shell
   gcc PosixMessageQueue.c -o file_stats
   ```
   
2. Run the program with the following command-line flags:

   ```shell
   ./file_stats [OPTIONS] [FILES]
   ```
   
   Replace `[OPTIONS]` with one or more of the following flags to specify the statistics to calculate:

   - `-l`: Calculate newline count.
   - `-w`: Calculate word count.
   - `-c`: Calculate character count.
   - `-L`: Calculate maximum line length.
   
3. The program will display the requested statistics for each file.

## Example

To calculate the newline count and word count for a file named "input.txt", run the following command:

   ```shell
   ./file_stats -l -w input.txt
   ```

The program will output the newline count and word count for the specified file.

## Notes

- The program supports processing multiple files simultaneously. Simply provide the file names as separate arguments.
- If no file names are provided, the program will use a default file.
- The program uses message queues for inter-process communication between the parent and child processes.
- The program relies on the POSIX message queue API and may require a POSIX-compliant operating system.

