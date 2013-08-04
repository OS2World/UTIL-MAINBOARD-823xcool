/* $Id: kprintf.h,v 1.2 2003/11/19 21:13:24 root Exp $ */

#ifndef KPRINTF_INCLUDED
#define KPRINTF_INCLUDED

/* Fancy ANSI sequences */

#define ANSI0   "\x1B[0;30m"
#define ANSI1   "\x1B[0;34m"
#define ANSI2   "\x1B[0;32m"
#define ANSI3   "\x1B[0;36m"
#define ANSI4   "\x1B[0;31m"
#define ANSI5   "\x1B[0;35m"
#define ANSI6   "\x1B[0;33m"
#define ANSI7   "\x1B[0m"
#define ANSI8   "\x1B[1;30m"
#define ANSI9   "\x1B[1;34m"
#define ANSI10  "\x1B[1;32m"
#define ANSI11  "\x1B[1;36m"
#define ANSI12  "\x1B[1;31m"
#define ANSI13  "\x1B[1;35m"
#define ANSI14  "\x1B[1;33m"
#define ANSI15  "\x1B[1;37m"

/* Prototype(s) */

void kprintf(char *fmt, ...);

#endif
