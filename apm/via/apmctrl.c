/* $Id: apmctrl.c,v 1.16 2004/12/02 04:25:37  Exp $ */

/* S[i|a]mple APM control applet */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INCL_BASE
#include <os2.h>

#include "viaapm.h"
#include "viaapmp.h"

/* Local variables */

static HFILE hf;

/* Perform IOCTL with necessary checks */

static void do_ioctl(PVOID pData, ULONG cbDataLenMax,  
                     PVOID pParams, ULONG cbParmLenMax,
                     ULONG function, ULONG category, HFILE hDevice)
{
 USHORT rc;
 ULONG pcbParmLen=cbParmLenMax, pcbDataLen=cbDataLenMax;
 
 if(rc=DosDevIOCtl2(hDevice, category, function, pParams, cbParmLenMax,
                    &pcbParmLen, pData, cbDataLenMax, &pcbDataLen))
 {
  printf("IOCtl #%02lx/%02lx failed, rc=%u\n", category, function, rc);
  DosClose(hDevice);
  exit(1);
 }
}

/* Reinitialize the driver */

static void reset()
{
 USHORT rc;

 do_ioctl(&rc, 2, NULL, 0, VIAAPM_CMD_RESET, IOCTL_CAT_VIAAPM, hf);
 if(!rc)
  printf("Reset OK\n");
 else
  printf("Reset failed, rc=%u\n", rc);
}

/* Set throttling */

static void set_throttling(char *arg)
{
 USHORT t;

 t=atoi(arg);
 if(t==0)
 {
  fprintf(stderr, "Invalid parameter <%s> ignored\n", arg);
  return;
 }
 if(strchr(arg, '%')!=NULL)
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
 do_ioctl(NULL, 0, &t, 2, VIAAPM_CMD_SET_THROTTLING, IOCTL_CAT_VIAAPM, hf);
}

/* Query throttling */

static void query_throttling()
{
 USHORT rc;

 do_ioctl(&rc, 2, NULL, 0, VIAAPM_CMD_QUERY_THROTTLING, IOCTL_CAT_VIAAPM, hf);
 if(rc==16)
  printf("100%% duty cycle (throttling disabled)\n");
 else
  printf("%u%% [+/-3%%] duty cycle (throttling enabled)\n",
         (unsigned int)(rc*20/3-3));
}

/* Query chipset */

static void query_chipset()
{
 USHORT rc;
 unsigned int j;
 static CHIPSET_TYPES;

 do_ioctl(&rc, 2, NULL, 0, VIAAPM_CMD_QUERY_CHIPSET, IOCTL_CAT_VIAAPM, hf);
 for(j=0; chipset_types[j]!=NULL&&j<rc; j++)
  ;
 if(chipset_types[j]!=NULL)
  printf("Chipset type: %s\n", chipset_types[j]);
 else
  printf("Unknown chipset type (%u)\n", rc);
}

/* ** Undocumented ** */

static void set_fsb(char *arg)
{
 unsigned long f;

 f=atoi(arg);
 if(strchr(arg, 'M')!=NULL)
  f*=1000000L;
 else if(strchr(arg, 'K')!=NULL||strchr(arg, 'k')!=NULL)
  f*=1000L;
 if(f<1000000||f>1000000000)
  fprintf(stderr, "FSB frequency of %lu Hz can't be accepted\n", f);
 else
  do_ioctl(NULL, 0, &f, sizeof(f), VIAAPM_CMD_SET_FSB, IOCTL_CAT_VIAAPM, hf);
}

/* ** Undocumented ** */

static void query_fsb()
{
 unsigned long f;

 do_ioctl(&f, sizeof(f), NULL, 0, VIAAPM_CMD_QUERY_FSB, IOCTL_CAT_VIAAPM, hf);
 fprintf(stderr, "FSB frequency is %lu Hz\n", f);
}

/* ** Undocumented ** */

static void set_cpu_freq(char *arg)
{
 unsigned long f;

 f=atoi(arg);
 if(strchr(arg, 'M')!=NULL)
  f*=1000000L;
 else if(strchr(arg, 'K')!=NULL||strchr(arg, 'k')!=NULL)
  f*=1000L;
 if(f<10000000)
  fprintf(stderr, "CPU frequency of %lu Hz can't be accepted\n", f);
 else
  do_ioctl(NULL, 0, &f, sizeof(f), VIAAPM_CMD_SET_CPU_FREQ, IOCTL_CAT_VIAAPM, hf);
}

/* ** Undocumented ** */

static void query_cpu_freq()
{
 unsigned long f;

 do_ioctl(&f, sizeof(f), NULL, 0, VIAAPM_CMD_QUERY_CPU_FREQ, IOCTL_CAT_VIAAPM, hf);
 f/=10000;
 fprintf(stderr, "CPU frequency is %lu.%02lu MHz\n", f/100, f%100);
}

/* Main routine */

int main(int argc, char **argv)
{
 unsigned int i;
 USHORT rc, t;
 ULONG action;

 if(argc<2||!stricmp(argv[1], "/?"))
 {
  fprintf(stderr, "APM/ACPI control program v 2.50 on " __DATE__ ", " __TIME__ "\n"
                  "\n"
                  "Usage: APMCTRL <option> [<option> ...]\n"
                  "\n"
                  "General VIA chipset options:\n"
                  "          /RESET     Reconfigure the chipset and reinstall idle hook\n"
                  " /T[:<value>[%%]]     Query/set throttling mode\n"
                  "              /Q     Query chipset type\n"
                  "\n"
/* This is just "FYI", we aren't going to document this. */
#if 0
                  "ICS942xx PLL options (where available):\n"
                  "      /FSB[:<x>]     Query/set front-side bus frequency (xxxxxxK[Hz]|xxxM[Hz])\n"
                  "  /CPUFREQ[:<x>]     Query/set CPU frequency (requires /CPUMUL for VIAAPM.SYS)\n"
#endif
                  );
  return(1);
 }
 if(DosOpen(APM_DRIVER, &hf, &action, 0L, FILE_NORMAL,
            OPEN_ACTION_FAIL_IF_NEW|OPEN_ACTION_OPEN_IF_EXISTS,
            OPEN_ACCESS_READONLY|OPEN_SHARE_DENYNONE, 0L))
 {
  fprintf(stderr, "APM driver not installed\n");
  return(2);
 }
 do_ioctl(&rc, 2, NULL, 0, VIAAPM_CMD_QUERY_HOOK, IOCTL_CAT_VIAAPM, hf);
 if(!rc)
 {
  fprintf(stderr, "APM hook not installed\n");
  return(3);
 }
 for(i=1; i<argc; i++)
 {
  if(!stricmp(argv[i], "/RESET"))
   reset();
  else if(!memicmp(argv[i], "/T:", 3))
   set_throttling(argv[i]+3);
  else if(!stricmp(argv[i], "/T"))
   query_throttling();
  else if(!stricmp(argv[i], "/Q"))
   query_chipset();
  else if(!memicmp(argv[i], "/FSB:", 5))
   set_fsb(argv[i]+5);
  else if(!stricmp(argv[i], "/FSB"))
   query_fsb();
  else if(!memicmp(argv[i], "/CPUFREQ:", 9))
   set_cpu_freq(argv[i]+9);
  else if(!stricmp(argv[i], "/CPUFREQ"))
   query_cpu_freq();
  else
   fprintf(stderr, "Invalid parameter <%s> ignored\n", argv[i]);
 }
 DosClose(hf);
 return(0);
}
