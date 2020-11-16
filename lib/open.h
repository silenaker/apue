#ifndef _OPEN_H
#define _OPEN_H

#include <sys/un.h>

#if defined(SCM_CREDS) && !defined(__APPLE__) /* BSD interface */
#define CREDSTRUCT cmsgcred
#define CR_UID cmcred_uid
#define SCM_CREDTYPE SCM_CREDS
#elif defined(SCM_CREDENTIALS) /* Linux interface */
#define CREDSTRUCT ucred
#define CR_UID uid
#define CREDOPT SO_PASSCRED
#define SCM_CREDTYPE SCM_CREDENTIALS
#elif defined(__APPLE__) && defined(__MACH__) /* Mac OS X */
#include "sys/ucred.h"
#define CREDSTRUCT xucred
#define CR_UID cr_uid
#define CREDOPT LOCAL_PEERCRED
#define SCM_CREDTYPE SCM_CREDS
#else
#error passing credentials is unsupported!
#endif

// #define DEBUG 1
// #define OPEN_BY_UNIX_SOCK_PAIR 1
#define OPEN_BY_UNIX_SOCK_CONN 1
// #define SEND_CRED 1

/* size of control buffer to send file descriptors and credentials */
#define RIGHTSLEN CMSG_LEN(sizeof(int))
#ifdef SEND_CRED
#define CREDSLEN CMSG_LEN(sizeof(struct CREDSTRUCT))
#define CONTROLLEN RIGHTSLEN + CREDSLEN
#else
#define CONTROLLEN RIGHTSLEN
#endif

#define CL_OPEN "open"         /* client’s request for server */
#define CS_OPEN "opend.socket" /* server’s well-known name */

int csopen(char*, int, uid_t*);

#endif