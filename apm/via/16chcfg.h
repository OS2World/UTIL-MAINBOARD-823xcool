/* $Id: 16chcfg.h,v 1.1 2002/06/13 00:11:38 root Stab $ */

#ifndef _16CHCFG_INCLUDED
#define _16CHCFG_INCLUDED

int config_chip();
void set_throttling(unsigned short cycle);
unsigned short query_throttling();

#endif
