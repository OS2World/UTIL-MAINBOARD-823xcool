/* $Id: vsprintf.h,v 1.1 2002/03/04 20:22:01 root Exp $ */

#ifndef VSPRINTF_INCLUDED
#define VSPRINTF_INCLUDED

unsigned long simple_strtoul(const char far *cp, char far **endp, unsigned int base);
int vsprintf(char *buf, const char *fmt, ...);

#endif
