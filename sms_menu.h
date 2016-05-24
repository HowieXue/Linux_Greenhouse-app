#ifndef _SMS_MENU_H_
#define _SMS_MENU_H_
#include <stdlib.h>
#include <string.h>
#include "utils.h"

typedef char *(*SMSMENUFUNC)(char *output, char *arg);

typedef struct sms_menu_t {
    void *items;                                        // 菜单项列表
    unsigned int idx_len;                               // 菜单引导序号长度
    int item_count;                                     // 菜单项数量
    const char *prefix_tip;                             // 菜单前面的提示
    const char *suffix_tip;                             // 菜单后面的提示
} SMSMENU;

typedef struct sms_menu_item_t {
    const char *help;                                   // 帮助
    SMSMENUFUNC func;                                   // 菜单处理函数
    SMSMENU *submenu;                                   // 子菜单
} SMSMENUITEM;

// 定义一个菜单的起始
#define SMSMENU_START(name, idxlen)             \
    static SMSMENUITEM name##Items[] = {

// 增加一项菜单项
#define ADD_SMSMENUITEM(help, func)             \
    { help, func, NULL },

// 增加一个带有子菜单的菜单项
#define ADD_SMSSUBMENUITEM(help, sub)           \
    { help, NULL, &sub },

// 定义一个菜单的结束
#define SMSMENU_END(name, idxlen, ptip, stip)   \
    };                                          \
    static SMSMENU name = { name##Items, idxlen, ARRAY_SIZE(name##Items), ptip, stip };

SMSMENU *get_top_menu();
char *exec_smscommand(SMSMENU *menu, char *output, char *sms, const char *prefix);

#endif//_SMS_MENU_H_

