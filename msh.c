/* 
 * msh - A mini shell program with job control
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
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include "util.h"
#include "jobs.h"


/* Global variables */
int verbose = 0;            /* if true, print additional output */

extern char **environ;      /* defined in libc */
static char prompt[] = "msh> ";    /* command line prompt (DO NOT CHANGE) */
static struct job_t jobs[MAXJOBS]; /* The job list */
/* End global variables */


/* Function prototypes */

/* Here are the functions that you will implement */
void eval(char *cmdline);
int builtin_cmd(char **argv);
void do_bgfg(char **argv);
void waitfg(pid_t pid);

void sigchld_handler(int sig);
void sigtstp_handler(int sig);
void sigint_handler(int sig);

/* Here are helper routines that we've provided for you */
void usage(void);
void sigquit_handler(int sig);



/*
 * main - The shell's main routine 
 */
int main(int argc, char **argv) 
{
    //fprintf(stdout,"hello?");
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

    /* Install the signal handlers */

    /* These are the ones you will need to implement */
    Signal(SIGINT,  sigint_handler);   /* ctrl-c */
    Signal(SIGTSTP, sigtstp_handler);  /* ctrl-z */
    Signal(SIGCHLD, sigchld_handler);  /* Terminated or stopped child */

    /* This one provides a clean way to kill the shell */
    Signal(SIGQUIT, sigquit_handler); 

    /* Initialize the job list */
    initjobs(jobs);

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
    //printf("while loop\n");
	fflush(stdout);
	fflush(stdout);
    } 

    exit(0); /* control never reaches here */
}
  
/* 
 * eval - Evaluate the command line that the user has just typed in
 * 
 * If the user has requested a built-in command (quit, jobs, bg or fg)
 * then execute it immediately. Otherwise, fork a child process and
 * run the job in the context of the child. If the job is running in
 * the foreground, wait for it to terminate and then return.  Note:
 * each child process must have a unique process group ID so that our
 * background children don't receive SIGINT (SIGTSTP) from the kernel
 * when we type ctrl-c (ctrl-z) at the keyboard.  
*/
void eval(char *cmdline) 
{
    char **cargv = calloc(MAXARGS, sizeof(char*));
    char *modified = calloc(MAXLINE, sizeof(char*));
    strcpy(modified, cmdline);
    char *temp = strtok(modified," \n");
    int i = 0;
    int bg = 0;
    pid_t pid;
    sigset_t mask;

    //printf("This is cmdline: %s\n",cmdline);
    while(temp != NULL && i < MAXARGS)
    {
        cargv[i] = calloc(strlen(temp),sizeof(char));
        strcpy(cargv[i],temp);
        i++;
        temp = strtok(NULL," \n");
    }
    if (cargv[0] == NULL || i == 0)
        return;

    if (strlen(cargv[i-1]) == 1 && cargv[i-1][0] == '&') 
    {
       // printf("This is cargv[i-1]: %s",cargv[i-1]);
        cargv[i-1] = NULL;
        free(cargv[i-1]);
        i--;
        bg = 1;
    }
    //printf("bg %d\n", bg);
    
    // buildin command
    // quit, jobs, bg, fg
    if (strcmp(cargv[0], "quit") == 0)
    {
        int j;
        for(j = 0;j < i;j++)
        {
           free(cargv[j]);
        }
        free(cargv);
        exit(0);
    }

    //system process
    // execute files from users
    //child & parent processes
    int test;
    //printf("before cmd: \n"); 
    
    if((test = builtin_cmd(cargv)) == 0)
    {  
        //printf("Built_in cmd: %d\n", test); 
        sigemptyset(&mask);
        sigaddset(&mask, SIGCHLD);
        sigprocmask(SIG_BLOCK, &mask, NULL);
            
        //child
       //fflush(stdout);
       
       if((pid = fork()) == 0)
       {
       //   printf("Here is child pid: %d",pid);
        //  printf("HI I am a child"); 
          // printf("Here is my pid: %d",getpid());
           //set
           if(bg)
           {
               setpgid(0,0);
           }

          sigprocmask(SIG_UNBLOCK, &mask, NULL);
          if(execve(cargv[0],cargv,environ) < 0)
          {
              app_error("Command not found.\n");
              exit(0);
          }

          //if (bg)
          {
              do_bgfg(cargv);
          } 
         // sleep(2);
          exit(0);

      }
      if (bg)
      {
         addjob(jobs,pid,BG,cmdline);
         sigprocmask(SIG_UNBLOCK, &mask, NULL);
         printf("[%d] (%d) ",pid2jid(jobs, pid), pid);
         int k = 0;
         while (cargv[k] != NULL)
         {
             printf("%s ", cargv[k]);
             k++;
         }
         printf("&\n");
      }
      else
      {
         addjob(jobs, pid, FG, cmdline);
         sigprocmask(SIG_UNBLOCK, &mask, NULL);
         waitfg(pid);
      }
   }

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
    if (strcmp(argv[0], "fg") == 0 || strcmp(argv[0], "bg") == 0)
    {
        do_bgfg(argv);
        //printf();
        return 1;
    }
    else if (strcmp(argv[0], "jobs") == 0)
    {
        listBGjobs(jobs);
        return 1;
    }

    return 0;
    /*
    int i = 0;
   while( argv[i] != NULL)
    {
       i++;
    }

    if(strcmp(argv[0],"quit") == 0 || strcmp(argv[0],"jobs") == 0 || strcmp(argv[0],"bg") == 0 || strcmp(argv[0],"fg") == 0)
    {
       return 1;   
    }
    return 0;  
    */
}

/* 
 * checkDigits - checks if the input string is composed of digits only 
 * returns 1 is contains only digits, 0 otherwise
 */
int checkDigits(char *str, int index)
{
     int i;
     for (i = index; i < strlen(str); i++)
     {
         if (!isdigit(str[i]))
             return 0;
     }
     return 1;
}

/**
 * changes the state of the job from foreground to background or vice versa
 */
void change_state(char **argv, int state)
{
    printf("change state\n");
    int num = atoi(++(argv[1]));
    struct job_t *temp = NULL;
    if (argv[1] == NULL)
    {
        if (argv[1][0] == '%' && checkDigits(argv[1], 1)) //jobid
        {
            num = atoi(++(argv[1]));
            temp = getjobjid(jobs, num);
        }
        else if (argv[1][0] != '%' && checkDigits(argv[1], 0)) // process id
        {
            num = atoi(argv[1]);
            temp = getjobpid(jobs, num);
        }
        if (temp == NULL) //invalid
        {
            app_error("Invalid job id or process id");
        }

        if(state)
        {
            temp->state = BG; 
        }
        else
        {
            temp->state = FG;
            waitfg(getpid());
        }

        printf("[%d] (%d) ", temp->jid, temp->pid);
        int i = 0;
        while (argv[i] != NULL)
        {
            printf("%s ", argv[i]);
        }
        printf("\n");

    }
    else //invalid
    {
        app_error("Invalid job id or process id");
    }
}

/* 
 * do_bgfg - Execute the builtin bg and fg commands
 */
void do_bgfg(char **argv) 
{        
    if(strcmp(argv[0],"bg") == 0)
        change_state(argv, 1);
    else if (strcmp(argv[0],"fg") == 0)
        change_state(argv, 0);

    printf("do_bgfg\n");

      
    /*
    int status;
    if(strcmp(argv[0],"bg") == 0)
    {
        if(waitpid(-1,&status,WNOHANG) < 0)
               unix_error("waitbg: waitpid error");

        if (WIFEXITED(status))
        {
             if(Signal(SIGCHLD,sigchld_handler) == SIG_ERR)
                 unix_error("signal error");
        }
    }
    else if (strcmp(argv[0],"fg") == 0)
    {
        if(waitpid(-1,&status,0) < 0)
               unix_error("waitfg: waitpid error");
        else
        {
           // Signal(
        }

    }
    */
}

/* 
 * waitfg - Block until process pid is no longer the foreground process
 */
void waitfg(pid_t pid)
{
   // printf("PID in waitfg: %d\n", pid);
    //struct job_t *temp = getjobpid(jobs,fgpid(jobs));
    //struct job_t *temp = getjobpid(jobs,pid);
    struct timespec time1, time2;
    time1.tv_sec = 0;
    time1.tv_nsec = 10;

    //Signal(SIGCHLD, sigchld_handler);

    //while (getjobpid(jobs,fgpid(jobs))->state != FG)
    /*
    int status;
    pid_t pid2 = waitpid(pid, &status, 0);


    */
    //while ()
    while (fgpid(jobs) == pid) //temp != NULL)
    {
//        printf("sleeping in waitfg\n");
        nanosleep(&time1,&time2);
    }
    return;
}

/*****************
 * Signal handlers
 *****************/

/* 
 * sigchld_handler - The kernel sends a SIGCHLD to the shell whenever
 *     a child job terminates (becomes a zombie), or stops because it
 *     received a SIGSTOP or SIGTSTP signal. The handler reaps all
 *     available zombie children, but doesn't wait for any other
 *     currently running children to terminate.  
 */
void sigchld_handler(int sig) 
{
    int status;
    struct job_t *temp; 
    pid_t pid;
//    printf("Went into sigchld handler\n");
    while ((pid = waitpid(-1,&status,WNOHANG|WUNTRACED)) > 0)
    {
        if (WIFEXITED(status))
        {
           deletejob(jobs,pid);
        }
        else if (WIFSTOPPED(status))
        {
           temp = getjobpid(jobs, getpid());
           temp->state = ST;
           printf("Job [%d] (%d) stopped by signal %d\n", pid2jid(jobs,pid), pid, WSTOPSIG(status));
        }
        else if (WIFSIGNALED(status))
        {
            printf("Job [%d] (%d) terminated by signal %d\n", pid2jid(jobs,pid), pid, WTERMSIG(status));
            deletejob(jobs,pid);
        }
    }

//    if (errno != ECHILD)
//        unix_error("waitpid error");
    return;
}

/* 
 * sigint_handler - The kernel sends a SIGINT to the shell whenver the
 *    user types ctrl-c at the keyboard.  Catch it and send it along
 *    to the foreground job.  
 */
void sigint_handler(int sig) 
{
    pid_t pid;
    pid_t pidfg = fgpid(jobs);
//    struct job_t *temp; 
    int status;

    kill(pidfg, SIGINT);
    if(getpid()==pidfg)
    {
        deletejob(jobs,pidfg);
        return;
    }
   /* while ((pid = waitpid(fgpid(jobs),&status,0)) > 0)
    {
        if (WIFEXITED(status))
        {
           deletejob(jobs,pid);
           
        }
        else if (WIFSTOPPED(status))
        {
           temp = getjobpid(jobs, getpid());
           temp->state = ST;
           printf("Job [%d] (%d) stopped by signal %d\n", pid2jid(jobs,pid), pid, WSTOPSIG(status));
           
        }
        else if (WIFSIGNALED(status))
        {
            printf("Job [%d] (%d) terminated by signal %d\n", pid2jid(jobs,pid), pid, WTERMSIG(status));
            deletejob(jobs,pid);
            
        }
    }*/
//    exit(0);
    return;
}

/*
 * sigtstp_handler - The kernel sends a SIGTSTP to the shell whenever
 *     the user types ctrl-z at the keyboard. Catch it and suspend the
 *     foreground job by sending it a SIGTSTP.  
 */
void sigtstp_handler(int sig) 
{
    pid_t pid;
    pid_t pidfg = fgpid(jobs);
    struct job_t *temp; 
    int status;


   kill(-pidfg, SIGTSTP);
    

    /*while ((pid = waitpid(pidfg,&status,WUNTRACED)) > 0)
    {
        if (WIFEXITED(status))
        {
           deletejob(jobs,pid);
        }
        else if (WIFSTOPPED(status))
        {
           temp = getjobpid(jobs, pid);
           temp->state = ST;
           printf("Job [%d] (%d) stopped by signal %d\n", pid2jid(jobs,pid), pid, WSTOPSIG(status));
        }
        else if (WIFSIGNALED(status))
        {
            printf("Job [%d] (%d) terminated by signal %d\n", pid2jid(jobs,pid), pid, WTERMSIG(status));
            deletejob(jobs,pid);
        }
    }*/
    //return;
    exit(0);
}

/*********************
 * End signal handlers
 *********************/



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



