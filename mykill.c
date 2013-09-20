#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>


/*
 * This program sends SIGUSR1 to
 * the specified process ID.
 */
int main(int argc, char **argv)
{
    // Veronica driving now
    pid_t pid = atoi(argv[1]);
    kill(pid,SIGUSR1);
    return 0;
}
