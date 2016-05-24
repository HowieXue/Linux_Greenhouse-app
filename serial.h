#ifndef _SERIAL_H_
#define _SERIAL_H_
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define SERIAL_MAX_LINECOUNT 700
typedef struct msg_t {
    struct msg_t *next;
    int len;
    char body[SERIAL_MAX_LINECOUNT];
} SERIALMSG, *PSERIALMSG;

extern int open_serial(const char *devname, const char *fmt);
extern int close_serial();
extern char *read_serial(char *buf);
char *timed_read_serial(char *buf, int to_sec, int to_msec);
extern int clear_serial(void);
extern int write_serial(const char *buf, int len);

#endif//_SERIAL_H_

