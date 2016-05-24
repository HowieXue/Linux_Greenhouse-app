#include "sms_menu.h"
#include <stdio.h>

static char *do_demo1(char *output, char *argv)
{
	sprintf(output, "Demo1 %s\n", argv);
	return output;
}

static char *do_demo2(char *output, char *argv)
{
	sprintf(output, "Demo2 %s\n", argv);
	return output;
}

static char *do_welcome(char *output, char *argv)
{
    sprintf(output, "SunplusAPP GPRS SMS menu demo\n");
    return output;
}

SMSMENU_START(demoSubMenu, 1)
    ADD_SMSMENUITEM("Demo1", do_demo1)
    ADD_SMSMENUITEM("Demo2", do_demo2)
SMSMENU_END(demoSubMenu, 1, NULL, NULL)

SMSMENU_START(demoMenu, 1)
    ADD_SMSSUBMENUITEM("Demo", demoSubMenu)
    ADD_SMSMENUITEM("Welcome", do_welcome)
SMSMENU_END(demoMenu, 1, NULL, NULL)

SMSMENU *get_top_menu() {
    return &demoMenu;
}

static char *print_menu_usage(SMSMENU *menu, char *output, const char *prefix)
{
    int i;
    char *p = output;
    char fmt[10];
    SMSMENUITEM *menu_item = (SMSMENUITEM *)(menu->items);
    sprintf(fmt, "%%0%dd", menu->idx_len);                      // 生成格式化字符串
    if(menu->prefix_tip)
        sprintf(p, "%s", menu->prefix_tip);
    strcat(p, "请回复序号:\n");
    p += strlen(p);
    for(i = 0; i < menu->item_count; i++)
    {
		if(prefix)
		{
			sprintf(p, "%s", prefix);
			p += strlen(p);
		}
        sprintf(p, fmt, i + 1);                                 // 生成序号
        p += strlen(p);
        sprintf(p, ".%s\n", menu_item[i].help);                // 打印帮助
		p += strlen(p);
    }
    if(menu->suffix_tip)
        sprintf(p, "%s", menu->suffix_tip);
    return output;
}

char *exec_smscommand(SMSMENU *menu, char *output, char *sms, const char *prefix)
{
    char *ret = output;
    char idxstr[10] = "";
    char *stopAt = NULL;
    int idx = -1;
    if(menu == NULL)
        menu = get_top_menu();
    // 从sms中取出与当前菜单的序号长度一致的字符串
	if(strlen(sms) >= menu->idx_len)
	{
		strncpy(idxstr, sms, menu->idx_len);
		sms += menu->idx_len;
		// 转换为序号
		idx = strtoul(idxstr, &stopAt, 10);
        idx--;
	}
    // 检测转换是否成功,并且序号合法
    if((stopAt != NULL) && (*stopAt == '\0') && (idx > -1) && (idx < menu->item_count))
    {
        // 取出菜单项
        SMSMENUITEM *menu_items = (SMSMENUITEM*)(menu->items);
        // 检查菜单项是否有对应的处理函数
        if(menu_items[idx].func != NULL)
        {
            // 有,则调用处理函数
            ret = (*menu_items[idx].func)(output, sms);
        }
        else
        {
            // 无,则进入子菜单进一步处理
			char newprefix[10];
			sprintf(newprefix, "%s%s", prefix ? prefix : "", idxstr);
            ret = exec_smscommand(menu_items[idx].submenu, output, sms, newprefix);
        }
    }
    else
    {
        ret = print_menu_usage(menu, output, prefix);                // 找不到匹配的菜单,则打印帮助
    }
    return ret;
}

