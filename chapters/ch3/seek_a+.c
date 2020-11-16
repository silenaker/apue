#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include "error.h"

int main(int argc, char* argv[]) {
  int fd;
  int n;
  off_t offset;
  char buf[10];

  if ((fd = open("foo", O_RDWR | O_APPEND | O_CREAT,
                 S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)) < 0) {
    err_sys("cannot open foo");
  }

  printf("write to foo: 123456789\n");
  if (write(fd, "123456789", 9) < 0) {
    err_sys("cannot write to foo");
  }

  offset = lseek(fd, 0, SEEK_CUR);
  printf("foo current offset: %lld\n", offset);

  printf("seek foo to 1\n");
  if (lseek(fd, 1, SEEK_SET) < 0) {
    err_sys("cannot seek foo");
  }

  printf("read foo 2 byte\n");
  if ((n = read(fd, buf, 2)) < 0) {
    err_sys("cannot read foo");
  }

  if (n < 2) {
    err_quit("read foo unexpectedly: expect 2 bytes, but got %d", n);
  }

  offset = lseek(fd, 0, SEEK_CUR);
  printf("foo current offset: %lld\n", offset);

  printf("write to foo: abcd\n");
  if (write(fd, "abcd", 4) < 0) {
    err_sys("cannot write to foo");
  }

  offset = lseek(fd, 0, SEEK_CUR);
  printf("foo current offset: %lld\n", offset);

  printf("read foo 1 byte\n");
  if ((n = read(fd, buf, 1)) < 0) {
    err_sys("cannot read foo");
  }

  if (n == 0) {
    printf("read EOF\n");
  } else {
    printf("read unexpectedly: expect EOF, buf got %d byte(s)", n);
  }
}