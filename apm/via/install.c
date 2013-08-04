/* $Id: install.c,v 1.7 2002/06/13 00:11:39 root Stab $ */

/* Hook installation hack - really, it may be replaced by GetDosVar#16 DevHlp
   but if it works, then don't touch it */

#include <stddef.h>

#include "std32.h"

#include "contrib/ldrtypes.h"
#include "contrib/ldrmte.h"

#include "externs.h"
#include "patchram.h"
#include "viaapm.h"

/* ASM imported routines */

struct ldrmte_s *locate_krnl_mte();
void install_hook(unsigned char *dest, void (*laddr)());
void uninstall_hook(unsigned char *dest);
void apm_idle_hook();
void sys1503_dead_end();
void sys1503_reipl();
unsigned short get_cs();

/* Kernel SAB */

struct locator os2krnl_sab[]=
{
 {0, 1, 'S'}, 
 {1, 1, 'A'},
 {1, 1, 'B'},
 {1, 1, ' '},
 {1, 1, 'K'},
 {1, 1, 'N'},
 {1, 1, 'L'},
 {-1, -1, 0} 
};

/* ABIOS block */

struct locator os2krnl_abios[]=
{
 {0, 1, 'A'}, 
 {1, 1, 'B'},
 {1, 1, 'I'},
 {1, 1, 'O'},
 {1, 1, 'S'},
 {1, 1, '.'},
 {1, 1, 'S'},
 {1, 1, 'Y'},
 {1, 1, 'S'},
 {8, 1024, 0x1C},                       /* APM field starts */
 {1, 1, 0x00},
 {11, 1, 0x00},                         /* Destination starts */
 {1, 1, 0x00},
 {1, 1, 0x00},
 {1, 1, 0x00},
 {1, 1, 0x00},
 {1, 1, 0x00},
 {8, 512, '\\'},
 {1, 1, 'O'},
 {1, 1, 'S'},
 {1, 1, '2'},
 {1, 1, '\\'},
 {-1, -1, 0} 
};

/* Patch area */

struct locator os2krnl_apm[]=
{
 {1, 1, 0x1C},                          /* APM field starts */
 {1, 1, 0x00},
 {11, 1, 0x00},                         /* Destination starts */
 {1, 1, 0x00},
 {1, 1, 0x00},
 {1, 1, 0x00},
 {1, 1, 0x00},
 {1, 1, 0x00},
 {8, 256, '\\'},
 {1, 1, 'O'},
 {1, 1, 'S'},
 {1, 1, '2'},
 {1, 1, '\\'},
 {-1, -1, 0} 
};

/* Warp 3 hook area - unreliable! */

struct locator os2krnl_warp3_abios[]=
{
 {0, 1, 'A'}, 
 {1, 1, 'B'},
 {1, 1, 'I'},
 {1, 1, 'O'},
 {1, 1, 'S'},
 {1, 1, '.'},
 {1, 1, 'S'},
 {1, 1, 'Y'},
 {1, 1, 'S'},
 {8, 512, 0x00},                        /* Destination starts */
 {1, 1, 0x00},
 {1, 1, 0x00},
 {1, 1, 0x00},
 {1, 1, 0x00},
 {1, 1, 0x00},
 {7, 1, 0x01},                          /* Beacon of hope (DDInitTime -
                                           reset to 0 after all DDs are
                                           processed) */
 {2, 1, 0x00},                          /* Known null area - syiGlobalFlags:
                                           changed when the daemons are about
                                           to start */
 {1, 1, 0x00},
 {1, 1, 0x00},
 {1, 1, 0x00},
 {1, 2, 0x00},                          /* [alignment] + BytesPerSector */
 {1, 1, 0x02},                          /* 200h */
 {1, 1, 0x00},                          /* FatDirtyBit */
 {16, 64, 0x01},
 {16, 256, '\\'},
 {1, 1, 'O'},
 {1, 1, 'S'},
 {1, 1, '2'},
 {1, 1, '\\'},
 {-1, -1, 0} 
};

/* Warp 3 patch area */
 
struct locator os2krnl_warp3_apm[]=
{
 {0, 1, 0x00},                          /* Destination starts */
 {1, 1, 0x00},
 {1, 1, 0x00},
 {1, 1, 0x00},
 {1, 1, 0x00},
 {1, 1, 0x00},
 {7, 1, 0x01},                          /* Beacon of hope (DDInitTime -
                                           reset to 0 after all DDs are
                                           processed) */
 {2, 1, 0x00},                          /* Known null area - syiGlobalFlags:
                                           changed when the daemons are about
                                           to start */
 {1, 1, 0x00},
 {1, 1, 0x00},
 {1, 1, 0x00},
 {1, 2, 0x00},                          /* [alignment] + BytesPerSector */
 {1, 1, 0x02},                          /* 200h */
 {1, 1, 0x00},                          /* FatDirtyBit */
 {16, 64, 0x01},
 {16, 256, '\\'},
 {1, 1, 'O'},
 {1, 1, 'S'},
 {1, 1, '2'},
 {1, 1, '\\'},
 {-1, -1, 0} 
};

/* Patch area for kernel dead-end #1 (trap screen with REIPL=OFF) */

struct locator os2krnl_trap_screen[]=
{
 {0, 1, 0x59},
 {1, 1, 0x66},
 {1, 1, 0x5B},
 {1, 1, 0x66},
 {1, 1, 0x58},
 {1, 1, 0xFB},
 {1, 1, 0xEB},
 {1, 1, 0xFD},
 {-1, -1, 0} 
};

/* Patch area for kernel dead-end #2 (SYS1503) */

struct locator os2krnl_sys1503[]=
{
 {0, 1, 0x2B},
 {1, 1, 0xDB},
 {1, 1, 0xB8},
 {1, 1, 0xDF},
 {1, 1, 0x05},
 {1, 1, 0x00},
 {1, 1, 0x00},
 {1, 1, 0x9A},                          /* +8: 6 bytes=addr */
 {7, 1, 0xF4},
 {1, 1, 0xEB},
 {1, 1, 0xFE},
 {-1, -1, 0} 
};

/* 32-bit kernel APM hook initialization */

int _far init32()
{
 struct ldrote_s *objtab;
 struct ldrmte_s *pmte;
 unsigned int i, j;
 struct area a;
 long o;
 int severity=HOOK_BAD_OTE;             /* Used to investigate the
                                           circumstances of search failure */
 static unsigned char *hook_address=NULL;
 int trap_hook_installed=0, idle_hook_installed=0;
 int sys1503_hook_installed=0;

 if(hook_address!=NULL)
 {
  /* Reinstate the hook at already known location */
  install_hook(hook_address, apm_idle_hook);
  return(HOOK_OK);
 }
 pmte=locate_krnl_mte();
 if(pmte==NULL)
  return(HOOK_NO_MTE);
 objtab=(struct ldrote_s *)pmte->mte_swapmte->smte_objtab;
 if(objtab==NULL)
  return(HOOK_NO_OBJTAB);
 /* Some segments in the middle of kernel are not accessible so we'd better
    refrain from looking into them.  */
 for(i=0; i<3&&i<pmte->mte_swapmte->smte_objcnt&&(!trap_hook_installed||!idle_hook_installed); i++)
 {
  /* Is this segment worth searching? */
  if(objtab[i].ote_size>16)
  {
   a.first=(char *)objtab[i].ote_base;
   a.len=objtab[i].ote_size;
   /* BUGBUG: hack for uninitialized areas - round down to the 32K boundary. */
   if(a.len>0x8000)
    a.len=0x8000;
   /* One of first code segments should contain the trap screen dead-end */
   if(!trap_hook_installed)
   {
    o=locate(os2krnl_trap_screen, (struct area *)SSToDS(&a), 0);
    if(o!=-1)
    {
     trap_hook_installed=1;
     a.pos=o;
     ac(0xFB);                          /* sti */
     ac(0xBA); aw(acpi_c2_port);        /* mov dx, <acpi_c2_port> */
     ac(0xEC);                          /* in al, dx */
     ac(0xEB); ac(0xFA);                /* jmp short ... */
    }
   }
   /* System Anchor block is at the segment we need to hack */
   if(!idle_hook_installed)
   {
    o=-1;
    severity=HOOK_NO_ANCHOR;
    do
    {
     o=locate(os2krnl_sab, (struct area *)SSToDS(&a), o+1);
    } while(o!=-1&&(o%16)!=0);
    if(o==-1)
    {
     /* Anchor block not found => try searching for a Warp 3 signature */
     if((o=locate(os2krnl_warp3_abios, (struct area *)SSToDS(&a), o+1))==-1||
        (o=locate(os2krnl_warp3_apm, (struct area *)SSToDS(&a), o))==-1)
      continue;
     /* Install the Warp 3 hook immediately ('o' already points to the APM hook
        offset) and jerk off */
     install_hook(hook_address=a.first+o, apm_idle_hook);
     idle_hook_installed=1;
     severity=HOOK_OK;
     continue;
    }
    /* Anchor block found => retrieve debug kernel information */
    if(a.first[o+7]!='R')
     is_debug_kernel=1;
    /* Find the ABIOS signature - if it's there, the APM block will go shortly
       afterwards */
    severity=HOOK_NO_APM_AREA;
    if((o=locate(os2krnl_abios, (struct area *)SSToDS(&a), o))==-1)
     continue;
    severity=HOOK_NO_APM_SIG;
    if((o=locate(os2krnl_apm, (struct area *)SSToDS(&a), o))==-1)
     continue;
    /* OK, we found the APM block - the patch area starts at 12th byte */
    o+=12;
    install_hook(hook_address=a.first+o, apm_idle_hook);
    idle_hook_installed=1;
    severity=HOOK_OK;
    continue;
   } /* !idle_hook_installed */
  }
 }
 /* If so far everything is still OK, then let's attempt installing the
    SYS1503 hook. */
 if(severity==HOOK_OK)
 {
  i=pmte->mte_swapmte->smte_objcnt-1;
  a.first=(char *)objtab[i].ote_base;
  a.len=objtab[i].ote_size;
  if((o=locate(os2krnl_sys1503, (struct area *)SSToDS(&a), 0))!=-1)
  {
   /* Copy the FAR32 address */
   for(j=0; j<6; j++)
    pf_OutMessage[j]=a.first[o+j+8];
   /* Compose the code to drive away from IBM's procedure */
   a.pos=o;
   ac(0xEA);                            /* jmp far ... */
   ad((unsigned long)(reipl_on_1503?sys1503_reipl:sys1503_dead_end));
   aw(get_cs());
   sys1503_hook_installed=1;
  }
 }
 /* If main hook is OK then check for other bugs */
 if(severity==HOOK_OK)
 {
  if(!trap_hook_installed)
   severity=HOOK_NO_TRAP_SCRN;
  else if(!sys1503_hook_installed)
   severity=HOOK_NO_SYS1503;
 }
 return(severity);
}
