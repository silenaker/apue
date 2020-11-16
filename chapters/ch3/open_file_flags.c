#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "apue.h"
#include "error.h"

void print_fl(int fd) {
  int val;

  if ((val = fcntl(fd, F_GETFL)) < 0)
    err_sys("fcntl error for fd %d", fd);

  switch (val & O_ACCMODE) {
    case O_RDONLY:
      printf("read only");
      break;
    case O_WRONLY:
      printf("write only");
      break;
    case O_RDWR:
      printf("read write");
      break;
    default:
      err_dump("unknown access mode");
  }

  if (val & O_APPEND)
    printf(", append");

  if (val & O_NONBLOCK)
    printf(", nonblocking");

  if (val & O_SYNC)
    printf(", synchronous writes");

#if !defined(_POSIX_C_SOURCE) && defined(O_FSYNC) && (O_FSYNC != O_SYNC)
  if (val & O_FSYNC)
    printf(", synchronous writes");
#endif

#if !defined(_POSIX_C_SOURCE) && defined(O_DSYNC) && (O_DSYNC != O_SYNC)
  if (val & O_DSYNC)
    printf(", synchronous writes data");
#endif

#if !defined(_POSIX_C_SOURCE) && defined(O_RSYNC) && (O_RSYNC != O_SYNC)
  if (val & O_RSYNC)
    printf(", synchronous reads");
#endif

#if defined(O_ASYNC)
  if (val & O_ASYNC)
    printf(", asynchronous reads/writes");
#endif

  putchar('\n');
}

int main(int argc, char* argv[]) {
  int fd;
  int flags = 0;
  char* token;
  char token_sep[] = ",";

  // only O_APPEND | O_NONBLOCK | O_SYNC | O_DSYNC | O_RSYNC | O_FSYNC | O_ASYNC
  // can be set

  if (argc != 3)
    err_quit(
        "usage: <program> <descriptor> "
        "<append,nonblock,sync,dsync,rsync,fsync,async>");

  fd = atoi(argv[1]);
  token = strtok(argv[2], token_sep);

  while (token) {
    if (strcmp(token, "append") == 0)
      flags |= O_APPEND;
    if (strcmp(token, "nonblock") == 0)
      flags |= O_NONBLOCK;
    if (strcmp(token, "sync") == 0)
      flags |= O_SYNC;
    if (strcmp(token, "dsync") == 0)
      flags |= O_DSYNC;
#if defined(O_RSYNC) && (O_RSYNC != O_SYNC)
    if (strcmp(token, "rsync") == 0)
      flags |= O_RSYNC;
#endif
#if defined(O_FSYNC) && (O_FSYNC != O_SYNC)
    if (strcmp(token, "fsync") == 0)
      flags |= O_FSYNC;
#endif
#if defined(O_ASYNC)
    if (strcmp(token, "async") == 0)
      flags |= O_ASYNC;
#endif

    token = strtok(NULL, token_sep);
  }

  printf("default flags: ");
  print_fl(fd);
  set_fl(fd, flags);
  printf("set flags: %d", flags);
  print_fl(fd);
}