#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include "utils.h"
#include <termios.h>

static struct termios stored_settings;
void set_keypress(void)
{
    struct termios new_settings;
    tcgetattr(0,&stored_settings);
    new_settings = stored_settings;
    /* Echo off, Disable canonical mode, and set buffer size to 1 byte */
    new_settings.c_lflag &= ~(ICANON|ECHO);
    new_settings.c_cc[VTIME] = 0;
    new_settings.c_cc[VMIN] = 1;
    tcsetattr(0,TCSANOW,&new_settings);
    return;
}

void reset_keypress(void)
{
    tcsetattr(0,TCSANOW,&stored_settings);
    return;
}

//=============================================================
// 语法格式：	char *strltrim(char *str)
// 实现功能：	去除字符串左侧的空白及非ASCII字符
// 入口参数：	str - 字符串首地址
// 出口参数：	字符串首地址
//=============================================================
char *strltrim(char *str)
{
	char *p = str;
	while(*p++ <= ' ');
	if(--p == str)
		return p;
	return strcpy(str, p);
}

//=============================================================
// 语法格式：	char *strrtrim(char *str)
// 实现功能：	去除字符串右侧的空白及非ASCII字符
// 入口参数：	str - 字符串首地址
// 出口参数：	字符串首地址
//=============================================================
char *strrtrim(char *str)
{
	char *p = str;
	while(*p++);
	while(*--p <= ' ');
	*++p = '\0';
	return str;
}

//=============================================================
// 语法格式：	char *strtrim(char *str)
// 实现功能：	去除字符串两侧的空白及非ASCII字符
// 入口参数：	str - 字符串首地址
// 出口参数：	字符串首地址
//=============================================================
char *strtrim(char *str)
{
	return strltrim(strrtrim(str));
}

char *strlshift(char *str, int n)
{
    return strcpy(str, str + n);
}

void dumpMemory(void *base, int size, int width)
{
	char convChar(unsigned char ch)
	{
		if(ch < ' ')
			return '.';
		if(ch >= 0x7F)
			return '.';
		return (char)ch;
	}
	int row, rest;
	int i, j;
	unsigned char *p = (unsigned char *)base;
	if(width <= 0)
		width = 16;
	row = size / width;
	rest = size % width;
	for(i = 0; i < row; i++)
	{
		printf("%p:", p);
		for(j = 0; j < width; j++)
			printf("%02X ", p[j]);
		printf("|");
		for(j = 0; j < width; j++)
			printf("%c", convChar(p[j]));
		p += width;
		printf("\n");
	}
	if(rest)
	{
		printf("%p:", p);
		for(j = 0; j < rest; j++)
			printf("%02X ", p[j]);
		for(; j < width; j++)
			printf("   ");
		printf("|");
		for(j = 0; j < rest; j++)
			printf("%c", convChar(p[j]));
		printf("\n");
	}
}

char *_gets_(FILE *input, char *buf)
{
    int doit = 1;
	int escape = 0;
	int pos = 0;
    set_keypress();
	while(doit)
	{
		char ch = getchar();
		switch(ch)
		{
		case '\b':
			escape = 0;
			if(pos > 0)
			{
    			putchar('\b');
    			putchar(' ');
    			putchar('\b');
				pos--;
			}
            continue;
			break;
		case '\\':
			buf[pos++] = ch;
			escape = 1;
			break;
		case '\n':
		case '\r':
			if(escape == 0)
			{
				buf[pos++] = '\0';
                doit = 0;
			}
			buf[pos++] = ch;
			escape = 0;
			break;
        case '\x1b':
            if((ch = getchar()) == '[')
            {
                do {
                    ch = getchar();
                } while(is_digit(ch));
            }
            else
            {
                putchar('^');
                putchar(ch);
            }
            continue;
            break;
		default:
			buf[pos++] = ch;
			escape = 0;
			break;
		}
        putchar(ch);
	}
    reset_keypress();
    return buf;
}

unsigned long getul(FILE *input, int base)
{
    char buf[128];
    _gets_(input, buf);
    return strtoul(buf, NULL, 10);
}

char _getch_(FILE *input)
{
    char ch;
    set_keypress();
    ch = getchar();
    putchar(ch);
    reset_keypress();
    return ch;
}

//=============================================================
// 语法格式：	int split_cmdline(char *input, char **output, int max_num)
// 实现功能：	分割命令行参数
// 入口参数：	input - 待分割的字符串
//				output - 用于保存分割之后的字符串首地址的数组
//				max_num - 最大分割的字符串数量
// 出口参数：	分割之后的字符串数量
//=============================================================
int split_cmdline(char *input, char **output, int max_num)
{
	int item = 1;
//	int s_quoteMark = 0;
//	int d_quoteMark = 0;
	char ch;
	if((output == NULL) || (max_num < 0))
		max_num = (((unsigned int)-1) >> 1);

	if(output) output[0] = input;
	while((ch = *input++) && (max_num > 0))
	{
		if((ch == '\"') || (ch == '\''))
		{
			char *tmp = strchr(input, ch);
			if(tmp == NULL)
				input = input + strlen(input);
			else
				input = tmp + 1;
		}
		else if(ch == '\\')
		{
            strlshift(input - 1, 1);
			//input++;
		}
		else if(ch == ' ')
		{
			if(output)
			{
				*(input - 1) = '\0';
				 output[item] = input;
			}
			item++;
			while(*input == ' ') input++;
			max_num--;
		}
	}
	return item;
}

int readUtilCharacter(int fd, void *buf, int len, char ch)
{
    char *p = (char *)buf;
    int count = 0;
    while(count < len)
    {
        int l = read(fd, p, 1);
        if(l <= 0)
            break;
        count++;
        if(p[0] == ch)
        {
            errno = 0;
            break;
        }
        p++;
        errno = ERANGE;
    }
    return count;
}

char *getFileName(char *fullpath)
{
    char *ret = strrchr(fullpath, '/');
    if(ret == NULL)
        ret = strrchr(fullpath, '\\');
    if(ret == NULL)
        return fullpath;
    return ++ret;
}

static int searchDir(const char *path, FTW_FUNC fn, int depth, int curdepth, void *arg)
{
	int ret = 0;
	DIR *dir = opendir(path);
	struct dirent *ent;
	if(dir == NULL)
		return -1;
	do {
		ent = readdir(dir);
		if(ent)
		{
			int flag = FTW_FILE;
			char fullpath[256];
			struct stat buf;
			if(strcmp(ent->d_name, ".") == 0)
				continue;
			if(strcmp(ent->d_name, "..") == 0)
				continue;
			strcpy(fullpath, path);
			if(fullpath[strlen(fullpath) - 1] != '/')
				strcat(fullpath, "/");
			strcat(fullpath, ent->d_name);
			if(stat(fullpath, &buf) != 0)
				flag = -1;
			else if(S_ISDIR(buf.st_mode))
				flag = FTW_FOLDER;

			if((ret = (*fn)(fullpath, &buf, flag, arg)) != 0)
				break;
			if(flag == FTW_FOLDER)
			{
				if((depth < 0) || (curdepth < depth))
				{
					if((ret = searchDir(fullpath, fn, depth, curdepth + 1, arg)) != 0)
						break;
				}
			}
		}
	} while(ent);
	closedir(dir);
	return ret == 0 ? (*fn)("..", NULL, FTW_RETPARENT, arg) : ret;
}

int fileTreeWalk(const char *path, FTW_FUNC fn, int depth, void *arg)
{
	int default_fn(const char *fullpath, const struct stat *sb, int flag, void *arg)
	{
		return 0;
	}
	int ret;
	struct stat buf;
	if((ret = stat(path, &buf)) != 0)
		return ret;
	if(!S_ISDIR(buf.st_mode))
		return -1;
	(*fn)(path, &buf, FTW_FOLDER, arg);
	return searchDir(path, fn == NULL ? default_fn : fn, depth, 1, arg);
}

