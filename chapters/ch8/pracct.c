#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/acct.h>
#include "error.h"

#define FMT "%-*.*s e=%6ld, chars = %7ld, %c %c %c %c\n"

/* convert comp_t to unsigned long */
static unsigned long compt2ulong(comp_t comptime) {
  unsigned long val;
  int exp;

  val = comptime & 0x1fff;    /* 13-bit fraction */
  exp = (comptime >> 13) & 7; /* 3-bit exponent (0-7) */

  while (exp-- > 0) {
    val *= 8;
  }

  return (val);
}

int main(int argc, char* argv[]) {
  struct acct acdata;
  FILE* fp;

  if (argc != 2) {
    err_quit("usage: pracct filename");
  }

  if ((fp = fopen(argv[1], "r")) == NULL) {
    err_sys("can’t open %s", argv[1]);
  }

  while (fread(&acdata, sizeof(acdata), 1, fp) == 1) {
    printf(
        FMT, (int)sizeof(acdata.ac_comm), (int)sizeof(acdata.ac_comm),
        acdata.ac_comm, compt2ulong(acdata.ac_etime), compt2ulong(acdata.ac_io),
        acdata.ac_flag & ACORE ? 'D' : ' ', acdata.ac_flag & AXSIG ? 'X' : ' ',
        acdata.ac_flag & AFORK ? 'F' : ' ', acdata.ac_flag & ASU ? 'S' : ' ');
  }

  if (ferror(fp)) {
    err_sys("read error");
  }

  exit(0);
}