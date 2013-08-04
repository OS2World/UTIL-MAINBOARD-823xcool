/* $Id: externs.c,v 1.7 2004/12/02 04:25:37  Exp $ */

#define INCL_BASE
#include <os2.h>

#include "viaapm.h"
#include "viaapmp.h"

/* Global and not-so-global variables */

unsigned short acpi_base=0;             /* Shared ACPI base address */
unsigned short acpi_c2_port=0;          /* Port to read in the hook */
unsigned short acpi_c3_port=0;          /* Port to read for ACPI C3 state */
int chip_type=CHIP_UNKNOWN;             /* Chip type */
PFN Device_Help=NULL;                   /* Device helper gateway */
int is_inited=0;                        /* Initialization flag */
int is_debug_kernel=0;                  /* Debug kernel flag */
unsigned char pf_OutMessage[6]={0,0,0,0,0,0}; /* FAR32 address */
int use_pll=PLL_NONE;                   /* ** Undocumented ** */
int cpu_multiplier=0;                   /* ** Undocumented ** */
int throttling_timer=TT_DEFAULT;        /* Throttling timer cycle */
unsigned long south_bridge=0;           /* VIA south bridge PCI address */
int no_hlt=0;                           /* Disable automatic HLT detection */

/* Command-line settings */

long quiet_init=0;
long verbose_init=0;
long reipl_on_1503=0;
