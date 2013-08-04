/* $Id: 16chcfg.c,v 1.11 2005/01/06 11:56:13 root Exp $ */

#include <libvia.h>

#include "viaapm.h"
#include "viaapmp.h"                    /* For program_throttling_timer() */
#include "16chcfg.h"
#include "externs.h"

/* CPU multiplier bit map. It comes from WPCREDIT description file for KT266A
   available at http://www.viatech.com and is limited to 1666 MHz (12.5x133)
   CPUs - i.e. Athlon XP 2000+. AMD technote #24309 tells that all subsequent
   multipliers share the value for 12.5. */

static unsigned char far kt266_cpu_mul[]={11, 12, 5, 6, 7, 8, 9, 10,
                                          3, 4, 0, 0, 0, 0, 0, 0};

static void program_throttling_timer(unsigned int mode, unsigned int base);

/*
 * PCI configuration section
 */

/* Set up the north bridge */

int config_chip()
{
 unsigned long r, m;
 unsigned char p, a;
 int chip, schip;

 /* BUGBUG: this approach is inefficient as it scans bus too many times! */
 if(!VIA_locate_pci_device(MKPCIID(0x1106, 0x3099))||!VIA_locate_pci_device(MKPCIID(0x1106, 0x3089)))
  chip=CHIP_KT266;
 else if(!VIA_locate_pci_device(MKPCIID(0x1106, 0x3156))) /* VT8372 */
  chip=CHIP_KN266;
 else if(!VIA_locate_pci_device(MKPCIID(0x1106, 0x3189))|| /* VT8377 (what about VT8368?) */
         !VIA_locate_pci_device(MKPCIID(0x1106, 0x3205))) /* KM400 */
  chip=CHIP_KT400;
 else if(!VIA_locate_pci_device(MKPCIID(0x1106, 0x2269))) /* VTxxxx (northbridge subfunction 2) */
  chip=CHIP_KT880;
 else if(!VIA_locate_pci_device(MKPCIID(0x1106, 0x0305))||!VIA_locate_pci_device(MKPCIID(0x1106, 0x3911))||
         !VIA_locate_pci_device(MKPCIID(0x1106, 0x3051))||!VIA_locate_pci_device(MKPCIID(0x1106, 0x3112))||
         !VIA_locate_pci_device(MKPCIID(0x1106, 0x6911)))
  chip=CHIP_KT133;
 else if(!VIA_locate_pci_device(MKPCIID(0x1106, 0x0391))) /* 3109 removed - what's that one for? */
  chip=CHIP_KX133;
 else
  chip=CHIP_UNKNOWN;
 switch(chip)
 {
  case CHIP_KT133:
  case CHIP_KX133:
  case CHIP_KT266:
  case CHIP_KN266:
  case CHIP_KT400:
   /* Offsets to commonly used locations relative to original KT133: */
   if(chip==CHIP_KT266||chip==CHIP_KN266)
    a=0x40;
   else if(chip==CHIP_KT400)
    a=0x80;
   else
    a=0;
   /* Enable the CPU disconnection from the north bridge */
   p=0x50+a;
   VIA_get_pci_config_space(p, &r, 4);
   VIA_set_pci_config_space(p, r|=0x800000, 4);
   /* Enable/disable HLT detection as instructed to */
   p=0x54+a;
   VIA_get_pci_config_space(p, &r, 4);
   m=(chip==CHIP_KT266||chip==CHIP_KN266)?0x200:0x100;
   if(no_hlt)
    r&=~m;
   else
    r|=m;
   VIA_set_pci_config_space(p, r, 4);
   /* The KT/KN266 chipsets also store the CPU multiplier (in a limited
      range) */
   if((chip==CHIP_KT266||chip==CHIP_KN266)&&cpu_multiplier==0)
   {
    cpu_multiplier=kt266_cpu_mul[r>>28]<<1;
    if(r&0x08000000)
     cpu_multiplier++;
   }
   break;
  case CHIP_KT880:
   /* Enable the CPU disconnection from the north bridge. The "HLT detection"
      bit hardly fulfils its designation here, and we are forced to use
      ACPI C2 readouts on KT880. */
   p=0x80;
   VIA_get_pci_config_space(p, &r, 4);
   VIA_set_pci_config_space(p, r|=0x00800000, 4);
   p=0x84;
   VIA_get_pci_config_space(p, &r, 4);
   VIA_set_pci_config_space(p, r|=0x00000200, 4);
   break;
  default:
   return(CONF_UNKNOWN_CHIP);
 }
 /* Locate and configure the south bridge */
 if(!VIA_locate_pci_device(MKPCIID(0x1106, 0x3074))||!VIA_locate_pci_device(MKPCIID(0x1106, 0x3109))||
    !VIA_locate_pci_device(MKPCIID(0x1106, 0x3147)))
  schip=CHIP_KT266;
 else if(!VIA_locate_pci_device(MKPCIID(0x1106, 0x3177)))
  schip=CHIP_KT400;
 else if(!VIA_locate_pci_device(MKPCIID(0x1106, 0x3227)))       /* VT8237 */
  schip=CHIP_KT880;
 else if(!VIA_locate_pci_device(MKPCIID(0x1106, 0x3057)))
  schip=chip;
 else
  schip=CHIP_UNKNOWN;
 switch(schip)
 { 
  case CHIP_KX133:
  case CHIP_KT133:
  case CHIP_KT266:
  case CHIP_KT400:
  case CHIP_KT880:
   p=0x40+((schip==CHIP_KT266||schip==CHIP_KT400||schip==CHIP_KT880)?0x40:0);
   /* Enable access to ACPI registers */
   VIA_get_pci_config_space(p, &r, 4);
   VIA_set_pci_config_space(p, r|=0x8000, 4);
   /* Query the register address */
   VIA_get_pci_config_space(p+8, &r, 4);
   acpi_base=(unsigned short)(r&0x0000FF80);
   /* Configure the throttling timer */
   if(schip!=CHIP_KT400&&schip!=CHIP_KT880&&throttling_timer!=TT_DEFAULT)
    program_throttling_timer(throttling_timer, p);
   break;
  default:
   return(CONF_UNKNOWN_CHIP);
 }
 chip_type=chip;
 if(chip_type!=CHIP_UNKNOWN)
  south_bridge=VIA_pci_mask;
 acpi_c2_port=acpi_base+0x14;
 acpi_c3_port=acpi_base+0x15;
 return(acpi_base>=0x80?0:CONF_BAD_ACPI);
}

/*
 * Throttling section
 */

/* Sets throttling mode via the ACPI registers */

void set_throttling(unsigned short cycle)
{
 unsigned long r;

 if(cycle==0)
  return;
 r=inpd(acpi_base+0x10)&0xFFFFFFE0;
 if(cycle<16)
  r|=0x10|cycle;
 outpd(acpi_base+0x10, r);
}

/* Queries the throttling mode */

unsigned short query_throttling()
{
 unsigned long r;

 r=inpd(acpi_base+0x10)&0x1F;
 if(r&0x10)
  return(r&0x0F);
 else
  return(16);
}

/* Programs the throttling timer with one of predefined cycle lengths */

static void program_throttling_timer(unsigned int mode, unsigned int base)
{
 unsigned long gc0, tc;

 VIA_get_pci_config_space(base, &gc0, 4);
 VIA_get_pci_config_space(base+12, &tc, 4);
 switch(throttling_timer)
 {
  case TT_FAST:
   tc|=0x10;
   break;
  case TT_MEDIUM:
   tc&=~0x1000;
   gc0=~0x0200;
   break;
  case TT_SLOW:
   tc&=~0x1000;
   gc0|=0x0200;
   break;
 }
 VIA_set_pci_config_space(base, gc0, 4);
 VIA_set_pci_config_space(base+12, tc, 4);
}
