#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[]) {
  fprintf(stderr, "EACCES: %s\n", strerror(EACCES));
  errno = ENOENT;
  if (argv[1]) {
    perror(argv[1]);
  } else {
    perror(argv[0]);
  }
  exit(0);
}