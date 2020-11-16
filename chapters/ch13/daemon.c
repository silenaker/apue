#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <syslog.h>
#include <unistd.h>
#include "apue.h"
#include "error.h"

#define LOCKFILE "daemon.pid"
#define LOCKFILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

sigset_t mask;

int already_running(void) {
  int fd;
  char buf[16];
  struct flock lock;

  if ((fd = open(LOCKFILE, O_RDWR | O_CREAT, LOCKFILE_MODE)) < 0) {
    syslog(LOG_ERR, "can’t open %s: %s", LOCKFILE, strerror(errno));
    exit(1);
  }

  lock.l_start = 0;
  lock.l_whence = SEEK_SET;
  lock.l_len = 0;
  lock.l_type = F_WRLCK;

  if (fcntl(fd, F_SETLK, &lock) < 0) {
    if (errno == EACCES || errno == EAGAIN) {
      close(fd);
      return (1);
    }
    syslog(LOG_ERR, "can’t lock %s: %s", LOCKFILE, strerror(errno));
    exit(1);
  }

  ftruncate(fd, 0);
  sprintf(buf, "%ld", (long)getpid());
  write(fd, buf, strlen(buf));
  return (0);
}

void reread(void) {
  /* ... */
}

void* thr_fn(void* arg) {
  int signo;

  for (;;) {
    if (sigwait(&mask, &signo) != 0) {
      syslog(LOG_ERR, "sigwait failed");
      exit(1);
    }
    switch (signo) {
      case SIGHUP:
        syslog(LOG_INFO, "Re-reading configuration file");
        reread();
        break;
      case SIGTERM:
        syslog(LOG_INFO, "got SIGTERM; exiting");
        exit(0);
      default:
        syslog(LOG_INFO, "unexpected signal %d\n", signo);
    }
  }

  return (0);
}

int main(int argc, char* argv[]) {
  int err;
  pthread_t tid;
  char* cmd;
  struct sigaction sa;

  if ((cmd = strrchr(argv[0], '/')) == NULL) {
    cmd = argv[0];
  } else {
    cmd++;
  }

  /*
   * Become a daemon.
   */
  daemonize(cmd);

  /*
   * Make sure only one copy of the daemon is running.
   */
  if (already_running()) {
    syslog(LOG_ERR, "daemon already running");
    exit(1);
  }

  /*
   * Restore SIGHUP default and block all signals.
   */
  sa.sa_handler = SIG_DFL;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  if (sigaction(SIGHUP, &sa, NULL) < 0) {
    err_quit("%s: can’t restore SIGHUP default");
  }

  sigfillset(&mask);
  if ((err = pthread_sigmask(SIG_BLOCK, &mask, NULL)) != 0) {
    err_exit(err, "SIG_BLOCK error");
  }

  /*
   * Create a thread to handle SIGHUP and SIGTERM.
   */
  err = pthread_create(&tid, NULL, thr_fn, 0);
  if (err != 0) {
    err_exit(err, "can’t create thread");
  }

  /*
   * Proceed with the rest of the daemon.
   */

  syslog(LOG_INFO, "daemon started");
  sleep(600);
  exit(0);
}