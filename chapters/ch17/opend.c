#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/syslog.h>
#include <unistd.h>
#include "apue.h"
#include "error.h"
#include "log.h"
#include "open.h"
#include "socket.h"

#define NALLOC 10
#define MAXARGC 50    /* max number of arguments in buf */
#define WHITE " \t\n" /* white space for tokenizing arguments */

struct cmsghdr* cmsg_ptr = NULL;

/* one Client struct per connected client */
typedef struct {
  int fd; /* fd, or -1 if available */
  uid_t uid;
} Client;

// error message returned to client
char errmsg[LINE_MAX];

// client request parameters
char* pathname;
int oflag;

// debug options
int debug;
int use_poll_loop;
int log_to_stderr;

int client_size;
int pollfd_size;

Client* client = NULL;
struct pollfd* pollfd = NULL;

// main event loop
void select_loop(void);
void poll_loop(void);

// handle client request
void handle_request(char*, int, int, uid_t);
// parse client request parameters
int buf_args(char*, int (*func)(int, char**));
int cli_args(int, char**);

// manipulate client array used for select
void client_alloc(void);
int client_add(int, uid_t);
void client_del(int);

// manipulate pollfd array used for poll
void pollfd_alloc(void);
int pollfd_add(int);
void pollfd_del(int);

// send open file descriptor back to client or send error back
int send_fd(int, int);
int send_err(int, int, const char*);

int main(int argc, char* argv[]) {
  int nread, c;
  char buf[LINE_MAX];

#ifdef OPEN_BY_UNIX_SOCK_PAIR
  for (;;) {
    if ((nread = read(STDIN_FILENO, buf, LINE_MAX)) < 0) {
      err_sys("read error on stream pipe");
    } else if (nread == 0) {
      /* client has closed the stream pipe */
      break;
    }
    handle_request(buf, nread, STDOUT_FILENO, 0);
  }
  exit(0);
#endif

#ifdef OPEN_BY_UNIX_SOCK_CONN
  log_open("open.serv", LOG_PID, LOG_USER);
  /* don’t want getopt() writing to stderr */
  opterr = 0;

  /* parse command line options */
  while ((c = getopt(argc, argv, "dp")) != EOF) {
    switch (c) {
      case 'd': /* debug */
        debug = log_to_stderr = 1;
        break;
      case 'p': /* use poll event loop */
        use_poll_loop = 1;
        break;
      case '?':
        err_quit("unrecognized option: -%c", optopt);
    }
  }

  if (debug == 0) {
    daemonize("opend");
  }

  /* never returns */
  if (use_poll_loop) {
    poll_loop();
  } else {
    select_loop();
  }
#endif
}

void select_loop(void) {
  int i, n, max_fd, maxi, listen_fd, cli_fd, nread;
  char buf[LINE_MAX];
  uid_t uid;
  fd_set rset, allset;

  FD_ZERO(&allset);

  /* obtain fd to listen for client requests on */
  if ((listen_fd = serv_listen(CS_OPEN)) < 0) {
    log_sys("serv_listen error");
  }

  FD_SET(listen_fd, &allset);
  max_fd = listen_fd;
  maxi = -1;

  for (;;) {
    rset = allset;  // read set to get read ready file descriptors
    if ((n = select(max_fd + 1, &rset, NULL, NULL, NULL)) < 0) {
      log_sys("select error");
    }

    if (FD_ISSET(listen_fd, &rset)) {
      /* accept new client request */
      if ((cli_fd = serv_accept(listen_fd, &uid)) < 0) {
        log_ret("serv_accept error: %d", cli_fd);
      } else {
        i = client_add(cli_fd, uid);
        FD_SET(cli_fd, &allset);
        if (cli_fd > max_fd) {
          max_fd = cli_fd;
        }
        if (i > maxi) {
          maxi = i;
        }
        log_msg("new connection: uid %d, fd %d", uid, cli_fd);
      }

      if (n == 1) {
        // no need to test client file descriptors
        continue;
      }
    }

    for (i = 0; i <= maxi; i++) {
      if ((cli_fd = client[i].fd) < 0) {
        continue;
      }

      if (FD_ISSET(cli_fd, &rset)) {
        if ((nread = read(cli_fd, buf, LINE_MAX)) < 0) {
          log_sys("read error on fd %d", cli_fd);
        } else if (nread == 0) {
          log_msg("closed: uid %d, fd %d", client[i].uid, cli_fd);
          client_del(cli_fd);
          FD_CLR(cli_fd, &allset);
          close(cli_fd);
        } else {
          /* process client’s request */
          handle_request(buf, nread, cli_fd, client[i].uid);
        }
      }
    }
  }
}

void poll_loop(void) {
  int i, listen_fd, cli_fd, nread, maxi;
  char buf[LINE_MAX];
  uid_t uid;

  /* obtain fd to listen for client requests on */
  if ((listen_fd = serv_listen(CS_OPEN)) < 0) {
    log_sys("serv_listen error");
  }

  pollfd_add(listen_fd);
  maxi = 0;

  for (;;) {
    if (poll(pollfd, maxi + 1, -1) < 0) {
      log_sys("poll error");
    }

    if (pollfd[0].revents & POLLIN) {
      /* accept new client request */
      if ((cli_fd = serv_accept(listen_fd, &uid)) < 0) {
        log_sys("serv_accept error: %d", cli_fd);
      }

      i = pollfd_add(cli_fd);
      client_add(cli_fd, uid);
      if (i > maxi) {
        maxi = i;
      }

      log_msg("new connection: uid %d, fd %d", uid, cli_fd);
    }

    for (i = 1; i <= maxi; i++) {
      if ((cli_fd = pollfd[i].fd) < 0) {
        continue;
      }

      if (pollfd[i].revents & POLLHUP) {
        goto hungup;
      } else if (pollfd[i].revents & POLLIN) {
        if ((nread = read(cli_fd, buf, LINE_MAX)) < 0) {
          log_sys("read error on fd %d", cli_fd);
        } else if (nread == 0) {
        hungup:
          /* the client closed the connection */
          log_msg("closed: uid %d, fd %d", client[i - 1].uid, cli_fd);
          pollfd_del(cli_fd);
          client_del(cli_fd);
          close(cli_fd);
        } else {
          /* process client’s request */
          handle_request(buf, nread, cli_fd, client[i - 1].uid);
        }
      }
    }
  }
}

void handle_request(char* buf, int nread, int fd, uid_t uid) {
  int new_fd;

  if (buf[nread - 1] != '\0') {
    if (uid) {
      snprintf(errmsg, LINE_MAX,
               "request from uid %d not null terminated: %*.*s\n", uid, nread,
               nread, buf);
    } else {
      snprintf(errmsg, LINE_MAX, "request not null terminated: %*.*s\n", nread,
               nread, buf);
    }
    send_err(fd, -1, errmsg);
    if (uid) {
      log_msg(errmsg);
    }
    return;
  }

  if (uid) {
    log_msg("request: %s, from uid %d", buf, uid);
  }

  /* parse args & set options */
  if (buf_args(buf, cli_args) < 0) {
    send_err(fd, -1, errmsg);
    if (uid) {
      log_msg(errmsg);
    }
    return;
  }

  if ((new_fd = open(pathname, oflag)) < 0) {
    snprintf(errmsg, LINE_MAX, "can’t open %s: %s\n", pathname,
             strerror(errno));
    send_err(fd, -1, errmsg);
    if (uid) {
      log_msg(errmsg);
    }
    return;
  }

  /* send the descriptor */
  if (send_fd(fd, new_fd) < 0) {
    if (uid) {
      log_sys("send_fd error");
    } else {
      err_sys("send_fd error");
    }
  }

  if (uid) {
    log_msg("sent fd %d over fd %d for %s", new_fd, fd, pathname);
  }

  /* we’re done with descriptor */
  close(new_fd);
}

/*
 * buf[] contains white-space-separated arguments.  We convert it to an
 * argv-style array of pointers, and call the user's function (optfunc)
 * to process the array.  We return -1 if there's a problem parsing buf,
 * else we return whatever optfunc() returns.  Note that user's buf[]
 * array is modified (nulls placed after each token).
 */
int buf_args(char* buf, int optfunc(int, char**)) {
  char *ptr, *argv[MAXARGC];
  int argc;

  if (strtok(buf, WHITE) == NULL) /* an argv[0] is required */
    return (-1);
  argv[argc = 0] = buf;
  while ((ptr = strtok(NULL, WHITE)) != NULL) {
    if (++argc >= MAXARGC - 1) /* -1 for room for NULL at end */
      return (-1);
    argv[argc] = ptr;
  }
  argv[++argc] = NULL;

  /*
   * Since argv[] pointers point into the user's buf[],
   * user's function can just copy the pointers, even
   * though argv[] array will disappear on return.
   */
  return optfunc(argc, argv);
}

/*
 * This function is called by buf_args(), which is called by handle_request().
 * buf_args() has broken up the client’s buffer into an argv[]-style array,
 * which we now process.
 */
int cli_args(int argc, char* argv[]) {
  if (argc != 3 || strcmp(argv[0], CL_OPEN) != 0) {
    strcpy(errmsg, "usage: open <oflag>\n");
    return (-1);
  }
  pathname = argv[1];
  oflag = atoi(argv[2]);
  return (0);
}

/* alloc more entries in the client[] array */
void client_alloc(void) {
  int i;

  if (client == NULL) {
    client = malloc(NALLOC * sizeof(Client));
  } else {
    client = realloc(client, (client_size + NALLOC) * sizeof(Client));
  }

  if (client == NULL) {
    err_sys("can’t alloc for client array");
  }

  /* initialize the new entries */
  for (i = client_size; i < client_size + NALLOC; i++) {
    client[i].fd = -1; /* fd of -1 means entry available */
  }

  client_size += NALLOC;
}

/*
 * Called by loop() when connection request from a new client arrives.
 */
int client_add(int fd, uid_t uid) {
  int i;

  /* first time we’re called */
  if (client == NULL) {
    client_alloc();
  }

again:
  for (i = 0; i < client_size; i++) {
    if (client[i].fd == -1) { /* find an available entry */
      client[i].fd = fd;
      client[i].uid = uid;
      return (i); /* return index in client[] array */
    }
  }
  /* client array full, time to realloc for more */
  client_alloc();
  goto again; /* and search again (will work this time) */
}

/*
 * Called by loop() when we’re done with a client.
 */
void client_del(int fd) {
  int i;

  for (i = 0; i < client_size; i++) {
    if (client[i].fd == fd) {
      client[i].fd = -1;
      return;
    }
  }

  log_quit("can’t find client entry for fd %d", fd);
}

void pollfd_alloc(void) {
  int i;

  if (pollfd == NULL) {
    pollfd = malloc(NALLOC * sizeof(struct pollfd));
  } else {
    pollfd = realloc(pollfd, (pollfd_size + NALLOC) * sizeof(struct pollfd));
  }

  if (pollfd == NULL) {
    err_sys("can’t alloc for pollfd array");
  }

  /* initialize the new entries */
  for (i = pollfd_size; i < pollfd_size + NALLOC; i++) {
    pollfd[i].fd = -1; /* fd of -1 means entry available */
  }

  pollfd += NALLOC;
}

int pollfd_add(int fd) {
  int i;

  /* first time we’re called */
  if (pollfd == NULL) {
    pollfd_alloc();
  }

again:
  for (i = 0; i < pollfd_size; i++) {
    if (pollfd[i].fd == -1) { /* find an available entry */
      pollfd[i].fd = fd;
      return (i);
    }
  }
  pollfd_alloc();
  goto again;
}

void pollfd_del(int fd) {
  int i;

  for (i = 0; i < pollfd_size; i++) {
    if (pollfd[i].fd == fd) {
      pollfd[i].fd = -1;
      return;
    }
  }

  log_quit("can’t find pollfd entry for fd %d", fd);
}

/*
 * Pass a file descriptor to another process.
 * If fd<0, then -fd is sent back instead as the error status.
 */
int send_fd(int fd, int fd_to_send) {
  struct iovec iov[1];
  struct msghdr msg;
  char buf[2]; /* send_fd()/recv_fd() 2-byte protocol */
  struct CREDSTRUCT* cred_ptr;

  iov[0].iov_base = buf;
  iov[0].iov_len = 2;
  msg.msg_iov = iov;
  msg.msg_iovlen = 1;
  msg.msg_name = NULL;
  msg.msg_namelen = 0;

  if (fd_to_send < 0) {
    msg.msg_control = NULL;
    msg.msg_controllen = 0;
    buf[1] = -fd_to_send; /* nonzero status means error */
    if (buf[1] == 0)
      buf[1] = 1; /* -256, etc. would screw up protocol */
  } else {
    if (cmsg_ptr == NULL && (cmsg_ptr = malloc(CONTROLLEN)) == NULL) {
      return (-1);
    }

    msg.msg_control = cmsg_ptr;
    msg.msg_controllen = CONTROLLEN;

    cmsg_ptr->cmsg_level = SOL_SOCKET;
    cmsg_ptr->cmsg_type = SCM_RIGHTS;
    cmsg_ptr->cmsg_len = RIGHTSLEN;
    *(int*)CMSG_DATA(cmsg_ptr) = fd_to_send; /* the fd to pass */

#ifdef SEND_CRED
    cmsg_ptr = CMSG_NXTHDR(&msg, cmsg_ptr);
    cmsg_ptr->cmsg_level = SOL_SOCKET;
    cmsg_ptr->cmsg_type = SCM_CREDTYPE;
    cmsg_ptr->cmsg_len = CREDSLEN;
    cred_ptr = (struct CREDSTRUCT*)CMSG_DATA(cmsg_ptr);
    cred_ptr->CR_UID = geteuid();
#endif

    buf[1] = 0; /* zero status means OK */
  }

  buf[0] = '\0'; /* null byte flag to recv_fd() */
  if (sendmsg(fd, &msg, 0) != 2)
    return (-1);
  return (0);
}

/*
 * Used when we had planned to send an fd using send_fd(),
 * but encountered an error instead.  We send the error back
 * using the send_fd()/recv_fd() protocol.
 */
int send_err(int fd, int errcode, const char* msg) {
  int n;

  if ((n = strlen(msg)) > 0)
    if (writen(fd, msg, n) != n) /* send the error message */
      return (-1);

  if (errcode >= 0)
    errcode = -1; /* must be negative */

  if (send_fd(fd, errcode) < 0)
    return (-1);

  return (0);
}
