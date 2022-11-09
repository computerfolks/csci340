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
