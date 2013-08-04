/* $Id: std32.h,v 1.1 2002/03/02 15:47:59 root Stab $ */

#ifndef STD32_INCLUDED
#define STD32_INCLUDED

#define INCL_BASE
#include <os2.h>

extern unsigned long TKSSBase;
#define SSToDS(p) ((void *)(TKSSBase+(unsigned long)(p)))

#endif
