/* $Id: parsearg.h,v 1.1 2002/03/04 20:22:01 root Exp $ */

#ifndef PARSEARG_INCLUDED
#define PARSEARG_INCLUDED

struct paramset
{
 char *t;
 int (*process)(char far *);
 unsigned short *trigger;
};

int ctparms(char far *s);
int parse_args(char far *p, struct paramset *pars);

#endif
