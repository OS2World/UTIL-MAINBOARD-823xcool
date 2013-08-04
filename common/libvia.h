/* $Id: libvia.h,v 1.4 2003/11/19 21:13:24 root Exp $ */

#ifndef LIBVIA_INCLUDED
#define LIBVIA_INCLUDED

/*
 * VIA PCI API
 */

/* PCI */

#define MKPCIMASK(b, d, f) (0x80000000|((b)<<16)|((d)<<11)|((f)<<8))
#define MKPCIID(v, d) ((((unsigned long)d)<<16)|(v))

#define PCI_MAX_BUS               256
#define PCI_MAX_DEV                32
#define PCI_MAX_FUNC                8

void VIA_get_pci_config_space(unsigned int reg, unsigned long far *dest, unsigned int count);
void VIA_set_pci_config_space(unsigned int reg, unsigned long data, unsigned int count);
/* Locates a device - in case of success, returns 1. Bus mask updated
   accordingly. Sample ID: 0x11063059.
   ID format: Most significant word  = vendor ID
              Least significant word = device ID */
int VIA_locate_pci_device(unsigned long id);
extern unsigned long VIA_pci_mask;

/* Low-level */

unsigned char VIA_inpb(unsigned short port);
unsigned long VIA_inpd(unsigned short port);
void VIA_outpb(unsigned short port, unsigned char data);
void VIA_outpd(unsigned short port, unsigned long data);

/* Translate from clumsy VIA interfaces to something more usual */

#define inp(port) VIA_inpb(port)
#define outp(port, v) VIA_outpb(port, v)
#define inpd(port) VIA_inpd(port)
#define outpd(port, v) VIA_outpd(port, v)

#endif
