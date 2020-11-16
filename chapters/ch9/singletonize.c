#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <syslog.h>
#include <unistd.h>

#define LOCKFILE "run.pid"
#define LOCKFILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

int main(int argc, char* argv[]) {
  int fd;
  char buf[16];

  if (argc < 2) {
    printf("usage: singletonize <command> [argv...]\n");
  }

  /*
   * when use O_TRUNC flag, lockfile will be truncated
   * becauseof advisory lock
   */
  fd = open(LOCKFILE, O_RDWR | O_CREAT, LOCKFILE_MODE);

  if (fd < 0) {
    printf("can’t open %s: %s\n", LOCKFILE, strerror(errno));
    exit(1);
  }

  if (flock(fd, LOCK_EX | LOCK_NB) < 0) {
    if (errno == EWOULDBLOCK) {
      close(fd);
    }
    printf("can’t lock %s: %s\n", LOCKFILE, strerror(errno));
    exit(1);
  }

  ftruncate(fd, 0);
  sprintf(buf, "%ld", (long)getpid());
  write(fd, buf, strlen(buf));
  if (execvp(argv[1], argv + 2) < 0) {
    printf("execv failed: %s\n", strerror(errno));
  }
  exit(1);
}