#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include "error.h"

static void sig_int(int); /* our signal-catching function */

int main(void) {
  char buf[LINE_MAX];
  pid_t pid;
  int status;

  if (signal(SIGINT, sig_int) == SIG_ERR)
    err_sys("signal error");

  printf("%% "); /* print prompt (printf requires %% to print %) */
  while (fgets(buf, LINE_MAX, stdin) != NULL) {
    if (buf[strlen(buf) - 1] == '\n')
      buf[strlen(buf) - 1] = 0; /* replace newline with null */

    if ((pid = fork()) < 0) {
      err_sys("fork error");
    } else if (pid == 0) { /* child */
      execlp(buf, buf, (char*)0);
      err_ret("couldn’t execute: %s", buf);
      exit(127);
    }

    /* parent */
    if ((pid = waitpid(pid, &status, 0)) < 0)
      err_sys("waitpid error");

    printf("%% ");
  }
  exit(0);
}

void sig_int(int signo) {
  printf("interrupt\n%% ");
}