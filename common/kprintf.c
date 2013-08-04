/* $Id: kprintf.c,v 1.1 2002/03/04 20:22:01 root Exp $ */

/*
 * Stand-alone printf library
 */

#include <stdarg.h>

#include "vsprintf.h"

unsigned short far pascal DosWrite(unsigned short hf, void far *bBuf,
                    unsigned short cbBuf, unsigned short far *pcbBytesWritten);

/* printf() hack */

void kprintf(char *fmt, ...)
{
 unsigned int ret, size;
 va_list marker;
 static char buffer[512];
 unsigned short rc;

 va_start(marker, (char *)fmt);
 DosWrite(1, (void far *)buffer, vsprintf(buffer, (char *)fmt, marker), &rc);
 va_end(marker);
}
