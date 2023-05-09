#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include "transfer.h"
#include "free_context.h"

#define LISTEN_BACKLOG 10
#define IDSIZE 32

int
main (void)
{
    int servfd = -1, clnfd = -1;
    socklen_t clnaddr_len;
    char * buf = NULL, * buf_begin;
    struct sockaddr_in servaddr, clnaddr;
    char clnaddr_str [INET_ADDRSTRLEN + 1] = {'\0'};
    int i, retcode;
    char archid [IDSIZE + 1] = {'\0'};
    int archfd;
    char * path = NULL;
    struct stat statbuf;
    struct free_context fc = {0};

    fc_add(&servfd, FC_FD, &fc);
    fc_add(&clnfd, FC_FD, &fc);
    fc_add(&archfd, FC_FD, &fc);
    fc_add(buf, FC_BUF, &fc);
    fc_add(path, FC_BUF, &fc);

    servfd = socket(PF_INET, SOCK_STREAM, 0);
    if (servfd == -1) {
        perror("socket");
        exit_free(&fc, EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = PF_INET;
    servaddr.sin_port = htons(1230);

    if (bind(servfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) == -1) {
        perror("bind");
        exit_free(&fc, EXIT_FAILURE);
    }

    if (listen(servfd, LISTEN_BACKLOG) == -1) {
        perror("listen");
        exit_free(&fc, EXIT_FAILURE);
    }

    buf = calloc(BUFSIZE + 1, sizeof(char));
    if (buf == NULL) {
        perror("calloc");
        exit_free(&fc, EXIT_FAILURE);
    }

    while (1) {
        printf("Waiting for an incoming connection...\n");

        clnaddr_len = sizeof(clnaddr);
        clnfd = accept(servfd, (struct sockaddr *) &clnaddr, &clnaddr_len);
        if (clnfd == -1) {
            perror("accept");
            exit_free(&fc, EXIT_FAILURE);
        }

        inet_ntop(PF_INET, &clnaddr.sin_addr, clnaddr_str, INET_ADDRSTRLEN);
        printf("The new connection has been arrived from '%s'\n", clnaddr_str);

        srand(time(NULL));
        for (i = 0; i < IDSIZE; i++)
            *(archid + i) = rand() % ('z' - 'a') + 'a';

        // Receive the source path of the target
        retcode = read_from_socket(clnfd, buf, 0);
        if (retcode == RFS_ERR_READ) {
            perror("read_from_socket");
            exit_free(&fc, EXIT_FAILURE);
        }
        else if (retcode == RFS_ERR_CON) {
            printf("The connection has been closed by the client\n");
            close(clnfd);
            continue;
        }

        buf_begin = buf;
        path = strdup(strsep(&buf, SEP_SEP));
        buf = buf_begin;
        snprintf(buf, BUFSIZE, "./scripts/archive.sh %s %s", archid, path);
        if (system(buf) != 0) {
            // Send the failure status
            retcode = write_to_socket(clnfd, buf, 0, REQ_FAIL_FMT);
            if (retcode == WTS_ERR_WRITE) {
                perror("write_to_socket");
                exit_free(&fc, EXIT_FAILURE);
            }
            close(clnfd);
            continue;
        }

        if (clnaddr.sin_addr.s_addr == htonl(INADDR_LOOPBACK)) {
            // Send the only ID of the archive, because the archive
            // also has been created on the remote machine
            retcode = write_to_socket(clnfd, buf, 0, REQ_ARCH_FMT, archid);
            if (retcode == WTS_ERR_WRITE) {
                perror("write_to_socket");
                exit_free(&fc, EXIT_FAILURE);
            }
            else if (retcode == WTS_ERR_CON) {
                printf("The connection has been closed by the client\n");
                close(clnfd);
                continue;
            }
        }
        else {
            archfd = open(archid, O_RDONLY);
            if (archfd == -1) {
                perror("open");
                exit_free(&fc, EXIT_FAILURE);
            }
            if (fstat(archfd, &statbuf) == -1) {
                perror("fstat");
                exit_free(&fc, EXIT_FAILURE);
            }

            // Send the request to receive the full data about the archive
            retcode = write_to_socket(clnfd, buf, 0, REQ_ARCH_FULL_FMT,
                archid, statbuf.st_size);
            if (retcode == WTS_ERR_WRITE) {
                perror("write_to_socket");
                exit_free(&fc, EXIT_FAILURE);
            }
            else if (retcode == WTS_ERR_CON) {
                printf("The connection has been closed by the client\n");
                close(clnfd);
                continue;
            }

            buf = realloc(buf, statbuf.st_size);
            if (read(archfd, buf, statbuf.st_size) == -1) {
                perror("read");
                exit_free(&fc, EXIT_FAILURE);
            }
            close(archfd);

            // Send the entire archive
            retcode = write_to_socket(clnfd, buf, statbuf.st_size, NULL);
            if (retcode == WTS_ERR_WRITE) {
                perror("write_to_socket");
                exit_free(&fc, EXIT_FAILURE);
            }
            else if (retcode == WTS_ERR_CON) {
                printf("The connection has been closed by the client\n");
                close(clnfd);
                continue;
            }

            snprintf(buf, BUFSIZE, "rm %s", archid);
            system(buf);
        }

        // Receive the answer to make sure all the data has been deliveried
        retcode = read_from_socket(clnfd, buf, 0);
        if (retcode == RFS_ERR_READ) {
            perror("read_from_socket");
            exit_free(&fc, EXIT_FAILURE);
        }
        else if (retcode == RFS_ERR_CON) {
            printf("The connection has been closed by the client\n");
            close(clnfd);
            continue;
        }

        // The socket of the client can be closed now
        close(clnfd);
    };

    exit_free(&fc, EXIT_SUCCESS);
}
