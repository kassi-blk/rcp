#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "consts.h"
#include "types.h"

static int
rfs_str_cb (const void * args [])
{
    const char * buf = *args;
    ssize_t bytes_read = *(ssize_t *) *(args + 1);

    if (strncmp(buf + bytes_read - SEP_END_LEN, SEP_END, SEP_END_LEN) == 0)
        return 1;
    return 0;
}

static int
rfs_bytes_cb (const void * args [])
{
    ssize_t bytes_read = *(ssize_t *) *args;
    size_t size = *(ssize_t *) *(args + 1);

    return bytes_read == size;
}

int
read_from_socket (int fd, void * buf, ssize_t size)
{
    ssize_t bytes_read = 0, retcode;
    cond_t cond;
    const void * args [2];

    if (size == 0) {
        cond = rfs_str_cb;
        args[0] = buf;
        args[1] = &bytes_read;
    }
    else {
        cond = rfs_bytes_cb;
        args[0] = &bytes_read;
        args[1] = &size;
    }

    memset(buf, 0, BUFSIZE);
    do {
        bytes_read += retcode = read(fd, buf + bytes_read, BUFSIZE);
        if (retcode == -1) {
            if (errno == EPIPE)
                return RFS_ERR_CON;
            return RFS_ERR_READ;
        }
        else if (retcode == 0)
            return RFS_ERR_CON;
    }
    while (!cond(args));

    return 0;
}

int
write_to_socket (int fd, void * buf, ssize_t len, const char * fmt, ...)
{
    ssize_t bytes_written = 0, retcode;

    if (fmt != NULL) {
        va_list ap;

        va_start(ap, fmt);
        len = vsnprintf(NULL, 0, fmt, ap);
        va_end(ap);

        memset(buf, 0, BUFSIZE);
        va_start(ap, fmt);
        vsnprintf((char *) buf, len + 1, fmt, ap);
        va_end(ap);
        // The buffer contains a character array
    }

    do {
        bytes_written += retcode = write(fd, buf + bytes_written,
            len - bytes_written);
        if (retcode == -1) {
            if (errno == EPIPE)
                return WTS_ERR_CON;
            return WTS_ERR_WRITE;
        }
    }
    while (bytes_written != len);

    return 0;
}
