/* $Id: libvia.c,v 1.1 2003/11/19 21:13:24 root Exp $ */

#include "libvia.h"

/* Byte count filter */

static unsigned long countmask[5]={0x00000000,
                                   0x000000FF,
                                   0x0000FFFF,
                                   0x00FFFFFF,
                                   0xFFFFFFFF};

unsigned long VIA_pci_mask;

static unsigned long via_get_pci_quad(unsigned int reg)
{
 VIA_outpd(0xCF8, VIA_pci_mask|reg);
 return(VIA_inpd(0xCFC));
}

static void via_set_pci_quad(unsigned int reg, unsigned long v)
{
 VIA_outpd(0xCF8, VIA_pci_mask|reg);
 VIA_outpd(0xCFC, v);
}

void VIA_get_pci_config_space(unsigned int reg, unsigned long far *dest, unsigned int count)
{
 unsigned long rc;
 unsigned int r, sh;
 unsigned long f1, f2;

 r=reg&~3;
 sh=reg&3;
 f1=via_get_pci_quad(r)&~countmask[sh];
 f2=(sh+count>4)?via_get_pci_quad(r+4)&countmask[sh]:0;
 rc=(f1>>(sh<<3))|(f2<<((4-sh)<<3));
 *dest=rc&countmask[count];
}

void VIA_set_pci_config_space(unsigned int reg, unsigned long v, unsigned int count)
{
 unsigned int r, sh;

 r=reg&~3;
 sh=reg&3;
 via_set_pci_quad(r, (via_get_pci_quad(r)&~(countmask[count]<<(sh<<3)))|((v&countmask[count])<<(sh<<3)));
 if(sh+count>4)
  via_set_pci_quad(r+4, (via_get_pci_quad(r+4)&~(countmask[sh+count-4]))|((v>>(4-sh<<3))&countmask[sh+count-4]));
}

int VIA_locate_pci_device(unsigned long id)
{
 unsigned long mask=MKPCIMASK(0, 0, 0), pm;
 unsigned int d, f;

 pm=VIA_pci_mask;
 for(d=0; d<PCI_MAX_DEV; d++)
 {
  for(f=0; f<PCI_MAX_FUNC; f++)
  {
   VIA_pci_mask=MKPCIMASK(0, d, f);
   if(via_get_pci_quad(0)==id)
    return(0);
  }
 }
 VIA_pci_mask=pm;
 return(1);
}
