/* $Id: 16init.c,v 1.35 2005/01/06 11:56:13 root Exp $ */

/* Device driver initialization and I/O part */

#define INCL_BASE
#include <os2.h>

#include <strat2.h>
#include <reqpkt.h>
#include <devcmd.h>
#include <dhcalls.h>
#include "contrib/apmcalls.h"

#include <kprintf.h>

#include "viaapm.h"
#include "viaapmp.h"
#include "16chcfg.h"
#include "16args.h"
#include "16smbus.h"
#include "externs.h"
#include "16pll.h"

/* Imports from the assembly-language module */

extern char EndOfCode, EndOfData;
extern int do_init32();

/* Local forward-referenced functions */

static USHORT strategy(RPH FAR *p, int is_primary);
static int send_init32(int init_time);
static int receive_init32();
static USHORT proc_ioctl(PRP_GENIOCTL p);

static char error_blurb[]="\n\x1B[1mERROR: %s\x1b[0m\n\n";

/*
 * Strategy section
 */

/* Primary strategy entry point */

void far primary_strategy()
{
 RPH FAR *p;

 /* Save the request packet address */
 _asm
 {
  mov word ptr p[0], bx
  mov word ptr p[2], es
 }
 p->Status=strategy(p, 1);
}

/* Secondary strategy entry point */

void far secondary_strategy()
{
 RPH FAR *p;

 /* Save the request packet address */
 _asm
 {
  mov word ptr p[0], bx
  mov word ptr p[2], es
 }
 p->Status=strategy(p, 0);
}

/* PLL initialization forwarder */

static void init_pll_scrn()
{
 unsigned long f;

 if(use_pll!=PLL_NONE)
 {
  if(init_pll(use_pll))
   kprintf("Warning: cannot initialize PLL - FSB control will be disabled\n");
  else if(verbose_init)
  {
   f=query_pll_freq();
   if(f==0)
   {
    kprintf("Warning: PLL lied about frequency - FSB control will be disabled\n");
    init_pll(PLL_NONE);
   }
   else
   {
    kprintf("Software programmable PLL: ");
    f/=10000L;
    if(cpu_multiplier!=0)
    {
     f=(f>>1L)*(unsigned long)cpu_multiplier;
     kprintf("CPU frequency is %lu.%02lu MHz\n", f/100L, f%100L);
    }
    else
     kprintf("FSB frequency is %lu.%02lu MHz\n", f/100L, f%100L);
   }
  }
 }
}

/* Initialization handler */

static USHORT init(RPH far *p, int is_primary)
{
 PRPINITOUT po;
 USHORT rc;
 char e[80];

 po=(RPINITOUT FAR *)p;
 if(is_primary)
 {
  Device_Help=((PRPINITIN)p)->DevHlpEP;
  parse_args(((PRPINITIN)p)->InitArgs, apm_params);
  if(!quiet_init)
   kprintf("\nVIA ACPI-compliant power saver v 2.51\n");
  if(verbose_init)
   kprintf("Will use a kernel ACPI hook%s to disconnect CPU\n\n", no_hlt?"":" and HLT detection");
 }
 /* Configure the devices */
 if(is_primary)
 {
  if((rc=config_chip())!=0)
  {
   if(rc==CONF_UNKNOWN_CHIP)
    sprintf(e, "No compatible chip detected");
   else if(rc==CONF_BAD_ACPI)
    sprintf(e, "Bad ACPI base port: 0x%04x", acpi_base);
   else
    sprintf(e, "error code=%u", rc);
   kprintf(error_blurb, e);
   po->CodeEnd=0;
   po->DataEnd=0;
   return(RC_BUG);
  }
  /* Configure the optional subfeatures */
  init_smbus();
  if(verbose_init&&smbus_addr>0)
   kprintf("SMBus on port 0x%04x\n", smbus_addr);
  init_pll_scrn();
 }
 else
 {
  /* Now, if we are the "secondary device", send IOCTL to the "primary device",
     which has completed init and will do the ring 0 part of job in a 32-bit
     segment */
  if(send_init32(1))
   return(RC_BUG);
 }
 /* Sign off (both devices will have to do the same) */
 if(is_primary)
 {
  if(verbose_init)
   DosSleep(5000);
  /* The "primary device" finishes normally */
  po->CodeEnd=(USHORT)&EndOfCode;
  po->DataEnd=(USHORT)&EndOfData;
  return(STATUS_DONE);
 }
 else
 {
  /* Allow the "secondary device" to die silently after it has served the
     purpose of initializing the "primary device" */
  po->CodeEnd=0;
  po->DataEnd=0;
  return(STATUS_DONE|STERR|ERROR_I24_QUIET_INIT_FAIL);
 }
 /* NOTREACHED */
}

/* Responds to APM BIOS events */

static USHORT FAR apm_hdl(PAPMEVENT pev)
{
 USHORT pstate;

 switch((USHORT)pev->ulParm1)
 {
  case APM_SETPWRSTATE:
   if((USHORT)(pev->ulParm2>>16)!=APM_PWRSTATEREADY)
   {
    /* Nothing to be done for suspend */
   }
   break;
  case APM_NORMRESUMEEVENT:;            /* fallthru */
  case APM_CRITRESUMEEVENT:
   config_chip();
   reconfigure_pll();
   break;
 }
 return(0);
}

/* Common strategy */

static USHORT strategy(RPH far *p, int is_primary)
{
 PRPSAVERESTORE psr;
 PVOID pparm, pdata;

 /* Save the request packet address */
 _asm
 {
  mov word ptr p[0], bx
  mov word ptr p[2], es
 }
 /* What kind of command was it? */
 switch(p->Cmd)
 {
  case CMDInit:
   return(init(p, is_primary));
  case CMDInitComplete:                 /* InitComplete - reinstall the hook
                                           if it was taken out by other
                                           APM-related drivers */
   is_inited=1;
   if(receive_init32()!=HOOK_OK)
    return(RC_BUG);
   if(!APMAttach())
    APMRegister(apm_hdl, APM_NOTIFYSETPWR|APM_NOTIFYNORMRESUME|APM_NOTIFYCRITRESUME, 0);
   return(STATUS_DONE);
  case CMDGenIOCTL:
   if(is_primary&&((PRP_GENIOCTL)p)->Category==IOCTL_CAT_VIAAPM)
    return(proc_ioctl((PRP_GENIOCTL)p));
   return(RC_BUG);                      /* Bogus request */
  case CMDSaveRestore:
   psr=(PRPSAVERESTORE)p;
   if(psr->FuncCode==0)
    ;                                   /* Save => do nothing */
   else if(psr->FuncCode==1)
   {
    config_chip();                      /* Restore => reconfigure the chip */
    reconfigure_pll();                  /* Restore the CPU frequency */
   }
   return(STATUS_DONE);
  case CMDOpen:
  case CMDClose:
  case CMDDeInstall:
  case CMDShutdown:
   return(STATUS_DONE);
  default:
   return(STATUS_DONE|STERR|STATUS_ERR_UNKCMD);
 }
 /* We shouldn't get here! */
 _asm
 {
  int 3
 }
}

/*
 * IOCTL communication with the outside world
 */

static USHORT proc_ioctl(PRP_GENIOCTL p)
{
 USHORT rc, v;
 PVOID pparm, pdata;
 USHORT parm_len, data_len;
 
 pparm=p->ParmPacket;
 pdata=p->DataPacket;
 parm_len=p->ParmLen;
 data_len=p->DataLen;
 /* Verify the access to parameter/result data areas */
 if(is_inited)
 {
  if(p->Function&0x20||p->Function==VIAAPM_CMD_RESET)
  {
   if(DevHelp_VerifyAccess(SELECTOROF(pdata), data_len, OFFSETOF(pdata),
                           VERIFY_READWRITE))
    return(RC_BUG);
  }
  if((p->Function&0x20)==0&&p->Function!=VIAAPM_CMD_RESET)
  {
   if(DevHelp_VerifyAccess(SELECTOROF(pparm), parm_len, OFFSETOF(pparm),
                           VERIFY_READONLY))
    return(RC_BUG);
  }
 }
 /* Process the functions */
 switch(p->Function)
 {
  case VIAAPM_CMD_SET_HOOK:
   rc=(USHORT)receive_init32();
   if(data_len>=2)
    *((PUSHORT)pdata)=rc;
   return(STATUS_DONE);
  case VIAAPM_CMD_QUERY_HOOK:
   /* QUERY_HOOK is now almost the same as SET_HOOK as we remember the
      hook address and searching the kernel is no longer a problem */
   if(data_len>=2)
    *((PUSHORT)pdata)=(receive_init32()==HOOK_OK)?1:0;
   return(STATUS_DONE);
  case VIAAPM_CMD_RESET:
   rc=config_chip();
   if(data_len>=2)
    *((PUSHORT)pdata)=rc;
   return(STATUS_DONE);
  case VIAAPM_CMD_SET_THROTTLING:
   if(parm_len>=2)
    v=*((PUSHORT)pparm);
   else
    return(RC_BUG);
   set_throttling(v);
   return(STATUS_DONE);
  case VIAAPM_CMD_QUERY_THROTTLING:
   rc=query_throttling();
   if(data_len>=2)
    *((PUSHORT)pdata)=rc;
   return(STATUS_DONE);
  case VIAAPM_CMD_QUERY_CHIPSET:
   if(data_len>=2)
    *((PUSHORT)pdata)=(USHORT)chip_type;
   return(STATUS_DONE);
  case VIAAPM_CMD_SET_FSB:
   if(parm_len<sizeof(ULONG))
    return(RC_BUG);
   return(set_pll_freq(*(PULONG)pparm)?RC_BUG:STATUS_DONE);
  case VIAAPM_CMD_QUERY_FSB:
   if(data_len<sizeof(ULONG))
    return(RC_BUG);
   *(PULONG)pdata=query_pll_freq();
   return(STATUS_DONE);
  case VIAAPM_CMD_SET_CPU_FREQ:
   if(parm_len<sizeof(ULONG)||cpu_multiplier==0)
    return(RC_BUG);
   return(set_pll_freq(((*(PULONG)pparm)/cpu_multiplier)<<1)?RC_BUG:STATUS_DONE);
  case VIAAPM_CMD_QUERY_CPU_FREQ:
   if(data_len<sizeof(ULONG)||cpu_multiplier==0)
    return(RC_BUG);
   *(PULONG)pdata=(query_pll_freq()>>1)*cpu_multiplier;
   return(STATUS_DONE);
 }
 return(RC_BUG);
}

/*
 * IOCTL IDC section
 */

/* Open the device and send init32 request IOCTL */

static int send_init32(int init_time)
{
 HFILE hf;
 USHORT action;
 USHORT rc;
 char e[80];

 if(DosOpen(APM_DRIVER, &hf, &action, 0L, FILE_NORMAL,
            OPEN_ACTION_FAIL_IF_NEW|OPEN_ACTION_OPEN_IF_EXISTS,
           OPEN_ACCESS_READONLY|OPEN_SHARE_DENYNONE, 0L))
  return(1);
 if(DosDevIOCtl2(&rc, 2, NULL, 0, VIAAPM_CMD_SET_HOOK, IOCTL_CAT_VIAAPM, hf))
 {
  DosClose(hf);
  return(1);
 }
 DosClose(hf);
 if(rc&&init_time)
 {
  if(rc==HOOK_NO_MTE||rc==HOOK_NO_OBJTAB)
   sprintf(e, "Error processing kernel tables");
  else if(rc==HOOK_NO_ANCHOR)
   sprintf(e, "Unknown kernel");
  else if(rc==HOOK_NO_APM_AREA)
   sprintf(e, "Can't find APM setup block");
  else if(rc==HOOK_NO_APM_SIG)
   sprintf(e, "Can't find APM hook variable");
  else if(rc==HOOK_NO_TRAP_SCRN)
   sprintf(e, "Can't install trap screen hook");
  else
   sprintf(e, "Hook installation failed, error code %u", rc);
  kprintf(error_blurb, e);
 }
 return(rc);
}

/* Handle the init32 request */

static int receive_init32()
{
 int rc;

 rc=do_init32();
 return(rc);
}
