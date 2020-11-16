#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static FILE* logfile;

static void sig_handler(int signo) {
  fprintf(logfile, "SIGHUP received, pid = %ld\n", (long)getpid());
}

static void pr_ids() {
  fprintf(logfile, "pid = %ld, ppid = %ld, pgrp = %ld, tpgrp = %ld\n",
          (long)getpid(), (long)getppid(), (long)getpgrp(),
          (long)tcgetpgrp(STDIN_FILENO));
}

int main(void) {
  char c;
  pid_t pid;
  int count = 0;

  logfile = fopen("./logfile", "a");
  setvbuf(logfile, NULL, _IOLBF, 0);
  signal(SIGHUP, sig_handler);

  for (;;) {
    count++;
    fprintf(logfile, "count %d\n", count);
    pr_ids();
    if (isatty(STDIN_FILENO)) {
      fprintf(logfile, "STDIN_FILENO is a tty\n");
    } else {
      fprintf(logfile, "STDIN_FILENO is not a tty\n");
    }
    sleep(5);
  }
}