#include "consts.h"
#include "struct.h"
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>

void
fc_add (void * object, int type, struct free_context * fc)
{
    if (type == FC_FD) {
        fc->fds_len++;
        fc->fds = realloc(fc->fds, fc->fds_len * sizeof(int));
        *(fc->fds + fc->fds_len - 1) = *(int *) object;
    }
    else if (type == FC_BUF) {
        fc->bufs_len++;
        fc->bufs = realloc(fc->bufs, fc->bufs_len * sizeof(void *));
        *(fc->bufs + fc->bufs_len - 1) = object;
    }
}

void
exit_free (struct free_context * fc, int status)
{
    int i;

    for (i = 0; i < fc->fds_len; i++)
        if (*(fc->fds + i) != -1)
            close(*(fc->fds + i));
    for (i = 0; i < fc->bufs_len; i++)
        if (*(fc->bufs + i) != NULL)
            free(*(fc->bufs + i));
    free(fc->fds);
    free(fc->bufs);
    exit(status);
}
