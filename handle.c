#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include "util.h"


/*
 * First, print out the process ID of this process.
 *
 * Then, set up the signal handler so that ^C causes
 * the program to print "Nice try.\n" and continue looping.
 *
 * Finally, loop forever, printing "Still here\n" once every
 * second.
 */
void handler(int sig);
void handler2(int sig);
int main(int argc, char **argv)
{
  struct timespec time1, time2;
  time1.tv_sec = 1;
  time1.tv_nsec = 0;

  pid_t pid = getpid();
  printf("%d\n",pid);
  while(1)
  {
     if(Signal(SIGINT,handler) == SIG_ERR)
     {
         unix_error("signal error");
     }
     if(Signal(SIGUSR1,handler2)== SIG_ERR)
     {
         unix_error("signal error");
     }
     printf("Still here\n");
     nanosleep(&time1,&time2);
  }
     return 0;
}

 void handler(int sig)
 {
    ssize_t bytes;
    const int STDOUT = 1;
    bytes = write(STDOUT, "Nice try.\n",10);
    if(bytes!=10)
        exit(-999);
 }

void handler2(int sig)
{
   ssize_t bytes;
   const int STDOUT = 1;
   bytes =  write(STDOUT, "exiting\n",8);
   if(bytes!=8)
   {
       exit(-999);
   }
   exit(1);
}

