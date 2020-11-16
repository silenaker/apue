#include "log.h"
#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

/*
 * Caller must define and set this: nonzero if
 * interactive, zero if daemon
 */
// extern int log_to_stderr;
static int log_to_stderr = 1;

/*
 * Print a message and return to caller.
 * Caller specifies "errnoflag" and "priority".
 */
static void log_doit(int errnoflag,
                     int error,
                     int priority,
                     const char* fmt,
                     va_list ap) {
  char buf[LINE_MAX];

  vsnprintf(buf, LINE_MAX - 1, fmt, ap);

  if (errnoflag) {
    snprintf(buf + strlen(buf), LINE_MAX - 1 - strlen(buf), ": %s",
             strerror(error));
  }

  strcat(buf, "\n");

  if (log_to_stderr) {
    fflush(stdout);
    fputs(buf, stderr);
    fflush(stderr);
  } else {
    syslog(priority, "%s", buf);
  }
}

/*
 * Initialize syslog(), if running as daemon.
 */
void log_open(const char* ident, int option, int facility) {
  if (log_to_stderr == 0)
    openlog(ident, option, facility);
}

/*
 * Nonfatal error related to a system call.
 * Print a message with the system's errno value and return.
 */
void log_ret(const char* fmt, ...) {
  va_list ap;

  va_start(ap, fmt);
  log_doit(1, errno, LOG_ERR, fmt, ap);
  va_end(ap);
}

/*
 * Fatal error related to a system call.
 * Print a message and terminate.
 */
void log_sys(const char* fmt, ...) {
  va_list ap;

  va_start(ap, fmt);
  log_doit(1, errno, LOG_ERR, fmt, ap);
  va_end(ap);
  exit(2);
}

/*
 * Nonfatal error unrelated to a system call.
 * Print a message and return.
 */
void log_msg(const char* fmt, ...) {
  va_list ap;

  va_start(ap, fmt);
  log_doit(0, 0, LOG_ERR, fmt, ap);
  va_end(ap);
}

/*
 * Fatal error unrelated to a system call.
 * Print a message and terminate.
 */
void log_quit(const char* fmt, ...) {
  va_list ap;

  va_start(ap, fmt);
  log_doit(0, 0, LOG_ERR, fmt, ap);
  va_end(ap);
  exit(2);
}

/*
 * Fatal error related to a system call.
 * Error number passed as an explicit parameter.
 * Print a message and terminate.
 */
void log_exit(int error, const char* fmt, ...) {
  va_list ap;

  va_start(ap, fmt);
  log_doit(1, error, LOG_ERR, fmt, ap);
  va_end(ap);
  exit(2);
}
