#ifndef _TRANSFER_H

#include "consts.h"
#include "types.h"

int read_from_socket (int fd, void * buf, ssize_t size);
int write_to_socket (int fd, void * buf, ssize_t len, const char * fmt, ...);

#define _TRANSER_H
#endif
