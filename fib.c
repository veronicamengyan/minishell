#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

const int MAX = 13;

static void doFib(int n, int doPrint);


/*
 * unix_error - unix-style error routine.
 */
inline static void 
unix_error(char *msg)
{
    fprintf(stdout, "%s: %s\n", msg, strerror(errno));
    exit(1);
}


int main(int argc, char **argv)
{
  int arg;
  int print;

  if(argc != 2){
    fprintf(stderr, "Usage: fib <num>\n");
    exit(-1);
  }

  if(argc >= 3){
    print = 1;
  }

  arg = atoi(argv[1]);
  if(arg < 0 || arg > MAX){
    fprintf(stderr, "number must be between 0 and %d\n", MAX);
    exit(-1);
  }

  doFib(arg, 1);

  return 0;
}

/* 
 * Recursively compute the specified number. If print is
 * true, print it. Otherwise, provide it to my parent process.
 *
 * NOTE: The solution must be recursive and it must fork
 * a new child for each call. Each process should call
 * doFib() exactly once.
 */
static void doFib(int n, int doPrint)
{
   pid_t pid;
   int status;
   int x=0;
   if(n<1 || n>13)
   {
       printf("Error: argument has to be from 1 to 13.\n");
       return;
   }
   if(n <= 2)
   {
       printf("%d\n",n-1);
       exit(n-1); 
   }
   //child
   if(n > 2)
   {
     if((pid = fork())==0)
     {
         if(doPrint)
         {
              printf("pid: %d\n, n:%d\n ",getpid(),n);
         }
        
         doFib(n-1,doPrint);
         doFib(n-2,doPrint);
         exit(n);
       
      }
    }
   //parent
   else
   {
      waitpid(-1,&status,0);
     if(WIFEXITED(status))
     {
         x += WEXITSTATUS(status);
         printf("%d\n",x);                                                              
     }
   }

}
