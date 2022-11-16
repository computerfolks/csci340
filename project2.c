// Jacob Weissman 23907251
// compile command: gcc WEISSMAN_23907251.c -pthread -lm -o WEISSMAN_23907251.exe
// execute command: ./WEISSMAN_23907251.exe InpArray.txt WEISSMAN_23907251.txt

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <math.h>

#include <unistd.h>
#include <fcntl.h>

// DECLARE GLOBAL VARIABLES
// array to record numbers from the file
int InpArray[16000];

// array to store the sums, initialized to 0 for all values, 16 total values
int sum[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// array to store the geometric averages, initialized to 1 for all values, 16 total values
double geo[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

// array to store the arithmetic averages, initialized to 0 for all values, 16 total values
double ari[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// array storing numbers 0 through 15 to be accessed by child threads
int thdno[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

// WORKER SUM FUNCTION
void *workersum(void *param)
{
  // determine which thread (0 through 15) we are working with
  int thread_index = *((int *)param);

  // access correct start location
  int start = (thread_index)*1000;

  // access correct end location. note: end is NOT inclusive
  int end = (thread_index + 1) * 1000;

  // set to 0 as a starting value
  sum[thread_index] = 0;

  // iterate through the InpArray from start to end, and record sum
  for (int sum_index = start; sum_index < end; sum_index++)
  {
    sum[thread_index] = sum[thread_index] + InpArray[sum_index];
  }
  pthread_exit(0);
}

// WORKER GEOMETRIC AVERAGE FUNCTION
void *workergeo(void *param)
{
  // determine which thread (0 through 15) we are working with
  int thread_index = *((int *)param);

  // access correct start location
  int start = (thread_index)*1000;

  // access correct end location. note: end is NOT inclusive
  int end = (thread_index + 1) * 1000;

  // set to 1 as a starting value
  geo[thread_index] = 1;

  // iterate through the InpArray from start to end
  for (int geo_index = start; geo_index < end; geo_index++)
  {
    // take the 1000th root first, to avoid exceeding max integer value
    double p = pow(InpArray[geo_index], (0.001));

    // multiply by previous values
    geo[thread_index] = geo[thread_index] * p;
  }
  pthread_exit(0);
}

// WORKER ARITHMETIC AVERAGE FUNCTION
void *workerari(void *param)
{
  // determine which thread (0 through 15) we are working with
  int thread_index = *((int *)param);

  // access correct start location
  int start = (thread_index)*1000;

  // access correct end location. note: end is NOT inclusive
  int end = (thread_index + 1) * 1000;

  // set to 0 as a starting value
  ari[thread_index] = 0;

  // iterate through the InpArray from start to end
  for (int ari_index = start; ari_index < end; ari_index++)
  {
    // nextvalue must be a double, so it can be divided by 1000 in the next step
    double nextvalue = InpArray[ari_index] + 0.0;

    // divide by 1000 first, to avoid exceeding max integer value
    // add to the total
    ari[thread_index] = ari[thread_index] + nextvalue / 1000;
  }
  pthread_exit(0);
}

// WORKER CHILD FUNCTION
// worker child which will spawn the workersum, workergeo, and workerari functions in pthreads
void *workerchild(void *param)
{
  // determine which thread (0 through 15) we are working with
  int thread_index = *((int *)param);

  // create three new children pthreads, one for each task
  pthread_t child_tid[3];
  pthread_attr_t child_attr[3];

  // initialize attr
  pthread_attr_init(&child_attr[0]);
  pthread_attr_init(&child_attr[1]);
  pthread_attr_init(&child_attr[2]);

  // create children pthreads
  pthread_create(&child_tid[0], &child_attr[0], workersum, &thread_index);
  pthread_create(&child_tid[1], &child_attr[1], workergeo, &thread_index);
  pthread_create(&child_tid[2], &child_attr[2], workerari, &thread_index);

  // wait for each pthread to finish before exiting workerchild pthread
  pthread_join(child_tid[0], NULL);
  pthread_join(child_tid[1], NULL);
  pthread_join(child_tid[2], NULL);
  pthread_exit(0);
}

int main(int argc, char *argv[])
{
  // ensure user has provided three command-line arguments
  if (argc != 3)
  {
    write(STDOUT_FILENO, "incorrect number of arguments", 29);
    return 1;
  }

  // open file that user has specified
  // declare file descriptor
  int f;
  if ((f = open(argv[1], O_RDONLY, 0)) == -1)
  {
    write(STDOUT_FILENO, "can't open user file", 20);
    return 2;
  }

  // DECLARE VARIABLES TO READ CONTENTS OF FILE
  //  declare buffer
  int buffer_size = 100;

  // char array to read from the file
  char fileline[buffer_size];

  // array_index will track the index as we place numbers into InpArray and fill InpArray
  int array_index = 0;

  // track what digit we are up to in the current number
  int current_number_index = 0;

  // use char array to record current number
  char current_number[5];

  // number of characters read for each iteration
  int contents;

  // READ CONTENTS OF FILE INTO ARRAY
  while ((contents = read(f, fileline, buffer_size)) > 0)
  {

    // loop for each character
    for (int ch = 0; ch < contents; ch++)
    {

      // if we have reached a newline character
      if (fileline[ch] == '\n')
      {

        // record current number
        InpArray[array_index] = atoi(current_number);

        // move to next number
        array_index++;

        // reset index
        current_number_index = 0;
      }
      else
      {

        // record current character
        current_number[current_number_index] = fileline[ch];

        // increment index in the current_number array
        current_number_index++;
      }
    }
  }

  // CREATE THREADS
  //  set number of threads
  const int threadlimit = 16;

  // create array of thread identifiers
  pthread_t tid[threadlimit];

  // create array of thread attr
  pthread_attr_t attr[threadlimit];

  // create pthreads
  for (int i = 0; i < threadlimit; i++)
  {

    // set default attributes
    pthread_attr_init(&attr[i]);

    // create pthread
    pthread_create(&tid[i], &attr[i], workerchild, &thdno[i]);
  }

  // wait for each pthread to finish
  for (int i = 0; i < threadlimit; i++)
  {
    pthread_join(tid[i], NULL);
  }

  // create output file
  int outputfile;

  // open output file
  if ((outputfile = creat(argv[2], 0644)) == -1)
  {
    write(STDOUT_FILENO, "cannot create output file", 25);
    return 3;
  }

  // declare variables to find the max of sum, geo, and ari
  int summax = 0;
  double geomax = 0;
  double arimax = 0;

  // DETERMINE MAX AND PRINT RESULTS FOR EACH THREAD
  for (int i = 0; i < threadlimit; i++)
  {
    // check for max values
    if (sum[i] > summax)
    {
      summax = sum[i];
    }
    if (geo[i] > geomax)
    {
      geomax = geo[i];
    }
    if (ari[i] > arimax)
    {
      arimax = ari[i];
    }

    // PRINT RESULTS FOR EACH THREAD
    // print thread number
    write(outputfile, "Worker Child Pthread Number = ", 30);

    // create char array of size 2
    char index_char[2];

    // if i is less than 10, there is only one digit to print
    if (i < 10)
    {

      // convert int to char
      index_char[0] = i + '0';
      write(outputfile, index_char, 1);
    }

    // if i is 10 or greater, there are two digits to print
    else
    {
      index_char[0] = i / 10 + '0';
      index_char[1] = i % 10 + '0';
      write(outputfile, index_char, 2);
    }
    write(outputfile, ":\t", 2);

    // PRINT GEOMETRIC AVERAGE
    write(outputfile, " Geometric Average = ", 21);

    // convert geo[i] to char array of known length
    // print to three decimal places
    // first determine number of digits before the decimal place
    int geodigit = 0;
    for (int ptestnum = (int)geo[i]; ptestnum > 0; ptestnum = ptestnum / 10)
    {
      geodigit++;
    }
    // create char array to be filled up with the digits in geo[i]
    char currentgeo[geodigit];

    // find value of only the parts of geo[i] that preceed the decimal point
    int geonum = (int)geo[i];

    // fill the array
    for (int geox = geodigit - 1; geox >= 0; geox--)
    {
      currentgeo[geox] = geonum % 10 + '0';
      geonum = geonum / 10;
    }

    // determine, to three decimal places, the digits after the decimal point
    char currentgeodec[3];

    // create an int with only the three numbers immediately after the decimal point
    int geonumdec = ((int)(geo[i] * 1000)) % 1000;

    // round the number, if necessary
    if (((int)(geo[i] * 10000)) % 10 >= 5)
    {
      geonumdec = geonumdec + 1;
    }

    // fill the array
    for (int geodecx = 3 - 1; geodecx >= 0; geodecx--)
    {
      currentgeodec[geodecx] = geonumdec % 10 + '0';
      geonumdec = geonumdec / 10;
    }
    write(outputfile, currentgeo, geodigit);
    write(outputfile, ".", 1);
    write(outputfile, currentgeodec, 3);

    // PRINT ARITHMETIC AVERAGE
    write(outputfile, "\t Arithmetic Average = ", 23);

    // convert ari[i] to char array of known length
    // print to three decimal places
    // first determine number of digits before the decimal place
    int aridigit = 0;
    for (int ptestnum = (int)ari[i]; ptestnum > 0; ptestnum = ptestnum / 10)
    {
      aridigit++;
    }
    // create char array to be filled up with the digits in ari[i]
    char currentari[aridigit];

    // fill the array
    int arinum = (int)ari[i];
    for (int arix = aridigit - 1; arix >= 0; arix--)
    {
      currentari[arix] = arinum % 10 + '0';
      arinum = arinum / 10;
    }

    // determine, to three decimal places, the digits after the decimal point
    char currentaridec[3];

    // create an int with only the three numbers immediately after the decimal point
    int arinumdec = ((int)(ari[i] * 1000)) % 1000;

    // round the number, if necessary
    if (((int)(ari[i] * 10000)) % 10 >= 5)
    {
      arinumdec = arinumdec + 1;
    }

    // fill the array
    for (int aridecx = 3 - 1; aridecx >= 0; aridecx--)
    {
      currentaridec[aridecx] = arinumdec % 10 + '0';
      arinumdec = arinumdec / 10;
    }
    write(outputfile, currentari, aridigit);
    write(outputfile, ".", 1);
    write(outputfile, currentaridec, 3);

    // PRINT SUM
    write(outputfile, "\t Sum = ", 8);

    // convert sum[i] to char array of known length
    // first determine number of digits
    int sumdigit = 0;
    for (int ptestnum = sum[i]; ptestnum > 0; ptestnum = ptestnum / 10)
    {
      sumdigit++;
    }
    // create char array to be filled up with the digits in sum[i]
    char currentsum[sumdigit];

    // fill the array
    int sumnum = sum[i];
    for (int sumx = sumdigit - 1; sumx >= 0; sumx--)
    {
      currentsum[sumx] = sumnum % 10 + '0';
      sumnum = sumnum / 10;
    }
    write(outputfile, currentsum, sumdigit);
    write(outputfile, "\n", 1);
  }

  // MAIN PROGRAM THREAD OUTPUT

  // PRINT GEOMETRIC AVERAGE MAX
  write(outputfile, "Main program thread: Max of the Geometric Averages = ", 53);

  // convert geomax to char array of known length
  // print to three decimal places
  // first determine number of digits before the decimal place
  int geodigit = 0;
  for (int ptestnum = (int)geomax; ptestnum > 0; ptestnum = ptestnum / 10)
  {
    geodigit++;
  }
  // create char array to be filled up with the digits in geomax
  char currentgeo[geodigit];

  // find value of only the parts of geomax that preceed the decimal point
  int geonum = (int)geomax;

  // fill the array
  for (int geox = geodigit - 1; geox >= 0; geox--)
  {
    currentgeo[geox] = geonum % 10 + '0';
    geonum = geonum / 10;
  }

  // determine, to three decimal places, the digits after the decimal point
  char currentgeodec[3];

  // create an int with only the three numbers immediately after the decimal point
  int geonumdec = ((int)(geomax * 1000)) % 1000;

  // round the number, if necessary
  if (((int)(geomax * 10000)) % 10 >= 5)
  {
    geonumdec = geonumdec + 1;
  }

  // fill the array
  for (int geodecx = 3 - 1; geodecx >= 0; geodecx--)
  {
    currentgeodec[geodecx] = geonumdec % 10 + '0';
    geonumdec = geonumdec / 10;
  }
  write(outputfile, currentgeo, geodigit);
  write(outputfile, ".", 1);
  write(outputfile, currentgeodec, 3);

  // PRINT ARITHMETIC AVERAGE MAX
  write(outputfile, "\nMain program thread: Max of the Arithmetic Averages = ", 55);
  // convert arimax to char array of known length
  // print to three decimal places
  // first determine number of digits before the decimal place
  int aridigit = 0;
  for (int ptestnum = (int)arimax; ptestnum > 0; ptestnum = ptestnum / 10)
  {
    aridigit++;
  }
  // create char array to be filled up with the digits in arimax
  char currentari[aridigit];

  // fill the array
  int arinum = (int)arimax;
  for (int arix = aridigit - 1; arix >= 0; arix--)
  {
    currentari[arix] = arinum % 10 + '0';
    arinum = arinum / 10;
  }

  // determine, to three decimal places, the digits after the decimal point
  char currentaridec[3];

  // create an int with only the three numbers immediately after the decimal point
  int arinumdec = ((int)(arimax * 1000)) % 1000;

  // round the number, if necessary
  if (((int)(arimax * 10000)) % 10 >= 5)
  {
    arinumdec = arinumdec + 1;
  }

  // fill the array
  for (int aridecx = 3 - 1; aridecx >= 0; aridecx--)
  {
    currentaridec[aridecx] = arinumdec % 10 + '0';
    arinumdec = arinumdec / 10;
  }
  write(outputfile, currentari, aridigit);
  write(outputfile, ".", 1);
  write(outputfile, currentaridec, 3);

  // PRINT SUM MAX
  write(outputfile, "\nMain program thread: Max of the Sums = ", 40);

  // convert summax to char array of known length
  // first determine number of digits
  int sumdigit = 0;
  for (int ptestnum = summax; ptestnum > 0; ptestnum = ptestnum / 10)
  {
    sumdigit++;
  }
  // create char array to be filled up with digits in summax
  char currentsum[sumdigit];

  // fill the array
  int sumnum = summax;
  for (int sumx = sumdigit - 1; sumx >= 0; sumx--)
  {
    currentsum[sumx] = sumnum % 10 + '0';
    sumnum = sumnum / 10;
  }
  write(outputfile, currentsum, sumdigit);

  // TERMINATE
  write(outputfile, "\nMain program thread: Terminating. \n", 36);

  // close files
  close(f);
  close(outputfile);

  return 0;
}