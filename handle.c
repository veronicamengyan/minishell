#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include "util.h"

void handler(int sig);
void handler2(int sig);

/*
 * First, print out the process ID of this process.
 *
 * Then, set up the signal handler so that ^C causes
 * the program to print "Nice try.\n" and continue looping.
 *
 * Finally, loop forever, printing "Still here\n" once every
 * second.
 */
int main(int argc, char **argv)
{
  // Veronica driving now
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
     // Sharon driving now
     if(Signal(SIGUSR1,handler2)== SIG_ERR)
     {
         unix_error("signal error");
     }
     printf("Still here\n");
     nanosleep(&time1,&time2);
  }
     return 0;
}

/*
 * This function catches SIGINT signals 
 * and prints "Nice try" everytime it 
 * receives the SIGINT signal. It prevent the
 * program from being terminated by SIGINT signal.
 */
 void handler(int sig)
 {
    // Veronica driving now
    ssize_t bytes;
    const int STDOUT = 1;
    bytes = write(STDOUT, "Nice try.\n",10);
    if(bytes!=10)
        exit(-999);
 }

/*
 * This function catches SIGUSR1 signals, 
 * prints "exiting" and exits. 
 *
 */
void handler2(int sig)
{
   // Sharon driving now
   ssize_t bytes;
   const int STDOUT = 1;
   bytes =  write(STDOUT, "exiting\n",8);
   if(bytes!=8)
   {
       exit(-999);
   }
   exit(1);
}

