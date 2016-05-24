#ifndef _UTILS_H_
#define _UTILS_H_
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#define ARRAY_SIZE(a)       (sizeof(a) / sizeof(a[0]))

#define is_digit(ch)    (((ch) >= '0') && ((ch) <= '9'))
#define is_hex(ch)      ((((ch) >= 'a') && ((ch) <= 'f')) || (((ch) >= 'A') && ((ch) <= 'F')) || is_digit(ch))

void set_keypress(void);
void reset_keypress(void);
char *strtrim(char *str);
char *strltrim(char *str);
char *strrtrim(char *str);
void dumpMemory(void *base, int size, int width);
char *_gets_(FILE *input, char *buf);
unsigned long getul(FILE *input, int base);
char _getch_(FILE *input);
int split_cmdline(char *input, char **output, int max_num);
int readUtilCharacter(int fd, void *buf, int len, char ch);
char *getFileName(char *fullpath);

#define FTW_INVALID         -1
#define FTW_FILE            1
#define FTW_FOLDER          2
#define FTW_RETPARENT       3
typedef int (*FTW_FUNC)(const char *fullpath, const struct stat *sb, int flag, void *arg);
int fileTreeWalk(const char *path, FTW_FUNC fn, int depth, void *arg);

#endif//_UTILS_H_
