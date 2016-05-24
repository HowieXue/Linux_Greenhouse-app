#ifndef _TELLIBRARY_H_
#define _TELLIBRARY_H_
#include "serial.h"
#include "uni.h"

#define PHONEN_MAXLEN       40
#define NAME_MAXLEN         20
#define SMSCONTENT_MAXLEN   120

typedef enum {
    PWRDOWN,                            // 关机
    CALLREADY,                          // 系统启动完成
    NONE,                               // DUMMY
    OK,                                 // OK
    HANGUP,                             // 挂机
    ERROR,                              // 错误
    MSGSENDED,                          // 短信发送完成
    CALLIN,                             // 有电话打入
    MSGRECVED,                          // 短信回执
    NEWMSG,                             // 新短信(存储在SIM卡)
    NEWMSG2,                            // 新短信
    COPS,                               // 运营商
    MSGCOUNT,                           // 短信数量
    MSGCONTENT,                         // 短信内容
    PEEKMSG,                            // 查询SIM卡内的短信
    RAW,                                // 无法识别
    WAITINPUT,                          // 等待输入
    PBLIST,                             // 电话本存储器列表
    PBSIZE                              // 电话本容量
} ATMSGTYPE;

typedef struct {
	short year;
	char month;
	char day;
} DATE;
typedef struct {
	char hour;
	char minute;
	char second;
	char pad;
} TIME;
typedef struct {
	char phone[PHONEN_MAXLEN+1];
	DATE date;
	TIME time;
	char content[SMSCONTENT_MAXLEN+1];
} TEXTMSG;

typedef struct {
    char phone[PHONEN_MAXLEN+1];
    char name[NAME_MAXLEN+1];
} PHONEBOOK;

typedef struct {
	ATMSGTYPE type;
    union {
    	unsigned long data;
        char string[SERIAL_MAX_LINECOUNT];
        TEXTMSG textmsg;
    } body;
} ATMSG;

// H/W definition
#define GPRS_RESET_PORT     "gpg"
#define GPRS_RESET_BIT      12
#define GPRS_SERIAL_PORT    "s3c2410_serial1"
//#define GPRS_SERIAL_PORT    "ttyS0"

int turn_on_gprs(void);
int turn_off_gprs(void);
int send_gprs_command(const char *cmd);
int init_gprs(void);
int anser_call(void);
int hangup_call(void);
int make_call(const char *phone_number);
int send_dtmf_call(const char *str);
int send_sms(const char *phone_number, const char *msg);
int send_long_sms(const char *phone_number, const char *msg);
int get_inbox_count(void);
int get_sms(TEXTMSG *msgList, int max);
// 调用该函数以便
int gprs_service_loop(ATMSG *msg);

int pb_get_storage(char list[][10]);
int pb_select_storage(const char *storage);

#endif//_TELLIBRARY_H_

