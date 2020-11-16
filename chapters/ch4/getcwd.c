#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "apue.h"
#include "error.h"

int main(void) {
  char* cwd;
  size_t size;

  if (chdir("/usr/local") < 0)
    err_sys("chdir failed");

  cwd = path_alloc(&size); /* our own function */

  if (getcwd(cwd, size) == NULL) {
    err_sys("getcwd failed");
  }

  printf("cwd = %s\n", cwd);
  exit(0);
}