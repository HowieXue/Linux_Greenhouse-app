#ifndef _UNI_H_
#define _UNI_H_
#include <iconv.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>

extern u_int16_t gb2uni(u_int16_t gbcode);
extern u_int16_t uni2gb(u_int16_t unicode);
extern char *unistr2gbstr(char *gbstr, const char *unistr);
extern char *gbstr2unistr(char *unistr, const char *gbstr);

#endif//_UNI_H_

