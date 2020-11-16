#ifndef _LOG_H
#define _LOG_H

void log_msg(const char*, ...);
void log_open(const char*, int, int);
void log_quit(const char*, ...) __attribute__((noreturn));
void log_ret(const char*, ...);
void log_sys(const char*, ...) __attribute__((noreturn));
void log_exit(int, const char*, ...) __attribute__((noreturn));

#endif
