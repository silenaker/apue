#include <error.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

/* one handler for both signals */
static void sig_usr(int);

int main(void) {
  if (signal(SIGUSR1, sig_usr) == SIG_ERR) {
    err_sys("can't catch SIGUSR1");
  }

  if (signal(SIGUSR2, sig_usr) == SIG_ERR) {
    err_sys("can't catch SIGUSR2");
  }

  printf("pid: %d\n", getpid());

  for (;;) {
    pause();
  }
}

/* argument is signal number */
static void sig_usr(int signo) {
  switch (signo) {
    case SIGUSR1:
      printf("received SIGUSR1\n");
      break;
    case SIGUSR2:
      printf("received SIGUSR2\n");
      break;
    default:
      err_dump("received signal %d\n", signo);
  }
}
