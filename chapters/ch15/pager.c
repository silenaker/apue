#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include "error.h"

/* environment variable, or default */
#define PAGER "/usr/bin/more"

// #define USE_POPEN 1;

int main(int argc, char* argv[]) {
  int n, fd[2];
  pid_t pid;
  char* argv0;
  char line[LINE_MAX];
  FILE *fpi, *fpo;

  if (argc != 2) {
    err_quit("usage: pager <pathname>");
  }

  if ((fpi = fopen(argv[1], "r")) == NULL) {
    err_sys("can’t open %s", argv[1]);
  }

#ifdef USE_POPEN
  if ((fpo = popen("${PAGER:-more}", "w")) == NULL) {
    err_sys("popen error");
  }
#else
  if (pipe(fd) < 0) {
    err_sys("pipe error");
  }
  if ((pid = fork()) < 0) {
    err_sys("fork error");
  } else if (pid == 0) {
    /* close write end */
    close(fd[1]);
    if (fd[0] != STDIN_FILENO) {
      if (dup2(fd[0], STDIN_FILENO) != STDIN_FILENO) {
        err_sys("dup2 error to stdin");
      }
      close(fd[0]);
    }

    if ((argv0 = strrchr(PAGER, '/')) != NULL) {
      argv0++;
    } else {
      argv0 = PAGER;
    }

    if (execl(PAGER, argv0, (char*)0) < 0) {
      err_sys("execl error for %s", PAGER);
    }
  }
  /* close read end */
  close(fd[0]);
  fpo = fdopen(fd[1], "w");
#endif

  /* parent copies input to output */
  while (fgets(line, LINE_MAX, fpi) != NULL) {
    if (fputs(line, fpo) < 0) {
      err_sys("write error to pager");
    }
  }

  if (ferror(fpi)) {
    err_sys("fgets error");
  }

#ifdef USE_POPEN
  if (pclose(fpo) == -1) {
    err_sys("pclose error");
  }
#else
  fclose(fpo);
  if (waitpid(pid, NULL, 0) < 0) {
    err_sys("waitpid error");
  }
#endif

  exit(0);
}