/* $Id: 16smbus.c,v 1.7 2004/10/14 15:36:08 root Exp $ */

#include <libvia.h>

#include "viaapm.h"
#include "16smbus.h"
#include "externs.h"

/* PCI space addresses */

#define PCI_686A_SMBUS_IO       0x90    /* I/O port */
#define PCI_8233_SMBUS_IO       0xD0
#define PCI_SMBUS_HOST_CFG      0xD2    /* Host configuration */
 #define PCI_SMBUS_SCI          0x08    /* SCI interrupt mode */
 #define PCI_SMBUS_ENABLED      0x01    /* Enable SMBus */
/* Slave triggers skipped */
#define PCI_SMBUS_REV           0xD6    /* SMBus revision ID */

/* I/O space addresses for lookup */

#define IO_SMBUS_HIGH         0x5000
#define IO_SMBUS_LOW          0x4100
#define IO_SMBUS_DECR         0x0040

/* I/O space */

#define SM_HOST_STATUS          0x00    /* Host status register */
 #define SM_HOST_BUSY           0x01
 #define SM_RESET               0xFF    /* "Magic" reset sequence for 8233
                                           and maybe 82C686 too */
#define SM_SLAVE_STATUS         0x01    /* Slave status register */
 #define SM_SLAVE_BUSY          0x01
#define SM_OP                   0x02    /* Operations register */
 #define SM_START               0x40    /* Start transaction */
 #define SM_BLOCK_OP            0x14    /* Block R/W flags */
#define SM_CMD                  0x03    /* Command register */
#define SM_ADDR                 0x04    /* Device address register */
 #define SM_ADDR_READ           0x01    /* Odd addresses are for reading */
#define SM_BYTECNT              0x05    /* Byte count register */
#define SM_BLOCK_DATA           0x07    /* Block operation data */

/* I/O address for SMBus transactions */

unsigned int smbus_addr=0;

/* Waits for SMBus to become ready for transaction */

static void smwait()
{
 unsigned long ctr=0;

 /* BUGBUG: This may turn into an endless loop so let's cheat! */
 while(((inp(smbus_addr+SM_HOST_STATUS)&SM_HOST_BUSY)||
       (inp(smbus_addr+SM_SLAVE_STATUS)&SM_SLAVE_BUSY))&&ctr<100000)
  ctr++;
}

/* Poll the operations register to commence data transfer */

static void smpoll()
{
 inp(smbus_addr+SM_OP);
}

/* Block-reads the device I/O space */

int read_smbus(unsigned char dev, char *dest, int bytes)
{
 int i;

 if(smbus_addr==0)
  return(-1);
 /* No further verification, unfortunately */
 smwait();
 outp(smbus_addr+SM_ADDR, (unsigned char)(dev|SM_ADDR_READ));
 outp(smbus_addr+SM_CMD, 0);
 outp(smbus_addr+SM_OP, SM_START|SM_BLOCK_OP);
 smwait();
 smpoll();
 for(i=0; i<bytes; i++)
  dest[i]=inp(smbus_addr+SM_BLOCK_DATA);
 return(0);
}

/* Block-writes the device I/O space */

int write_smbus(unsigned char dev, char *dest, int bytes)
{
 int i;

 if(smbus_addr==0)
  return(-1);
 /* No further verification, unfortunately. */
 smwait();
 smpoll();
 /* Hack for 8233 */
 outp(smbus_addr+SM_HOST_STATUS, SM_RESET);
 smwait();
 /* Set transfer parameters and do it */
 outp(smbus_addr+SM_CMD, 0);
 outp(smbus_addr+SM_ADDR, (unsigned char)(dev&~SM_ADDR_READ));
 outp(smbus_addr+SM_BYTECNT, (unsigned char)bytes);
 outp(smbus_addr+SM_OP, SM_START|SM_BLOCK_OP);
 for(i=0; i<bytes; i++)
  outp(smbus_addr+SM_BLOCK_DATA, dest[i]);
 return(0);
}

/* Initializes the System Management bus. Presumes that the south bridge
   has been "selected" in LIBVIA. */

int init_smbus()
{
 unsigned long v;
 unsigned int n, p, j;

 if(south_bridge==0)
  return(-1);
 VIA_pci_mask=south_bridge;
 n=(chip_type==CHIP_KT133||chip_type==CHIP_KX133)?PCI_686A_SMBUS_IO:PCI_8233_SMBUS_IO;
 VIA_get_pci_config_space((unsigned char)n, &v, 4);
 if(v==1)                               /* Means "unconfigured" */
 {
  /* Attempt the locations from "standard" SMBus address down to the
     "standard" ACPI address. */
  for(p=IO_SMBUS_HIGH; p>=IO_SMBUS_LOW; p-=IO_SMBUS_DECR)
  {
   /* Probe 16 locations - a dirty hack, but we weren't expected here! */
   for(j=0; j<16&&inp(j)!=0xFF; j++)
    ;
   if(j==16)
    break;
  }
  if(p<IO_SMBUS_LOW)
   return(1);
  /* Set the new field */
  v=p|1;
  VIA_set_pci_config_space((unsigned char)p, v, 4);
 }
 smbus_addr=v&0xFFF0;
 n=PCI_SMBUS_HOST_CFG&0xFC;
 VIA_get_pci_config_space((unsigned char)n, &v, 4);
 n|=((unsigned long)PCI_SMBUS_ENABLED)<<16;
 VIA_set_pci_config_space((unsigned char)n, v, 4);
 return(0);
}
