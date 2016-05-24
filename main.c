#include "telLibrary.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"
#include "sms_menu.h"

extern int send_and_recv_via_udp(const char *ip, unsigned short port, const char *sendString, char *recvString);
static int get_temp_data(char out[1024])
{
    return send_and_recv_via_udp("127.0.0.1", 9600, "qt", out);
}
static int get_humi_data(char out[1024])
{
    return send_and_recv_via_udp("127.0.0.1", 9600, "qh", out);
}
static int get_light_data(char out[1024])
{
    return send_and_recv_via_udp("127.0.0.1", 9600, "ql", out);
}
static int get_sec_data(char out[1024])
{
    return send_and_recv_via_udp("127.0.0.1", 9600, "qp", out);
}
static int get_temp_threshold_data(char out[1024])
{
    return send_and_recv_via_udp("127.0.0.1", 9600, "qT", out);
}
static int get_humi_threshold_data(char out[1024])
{
    return send_and_recv_via_udp("127.0.0.1", 9600, "qH", out);
}
static int get_light_threshold_data(char out[1024])
{
    return send_and_recv_via_udp("127.0.0.1", 9600, "qL", out);
}
static int set_temp_threshold_data(const char *value)
{
    char tmp[1024] = "cT";
    strcat(tmp, value);
    return send_and_recv_via_udp("127.0.0.1", 9600, tmp, tmp);
}
static int set_humi_threshold_data(const char *value)
{
    char tmp[1024] = "cH";
    strcat(tmp, value);
    return send_and_recv_via_udp("127.0.0.1", 9600, tmp, tmp);
}
static int set_light_threshold_data(const char *value)
{
    char tmp[1024] = "cL";
    strcat(tmp, value);
    return send_and_recv_via_udp("127.0.0.1", 9600, tmp, tmp);
}
static int fanCtrl(int on_off)
{
    char tmp[1024];
    sprintf(tmp, "cm0");
    send_and_recv_via_udp("127.0.0.1", 9600, tmp, tmp);
    sprintf(tmp, "cf%d", !!on_off);
    return send_and_recv_via_udp("127.0.0.1", 9600, tmp, tmp);
}
static int heaterCtrl(int on_off)
{
    char tmp[1024];
    sprintf(tmp, "cm0");
    send_and_recv_via_udp("127.0.0.1", 9600, tmp, tmp);
    sprintf(tmp, "ce%d", !!on_off);
    return send_and_recv_via_udp("127.0.0.1", 9600, tmp, tmp);
}
static int humiCtrl(int on_off)
{
    char tmp[1024];
    sprintf(tmp, "cm0");
    send_and_recv_via_udp("127.0.0.1", 9600, tmp, tmp);
    sprintf(tmp, "cu%d", !!on_off);
    return send_and_recv_via_udp("127.0.0.1", 9600, tmp, tmp);
}
static int autoCtrl(int on_off)
{
    char tmp[1024];
    sprintf(tmp, "cm%d", !!on_off);
    return send_and_recv_via_udp("127.0.0.1", 9600, tmp, tmp);
}

static char *do_query(char *output, char *arg)
{
    char sensor_data[4][100] = {""};
    get_temp_data(sensor_data[0]);
    get_humi_data(sensor_data[1]);
    get_light_data(sensor_data[2]);
    get_sec_data(sensor_data[3]);
    sprintf(output, "智能温室大棚系统\n温度:%s\n湿度:%s\n光照:%s\n安防:%s\n"
        , sensor_data[0], sensor_data[1], sensor_data[2], sensor_data[3]);
    return output;
}

static char *skip_blank(char *str)
{
    char *p = str;
    while((*p == ' ') || (*p == '\t') || (*p == '+') || (*p == ','))
        p++;
    return p;
}

static char *do_humCtrl0(char *output, char *arg)
{
    humiCtrl(0);
    return NULL;
}
static char *do_humCtrl1(char *output, char *arg)
{
    humiCtrl(1);
    return NULL;
}

static char *do_heaterCtrl0(char *output, char *arg)
{
    heaterCtrl(0);
    return NULL;
}

static char *do_heaterCtrl1(char *output, char *arg)
{
    heaterCtrl(1);
    return NULL;
}

static char *do_fanCtrl0(char *output, char *arg)
{
    fanCtrl(0);
    return NULL;
}

static char *do_fanCtrl1(char *output, char *arg)
{
    fanCtrl(1);
    return NULL;
}

static char *do_autoMode(char *output, char *arg)
{
    autoCtrl(1);
    return NULL;
}

static char *do_setHumThreshold(char *output, char *arg)
{
    arg = skip_blank(arg);
    set_humi_threshold_data(arg);
    return NULL;
}

static char *do_setTemThreshold(char *output, char *arg)
{
    arg = skip_blank(arg);
    set_temp_threshold_data(arg);
    return NULL;
}

static char *do_setLxThreshold(char *output, char *arg)
{
    arg = skip_blank(arg);
    set_light_threshold_data(arg);
    return NULL;
}

SMSMENU_START(greenHouseMenu, 2)
    ADD_SMSMENUITEM("状态查询  ", do_query)
    ADD_SMSMENUITEM("关加湿器", do_humCtrl0)
    ADD_SMSMENUITEM("开加湿器", do_humCtrl1)
    ADD_SMSMENUITEM("关加热器", do_heaterCtrl0)
    ADD_SMSMENUITEM("开加热器", do_heaterCtrl1)
    ADD_SMSMENUITEM("关风扇", do_fanCtrl0)
    ADD_SMSMENUITEM("开风扇", do_fanCtrl1)
    ADD_SMSMENUITEM("自动模式", do_autoMode)
    ADD_SMSMENUITEM("设置湿度", do_setHumThreshold)
    ADD_SMSMENUITEM("设置温度", do_setTemThreshold)
    ADD_SMSMENUITEM("设置光照", do_setLxThreshold)
SMSMENU_END(greenHouseMenu, 2, NULL, "设置项请在序号后+设定值")

int main(int argc, char *argv[])
{
    int res = 0;
    if((res = init_gprs()) != 0)
    {
        printf("res = %d\n", res);
        perror("init");
        exit(1);
    }
    printf("System started!\n");
    while(1)
    {
        ATMSG msg;
        if(gprs_service_loop(&msg) == 0)
        {
            switch(msg.type)
            {
            case CALLREADY:
                init_gprs();
                break;
            case MSGCONTENT:
                {
                    char sendback[1024]="";
                    TEXTMSG *sms = &(msg.body.textmsg);
                    printf("recv a new msg from %s: %s\n", sms->phone, sms->content);
                    char *back = exec_smscommand(&greenHouseMenu, sendback, sms->content, NULL);
                    if(back)
                    {
                        send_long_sms(sms->phone, back);
                    }
                }
                break;
            case CALLIN:
                if(strlen(msg.body.string) > 0)
                {
                    printf("........call by %s\n", msg.body.string);
                    hangup_call();
                    char sendback[1024]="";
                    char *back = exec_smscommand(&greenHouseMenu, sendback, "", NULL);
                    if(back)
                    {
                        send_long_sms(msg.body.string, back);
                    }
                }
                break;
            default:
                break;
            }
        }
    }
    return 0;
}

