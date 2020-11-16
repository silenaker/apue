#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
  int i;
  char** ptr;
  extern char** environ;

  for (ptr = environ; *ptr != 0; ptr++) /* and all env strings */
    printf("%s\n", *ptr);

  exit(0);
}