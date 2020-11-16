#ifndef _SOCKET_H
#define _SOCKET_H

#include <unistd.h>

int serv_listen(const char*);
int serv_accept(int, uid_t*);
int cli_conn(const char*);

#endif