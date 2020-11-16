#include <stdio.h>

void set_stdout_buf() {
  char databuf[BUFSIZ];

  // databuf is on stack, this use is incorrect
  if (setvbuf(stdout, databuf, _IOLBF, BUFSIZ) != 0) {
    fputs("setvbuf for stdout error\n", stderr);
  }
}

int main() {
  set_stdout_buf();
  fputs("hello, world\n", stdout);
}