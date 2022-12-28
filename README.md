# csci340
project 1:

Design and implement a program that, when executed, has both a parent process and a child process. You use the fork()
system call to create a child process. The exec() system call is NOT to be used in this project. The parent and child utilize only two IPC
resources between them: one POSIX message queue and one ordinary pipe.  

Your program execution and command line arguments follows this syntax to provide counts for each input given on the command line:

./MYLASTNAME_MYSTUDENTID.exe [OPTION]... BUFFER_SIZE [INPUT FILE]...

The OPTIONS are the following. If none are specified on the command line then -ncb is used by default.

-n   prints the newline count for each input 
-c   prints the word counts for each input
-b   prints the character counts for each input
-m   prints the maximum line length for each input

BUFFER_SIZE is a number between 32 and 256 inclusively and must be specified. 

[INPUT FILE] contains a list of input files. If no input file is specified, then myinpfile.txt is used. 

You are NOT permitted to use any C standard I/O library functions nor string library functions.

The child process will read the input as given by [INPUT FILE]. The child will write the input file contents
in chunks as indicated by BUFFER_SIZE to the pipe. The parent will read this content from the pipe in chunks as indicated by
BUFFER_SIZE.

The parent process will compute the counts as indicated by [OPTION] for each input file separately from any other input file.

The parent process will send each count (as indicated by [OPTION]) for each input file as its own POSIX message into the POSIX message
queue. Therefore, as an example, if -ncbm is indicated with three input files on the command line, the parent process will send at least
4 * 3 = 12 messages into the POSIX message queue.

The child process will print the counts for each file to the screen.

project 2:
In your second project you will design and implement a POSIX pthreads multithreaded program that, when executed, has a main program
   thread, 16 worker child pthreads, and each of the 16 worker child pthreads has its own 3 worker computation child (that is, grandchild of 
   the main) pthreads.    

   Overall, you have 1 main program thread, 16 worker child pthreads, and 48 worker computation pthreads.

The input file has 16,000 numbers. Your main program thread will open and read the input file into a 16,000 element integer array 
   called InpArray. Each of the 16 worker child pthreads will be responsible for processing its own 1,000 number segment of this integer
   array as we saw in the lectures for data parallelism.

However, what is different in this project from the lectures is that each of the 16 worker child pthreads will make available the
   geometric average (the nth root of the product of n numbers), the arithmetic average, and the sum for its 1,000 number segment (of
   the InpArray) to the main program thread. Please note that the worker child pthreads do not actually do the computations themselves. 
   Instead, the 3 worker computation child  pthreads (grandchildren) will do the computations while the worker child pthread (the parent) 
   just simply waits for the computations to be completed.

   That is, the first worker computation child pthread determines the geometric average computation, the second worker compuutation child
   pthread does the arithmetic average, and the third worker computation child pthread does the sum computation.

   When each worker child pthread has received its 3 results from the worker computation child pthreads, the worker child pthread (parent)
   will make these results available to the main program thread using global array(s) and then terminate.

   After all of the 16 worker pthreads have terminated, the main program thread will compute and provide output for:
   a) the geometric average, arithmetic average, and sum computed by each worker child pthread
   b) The maximum of the 16 geometric average numbers
   c) The maximum of the 16 arithmetic average numbers
   d) The maximum of the 16 sum numbers
