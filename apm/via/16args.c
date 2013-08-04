/* $Id: 16args.c,v 1.6 2004/12/02 04:25:37  Exp $ */

#include <stddef.h>
#include <string.h>

#include <vsprintf.h>
#include <kprintf.h>

#include "16args.h"
#include "externs.h"
#include "viaapmp.h"
#include "16pll.h"

/* Parameters map */

static int set_ics94215(char far *s);
static int set_ics94228(char far *s);
static int set_cpu_multiplier(char far *s);
static int set_throttling_timer(char far *s);

#define toupper(c) ((c>='a'&&c<='z')?(c-32):c)

struct paramset apm_params[]=
{
 {"SYS1503", NULL, (int *)&reipl_on_1503}, /* Re-IPL at SYS1503 */
 {"PLL:ICS94215", set_ics94215, NULL},     /* ICS94215 PLL (not tested) */
 {"PLL:ICS94228", set_ics94228, NULL},     /* ICS94228 PLL */
 {"CPUMUL:", set_cpu_multiplier, NULL},    /* (CPU freq/FSB freq) */
 {"THROT:", set_throttling_timer, NULL},   /* Throttling timer mode */
 {"!HLT", NULL, (int *)&no_hlt},           /* No automatic HLT detection -
                                              ACPI only */
 {"Q", NULL, (int *)&quiet_init},          /* Quiet initialization */
 {"V", NULL, (int *)&verbose_init},        /* Verbose initialization */
 {NULL, NULL}
};

/*
 * Worker routines
 */

static int set_ics94215(char far *s)
{
 use_pll=PLL_ICS94215;
 return(0);
}

static int set_ics94228(char far *s)
{
 use_pll=PLL_ICS94228;
 return(0);
}

static int set_cpu_multiplier(char far *s)
{
 if(s==NULL)
  return(1);
 cpu_multiplier=simple_strtoul(s, NULL, 10)<<1;
 if(cpu_multiplier==0)
  return(1);
 if(_fstrchr(s, '.')!=NULL)
  cpu_multiplier++;
 return(0);
}

static int set_throttling_timer(char far *s)
{
 char c;

 if(s==NULL)
  return(1);
 c=toupper(*s);
 if(c=='F')
  throttling_timer=TT_FAST;
 else if(c=='M')
  throttling_timer=TT_MEDIUM;
 else if(c=='S')
  throttling_timer=TT_SLOW;
 else
  return(1);
 return(0);
}
