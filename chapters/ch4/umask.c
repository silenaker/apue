#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "error.h"

#define RWRWRW (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)

int main(void) {
  mode_t pre_umask;

  pre_umask = umask(0);
  // default umask is inherited from parent process, typical is 022 (octal)
  printf("default umask: 0%o\n", pre_umask);

  /* expect rw-rw-rw- */
  if (creat("foo", RWRWRW) < 0)
    err_sys("creat error for foo");

  umask(S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);

  /* expect rw------- */
  if (creat("bar", RWRWRW) < 0)
    err_sys("creat error for bar");

  exit(0);
}