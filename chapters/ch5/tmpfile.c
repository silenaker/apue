#include <limits.h>
#include <stdio.h>
#include "error.h"

int main(void) {
  char line[LINE_MAX];
  FILE* fp;

  if ((fp = tmpfile()) == NULL) { /* create temp file */
    err_sys("tmpfile error");
  }

  fputs("one line of output\n", fp); /* write to temp file */
  rewind(fp);                        /* then read it back */

  if (fgets(line, sizeof(line), fp) == NULL) {
    err_sys("fgets error");
  }

  fputs(line, stdout); /* print the line we wrote */
}