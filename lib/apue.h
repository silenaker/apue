#ifndef _APUE_H
#define _APUE_H

#include <unistd.h>

typedef void Sigfunc(int);

char* path_alloc(size_t*);
long open_max(void);

int set_cloexec(int);
void clr_fl(int, int);
void set_fl(int, int);

void pr_exit(int);

void pr_mask(const char*);
Sigfunc* signal_intr(int, Sigfunc*);

void daemonize(const char*);

void sleep_us(unsigned int);
ssize_t readn(int, void*, size_t);
ssize_t writen(int, const void*, size_t);

int tty_cbreak(int);
int tty_raw(int);
int tty_reset(int);
void tty_atexit(void);
struct termios* tty_termios(void);

int ptym_open(char*, int);
int ptys_open(char*);

int lock_reg(int, int, int, off_t, int, off_t);

#define read_lock(fd, offset, whence, len) \
  lock_reg((fd), F_SETLK, F_RDLCK, (offset), (whence), (len))
#define readw_lock(fd, offset, whence, len) \
  lock_reg((fd), F_SETLKW, F_RDLCK, (offset), (whence), (len))
#define write_lock(fd, offset, whence, len) \
  lock_reg((fd), F_SETLK, F_WRLCK, (offset), (whence), (len))
#define writew_lock(fd, offset, whence, len) \
  lock_reg((fd), F_SETLKW, F_WRLCK, (offset), (whence), (len))
#define un_lock(fd, offset, whence, len) \
  lock_reg((fd), F_SETLK, F_UNLCK, (offset), (whence), (len))

pid_t lock_test(int, int, off_t, int, off_t);

#define is_read_lockable(fd, offset, whence, len) \
  (lock_test((fd), F_RDLCK, (offset), (whence), (len)) == 0)
#define is_write_lockable(fd, offset, whence, len) \
  (lock_test((fd), F_WRLCK, (offset), (whence), (len)) == 0)

#endif