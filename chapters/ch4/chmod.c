#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include "error.h"

int main(void) {
  struct stat statbuf;
  int fd;

  umask(0);

  if ((fd = open("./foo", O_RDONLY | O_CREAT, S_IRUSR)) < 0) {
    err_sys("open foo failed");
  }

  if (fstat(fd, &statbuf) < 0) {
    err_sys("stat error for foo");
  }

  /* turn on set-group-ID and turn off group-execute */
  if (fchmod(fd, (statbuf.st_mode & ~S_IXGRP) | S_ISGID) < 0) {
    err_sys("chmod error for foo");
  }

  close(fd);

  if ((fd = open("./bar", O_RDONLY | O_CREAT, S_IRUSR)) < 0) {
    err_sys("open bar failed");
  }

  /* set absolute mode to "rw-r--r--" */
  if (fchmod(fd, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH) < 0) {
    err_sys("chmod error for bar");
  }

  exit(0);
}