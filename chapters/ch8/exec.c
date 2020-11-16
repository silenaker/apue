#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include "error.h"

char* env_init[] = {"USER=unknown", "PATH=/tmp", NULL};

int main(void) {
  pid_t pid;

  if ((pid = fork()) < 0) {
    err_sys("fork error");
  } else if (pid == 0) {
    if (execle("echo_env", "echo_env", (char*)0, env_init) < 0) {
      err_sys("execle error");
    }
  }

  if (waitpid(pid, NULL, 0) < 0) {
    err_sys("wait error");
  }

  printf("\n");

  if ((pid = fork()) < 0) {
    err_sys("fork error");
  } else if (pid == 0) { /* inherit environment */
    if (execl("echo_env", "echo_env", (char*)0) < 0)
      err_sys("execlp error");
  }

  exit(0);
}