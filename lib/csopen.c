#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <unistd.h>
#include "error.h"
#include "open.h"
#include "socket.h"

static struct cmsghdr* cmsg_ptr = NULL;

int recv_fd(int, uid_t*, ssize_t(int, const void*, size_t));

/*
 * Open the file by sending the "name" and "oflag" to the connection server
 * and reading a file descriptor back.
 */
int csopen(char* name, int oflag, uid_t* uid_ptr) {
  pid_t pid;
  int len;
  char buf[10];
  struct iovec iov[3];
  static int fd[2] = {-1, -1};

#ifdef OPEN_BY_UNIX_SOCK_PAIR
  /* fork/exec our open server first time */
  if (fd[0] < 0) {
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, fd) < 0) {
      err_ret("socketpair error");
      return (-1);
    }

    if ((pid = fork()) < 0) {
      err_ret("fork error");
      return (-1);
    } else if (pid == 0) {
#ifdef DEBUG
      pause();
#endif
      close(fd[0]);
      if (fd[1] != STDIN_FILENO && dup2(fd[1], STDIN_FILENO) != STDIN_FILENO) {
        err_sys("dup2 error to stdin");
      }
      if (fd[1] != STDOUT_FILENO &&
          dup2(fd[1], STDOUT_FILENO) != STDOUT_FILENO) {
        err_sys("dup2 error to stdout");
      }
      if (execl("opend", "opend", (char*)0) < 0) {
        err_sys("execl opend error");
      }
    }
    close(fd[1]);
  }
#endif

#ifdef OPEN_BY_UNIX_SOCK_CONN
  if (fd[0] < 0) { /* open connection to conn server */
    if ((fd[0] = cli_conn(CS_OPEN)) < 0) {
      err_ret("cli_conn error");
      return (-1);
    }
  }
#endif

  iov[0].iov_base = CL_OPEN " ";
  iov[0].iov_len = strlen(CL_OPEN) + 1;
  iov[1].iov_base = name;
  iov[1].iov_len = strlen(name);
  sprintf(buf, " %d", oflag);
  iov[2].iov_base = buf;
  iov[2].iov_len = strlen(buf) + 1; /* +1 for null at end of buf */
  len = iov[0].iov_len + iov[1].iov_len + iov[2].iov_len;
  if (writev(fd[0], iov, 3) != len) {
    err_ret("writev error");
    return (-1);
  }
  /* read descriptor, returned errors handled by write() */
  return recv_fd(fd[0], uid_ptr, write);
}

/*
 * Receive a file descriptor from a server process.  Also, any data
 * received is passed to userfunc(STDERR_FILENO, buf, nbytes).
 * We have a 2-byte protocol for receiving the fd from send_fd().
 */
int recv_fd(int fd,
            uid_t* uid_ptr,
            ssize_t userfunc(int, const void*, size_t)) {
  int new_fd, nr, status;
  char* ptr;
  char buf[LINE_MAX];
  struct iovec iov[1];
  struct msghdr msg;
  struct CREDSTRUCT* cred_ptr;

#if defined(CREDOPT)
  const int on = 1;
  if (setsockopt(fd, SOL_SOCKET, CREDOPT, &on, sizeof(int)) < 0) {
    err_ret("setsockopt error");
    return (-1);
  }
#endif

  status = -1;
  for (;;) {
    iov[0].iov_base = buf;
    iov[0].iov_len = sizeof(buf);
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;
    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    if (cmsg_ptr == NULL && (cmsg_ptr = malloc(CONTROLLEN)) == NULL) {
      return (-1);
    }
    msg.msg_control = cmsg_ptr;
    msg.msg_controllen = CONTROLLEN;
    if ((nr = recvmsg(fd, &msg, 0)) < 0) {
      err_ret("recvmsg error");
      return (-1);
    } else if (nr == 0) {
      err_ret("connection closed by server");
      return (-1);
    }

    /*
     * See if this is the final data with null & status.  Null
     * is next to last byte of buffer; status byte is last byte.
     * Zero status means there is a file descriptor to receive.
     */
    for (ptr = buf; ptr < &buf[nr];) {
      if (*ptr++ == '\0') {
        if (ptr != &buf[nr - 1]) {
          err_dump("message format error");
        }
        status = *ptr & 0xFF; /* prevent sign extension */
        if (status == 0) {
          if (msg.msg_controllen != CONTROLLEN) {
            err_dump("status = 0 but no fd");
          }

          for (cmsg_ptr = CMSG_FIRSTHDR(&msg); cmsg_ptr != NULL;
               cmsg_ptr = CMSG_NXTHDR(&msg, cmsg_ptr)) {
            if (cmsg_ptr->cmsg_level != SOL_SOCKET) {
              continue;
            }
            switch (cmsg_ptr->cmsg_type) {
              case SCM_RIGHTS:
                new_fd = *(int*)CMSG_DATA(cmsg_ptr);
                break;
              case SCM_CREDTYPE:
                // credentials
                cred_ptr = (struct CREDSTRUCT*)CMSG_NXTHDR(&msg, cmsg_ptr);
                *uid_ptr = cred_ptr->CR_UID;
              default:
                status = -1;
                err_msg("unexpected control message type: %d",
                        cmsg_ptr->cmsg_type);
            }
          }
        } else {
          status = -status;
        }
        nr -= 2;
      }
    }

    if (nr > 0 && userfunc(STDERR_FILENO, buf, nr) != nr) {
      return (-1);
    }

    if (status < 0) {
      return status;
    } else {
      return new_fd;
    }
  }
}
