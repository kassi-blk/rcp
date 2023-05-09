#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "transfer.h"
#include "free_context.h"

#define IDSIZE 32

int
main (int argc, char * argv [])
{
    char * hostname;
    char * src_path;
    char * dst_path;
    int sfd = -1, archfd = -1;
    struct addrinfo * addrlist, * p;
    struct sockaddr servaddr;
    char * buf = NULL, * buf_begin;
    const char * token;
    char * archid = NULL;
    char archname [IDSIZE + 8] = {'\0'};
    int retcode;
    struct free_context fc = {0};

    fc_add(&sfd, FC_FD, &fc);
    fc_add(&archfd, FC_FD, &fc);
    fc_add(buf, FC_BUF, &fc);
    fc_add(archid, FC_BUF, &fc);

    argc--;
    if (argc != 2) {
        char * basename = strrchr(argv[0], '/');

        basename++;
        printf("Usage: %s hostname:src_path dst_path\n", basename);
        exit_free(&fc, EXIT_FAILURE);
    }
    hostname = strsep(&argv[1], ":");
    src_path = strsep(&argv[1], ":");
    dst_path = argv[2];

    if (getaddrinfo(hostname, "1230", NULL, &addrlist) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(errno));
        exit_free(&fc, EXIT_FAILURE);
    }

    for (p = addrlist; p != NULL; p = addrlist->ai_next) {
        sfd = socket(PF_INET, SOCK_STREAM, 0);
        if (sfd == -1) {
            perror("socket");
            exit_free(&fc, EXIT_FAILURE);
        }
        break;
    }
    if (p == NULL) {
        printf("Failed to connect to '%s'\n", hostname);
        exit_free(&fc, EXIT_FAILURE);
    }
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr = *p->ai_addr;
    freeaddrinfo(addrlist);

    while (connect(sfd, &servaddr, sizeof(servaddr)) == -1) {
        printf("Failed to connect to the server. Reconnecting...\n");
        close(sfd);
        sleep(5);
        sfd = socket(PF_INET, SOCK_STREAM, 0);
        if (sfd == -1) {
            perror("socket");
            exit_free(&fc, EXIT_FAILURE);
        }
    }

    buf = calloc(BUFSIZE + 1, sizeof(char));
    if (buf == NULL) {
        perror("calloc");
        exit_free(&fc, EXIT_FAILURE);
    }

    // Send the source path of the target
    retcode = write_to_socket(sfd, buf, 0, REQ_PATH_FMT, src_path);
    if (retcode == WTS_ERR_WRITE) {
        perror("write_to_socket");
        exit_free(&fc, EXIT_FAILURE);
    }
    else if (retcode == WTS_ERR_CON) {
        printf("The connection has been closed by the server\n");
        exit_free(&fc, EXIT_FAILURE);
    }

    // Receive an information in which a format to process the archive
    retcode = read_from_socket(sfd, buf, 0);
    if (retcode == RFS_ERR_READ) {
        perror("read_from_socket");
        exit_free(&fc, EXIT_FAILURE);
    }
    else if (retcode == RFS_ERR_CON) {
        printf("The connection has been closed by the server\n");
        exit_free(&fc, EXIT_FAILURE);
    }

    buf_begin = buf;
    if (strncmp(buf, REQ_FAIL_FMT, strlen(REQ_FAIL_FMT) - strlen(SEP_END)) == 0) {
        printf("Failed to receive the target from '%s'\n", hostname);
        exit_free(&fc, EXIT_FAILURE);
    }
    archid = strdup(strsep(&buf, SEP_SEP));
    buf++;
    token = strsep(&buf, SEP_SEP);
    buf = buf_begin;
    if (*token != '\0') {
        size_t archsize;

        archsize = strtoul(token, NULL, 0);
        buf = realloc(buf, archsize);

        retcode = read_from_socket(sfd, buf, archsize);
        if (retcode == RFS_ERR_READ) {
            perror("read_from_socket");
            exit_free(&fc, EXIT_FAILURE);
        }
        else if (retcode == RFS_ERR_CON) {
            printf("The connection has been closed by the server\n");
            exit_free(&fc, EXIT_FAILURE);
        }

        snprintf(archname, sizeof(archname), "%s.tar", archid);
        archfd = open(archname, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
        if (archfd == -1) {
            perror("open");
            exit_free(&fc, EXIT_FAILURE);
        }
        if (write(archfd, buf, archsize) == -1) {
            perror("write");
            exit_free(&fc, EXIT_FAILURE);
        }
        close(archfd);
    }

    // Send the answer
    retcode = write_to_socket(sfd, buf, 0, REQ_OK_FMT);
    if (retcode == WTS_ERR_WRITE) {
        perror("write_to_socket");
        exit_free(&fc, EXIT_FAILURE);
    }

    snprintf(buf, BUFSIZE, "./scripts/extract.sh %s %s %s",
        archid, src_path, dst_path);
    system(buf);

    exit_free(&fc, EXIT_SUCCESS);
}