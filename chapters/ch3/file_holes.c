#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include "error.h"

char buf1[] = "abcdefghij";  // size 10
char buf2[] = "ABCDEFGHIJ";  // size 10

int main(void) {
  int fd;

  if ((fd = creat("file.hole", S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) < 0)
    err_sys("creat error");

  if (write(fd, buf1, 10) != 10)
    err_sys("buf1 write error");

  /* offset now = 10 */
  if (lseek(fd, 16384, SEEK_SET) == -1)
    err_sys("lseek error");

  /* offset now = 16384 */
  if (write(fd, buf2, 10) != 10)
    err_sys("buf2 write error");

  /* offset now = 16394 */
  exit(0);
}