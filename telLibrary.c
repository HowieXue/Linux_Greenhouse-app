#include "telLibrary.h"
#include <sys/ioctl.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>

static pthread_mutex_t gprsReadOpLocker = PTHREAD_MUTEX_INITIALIZER;

#define LOCK_READER()                                                   \
    do {                                                                \
        /*printf("**** %s:%d lock!\n", __func__, __LINE__);*/           \
        pthread_mutex_lock(&gprsReadOpLocker);

#define UNLOCK_READER()                                                 \
        pthread_mutex_unlock(&gprsReadOpLocker);                            \
        /*printf("0000 %s:%d unlock!\n", __func__, __LINE__);*/         \
        usleep(1);/* add to force current thread release the mutex*/    \
    } while(0)

int turn_on_gprs(void)
{
    int fd = open("/dev/" GPRS_RESET_PORT, O_RDWR);
    if(fd < 0)
        return -1;
    ioctl(fd, 0x01, GPRS_RESET_BIT);                // set GPG12 output
    ioctl(fd, 0x11, GPRS_RESET_BIT);                // GPG12 output 1
    close(fd);
    return 0;
}

int turn_off_gprs(void)
{
    int fd = open("/dev/" GPRS_RESET_PORT, O_RDWR);
    if(fd < 0)
        return -1;
    ioctl(fd, 0x01, GPRS_RESET_BIT);                // set GPG12 output
    ioctl(fd, 0x10, GPRS_RESET_BIT);                // GPG12 output 0
    close(fd);
    return 0;
}

static ATMSG *parse_gprs_response(char *resp)
{
    static ATMSG lastMsg = { NONE };
    if(strcmp(resp, "OK\r\n") == 0)
        lastMsg.type = OK;
    else if(strcmp(resp, "NORMAL POWER DOWN\r\n") == 0)
        lastMsg.type = PWRDOWN;
    else if(strcmp(resp, "NO CARRIER\r\n") == 0)
        lastMsg.type = HANGUP;
    else if(strcmp(resp, "Call Ready\r\n") == 0)
        lastMsg.type = CALLREADY;
    else if(resp[0] == '>')
        lastMsg.type = WAITINPUT;
    else if(strncmp(resp, "+CME ERROR:", 11) == 0)
    {
        lastMsg.type = ERROR;
        lastMsg.body.data = strtoul(&resp[11], NULL, 10);
    }
    else if(strncmp(resp, "+CMGS:", 6) == 0)
    {
        lastMsg.type = MSGSENDED;
        lastMsg.body.data = strtoul(&resp[6], NULL, 10);
    }
    else if(strncmp(resp, "+CMS ERROR:", 11) == 0)
    {
        lastMsg.type = ERROR;
        lastMsg.body.data = strtoul(&resp[11], NULL, 10);
    }
    else if(strcmp(resp, "RING\r\n") == 0)
    {
        lastMsg.type= CALLIN;
        strcpy(lastMsg.body.string, "");
    }
    else if(strncmp(resp, "+CLIP:", 6) == 0)
    {
        char *p = strtok(resp, ":");
        p = strtok(NULL, ",");
        lastMsg.type = CALLIN;
        sscanf(p, "%*[ \"]%[^\"\r\n]", lastMsg.body.string);
    }
    else if(strncmp(resp, "+CDS:", 5) == 0)
        lastMsg.type = MSGRECVED;
//      else if(strncmp(tmpbuf, "+CMT:", 5) == 0)
    else if(strncmp(resp, "+CMTI:", 6) == 0)
    {
        // new message
        // +CMTI: "SM",2
        char *p = strtok(resp, ",");
        p = strtok(NULL, ",");
        lastMsg.type = NEWMSG;
        lastMsg.body.data = strtoul(p, NULL, 10);
    }
    else if(strncmp(resp, "+CMT:", 5) == 0)
    {
        // new message with message content
        // +CMT: "002B0038003600310033003800310030003400360036003700350033",,"11/04/20,16:32:29+32"
        // 0030
        lastMsg.type = NEWMSG2;
        char tmpPhoneNumber[100];
        int year, month, day, hour, minute, second, pad;
        int ret = sscanf(resp, "+CMT:%*[^\"]\"%[^\"]\",%*[^,],\"%d/%d/%d,%d:%d:%d+%d",
                tmpPhoneNumber, &year, &month, &day, &hour, &minute, &second, &pad);
        if(ret != 8)
        {
            ret = sscanf(resp, "+CMT:%*[^\"]\"%[^\"]\",,\"%d/%d/%d,%d:%d:%d+%d",
                tmpPhoneNumber, &year, &month, &day, &hour, &minute, &second, &pad);
        }
        if(ret == 8)
        {
            TEXTMSG *txtmsg = &(lastMsg.body.textmsg);
            txtmsg->date.year = year;
            txtmsg->date.month = month;
            txtmsg->date.day = day;
            txtmsg->time.hour = hour;
            txtmsg->time.minute = minute;
            txtmsg->time.second = second;
            txtmsg->time.pad = pad;
            unistr2gbstr(txtmsg->phone, tmpPhoneNumber);
        }
    }
    else if(strncmp(resp, "+CMGR:", 6) == 0)
    {
        // read message using AT+CMGR=<index>
        // +CMGR: "REC UNREAD","00310030003000380036",,"10/07/07,16:56:53+32"
        // 005400430020529E59579910
        lastMsg.type = NEWMSG2;
        char tmpPhoneNumber[100];
        int year, month, day, hour, minute, second, pad;
        int ret = sscanf(resp, "+CMGR:%*[^,],\"%[^\"]\",%*[^,],\"%d/%d/%d,%d:%d:%d+%d",
                tmpPhoneNumber, &year, &month, &day, &hour, &minute, &second, &pad);
        if(ret != 8)
        {
            ret = sscanf(resp, "+CMGR:%*[^,],\"%[^\"]\",,\"%d/%d/%d,%d:%d:%d+%d",
                tmpPhoneNumber, &year, &month, &day, &hour, &minute, &second, &pad);
        }
        if(ret == 8)
        {
            TEXTMSG *txtmsg = &(lastMsg.body.textmsg);
            txtmsg->date.year = year;
            txtmsg->date.month = month;
            txtmsg->date.day = day;
            txtmsg->time.hour = hour;
            txtmsg->time.minute = minute;
            txtmsg->time.second = second;
            txtmsg->time.pad = pad;
            unistr2gbstr(txtmsg->phone, tmpPhoneNumber);
        }
    }
    else if(strncmp(resp, "+CPMS:", 6) == 0)
    {
        // response for inbox count query
        // +CPMS: "SM",5,70,"SM",5,70,"SM",5,70
        char *p = strtok(resp, ",");
        p = strtok(NULL, ",");
        lastMsg.type = MSGCOUNT;
        lastMsg.body.data = strtoul(p, NULL, 10);
    }
    else if(strncmp(resp, "+CMGL:", 6) == 0)
    {
        // response for list all msg
        // +CMGL: 8,"REC UNREAD","00310030003000380036",,"10/07/07,16:56:53+32"
        // 005400430020529E59579910
        lastMsg.type = PEEKMSG;
        char tmpPhoneNumber[100];
        int year, month, day, hour, minute, second, pad;
        int ret = sscanf(resp, "+CMGL:%*d,%*[^,],\"%[^\"]\",%*[^,],\"%d/%d/%d,%d:%d:%d+%d",
            tmpPhoneNumber, &year, &month, &day, &hour, &minute, &second, &pad);
        if(ret != 8)
        {
            ret = sscanf(resp, "+CMGL:%*d,%*[^,],\"%[^\"]\",,\"%d/%d/%d,%d:%d:%d+%d",
                tmpPhoneNumber, &year, &month, &day, &hour, &minute, &second, &pad);
        }
        if(ret == 8)
        {
            TEXTMSG *txtmsg = &(lastMsg.body.textmsg);
            txtmsg->date.year = year;
            txtmsg->date.month = month;
            txtmsg->date.day = day;
            txtmsg->time.hour = hour;
            txtmsg->time.minute = minute;
            txtmsg->time.second = second;
            txtmsg->time.pad = pad;
            unistr2gbstr(txtmsg->phone, tmpPhoneNumber);
        }
    }
    else if(strncmp(resp, "+COPS:", 6) == 0)
    {
        // 运营商
        char *p = strtok(resp, ",");
        char *last = p;
        while((p = strtok(NULL, ",")) != NULL)
            last = p;
        lastMsg.type = COPS;
        strcpy(lastMsg.body.string, last);
    }
    /* 电话本相关 */
    else if(strncmp(resp, "+CPBS: ", 7) == 0)
    {
        switch(resp[7])
        {
        case '(':
            // +CPBS: ("MC","RC","DC","LD","LA","ME","SM","FD","ON","BN","SD","VM")
            lastMsg.type = PBLIST;
            sscanf(&resp[8], "%[^)\r\n]", lastMsg.body.string);
            break;
        case '\"':
            // +CPBS: "SM",1,200
            lastMsg.type = PBSIZE;
            {
                unsigned long used = 0;
                unsigned long max = 0;
                char *ps = strchr(resp, ',');
                if(ps != NULL)
                {
                    ps++;
                    used = strtoul(ps, &ps, 10);
                    if(*ps == ',')
                    {
                        ps++;
                        max = strtoul(ps, NULL, 10);
                    }
                }
                lastMsg.body.data = ((used << 16) & (~0xFFFF)) | (max & 0xFFFF);
            }
            break;
        }
    }
    /* 电话本结束 */
    else if((lastMsg.type == NEWMSG2) || (lastMsg.type == PEEKMSG))
    {
        lastMsg.type = MSGCONTENT;
        unistr2gbstr(lastMsg.body.textmsg.content, resp);
    }
    else
    {
        lastMsg.type = RAW;
        strcpy(lastMsg.body.string, resp);
    }
    return &lastMsg;
}

static int send_gprs_command_ex(const char *cmd, unsigned long timeout)
{
    int ret = -1;
    int try = 2;
    char resp[SERIAL_MAX_LINECOUNT];
    LOCK_READER();
    write_serial(cmd, 0);
    while(try-- > 0)
    {
        if(timed_read_serial(resp, timeout/1000, timeout % 1000))
        {
            ATMSG *msg = parse_gprs_response(resp);
            if(msg->type == OK)
            {
                ret = 0;
                break;
            }
        }
    }
    UNLOCK_READER();
    return ret;
}

int send_gprs_command(const char *cmd)
{
	int trytime = 0;
	while(trytime < 3)
	{
		if(send_gprs_command_ex(cmd, 800) == 0)
			return 0;
		trytime++;
	}
	return 1;
}

int init_gprs(void)
{
    int try = 10;
    int res = open_serial("/dev/" GPRS_SERIAL_PORT, "115200,8,1,n");
    if(res != 0)
        return res;
    while(send_gprs_command("AT\r\n")&& (try-- > 0))
    {
        turn_off_gprs();
        sleep(2);
        turn_on_gprs();
        usleep(500);
    }
    if(send_gprs_command("ATE0\r\n"))               // 关闭回显
        return 1;
    if(send_gprs_command("AT+CMIC=0,15\r\n"))
        return 2;                                   // 麦克设置异常
    if(send_gprs_command("AT+CMGF=1\r\n"))          // 设置短信为TEXT:1格式,PDU:0
        return 3;                                   // 短信格式设置异常
    if(send_gprs_command("AT+CHFA=1\r\n"))          // 使用耳机
        return 4;
    if(send_gprs_command("AT+CLVL=100\r\n"))        // 设置最大音量
        return 5;
    if(send_gprs_command("AT+CSMP=17,0,2,25\r\n"))  // 设置短信回执,中文短消息,test
        return 1;
    if(send_gprs_command("AT+CSCS=\"UCS2\"\r\n"))   // 设置编码为UCS2
        return 6;
    if(send_gprs_command("AT+CLIP=1\r\n"))          // 设置来电显示
        return 7;
    /*
    AT+CNMI=2,2,0,0,0可以打开新消息不保存直接提示的功能,提示功能使用下面的格式:
        +CMT: "002B0038003600310033003800310030003400360036003700350033",,"11/04/20,16:32:29+32"
        0030
    AT+CNMI=2,1,0,0,0可以打开新消息保存并提示的功能,提示功能使用下面的格式:
        +CMTI: "SM",2
    */
    if(send_gprs_command("AT+CNMI=2,2,0,0,0\r\n"))  // 启动新短信提示,并且不保存到SIM卡内,
        return 8;                                   // AT+CNMI=2,1,0,0,0是提示并且保存
    return 0;
}

// 接电话
int anser_call(void)
{
    return send_gprs_command_ex("ATA\r\n", 1000);
}

int hangup_call(void)
{
    return send_gprs_command_ex("ATH\r\n", 1000);
}

// 打电话
int make_call(const char *phone_number)
{
	char tmp[100];
	sprintf(tmp, "ATD%s;\r\n", phone_number);
	return send_gprs_command_ex(tmp, 1000);
}

// 二次拨号
int send_dtmf_call(const char *str)
{
	if(strlen(str) <= 0)
		return 1;
	char tmpString[100] = "AT+VTS=\"";
	char *p = &tmpString[strlen(tmpString)];
	while(*str)
	{
		*p++ = *str++;
		*p++ = ',';
	}
	p--;
	*p++ = '\"';
	*p = '\0';
	return send_gprs_command_ex(tmpString, 1000);
}

// 长短信长度在计算时,如果短信出现中文,则不得超过70字符(有人说64)
// 如果纯英文,则不超过140字符

static int get_sms_len(const char *sms, int *chs)
{
    int len = 0;
    const char *p = sms;
    if(chs) *chs = 0;
    while(*p)
    {
        if(*p & 0x80)
        {
            p++;
            if(chs) *chs = 1;
        }
        len++;
        p++;
    }
    return len;
}

static int calc_parts(int len, int chs)
{
    const int maxlen = chs ? (SMSCONTENT_MAXLEN / 2) : SMSCONTENT_MAXLEN;
    if(len > maxlen)
    {
        int parts = (len + maxlen - 1) / maxlen;
        int newparts = parts;
        do
        {
            parts = newparts;
            if(parts <= 9)
                newparts = ((len + 5 * parts) + maxlen - 1) / maxlen;
            else if(parts <= 99)
                newparts = ((len + 6 * 9 + (parts - 9) * 7) + maxlen - 1) / maxlen;
            else if(parts <= 999)
                newparts = ((len + 7 * 9 + 8 * 90 + (parts - 99) * 9) + maxlen - 1) / maxlen;
            else if(parts <= 9999)
                newparts = ((len + 8 * 9 + 9 * 90 + 10 * 900 + (parts - 999) * 11) + maxlen - 1) / maxlen;
            else
                return -1;
        } while(newparts != parts);
		return parts;
    }
    else
        return 1;
}

static int split_parts(const char *msg, char *parts[])
{
    int chs;
    int msmlen = get_sms_len(msg, &chs);
    const int maxlen = chs ? (SMSCONTENT_MAXLEN / 2) : SMSCONTENT_MAXLEN;
    int nparts = calc_parts(msmlen, chs);
    if(parts == NULL)
        return nparts;
    if(nparts <= 0)
        return 0;
    else if(nparts == 1)
    {
        strcpy(parts[0], msg);
    }
    else
    {
        int curIdx = 0;                 // 块编号
        const char *pMsg = msg;         // 源消息
        char *pPart = parts[curIdx];    // 分块之后的消息
        // 首先写入分块之后的短信头
        int plen = sprintf(pPart, "(%d/%d)", curIdx + 1, nparts);
        pPart += plen;
        while(*pMsg)
        {
            if(*pMsg & 0x80)
                *pPart++ = *pMsg++;
            *pPart++ = *pMsg++;
            plen++;
            if(plen >= maxlen)
            {
                curIdx++;
                if(curIdx >= nparts)
                    break;
                pPart = parts[curIdx];
                plen = sprintf(pPart, "(%d/%d)", curIdx + 1, nparts);
                pPart += plen;
            }
        }
        if(*pMsg)
            printf("WARNING!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
    }
    return nparts;
}

int send_long_sms(const char *phone_number, const char *msg)
{
    int nparts = split_parts(msg, NULL);
    if(nparts > 1)
    {
        int i;
        char **parts = (char **)malloc(nparts * sizeof(char *));
        for(i = 0; i < nparts; i++)
        {
            parts[i] = (char *)malloc(SMSCONTENT_MAXLEN + 1);
            memset(parts[i], 0, SMSCONTENT_MAXLEN + 1);
        }
        split_parts(msg, parts);
        for(i = 0; i < nparts; i++)
        {
//            printf("phone_number = [%s], content = [%s]\n", phone_number, parts[i]);
            int res = send_sms(phone_number, parts[i]);
            if(res != 0)
                break;
        }
        for(i = 0; i < nparts; i++)
            free(parts[i]);
        free(parts);
        return 0;
    }
    else
        return send_sms(phone_number, msg);
    return -1;
}

// 发短信
int send_sms(const char *phone_number, const char *msg)
{
    int ret = -1;
    int run = 1;
	char tmpUniNum[100];
	char tmpUniStr[SERIAL_MAX_LINECOUNT];
	char tmp[SERIAL_MAX_LINECOUNT];
    gbstr2unistr(tmpUniNum, phone_number);
    gbstr2unistr(tmpUniStr, msg);
#if 1
    int len = sprintf(tmp, "AT+CMGS=\"%s\"\r", tmpUniNum);
    LOCK_READER();
    write_serial(tmp, len);
//    printf("send : %s\n", tmp);
    ret = -1;
    run = 1;
    while(run)
    {
        char resp[SERIAL_MAX_LINECOUNT] = "";
        if(timed_read_serial(resp, 5, 0))
        {
//            printf("recv : %s\n", resp);
            ATMSG *msg = parse_gprs_response(resp);
//            printf("msg->type = %d(%d)\n", msg->type, WAITINPUT);
            switch(msg->type)
            {
            case WAITINPUT:
            case RAW:
                ret = 0;
            case OK:
            case ERROR:
                run = 0;
                break;
            default:
                break;
            }
        }
        else
            run = 0;
    }
//    printf("ret = %d\n", ret);
    if(ret == 0)
    {
        len = sprintf(tmp, "%s", tmpUniStr);
        write_serial(tmp, len);
        write_serial("\032", 1);
//        printf("wrotten : %s\n", tmp);
        ret = -1;
        run = 1;
        while(run)
        {
            char resp[SERIAL_MAX_LINECOUNT] = "";
            if(timed_read_serial(resp, 15, 0))
            {
                ATMSG *msg = parse_gprs_response(resp);
                switch(msg->type)
                {
                case MSGSENDED:
                    ret = 0;
                    break;
                case OK:
                case ERROR:
                    run = 0;
                    break;
                default:
                    break;
                }
            }
            else
            {
                printf("timeout!!!!!!!!\n");
                run = 0;
            }
        }
    }
    else
        write_serial("\033", 1);
    UNLOCK_READER();
#else
	sprintf(tmp, "AT+CMGS=\"%s\"\r\n%s", tmpUniNum, tmpUniStr);//032: Ctrl-Z, 发送; 033: ESC, 取消
	printf("%s\n", tmp);
	LOCK_READER();
    clear_serial();
    write_serial(tmp, 0);
    write_serial("\032", 1);
    while(run)
    {
        char resp[SERIAL_MAX_LINECOUNT] = "";
        if(timed_read_serial(resp, 15, 0))
        {
            ATMSG *msg = parse_gprs_response(resp);
            switch(msg->type)
            {
            case MSGSENDED:
                ret = 0;
                break;
            case OK:
                run = 0;
                break;
            case ERROR:
                run = 0;
                break;
            default:
                break;
            }
        }
        else
        {
            printf("timeout!!!!!!!!\n");
            run = 0;
        }
    }
    UNLOCK_READER();
#endif
    return ret;
}

int get_inbox_count(void)
{
	int count = -1;
	int run = 1;
	if(send_gprs_command("AT\r\n"))
		return 0;
    LOCK_READER();
    clear_serial();
    write_serial("AT+CPMS?\r\n", 0);
	while(run)
	{
        char resp[SERIAL_MAX_LINECOUNT];
        if(read_serial(resp))
        {
            ATMSG *msg = parse_gprs_response(resp);
            switch(msg->type)
            {
            case MSGCOUNT:
                count = msg->body.data;
                break;
            case OK:
                run = 0;
                break;
            case ERROR:
                run = 0;
                break;
            default:
                break;
            }
        }
	}
    UNLOCK_READER();
	return count;
}

int read_sms(int index, TEXTMSG *sms)
{
    int ret = -1;
    int run = 1;
    if(send_gprs_command("AT\r\n"))
        return -1;
    char cmd[100];
    sprintf(cmd, "AT+CMGR=%d\r\n", index);
    LOCK_READER();
    write_serial(cmd, 0);
    while(run)
    {
        char resp[SERIAL_MAX_LINECOUNT];
        if(timed_read_serial(resp, 3, 0))
        {
            ATMSG *msg = parse_gprs_response(resp);
            switch(msg->type)
            {
            case OK:
            case ERROR:
                run = 0;
                break;
            case MSGCONTENT:
                ret = 0;
                memcpy(sms, &(msg->body.textmsg), sizeof(*sms));
    //          strcpy(msgList[count].content, (char*)msg.data);
                break;
            default:
                break;
            }
        }
        else
            run = 0;
    }
    UNLOCK_READER();
    return ret;
}

int get_sms(TEXTMSG *msgList, int max)
{
	int count = 0;
	int run = 1;
    if(send_gprs_command("AT\r\n"))
		return 0;
    LOCK_READER();
    write_serial("AT+CMGL=\"ALL\"\r\n", 0);
    while(run)
    {
        char resp[SERIAL_MAX_LINECOUNT];
        if(timed_read_serial(resp, 3, 0))
        {
            ATMSG *msg = parse_gprs_response(resp);
    		switch(msg->type)
    		{
    		case OK:
    		case ERROR:
    			run = 0;
    			break;
    		case MSGCONTENT:
    			memcpy(&(msgList[count]), &(msg->body.textmsg), sizeof(TEXTMSG));
    			count++;
    			if(count >= max)
    				run = 0;
    			break;
    		default:
    			break;
    		}
        }
        else
            run = 0;
    }
    UNLOCK_READER();
    return count;
}

int gprs_service_loop(ATMSG *msg)
{
    int ret = -1;
    char resp[SERIAL_MAX_LINECOUNT];
    LOCK_READER();
    if(timed_read_serial(resp, 0, 100))
    {
        ATMSG *m = parse_gprs_response(resp);
        if(m)
        {
            memcpy(msg, m, sizeof(*m));
            ret = 0;
        }
    }
    UNLOCK_READER();
    return ret;
}

/* 电话本操作类 */
int pb_get_storage(char list[][10])
{
	int count = -1;
	int run = 1;
	if(send_gprs_command("AT\r\n"))
		return 0;
    LOCK_READER();
    clear_serial();
    write_serial("AT+CPBS=?\r\n", 0);
	while(run)
	{
        char resp[SERIAL_MAX_LINECOUNT];
        if(read_serial(resp))
        {
            ATMSG *msg = parse_gprs_response(resp);
            switch(msg->type)
            {
            case PBLIST:
                // TODO: find the list
                break;
            case OK:
                run = 0;
                break;
            case ERROR:
                run = 0;
                break;
            default:
                break;
            }
        }
	}
    UNLOCK_READER();
	return count;
}

int pb_select_storage(const char *storage)
{
    char tmp[10] = "AT+CPBS=";
    strcat(tmp, storage);
    strcat(tmp, "\r\n");
    return send_gprs_command(tmp);
}

int pb_read(int index, PHONEBOOK *pb)
{
    // read the phone book
    // AT+CPBR
    return 0;
}

int pb_write(int index, const PHONEBOOK *pb, int del)
{
    // write or delete a phone book
    // AT+CPBW
    return 0;
}

