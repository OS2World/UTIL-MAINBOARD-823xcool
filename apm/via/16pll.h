/* $Id: 16pll.h,v 1.2 2002/06/08 20:34:05 root Stab $ */

#ifndef _16PLL_INCLUDED
#define _16PLL_INCLUDED

/* Prototypes */

unsigned long query_pll_freq();
int set_pll_freq(unsigned long hz);
int init_pll(int t);
int query_pll();
void reconfigure_pll();

#endif
