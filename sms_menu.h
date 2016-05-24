#ifndef _SMS_MENU_H_
#define _SMS_MENU_H_
#include <stdlib.h>
#include <string.h>
#include "utils.h"

typedef char *(*SMSMENUFUNC)(char *output, char *arg);

typedef struct sms_menu_t {
    void *items;                                        // �˵����б�
    unsigned int idx_len;                               // �˵�������ų���
    int item_count;                                     // �˵�������
    const char *prefix_tip;                             // �˵�ǰ�����ʾ
    const char *suffix_tip;                             // �˵��������ʾ
} SMSMENU;

typedef struct sms_menu_item_t {
    const char *help;                                   // ����
    SMSMENUFUNC func;                                   // �˵�������
    SMSMENU *submenu;                                   // �Ӳ˵�
} SMSMENUITEM;

// ����һ���˵�����ʼ
#define SMSMENU_START(name, idxlen)             \
    static SMSMENUITEM name##Items[] = {

// ����һ��˵���
#define ADD_SMSMENUITEM(help, func)             \
    { help, func, NULL },

// ����һ�������Ӳ˵��Ĳ˵���
#define ADD_SMSSUBMENUITEM(help, sub)           \
    { help, NULL, &sub },

// ����һ���˵��Ľ���
#define SMSMENU_END(name, idxlen, ptip, stip)   \
    };                                          \
    static SMSMENU name = { name##Items, idxlen, ARRAY_SIZE(name##Items), ptip, stip };

SMSMENU *get_top_menu();
char *exec_smscommand(SMSMENU *menu, char *output, char *sms, const char *prefix);

#endif//_SMS_MENU_H_

