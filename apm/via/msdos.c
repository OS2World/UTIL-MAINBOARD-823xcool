/* $Id: msdos.c,v 1.11 2004/12/02 04:25:37  Exp $ */

/* DOS-based INT 28h idle hook and services program */

#include <ctype.h>
#include <dos.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#include "viaapm.h"
#include "viaapmp.h"
#include "16chcfg.h"
#include "16smbus.h"
#include "16pll.h"
#include "externs.h"

#include <parsearg.h>
#include <vsprintf.h>

/* From DET_X86.ASM */

unsigned short detect_x86();

/* Microsoft missed it in DDK */

#define MK_FP(seg, ofs) (void far *)(((unsigned long)(seg)<<16)+(unsigned long)(unsigned)(ofs))
typedef void (_interrupt _far *inthdl)();

/* Parameters map */

static int set_ics94215(char far *s);
static int set_ics94228(char far *s);
static int set_cpu_multiplier(char far *s);
static int query_cpu_freq(char far *s);
static int set_cpu_freq(char far *s);
static int set_throttling_p(char far *s);
static int query_throttling_p(char far *s);
static int display_help(char far *s);

static int install_tsr=0;

struct paramset dos_params[]=
{
 {"INSTALL", NULL, (int *)&install_tsr},   /* TSR mode */
 {"VERBOSE", NULL, (int *)&verbose_init},  /* Verbose initialization */
 {"PLL:ICS94215", set_ics94215, NULL},     /* ICS94215 PLL (not tested) */
 {"PLL:ICS94228", set_ics94228, NULL},     /* ICS94228 PLL */
 {"CPUMUL:", set_cpu_multiplier, NULL},    /* (CPU freq/FSB freq) */
 {"CPUFREQ:", set_cpu_freq, NULL},         /* Set CPU frequency */
 {"CPUFREQ", query_cpu_freq, NULL},        /* Query CPU frequency */
 {"T:", set_throttling_p, NULL},           /* Set CPU throttling */
 {"T", query_throttling_p, NULL},          /* Query CPU throttling */
 {"?", display_help, NULL},                /* Show help */
 {"H", display_help, NULL},                /* ----''---- */
 {"!HLT", NULL, (int *)&no_hlt},           /* Disable HLT detection */
 {NULL, NULL}
};

/* Resident stripe */

static unsigned char resident[]=
{
 0x52, 0x50, 0xBA, 0xBA, 0xBA, 0xEC, 0x58, 0x5A, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA
};

#define RES_IO_PORT              0x03   /* ACPI C2 I/O port address */
#define RES_CHAIN                0x09   /* Next address in chain */

/* Static data */

static char error_blurb[]="\nERROR: %s\n\a";
static char *prg_name;

/*
 * HACK: supply DOS-compatible kprintf()
 */

void kprintf(char *fmt, ...)
{
 va_list marker;
 char msg[256];
 int i, s=1;

 va_start(marker, (char *)fmt);
 vsprintf(msg, (char *)fmt, marker);
 va_end(marker);
 for(i=0; msg[i]!='\0'; i++)
 {
  if(msg[i]==0x1B)
   s=0;
  else if(!s&&isalpha(msg[i]))
   s=1;
  else if(s)
   fputc(msg[i], stderr);
 }
}

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
 if(strchr(s, '.'))
  cpu_multiplier++;
 return(0);
}

static int chk_cpumul()
{
 init_smbus();
 init_pll(use_pll);
 if(cpu_multiplier==0)
 {
  fprintf(stderr, "Place /CPUMUL:<n> in front of /CPUFREQ to set CPU multiplier\n");
  return(1);
 }
 return(0);
}

static int set_cpu_freq(char far *arg)
{
 static unsigned long f, pf, m=1;

 if(chk_cpumul())
  return(1);
 pf=simple_strtoul(arg, NULL, 10);
 if(strchr(arg, 'M'))
  m=1000000L;
 else if(strchr(arg, 'K')||strchr(arg, 'k'))
  m=1000L;
 f=pf*m;
 if(f<10000000)
 {
  fprintf(stderr, "CPU frequency of %lu Hz can't be accepted\n", f);
  return(1);
 }
 return(set_pll_freq((f/cpu_multiplier)<<1));
}

static int query_cpu_freq(char far *arg)
{
 unsigned long f;

 if(chk_cpumul())
  return(1);
 f=query_pll_freq();
 if(f==0)
  return(1);
 f=f/2*cpu_multiplier;
 f/=10000;
 printf("CPU frequency is %lu.%02lu MHz\n", f/100, f%100);
 return(0);
}

static int set_throttling_p(char far *arg)
{
 unsigned short t;

 t=simple_strtoul(arg, NULL, 10);
 if(t==0)
  return(1);
 if(strchr(arg, '%'))
 {
  if(t==100)
   t=16;
  else
  {
   t=(t+6)*3/20;
   if(t==16)
    t--;                              /* Enable throttling if not 100% */
  }
 }
 if(t==0)
  t=1;
 set_throttling(t);
 printf("Throttling set to ");
 query_throttling_p(NULL);
 return(0);
}

static int query_throttling_p(char far *arg)
{
 unsigned short rc;

 if(arg!=NULL)
  return(1);
 rc=query_throttling();
 if(rc==16)
  printf("100%% duty cycle (throttling disabled)\n");
 else
  printf("%u%% [+/-3%%] duty cycle (throttling enabled)\n",
         (unsigned int)(rc*20/3-3));
 return(0);
}

/*
 * Initialization
 */

/* Main initialization point */

static int init()
{
 unsigned short rc;
 char e[256];

 /* Prevent hangs on 80286s */
 if(detect_x86()<0x386)
 {
  fprintf(stderr, "This program is intended for 32-bit systems with VIA chipsets\n");
  return(1);
 }
 if((rc=config_chip())!=0)
 {
  if(rc==CONF_UNKNOWN_CHIP)
   sprintf(e, "No compatible chip detected");
  else if(rc==CONF_BAD_ACPI)
   sprintf(e, "Bad ACPI base port: 0x%04x", acpi_base);
  else
   sprintf(e, "error code=%u", rc);
  fprintf(stderr, error_blurb, e);
  return(1);
 }
 return(0);
}

/* TSR dead-end */

static void stay_resident()
{
 void (_interrupt _far *prev_int28)();
 char *p;

 *(unsigned short *)&resident[RES_IO_PORT]=acpi_c2_port;
 prev_int28=_dos_getvect(0x28);
 *(unsigned long *)&resident[RES_CHAIN]=(unsigned long)prev_int28;
 p=MK_FP(_psp, 0x80);
 *p++='\0';
 *p++='\r';
 memcpy(p, resident, sizeof(resident));
 _dos_setvect(0x28, (inthdl)p);
 printf("Installing resident idle hook.\n");
 _dos_keep(0, 9);
}

/* Show the help screen and bail out */

static int display_help(char far *arg)
{
 printf("Usage: %s <parameters>\n\n"
        "   /INSTALL = Install INT 28h idle hook\n"
        "      /!HLT = For /!INSTALL, disables automatic HLT detection\n"
        "         /T = Query throttling\n"
        " /T:[value] = Set throttling\n", prg_name);
 exit(1);
}

/* Main routine */

int main(int argc, char **argv)
{
 unsigned int j;
 static CHIPSET_TYPES;
 char *p;
 char cmdline[256];

 prg_name=argv[0];
 if(init())
  return(1);
 for(j=0; chipset_types[j]!=NULL&&j<chip_type; j++)
  ;
 if(chipset_types[j]!=NULL)
  p=chipset_types[j];
 else
  p="Unknown chipset";
 printf("DOS control utility v 1.20 for VIA chipsets (%s)\n\n", p);
 if(argc<2)
  display_help(NULL);
 /* Gather the command line as required by the library routine */
 p=MK_FP(_psp, 0x80);
 *(p+(*p)+1)='\0';
 strcpy(cmdline, argv[0]);
 strcat(cmdline, " ");
 strcat(cmdline, p+1);
 if(parse_args(cmdline, dos_params))
  return(1);
 if(install_tsr)
  stay_resident();
 return(0);
}
