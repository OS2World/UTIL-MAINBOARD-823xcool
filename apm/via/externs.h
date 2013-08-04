/* $Id: externs.h,v 1.7 2004/12/02 04:25:37  Exp $ */

#ifndef EXTERNS_INCLUDED
#define EXTERNS_INCLUDED

#ifndef OS2_INCLUDED
#define INCL_BASE
#include <os2.h>
#endif

extern unsigned short acpi_base;
extern unsigned short acpi_c2_port;
extern unsigned short acpi_c3_port;
extern int chip_type;
extern PFN Device_Help;
extern int is_inited;
extern int is_debug_kernel;
extern unsigned char pf_OutMessage[6];
extern int use_pll;
extern int cpu_multiplier;
extern int throttling_timer;
extern unsigned long south_bridge;
extern int no_hlt;

extern long quiet_init;
extern long verbose_init;
extern long reipl_on_1503;

#endif
