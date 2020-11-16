#include <limits.h>
#include <stdlib.h>
#include <unistd.h>
#include "error.h"

int main(void) {
  int n;
  int fd[2];
  pid_t pid;
  char line[LINE_MAX];

  if (pipe(fd) < 0)
    err_sys("pipe error");

  if ((pid = fork()) < 0) {
    err_sys("fork error");
  } else if (pid > 0) {
    /* parent */
    close(fd[0]);
    write(fd[1], "hello world\n", 12);
  } else {
    /* child */
    close(fd[1]);
    n = read(fd[0], line, LINE_MAX);
    write(STDOUT_FILENO, line, n);
  }

  exit(0);
}
