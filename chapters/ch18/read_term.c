#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include "error.h"

int main(void) {
  struct termios term;
  long vdisable;
  int read_n;
  char buf[LINE_MAX];

  if (isatty(STDIN_FILENO) == 0) {
    err_quit("standard input is not a terminal device");
  }

  if ((vdisable = fpathconf(STDIN_FILENO, _PC_VDISABLE)) < 0) {
    err_quit("fpathconf error or _POSIX_VDISABLE not in effect");
  }

  /* fetch tty state */
  if (tcgetattr(STDIN_FILENO, &term) < 0) {
    err_sys("tcgetattr error");
  }

  term.c_cc[VINTR] = vdisable; /* disable INTR character */
  term.c_cc[VEOF] = 2;         /* EOF is Control-B */

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &term) < 0) {
    err_sys("tcsetattr error");
  }

  while ((read_n = read(STDIN_FILENO, buf, LINE_MAX - 1)) > 0) {
    buf[read_n] = '\0';
    printf("read characters: %s", buf);
    fflush(stdout);
  }

  if (read_n < 0) {
    err_sys("read error");
  }

  printf("only read EOF, terminate!\n");
  exit(0);
}