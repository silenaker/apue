#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include "error.h"

int main(void) {
  char buf[LINE_MAX];
  pid_t pid;
  int status;

  /* print prompt (printf requires %% to print %) */
  printf("%% ");
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