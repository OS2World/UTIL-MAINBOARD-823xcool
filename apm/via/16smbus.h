/* $Id: 16smbus.h,v 1.2 2002/06/13 00:11:38 root Stab $ */

#ifndef _16SMBUS_INCLUDED
#define _16SMBUS_INCLUDED

int read_smbus(unsigned char dev, char *dest, int bytes);
int write_smbus(unsigned char dev, char *dest, int bytes);
int init_smbus();

extern unsigned int smbus_addr;

#endif
