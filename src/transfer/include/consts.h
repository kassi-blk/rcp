#ifndef _TRANSFER_CONSTS_H

#define BUFSIZE 8192

#define SEP_SEP "\r\n"
#define SEP_END "\r\n\r\n"
#define SEP_SEP_LEN 2
#define SEP_END_LEN 4
#define REQ_PATH_FMT "%s"SEP_END
#define REQ_ARCH_FMT "%s"SEP_END
#define REQ_ARCH_FULL_FMT "%s"SEP_SEP"%lu"SEP_END
#define REQ_OK_FMT "OK"SEP_END
#define REQ_FAIL_FMT "FAIL"SEP_END

#define RFS_ERR_READ 1
#define RFS_ERR_CON 2
#define WTS_ERR_WRITE 1
#define WTS_ERR_CON 2

#define _TRANSFER_CONSTS_H
#endif