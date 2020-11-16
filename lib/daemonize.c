#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <syslog.h>
#include "apue.h"
#include "error.h"

// #define DEBUG 1
// #define NO_ERROR 1

void daemonize(const char* cmd) {
  int i, fd0, fd1, fd2;
  pid_t pid;
  struct rlimit rl;
  struct sigaction sa;
  int n;

  /*
   * Clear file creation mask.
   */
  umask(0);

  /*
   * Get maximum number of file descriptors.
   */
  if (getrlimit(RLIMIT_NOFILE, &rl) < 0) {
    err_quit("%s: can't get file limit", cmd);
  }
  if (rl.rlim_max == RLIM_INFINITY) {
    rl.rlim_max = 1024;
  }

  /*
   * Ensure future opens won't allocate controlling TTYs.
   */
  sa.sa_handler = SIG_IGN;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  if (sigaction(SIGHUP, &sa, NULL) < 0) {
    err_quit("%s: can't ignore SIGHUP", cmd);
  }

  /*
   * Change the current working directory to the root (may encounter problems
   * when user has no super previledges) so we won't prevent file systems from
   * being unmounted.
   */
#ifndef NO_ERROR
  if (chdir("/") < 0) {
    err_quit("%s: can't change directory to /", cmd);
  }
#endif

  /*
   * fork and become a session leader to lose controlling TTY.
   */
  if ((pid = fork()) < 0) {
    err_quit("%s: can't fork", cmd);
  } else if (pid != 0) { /* parent */
#ifdef DEBUG
    // let child run
    n = sleep(10);
    printf("sleepped %d second(s)\n", 10 - n);
    printf("parent process exit\n");
#endif
    exit(0);
  }

  setsid();
#ifdef DEBUG
  kill(getpid(), SIGSTOP);  // for debugger attaching
#endif

  /*
   * fork secondly to avoid open a controlling terminal
   */
  if ((pid = fork()) < 0) {
    err_quit("%s: can't fork", cmd);
  } else if (pid != 0) { /* parent */
#ifdef DEBUG
    // let child run
    n = sleep(20);
    printf("sleepped %d second(s)\n", 20 - n);
#endif
    exit(0);
  }

  /*
   * Close all open file descriptors.
   */
  for (i = 0; i < rl.rlim_max; i++) {
    close(i);
  }

#ifdef DEBUG
  kill(getpid(), SIGSTOP);  // for debugger attaching
#endif

  /*
   * Attach file descriptors 0, 1, and 2 to /dev/null.
   */
  fd0 = open("/dev/null", O_RDWR);
  fd1 = dup(0);
  fd2 = dup(0);

  /*
   * Initialize the log file.
   */
  openlog(cmd, LOG_CONS | LOG_PID, LOG_DAEMON);
  if (fd0 != 0 || fd1 != 1 || fd2 != 2) {
    syslog(LOG_ERR, "unexpected file descriptors %d %d %d", fd0, fd1, fd2);
    exit(1);
  }
}
