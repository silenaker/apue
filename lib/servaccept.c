#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <time.h>
#include "socket.h"

#define STALE 600 /* client's name can't be older than this (sec) */

/*
 * Wait for a client connection to arrive, and accept it.
 * We also obtain the client's user ID from the pathname
 * that it must bind before calling us.
 * Returns new fd if all OK, <0 on error
 */
int serv_accept(int listen_fd, uid_t* uid_ptr) {
  int cli_fd, err, rval;
  socklen_t len;
  time_t staletime;
  struct sockaddr_un un;
  struct stat statbuf;
  char* name;

  /* allocate enough space for longest name plus terminating null */
  if ((name = malloc(sizeof(un.sun_path) + 1)) == NULL) {
    return (-1);
  }

  len = sizeof(un);
  if ((cli_fd = accept(listen_fd, (struct sockaddr*)&un, &len)) < 0) {
    free(name);
    return (-2); /* often errno=EINTR, if signal caught */
  }

  /* obtain the client's uid from its calling address */
  len -= offsetof(struct sockaddr_un, sun_path); /* len of pathname */
  memcpy(name, un.sun_path, len);
  name[len] = 0; /* null terminate */

  if (stat(name, &statbuf) < 0) {
    rval = -3;
    goto errout;
  }

#ifdef S_ISSOCK /* not defined for SVR4 */
  if (S_ISSOCK(statbuf.st_mode) == 0) {
    rval = -4; /* not a socket */
    goto errout;
  }
#endif

  if ((statbuf.st_mode & (S_IRWXG | S_IRWXO)) ||
      (statbuf.st_mode & S_IRWXU) != S_IRWXU) {
    rval = -5; /* is not rwx------ */
    goto errout;
  }

  staletime = time(NULL) - STALE;
  if (statbuf.st_atime < staletime || statbuf.st_ctime < staletime ||
      statbuf.st_mtime < staletime) {
    rval = -6; /* i-node is too old */
    goto errout;
  }

  if (uid_ptr != NULL)
    *uid_ptr = statbuf.st_uid; /* return uid of caller */

  unlink(name); /* we're done with pathname now */
  free(name);
  return (cli_fd);

errout:
  err = errno;
  close(cli_fd);
  free(name);
  errno = err;
  return (rval);
}
