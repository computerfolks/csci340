// Jacob Weissman 23907251

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <time.h>
#include <errno.h>
#include <sys/utsname.h>
#include <sys/wait.h>

const int MSG_SIZE = 100;
//capacity must be 10 or under
const int MSG_CAPACITY = 10;

int main(int argc, const char *argv[]){

  //check if only one argument is provided, in which case we return error
  if (argc==1){
    write(STDOUT_FILENO, "only one arg", 12);
    return 1;
  }

  //CREATE PIPE
  pid_t pid;
  int fd[2];
  if (pipe(fd) < 0) {
    write(STDOUT_FILENO, "pipe failed", 11);
    return 2;
  }

  //CREATE POSIX MESSAGE QUEUE
  struct mq_attr attr;
  attr.mq_maxmsg = MSG_CAPACITY;
  attr.mq_msgsize = MSG_SIZE;
  //name is arbitrary, but it must begin with /
  const char * mqname = "/msgq1weissman";
  mqd_t mqd = mq_open(mqname,  O_CREAT | O_RDWR, 0644, &attr);
  if (mqd == (mqd_t) -1) {
    write(STDOUT_FILENO, "message queue creation error", 29);
    mq_unlink(mqname);
    return 3;
  }

  //DECLARE VARIABLES
  //let defaultfile reperesent whether or not the user has specified a file
  // 0 means no file specified
  int defaultfile = 1;

  //argc represents the number of arguments. the first argument will be the executable
  //let custom represent whether or not the user has specified custom options
  // 1 means custom options specified
  int custom = 1;

  //create variables to keep track of each option: n, c, b, m
  //let 1 represent the option is selected, 0 the option is not selected
  int n = 0;
  int c = 0;
  int b = 0;
  int m = 0;

  //keep track of the number of files specified by the user
  int number_of_files;

  //create variable to keep track of buffer size specified by user in command line
  int buffer_size;

  //COLLECT COMMAND LINE ARGUMENTS
  //check if only two arguments provided, in which case arguments are executable [0] and buffer size [1]
  //in this case, we need to use default options -ncb and default file
  if (argc == 2){
    //set default options
    n = 1;
    c = 1;
    b = 1;
    m = 0;
    custom = 0;
    //set buffer size
    buffer_size = atoi(argv[1]);
    //ensure user has selected a valid buffer size
    if (buffer_size < 32 || buffer_size > 256){
      write(STDOUT_FILENO, "illegal buffer size", 19);
      return 4;
    }
    defaultfile = 0;
    number_of_files = 1;
  }

  //check if more than two arguments provided
  if (argc > 2){
    //if custom options are selected, they all must be contained in the second argument
    //examine which options are selected and save
    for (int i = 0; argv[1][i]; i++) {
      if (argv[1][i]=='n'){
        n = 1;
      } else if (argv[1][i]=='c'){
        c = 1;
      } else if (argv[1][i]=='b'){
        b = 1;
      } else if (argv[1][i]=='m'){
        m = 1;
      }
      //if the second argument contains a number,  no options have been provided (because the second argument is the buffer size), and should instead use default options
      else if (argv[1][i] == '0' || argv[1][i] == '1' || argv[1][i] == '2' || argv[1][i] == '3' || argv[1][i] == '4' || argv[1][i] == '5' || argv[1][i] == '6' || argv[1][i] == '7' || argv[1][i] == '8' || argv[1][i] == '9') {
        //user has not selected custom options because second argument is buffer size
        //set default options
        n = 1;
        c = 1;
        b = 1;
        m = 0;
        //set custom to 0. if not set to 0 in this step, custom will equal 1 in the next step
        custom = 0;

        //set buffer size
        buffer_size = atoi(argv[1]);
        if (buffer_size < 32 || buffer_size > 256){
          write(STDOUT_FILENO, "illegal buffer size", 19);
          return 5;
        }
        //exit loop
        break;
      }
    }

    //if more than two arguments and custom options were selected
    if (custom == 1) {
      //set the buffer, which will be third argument
      buffer_size = atoi(argv[2]);
      if (buffer_size < 32 || buffer_size > 256){
          write(STDOUT_FILENO, "illegal buffer size", 19);
          return 6;
        }
    }

    //if the user has provided only three arguments, and the user has also provided custom options, use default file
    if (argc == 3 && custom == 1){
      number_of_files = 1;
      defaultfile = 0;
    } else {
      //in all other cases with at least three arguments, we have to create a file descriptor for each file argument the user has provided
      //the size will be equal to the number of arguments, minus one each for the executable file and buffer size, minus 1 if the user provided custom options
      number_of_files = argc - 2 - custom;
    }
  }
  //use array int f[] for file descriptors
  int f[number_of_files];
  //store names of files
  const char * file_names[number_of_files];
  //store size of each file name in characters
  int file_names_size[number_of_files];

  //CREATE FILE DESCRIPTORS
  //if file has not been specified, use default
  if (defaultfile == 0){
    //open file myinpfile.txt
    if ((f[0] = open("myinpfile.txt", O_RDONLY, 0)) == -1){
      write(STDOUT_FILENO, "can't open myinpfile.txt", 25);
      return 7;
    }
    //save myinpfile.txt as file name
    file_names[0] = "myinpfile.txt";
    //save number of characters that myinpfile.txt has
    file_names_size[0] = 13;
  } else {
    //open all files user has specified using a loop
    for (int i = 0; i < number_of_files; i++) {
      int fsize = 0;
      for (int j = 0; argv[i+ 2 + custom][j]; j++){
        //read each command line argument and count number of characters
        fsize++;
      }
      if ((f[i] = open(argv[i + 2 + custom], O_RDONLY, 0)) == -1){
        write(STDOUT_FILENO, "can't open user file ", 22);
        write(STDOUT_FILENO, argv[i + 2 + custom], fsize);
        return 8;
      } else {
        //save file name
        file_names[i] = argv[i + 2 + custom];
        //save file name size
        file_names_size[i] = fsize;
      }
    }
  }

  //FORK
  pid = fork();

  if (pid < 0) {
    write(STDOUT_FILENO, "fork fail", 9);
    return 9;
  }

  //CHILD PROCESS
  else if (pid == 0){
   //READ FROM FILE, WRITE TO PIPE
   close (fd[0]); // close read end of pipe
   //PART 2
   //get process id for child
   int child_proc_id = getpid();
   //convert child_proc_id integer to char array of known length
   //first determine length
   int pdigit = 0;
   for (int ptestnum = child_proc_id; ptestnum > 0; ptestnum = ptestnum/10){
     pdigit++;
   }
   //create char array
   char child_proc_id_char[pdigit];
   //fill the array
   int pnum = child_proc_id;
   for (int px = pdigit - 1; px >= 0; px--){
     child_proc_id_char[px] = pnum % 10 + '0';
     pnum = pnum / 10;
   }
   write(STDOUT_FILENO, "Process ID is:\t", 15);
   write(STDOUT_FILENO, child_proc_id_char, pdigit);
   write(STDOUT_FILENO, "\n", 1);
 
   //get child process id for child
   int child_parent_proc_id = getppid();
   //convert child_proc_id integer to char array of known length
   //first determine length
   pdigit = 0;
   for (int ptestnum = child_parent_proc_id; ptestnum > 0; ptestnum = ptestnum/10){
     pdigit++;
   }
   //create char array
   char child_parent_proc_id_char[pdigit];
   //fill the array
   pnum = child_parent_proc_id;
   for (int px = pdigit - 1; px >= 0; px--){
     child_parent_proc_id_char[px] = pnum % 10 + '0';
     pnum = pnum / 10;
   }
   write(STDOUT_FILENO, "Parent process ID is:\t", 22);
   write(STDOUT_FILENO, child_parent_proc_id_char, pdigit);
   write(STDOUT_FILENO, "\n", 1);
 
   //print child current working directory
   //at the start, we do not know what size the current working directory will be
   char cwd[200];
   //fill it up with ` symbol
   for (int y = 0; y < 200; y++){
     cwd[y] = '`';
   }
   //get the working directory
   getcwd(cwd, 200);
 
   //now get rid of the junk end symbols
   //determine the size (number of char)
   int cwd_size = 0;
   for (int y = 0; y < 200; y++){
     //search until hitting the ` symbol, make sure several are hit to be safe
     if (y > 3 && cwd[y] == '`' && cwd[y - 1] == '`' && cwd[y - 2] == '`'){
       cwd_size = y - 2;
       break;
     }
   }
   //fill up the array to print
   char cwd_print[cwd_size];
   for (int y = 0; y < cwd_size; y++){
     cwd_print[y] = cwd[y];
   }
   write(STDOUT_FILENO, "Process current working directory is:\t", 38);
   write(STDOUT_FILENO, cwd_print, cwd_size);
   write(STDOUT_FILENO, "\n", 1);
 
   //print hostname
   //at the start, we do not know what size the hostname will be
   char hostname[200];
   //fill it up with ` symbol
   for (int y = 0; y < 200; y++){
     hostname[y] = '`';
   }
   //get the hostname
   gethostname(hostname, 200);
 
   //now get rid of the junk end symbols
   //determine the size (number of char)
   int hostname_size = 0;
   for (int y = 0; y < 200; y++){
     //search until hitting the ` symbol, make sure several are hit to be safe
     if (y > 3 && hostname[y] == '`' && hostname[y - 1] == '`' && hostname[y - 2] == '`'){
       hostname_size = y - 2;
       break;
     }
   }
   //fill up the array to print
   char hostname_print[hostname_size];
   for (int y = 0; y < hostname_size; y++){
     hostname_print[y] = hostname[y];
   }
   write(STDOUT_FILENO, "Hostname is:\t", 13);
   write(STDOUT_FILENO, hostname_print, hostname_size);
   write(STDOUT_FILENO, "\n", 1);

    //PART 1 - READ FROM FILE, WRITE TO PIPE
    //char array for child process to read from the file
    char fileline[buffer_size];
    //declare variable to check how many characters were read from the file
    int mread;
    //for each file
    for (int child_file_index = 0; child_file_index < number_of_files; child_file_index++) {
      //read from the file
      while ((mread = read(f[child_file_index], fileline, buffer_size)) > 0) {
        close (fd[0]); // close read end of pipe
        //write to the pipe
        if (write(fd[1], fileline, mread) != mread) {
          write(STDOUT_FILENO, "cannot write file", 17);
          close(f[child_file_index]);
          return 10;
        }
      }
      //once the entire file has been read from the file and written to the pipe
      //put on the pipe a signal that the end of the file has been reached
      if (write(fd[1], "`~$~`", 5) != 5) {
          write(STDOUT_FILENO, "cannot write file", 17);
          return 11;
      }
    }
    close (fd[1]); // close write end of pipe

    //READ FROM POSIX MESSAGE QUEUE
    //declare variables
    //calculate number of expected messages
    int messages = (n + b + c + m) * number_of_files;
    char buf[MSG_SIZE];
    unsigned int prio;
    ssize_t numRead;
    //use variable to keep track of whether or not each of the custom options has been dealt with yet
    //this system takes advantage of the known order that the parent will send messages onto the pipe
    int nseen = 0;
    int cseen = 0;
    int bseen = 0;
    int mseen = 0;

    //loop through known number of expected messages
    for (int mnum = 0; mnum < messages; mnum++) {
      //block and receive message by using NULL
      numRead = mq_timedreceive(mqd, buf, attr.mq_msgsize, &prio, NULL);

      if (numRead == -1) 
        {
          write(STDOUT_FILENO, "child message queue read error", 31);
          return 12;
        }
      //write name of file
      if (number_of_files > 1) {
        //divide messsage number by n+b+c+m to get which file name in the array
        write(STDOUT_FILENO, file_names[mnum / (n + b + c + m)], file_names_size[mnum / (n + b + c + m)]);
      } else {
        write(STDOUT_FILENO, file_names[0], file_names_size[0]);
      }
      write(STDOUT_FILENO, ":", 1);
      //check which option to print
      if (nseen == 0 && n == 1) {
        //if newline count has been selected by the user and has not been printed yet for this file
        write(STDOUT_FILENO, "\tnewline count is:\t", 19);
        nseen = 1;
      } else if (cseen == 0 && c == 1){
        //if newline count is either not selected, or has already been printed, and word count has been selected by the user and has not been printed yet for this file
        write(STDOUT_FILENO, "\tword count is:\t", 16);
        cseen = 1;
      } else if (bseen == 0 && b == 1){
        //same logic as cseen
        write(STDOUT_FILENO, "\tcharacter count is:\t", 21);
        bseen = 1;
      } else if (mseen == 0 && m == 1){
        //same logic as cseen
        write(STDOUT_FILENO, "\tmaximum line length is:\t", 25);
        mseen = 1;
      }
      //print message from message queue
      write(STDOUT_FILENO, buf, (size_t) numRead);
      write(STDOUT_FILENO, "\n", 1);
      //if the messages for that particular file have all been processed
      if ((mnum + 1) % (n + b + c + m) == 0){
        //reset all the variables for the next file's set of messages
        nseen = 0;
        cseen = 0;
        bseen = 0;
        mseen = 0;
      }
    }
    
    //close child message queue
    mq_close(mqd);
    mq_unlink(mqname);
    write(STDOUT_FILENO,  "Child: Terminating.\n", 20);

    return 0;
  }


  
  //PARENT PROCESS
  else {
    //PART 2
    //OS INFO
    struct utsname uts;
    uname(&uts);
    write(STDOUT_FILENO, "OS name is:\t", 12);
    write(STDOUT_FILENO, uts.sysname, 30);
    write(STDOUT_FILENO, "\n", 1);
    write(STDOUT_FILENO, "OS release is:\t", 15);
    write(STDOUT_FILENO, uts.release, 70);
    write(STDOUT_FILENO, "\n", 1);
    write(STDOUT_FILENO, "OS version is:\t", 15);
    write(STDOUT_FILENO, uts.version, 70);
    write(STDOUT_FILENO, "\n", 1);

    //PROCESS INFO
    //get process id for parent
    int parent_proc_id = getpid();
    //convert parent_proc_id integer to char array of known length
    //first determine length
    int pdigit = 0;
    for (int ptestnum = parent_proc_id; ptestnum > 0; ptestnum = ptestnum/10){
      pdigit++;
    }
    //create char array
    char parent_proc_id_char[pdigit];
    //fill the array
    int pnum = parent_proc_id;
    for (int px = pdigit - 1; px >= 0; px--){
      parent_proc_id_char[px] = pnum % 10 + '0';
      pnum = pnum / 10;
    }
    write(STDOUT_FILENO, "Process ID is:\t", 15);
    write(STDOUT_FILENO, parent_proc_id_char, pdigit);
    write(STDOUT_FILENO, "\n", 1);

    //get parent process id for parent
    int parent_parent_proc_id = getppid();
    //convert parent_proc_id integer to char array of known length
    //first determine length
    pdigit = 0;
    for (int ptestnum = parent_parent_proc_id; ptestnum > 0; ptestnum = ptestnum/10){
      pdigit++;
    }
    //create char array
    char parent_parent_proc_id_char[pdigit];
    //fill the array
    pnum = parent_parent_proc_id;
    for (int px = pdigit - 1; px >= 0; px--){
      parent_parent_proc_id_char[px] = pnum % 10 + '0';
      pnum = pnum / 10;
    }
    write(STDOUT_FILENO, "Parent process ID is:\t", 22);
    write(STDOUT_FILENO, parent_parent_proc_id_char, pdigit);
    write(STDOUT_FILENO, "\n", 1);

    //print parent current working directory
    //at the start, we do not know what size the current working directory will be
    char cwd[200];
    //fill it up with ` symbol
    for (int y = 0; y < 200; y++){
      cwd[y] = '`';
    }
    //get the working directory
    getcwd(cwd, 200);

    //now get rid of the junk end symbols
    //determine the size (number of char)
    int cwd_size = 0;
    for (int y = 0; y < 200; y++){
      //search until hitting the ` symbol, make sure several are hit to be safe
      if (y > 3 && cwd[y] == '`' && cwd[y - 1] == '`' && cwd[y - 2] == '`'){
        cwd_size = y - 3;
        break;
      }
    }
    //fill up the array to print
    char cwd_print[cwd_size];
    for (int y = 0; y < cwd_size; y++){
      cwd_print[y] = cwd[y];
    }
    write(STDOUT_FILENO, "Process current working directory is:\t", 38);
    write(STDOUT_FILENO, cwd_print, cwd_size);
    write(STDOUT_FILENO, "\n", 1);

    //print hostname
    //at the start, we do not know what size the hostname will be
    char hostname[200];
    //fill it up with ` symbol
    for (int y = 0; y < 200; y++){
      hostname[y] = '`';
    }
    //get the hostname
    gethostname(hostname, 200);

    //now get rid of the junk end symbols
    //determine the size (number of char)
    int hostname_size = 0;
    for (int y = 0; y < 200; y++){
      //search until hitting the ` symbol, make sure several are hit to be safe
      if (y > 3 && hostname[y] == '`' && hostname[y - 1] == '`' && hostname[y - 2] == '`'){
        hostname_size = y - 3;
        break;
      }
    }
    //fill up the array to print
    char hostname_print[hostname_size];
    for (int y = 0; y < hostname_size; y++){
      hostname_print[y] = hostname[y];
    }
    write(STDOUT_FILENO, "Hostname is:\t", 13);
    write(STDOUT_FILENO, hostname_print, hostname_size);
    write(STDOUT_FILENO, "\n", 1);


    //PART 1
    close (fd[1]); // close write end of pipe
    //DECLARE VARIABLES
    //char array for parent process to read from the pipe
    char pipeline[buffer_size];
    //track all of the stats
    int newline_count = 0;
    int word_count = 0;
    int char_count = 0;
    int max_line = 0;
    //use array of size number of files to store all of the stats
    int file_newline_count[number_of_files];
    int file_word_count[number_of_files];
    int file_char_count[number_of_files];
    int file_max_line[number_of_files];
    //use characters_seen to keep track of whether to increment for the last word
    int characters_seen = 0;
    //keep track of number of characters on the current line
    int curr_line = 0;
    //detect end of file, start by initializing to random characters
    char charchecker1 = '.';
    char charchecker2 = '.';
    char charchecker3 = '.';
    char charchecker4 = '.';
    char charchecker5 = '.';

    //begin at index 0, and increment when end of file is detected
    int parent_file_index = 0;
    //count number of characters read from pipe
    int nw;
    //READ FROM PIPE AND PROCESS STATS
    //loop until end of pipe is reached
    while ((nw = read(fd[0], pipeline, buffer_size)) > 0) {
      close (fd[1]); // close write end of pipe
      //compute the counts as indicated by the options
      //loop for each character
      for (int ch = 0; ch < nw; ch++){
        //check for newline
        if (pipeline[ch] == '\n'){
          newline_count++;
          //check if current line length is greater than max_line seen so far
          if (curr_line > max_line) {
            max_line = curr_line;
          }
          //reset curr line
          curr_line = 0;
        }
        //check for a space, tab, or newline that can separate a word
        if (pipeline[ch] == '\n' || pipeline[ch] == '\t' || pipeline[ch] == ' '){
          //if we have already had characters prior, this is a new word (in contrast to two spaces in a row)
          if (characters_seen == 1){
            word_count++;
            //toggle the characters_seen off, so that if two space are consecutive, we know to not consider the second space as separating words
            characters_seen = 0;
          }
        } else {
          //if current character is an actual character (and not a space, tab, or newline)
          //update character checkers
          charchecker5 = charchecker4;
          charchecker4 = charchecker3;
          charchecker3 = charchecker2;
          charchecker2 = charchecker1;
          charchecker1 = pipeline[ch];
          //check if end of file has been reached using five very uncommon symbols to be found consecutively
          if (charchecker1 == '`' && charchecker2 == '~' && charchecker3 == '$' && charchecker4 == '~' && charchecker5 == '`'){
              //remove the last four characters that were signalling end of file and should not be counted as part of the file
              char_count = char_count - 4;
              //possible clean up is needed to count the last word
              if (characters_seen == 1){
                word_count++;
              }
              //subtract the word that has been counted for the end of file
              word_count--;
              //subtract the newline that has been counted for the end of file message
              newline_count--;
              //possible cleanup is needed to change zero value for maximum line length
              if (newline_count == 0){
                max_line = char_count;
              }
              //save results
              file_newline_count[parent_file_index] = newline_count;
              file_word_count[parent_file_index] = word_count;
              file_char_count[parent_file_index] = char_count;
              file_max_line[parent_file_index] = max_line;
              //reset variables
              newline_count = 0;
              word_count = 0;
              char_count = 0;
              max_line = 0;
              characters_seen = 0;
              curr_line = 0;
              //increment parent_file_index
              parent_file_index++;
          } else {
          //if end of file has not been reached, and the character is just a plain character
          //increase current line length
          curr_line++;
          //increase character count
          char_count++;
          //toggle the characters_seen on
          characters_seen = 1;
          }
        }
      }
    }

    //SEND TO POSIX MESSAGE QUEUE
    //send to message queue the results for each file
    for (int parent_iterate_file = 0; parent_iterate_file < number_of_files; parent_iterate_file++) {

      //maximum of four messages will be sent per file, corresponding to each option     
      //send options according to what the user has requested
      if (n == 1){
        //newline count
        //convert newline count to char array of known length
        //first determine length
        int digit = 0;
        for (int testnum = file_newline_count[parent_iterate_file]; testnum > 0; testnum = testnum/10){
          digit++;
        }
        //create char array
        char newline_count_char[digit];
        //fill the array
        int num = file_newline_count[parent_iterate_file];
        for (int x = digit - 1; x >= 0; x--){
          newline_count_char[x] = num % 10 + '0';
          num = num / 10;
        }
        //Write the newline message. if newline is 0, handle special case so something actually is sent on the message queue and it is not blank
        if (file_newline_count[parent_iterate_file] == 0){
          if (mq_timedsend(mqd, "0", 1, 1, NULL) == -1) {
            write(STDOUT_FILENO, "message send error", 19);
            return 13;
          }
        } else {
          if (mq_timedsend(mqd, newline_count_char, digit, 1, NULL) == -1) {
            write(STDOUT_FILENO, "message send error", 19);
            return 14;
          }
        }
      }
      if (c == 1){
        //word count
        //convert word count to char array of known length
        //first determine length
        int digit = 0;
        for (int testnum = file_word_count[parent_iterate_file]; testnum > 0; testnum = testnum/10){
          digit++;
        }
        //create char array
        char word_count_char[digit];
        //fill the array
        int num = file_word_count[parent_iterate_file];
        for (int x = digit - 1; x >= 0; x--){
          word_count_char[x] = num % 10 + '0';
          num = num / 10;
        }
        
        if (mq_timedsend(mqd, word_count_char, digit, 1, NULL) == -1) {
            write(STDOUT_FILENO, "message send error", 19);
            return 15;
        }
      }
      if (b == 1){
        //character count
        //convert character count to char array of known length
        //first determine length
        int digit = 0;
        for (int testnum = file_char_count[parent_iterate_file]; testnum > 0; testnum = testnum/10){
          digit++;
        }
        //create char array
        char char_count_char[digit];
        //fill the array
        int num = file_char_count[parent_iterate_file];
        for (int x = digit - 1; x >= 0; x--){
          char_count_char[x] = num % 10 + '0';
          num = num / 10;
        }

        if (mq_timedsend(mqd, char_count_char, digit, 1, NULL) == -1) {
            write(STDOUT_FILENO, "message send error", 19);
            return 16;
        }
      }
      if (m == 1){
        //maxline length
        //convert maxline length to char array of known length
        //first determine length
        int digit = 0;
        for (int testnum = file_max_line[parent_iterate_file]; testnum > 0; testnum = testnum/10){
          digit++;
        }
        //create char array
        char char_max_line[digit];
        //fill the array
        int num = file_max_line[parent_iterate_file];
        for (int x = digit - 1; x >= 0; x--){
          char_max_line[x] = num % 10 + '0';
          num = num / 10;
        }

        if (mq_timedsend(mqd, char_max_line, digit, 1, NULL) == -1) {
            write(STDOUT_FILENO, "message send error", 19);
            return 17;
        }
      }
    }
  //close parent message queue
  mq_close(mqd);
	mq_unlink(mqname);
  //wait for child process to return
  wait(NULL);
	write(STDOUT_FILENO,  "Parent: Terminating.\n", 21);
	return 0;
  }
}