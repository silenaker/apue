#ifndef _TELL_WAIT_H
#define _TELL_WAIT_H

#include <unistd.h>

void TELL_WAIT(void);
void TELL_PARENT(pid_t);
void TELL_CHILD(pid_t);
void WAIT_PARENT(void);
void WAIT_CHILD(void);

#endif