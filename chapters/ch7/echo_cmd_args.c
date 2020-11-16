#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
  int i;

  /* echo all command-line args */
  for (i = 0; i < argc; i++) {
    printf("argv[%d]: %s\n", i, argv[i]);
  }

  // the loop same as
  for (i = 0; argv[i] != NULL; i++) {
    printf("argv[%d]: %s\n", i, argv[i]);
  }

  exit(0);
}