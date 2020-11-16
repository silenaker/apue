#include "open.h"
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "error.h"

#define BUFFSIZE 4096

int main(int argc, char* argv[]) {
  int n, fd;
  char buf[BUFFSIZE];
  char line[LINE_MAX];
  uid_t uid = 0;

  /* read filename to cat from stdin */
  while (fgets(line, LINE_MAX, stdin) != NULL) {
    if (line[strlen(line) - 1] == '\n') {
      line[strlen(line) - 1] = 0; /* replace newline with null */
    }

    /* open the file */
    if ((fd = csopen(line, O_RDONLY, &uid)) < 0) {
      continue; /* csopen() prints error from server */
    }

    if (uid != 0) {
      printf("received server uid: %d\n", uid);
    }

    /* and cat to stdout */
    while ((n = read(fd, buf, BUFFSIZE)) > 0) {
      if (write(STDOUT_FILENO, buf, n) != n) {
        err_sys("write error");
      }
    }

    if (n < 0) {
      err_sys("read error");
    }
    close(fd);
  }

  exit(0);
}