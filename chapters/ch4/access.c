#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "error.h"

int main(int argc, char* argv[]) {
  if (argc != 2) {
    err_quit("usage: <program> <pathname>");
  }

  /* test for existence of file */
  if (access(argv[1], F_OK) < 0) {
    err_ret("access error for %s", argv[1]);
  } else {
    printf("file exists\n");
  }

  /* test for execute or search permission */
  if (access(argv[1], X_OK) < 0) {
    err_ret("access error for %s", argv[1]);
  } else {
    printf("execute or search access OK\n");
  }

  /* test for write permission */
  if (access(argv[1], W_OK) < 0) {
    err_ret("access error for %s", argv[1]);
  } else {
    printf("write access OK\n");
  }

  /* test for read permission */
  if (access(argv[1], R_OK) < 0) {
    err_ret("access error for %s", argv[1]);
  } else {
    printf("read access OK\n");
  }

  exit(0);
}