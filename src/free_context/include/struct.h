#ifndef _FREE_CONTEXT_STRUCT_H

struct free_context {
    int * fds;
    unsigned fds_len;
    void ** bufs;
    unsigned bufs_len;
};

#define _FREE_CONTEXT_STRUCT_H
#endif