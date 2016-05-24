#include "serial.h"
#include "lnklst.h"
#include <termios.h>
#include <sys/select.h>
#include <sys/time.h>           //gettimeofday()
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <time.h>
#include <errno.h>

static unsigned int parse_speed(int baudRate)
{
    switch(baudRate)
    {
    case 50:
        return B50;
    case 75:
        return B75;
    case 110:
        return B110;
    case 134:
        return B134;
    case 150:
        return B150;
    case 200:
        return B200;
    case 300:
        return B300;
    case 600:
        return B600;
    case 1200:
        return B1200;
    case 1800:
        return B1800;
    case 2400:
        return B2400;
    case 4800:
        return B4800;
    case 9600:
        return B9600;
    case 19200:
        return B19200;
    case 38400:
        return B38400;
    case 57600:
        return B57600;
    case 115200:
        return B115200;
    case 230400:
        return B230400;
    default:
        return B0;
    }
    return B0;
}

static unsigned int parse_databit(int databit)
{
    switch(databit)
    {
    case 5:
        return CS5;
    case 6:
        return CS6;
    case 7:
        return CS7;
    case 8:
    default:
        return CS8;
    }
    return CS8;
}

static unsigned int parse_parity(char parity)
{
    switch(parity)
    {
    case 'N':
    case 'n':
        return 0;
    case 'O':
    case 'o':
        return PARENB | PARODD;
    case 'E':
    case 'e':
        return PARENB;
    default:
        return 0;
    }
    return 0;
}

static int parse_serial_param_string(const char *param_string, struct termios *op)
{
    // param format:
    // baudrate,data bit,stop bit,parity:
    // for example: 115200,8,1,n  or  115200,,,n to set default data bit = 8, stop bit = 1
    unsigned int baudrate = 115200, databit = 8, stopbit = 1;
    char parity = 'n';
    int ret = sscanf(param_string, "%ud,%ud,%ud,%c", &baudrate, &databit, &stopbit, &parity);
    if(ret < 4)
        return -1;
    if(stopbit != 1)
        return -1;
    baudrate = parse_speed(baudrate);
    if(baudrate == B0)
        return -1;
    cfsetispeed(op, baudrate);
    cfsetospeed(op, baudrate);
    op->c_cflag = (op->c_cflag & ~CSIZE) | parse_databit(databit);
    op->c_cflag &= ~CSTOPB;                // Stop Bit
    op->c_cflag = (op->c_cflag & ~(PARENB | PARODD)) | parse_parity(parity);
    return 0;
}

static LISTHEAD serialMsgList = LISTHEAD_INITIALIZER;
static sem_t serialMsgOpWaiter;
static pthread_mutex_t serialOpLocker = PTHREAD_MUTEX_INITIALIZER;
#define LOCK_SERIAL()                                               \
    do {                                                            \
        pthread_mutex_lock(&serialOpLocker);

#define UNLOCK_SERIAL()                                             \
        pthread_mutex_unlock(&serialOpLocker);                      \
        usleep(1);                                                  \
    } while(0)

static volatile int serialFD = -1;
static volatile int serialMonitorRunning = 0;

static void *serial_monitor(void *param)
{
    PSERIALMSG newMsg = (PSERIALMSG)malloc(sizeof(SERIALMSG));
    memset(newMsg, 0, sizeof(SERIALMSG));
    char *pCurrentPos = newMsg->body;
    serialMonitorRunning = 1;
    while(serialFD >= 0)
    {
        struct timeval to = {
            .tv_sec = 0,
            .tv_usec = 10000,
        };
        fd_set rset;
        FD_ZERO(&rset);
        FD_SET(serialFD, &rset);
        if(select(serialFD + 1, &rset, NULL, NULL, &to) > 0)
        {
            int len = read(serialFD,
                pCurrentPos, 1);
//                &newMsg->body[newMsg->len], 1);
//                &newMsg->body[newMsg->len], sizeof(newMsg->body) - newMsg->len);
            if(len <= 0)
                continue;
            newMsg->len += len;
//            if(newMsg->body[newMsg->len - len] == '\n')
            if(*pCurrentPos == '\n')
            {
                // got a full line
                LOCK_SERIAL();
                List_Appand(&serialMsgList, newMsg);
                UNLOCK_SERIAL();
                //printf("**%s", newMsg->body);
                sem_post(&serialMsgOpWaiter);
                newMsg = (PSERIALMSG)malloc(sizeof(SERIALMSG));
                memset(newMsg, 0, sizeof(SERIALMSG));
                pCurrentPos = newMsg->body;
                len = 0;
            }
            else if(newMsg->len >= sizeof(newMsg->body))
            {
                // line buffer is full. it should not be happen!!!
                printf("!!!!!!!!!!!!!WARNING!!!!!!!!!!!!!!! line buffer full!!!!\n");
            }
            pCurrentPos += len;
        }
    }
    serialMonitorRunning = 0;
    return NULL;
}

int open_serial(const char *devname, const char *fmt)
{
    struct termios option;
    close_serial();
    do {
        pthread_t pt;
        serialFD = open(devname, O_RDWR);
        if(serialFD < 0)
            break;
        tcgetattr(serialFD, &option);
        parse_serial_param_string(fmt, &option);
        option.c_lflag = 0;
        option.c_oflag = 0;
        option.c_iflag = 0;
        tcsetattr(serialFD, TCSANOW, &option);
        if(pthread_create(&pt, NULL, serial_monitor, NULL))
            close_serial();
    } while(0);
    return (serialFD >= 0) ? 0 : -1;
}

int close_serial()
{
    if(serialFD >= 0)
        close(serialFD);
    serialFD = -1;
    while(serialMonitorRunning)
        usleep(10);
    clear_serial();
    return 0;
}

char *read_serial(char *buf)
{
    PSERIALMSG msg = NULL;
    if(sem_wait(&serialMsgOpWaiter))
        return NULL;
    LOCK_SERIAL();
    msg = List_Pop(&serialMsgList);
    UNLOCK_SERIAL();
    strcpy(buf, msg->body);
    free(msg);
    return buf;
}

char *timed_read_serial(char *buf, int to_sec, int to_msec)
{
    int s;
    struct timespec ts;
    struct timeval tv;
//    if(sys_clock_gettime(CLOCK_REALTIME, &ts) == -1)
//        return NULL;
    gettimeofday(&tv, NULL);
    ts.tv_sec = tv.tv_sec + to_sec;
    ts.tv_nsec = tv.tv_usec * 1000 + (to_msec * 1000 * 1000);
    ts.tv_sec += ts.tv_nsec / (1000 * 1000 * 1000);
    ts.tv_nsec %= 1000 * 1000 * 1000;
    while((s=  sem_timedwait(&serialMsgOpWaiter, &ts)) == -1 && (errno == EINTR))
        continue;               // restart when interrupted by handler
    if(s == -1)
        return NULL;
    PSERIALMSG msg = NULL;
    LOCK_SERIAL();
    msg = List_Pop(&serialMsgList);
    UNLOCK_SERIAL();
    strcpy(buf, msg->body);
    free(msg);
    return buf;
}

int clear_serial(void)
{
    PSERIALMSG msg = NULL;
    LOCK_SERIAL();
    while((msg = List_Pop(&serialMsgList)) != NULL)
        free(msg);
    UNLOCK_SERIAL();
	sem_init(&serialMsgOpWaiter, 0, 0);
    return 0;
}

int write_serial(const char *buf, int len)
{
    if(len <= 0)
        len = strlen(buf);
    return write(serialFD, buf, len);
}

