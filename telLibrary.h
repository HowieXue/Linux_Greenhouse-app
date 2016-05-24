#ifndef _TELLIBRARY_H_
#define _TELLIBRARY_H_
#include "serial.h"
#include "uni.h"

#define PHONEN_MAXLEN       40
#define NAME_MAXLEN         20
#define SMSCONTENT_MAXLEN   120

typedef enum {
    PWRDOWN,                            // �ػ�
    CALLREADY,                          // ϵͳ�������
    NONE,                               // DUMMY
    OK,                                 // OK
    HANGUP,                             // �һ�
    ERROR,                              // ����
    MSGSENDED,                          // ���ŷ������
    CALLIN,                             // �е绰����
    MSGRECVED,                          // ���Ż�ִ
    NEWMSG,                             // �¶���(�洢��SIM��)
    NEWMSG2,                            // �¶���
    COPS,                               // ��Ӫ��
    MSGCOUNT,                           // ��������
    MSGCONTENT,                         // ��������
    PEEKMSG,                            // ��ѯSIM���ڵĶ���
    RAW,                                // �޷�ʶ��
    WAITINPUT,                          // �ȴ�����
    PBLIST,                             // �绰���洢���б�
    PBSIZE                              // �绰������
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
// ���øú����Ա�
int gprs_service_loop(ATMSG *msg);

int pb_get_storage(char list[][10]);
int pb_select_storage(const char *storage);

#endif//_TELLIBRARY_H_

