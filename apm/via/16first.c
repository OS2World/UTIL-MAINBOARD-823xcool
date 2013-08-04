/* $Id: 16first.c,v 1.3 2002/03/08 14:29:25 root Stab $ */

/* Device driver header */

#define INCL_BASE
#include <os2.h>

#include <devhdr.h>
#include <strat2.h>
#include <reqpkt.h>

extern primary_strategy();              /* Primary strategy - initialize
                                           and receive IOCTL */
extern secondary_strategy();            /* Secondary strategy - initialize
                                           and send IOCTL */

/* OS/2 device header */

static DDHDR ddhdrs[2]=
{
 {
  (PVOID)&ddhdrs[1],
  (USHORT)DEVLEV_3|DEV_30|DEV_CHAR_DEV,
  (USHORT)(void near *)primary_strategy,
  (USHORT)NULL,
  "VIAAPM$ ",
  0,
  0,
  0,
  0,
  DEV_SAVERESTORE|DEV_INITCOMPLETE
 },
 {
  (PVOID)0xFFFFFFFF,
  (USHORT)DEVLEV_3|DEV_30|DEV_CHAR_DEV,
  (USHORT)(void near *)secondary_strategy,
  (USHORT)NULL,
  "VIAAPM2$",
  0,
  0,
  0,
  0,
  0L
 },
 
};
