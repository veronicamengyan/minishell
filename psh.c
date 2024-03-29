/* 
 * psh - A prototype tiny shell program with job control
 * 
 * Name: Sharon Hom
 * Login ID: sharon
 *
 * Name: Meng Yan
 * Login ID: veronica
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include "util.h"



/* Global variables */
int verbose = 0;            /* if true, print additional output */

extern char **environ;      /* defined in libc */
static char prompt[] = "psh> ";    /* command line prompt (DO NOT CHANGE) */
/* End global variables */


/* Function prototypes */

/* Here are the functions that you will implement */
void eval(char *cmdline);
int builtin_cmd(char **argv);

/* Here are helper routines that we've provided for you */
void usage(void);
void sigquit_handler(int sig);



/*
 * main - The shell's main routine 
 */
int main(int argc, char **argv) 
{
    char c;
    char cmdline[MAXLINE];
    int emit_prompt = 1; /* emit prompt (default) */

    /* Redirect stderr to stdout (so that driver will get all output
     * on the pipe connected to stdout) */
    dup2(1, 2);

    /* Parse the command line */
    while ((c = getopt(argc, argv, "hvp")) != EOF) {
        switch (c) {
        case 'h':             /* print help message */
            usage();
	    break;
        case 'v':             /* emit additional diagnostic info */
            verbose = 1;
	    break;
        case 'p':             /* don't print a prompt */
            emit_prompt = 0;  /* handy for automatic testing */
	    break;
	default:
            usage();
	}
    }


    /* This one provides a clean way to kill the shell */
    Signal(SIGQUIT, sigquit_handler); 

    /* Execute the shell's read/eval loop */
    while (1) {

	/* Read command line */
	if (emit_prompt) {
	    printf("%s", prompt);
	    fflush(stdout);
	}
	if ((fgets(cmdline, MAXLINE, stdin) == NULL) && ferror(stdin))
	    app_error("fgets error");
	if (feof(stdin)) { /* End of file (ctrl-d) */
	    fflush(stdout);
	    exit(0);
	}

	/* Evaluate the command line */
	eval(cmdline);
	fflush(stdout);
	fflush(stdout);
    }

    exit(0); /* control never reaches here */
}
  
/* 
 * eval - Evaluate the command line that the user has just typed in
 * 
 * If the user has requested a built-in command (quit)
 * then execute it immediately. Otherwise, fork a child process and
 * run the job in the context of the child. If the job is running in
 * the foreground, wait for it to terminate and then return. 
*/
void eval(char *cmdline) 
{
    char **cargv = calloc(50, sizeof(char*));
    char *temp = strtok(cmdline," \n");
    int i = 0;
    pid_t pid;
    while(temp != NULL && i <= 49)
    {
        cargv[i] = calloc(strlen(temp),sizeof(char));
        strcpy(cargv[i],temp);
        i++;
        temp = strtok(NULL," \n");
    }
    
    // buildin command
    int test;
    if((test=builtin_cmd(cargv)) == 1)
    {
        int j;
        for(j = 0;j < i;j++)
        {
           free(cargv[j]);
        }
        free(cargv);
        exit(0);
    }
    //free later;

    // execute files from users
    //child process
    if((pid = fork()) == 0)
    {
       if(execve(cargv[0],cargv,environ) < 0)
       {
           app_error("Command not found.\n");
           exit(0);
       }
    }

    //parent process
    else
    {
        int status;
        if(waitpid(pid,&status,0) < 0)
               unix_error("waitfg:waitpid error");
        else
        {
           // printf("%d",pid);
        }
    }
   int j;
   for(j = 0;j < i;j++)
   {
        free(cargv[j]);
   }
   free(cargv);

   return;

}


/* 
 * builtin_cmd - If the user has typed a built-in command then execute                       
 *    it immediately. 
 * Return 1 if a builtin command was executed; return 0
 * if the argument passed in is *not* a builtin command.
 */
int builtin_cmd(char **argv) 
{
    int i = 0;
   while( argv[i] != NULL)
    {
       i++;
    }
    if(strcmp(argv[0],"quit") == 0)
    {
       return 1;   
    }
    return 0;     /* not a builtin command */
}





/***********************
 * Other helper routines
 ***********************/

/*
 * usage - print a help message
 */
void usage(void) 
{
    printf("Usage: shell [-hvp]\n");
    printf("   -h   print this message\n");
    printf("   -v   print additional diagnostic information\n");
    printf("   -p   do not emit a command prompt\n");
    exit(1);
}

/*
 * sigquit_handler - The driver program can gracefully terminate the
 *    child shell by sending it a SIGQUIT signal.
 */
void sigquit_handler(int sig) 
{
    printf("Terminating after receipt of SIGQUIT signal\n");
    exit(1);
}



