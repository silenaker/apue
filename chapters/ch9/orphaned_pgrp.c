#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "error.h"

static FILE* logfile;

static void sig_handler(int signo) {
  fprintf(logfile, "SIGHUP received, pid = %ld\n", (long)getpid());
}

static void pr_ids(char* name) {
  fprintf(logfile, "%s: pid = %ld, ppid = %ld, pgrp = %ld, tpgrp = %ld\n", name,
          (long)getpid(), (long)getppid(), (long)getpgrp(),
          (long)tcgetpgrp(STDIN_FILENO));
}

int main(void) {
  char c;
  pid_t pid;

  logfile = fopen("./logfile", "a");
  setvbuf(logfile, NULL, _IOLBF, 0);
  pr_ids("parent");

  if ((pid = fork()) < 0) {
    err_sys("fork error");
  } else if (pid > 0) { /* parent */
    sleep(5);           /* sleep to let child stop itself */
  } else {              /* child */
    pr_ids("child");
    signal(SIGHUP, sig_handler); /* establish signal handler */
    kill(getpid(), SIGTSTP);     /* stop ourself */
    pr_ids("child");             /* prints only if we’re continued */

    for (;;) {
      if (isatty(STDIN_FILENO)) {
        fprintf(logfile, "STDIN_FILENO is a tty\n");
        if (read(STDIN_FILENO, &c, 1) != 1) {
          fprintf(logfile, "read error %d on controlling TTY\n", errno);
        }
      } else {
        fprintf(logfile, "STDIN_FILENO is not a tty\n");
      }
      pause();
      pr_ids("child");
    }
  }

  exit(0);
}