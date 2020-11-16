#include <limits.h>
#include <stdio.h>

int main(void) {
  char name[L_tmpnam];
  char line[LINE_MAX];
  FILE* fp;

  printf("%s\n", tmpnam(NULL)); /* first temp name */
  tmpnam(name);                 /* second temp name */
  printf("%s\n", name);
}