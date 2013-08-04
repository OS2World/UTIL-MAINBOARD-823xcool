/* $Id: 16pll.c,v 1.3 2002/06/12 17:31:49 root Stab $ */

#include <libvia.h>

#include "viaapm.h"
#include "viaapmp.h"
#include "16pll.h"
#include "16smbus.h"
#include "externs.h"

/* Configuration settings */

#define BAD_DIVISOR                0    /* If cannot be estimated */
#define MIN_ERR                    1    /* Less accuracy means more speed */
#define PLL_BYTES                 32    /* Max. bytes to hold PLL data */
#define RESTORE_FREQ      0xFFFFFFFF    /* Magic word to restore the frequency */

/* ICS94215/94228 parameters */

#define ICS942XX_BASE  (14318180>>1)    /* Base frequency */
#define ICS942XX_BYTES            17    /* Configuration size */
#define ICS942XX_DEV            0xD2    /* SMBus/I2C device ID */
#define ICS942XX_MIN_REF           2    /* Min. divider */
#define ICS942XX_MIN_VCO           8    /* Min. divider */
/* Enable/disable capabilities register */
#define ICS94215_CAPS           0x01
 #define ICS94215_ENABLE        0x00    /* CPUCLK (BUGBUG: do we need 0x40?) */
#define ICS94228_CAPS           0x05
 #define ICS94228_ENABLE        0x00    /* CPUCLK+FS3 (readback). The Win32
                                           tool CPUFSB sets 0xC0 here but it
                                           leads to intermittent crashes. */
/* Byte Count register */
#define ICS942XX_BYTE_COUNT     0x08    /* Should be ==ICS942XX_BYTES */
/* VCO Control */
#define ICS94215_VCO_CTRL       0x0A    /* VCO/watchdog configuration */
 #define ICS94215_VCO_ENABLE    0x80    /* Enable the custom frequency */
/* VCO/REF Data */
#define ICS94215_REF            0x0B
#define ICS94215_VCO            0x0C
#define ICS94228_REF            0x09
#define ICS94228_VCO            0x0A

/* static data */

static int pll_type=PLL_NONE;
static unsigned char pll_dev=0;
static unsigned char pll_bytes=0;
static unsigned long prev_freq=0;

/* PLL divisor calculation. Returns BAD_DIVISOR if failed. */

static unsigned short get_divisor(unsigned long hz)
{
 unsigned long vco, ref, err_vco, min_err=0xFFFFFFFF, min_vco=0, min_ref=0;
 unsigned long v;

 v=hz/(ICS942XX_BASE/1000);
 if(v<1000)                             /* Bogus frequency */
  return(BAD_DIVISOR);
 /* Now choose VCO and REF that will become V after division. According to
    ICS94215 specs, VCO = 8000...519000, REF=2...129. However, REF=2 doesn't
    work on an ICS94228. */
 for(ref=3; ref<=129; ref++)
 {
  vco=ref*v;
  if(vco>=8000&&vco<=519000)
  {
   err_vco=vco%1000;
   vco/=1000;
   if(err_vco>500)
   {
    err_vco=1000-err_vco;
    vco++;
   }
   if(err_vco<min_err)
   {
    min_vco=vco;
    min_ref=ref;
    min_err=err_vco;
    if(err_vco<MIN_ERR)
     break;
   }
  }
 }
 return((unsigned short)(min_ref-ICS942XX_MIN_REF)+
        ((unsigned short)((min_vco-ICS942XX_MIN_VCO)<<7)));
}

/* Reads the PLL registers */

static int read_pll(char *dest)
{
 int rc;
 unsigned char i, c;

 _asm{cli};
 rc=read_smbus(pll_dev, dest, pll_bytes);
 _asm{sti};
 if(rc)
  return(rc);
 c=dest[0];
 for(i=1; i<pll_bytes; i++)
 {
  if(dest[i]!=c)
   return(0);
 }
 return(-1);                            /* All PLL bytes matched, it's
                                           unlikely */
}

/* Writes the PLL registers */

static void write_pll(char *dest)
{
 _asm{cli};
 write_smbus(pll_dev, dest, pll_bytes);
 _asm{sti};
}

/* Queries the PLL output frequency (Hz). 0=invalid frequency. */

unsigned long query_pll_freq()
{
 char pll_data[PLL_BYTES];
 unsigned char b1, b2;
 unsigned long vco, ref, r;

 if(pll_type==PLL_NONE||read_pll(pll_data))
  return(0);
 switch(pll_type)
 {
  case PLL_ICS94215:
   b1=pll_data[ICS94215_REF];
   b2=pll_data[ICS94215_VCO];
   break;
  case PLL_ICS94228:
   b1=pll_data[ICS94228_REF];
   b2=pll_data[ICS94228_VCO];
   break;
  default:
   return(0);
 }
 vco=((unsigned long)b2<<1)+((b1&0x80)?1:0);
 ref=b1&0x7F;
 r=(unsigned long)ICS942XX_BASE/(ref+ICS942XX_MIN_REF)*(vco+ICS942XX_MIN_VCO);
 return(r);
}

/* Sets the PLL output frequency. Returns non-zero if failed. */

int set_pll_freq(unsigned long hz)
{
 char pll_data[PLL_BYTES];
 unsigned char b1, b2;
 unsigned short freq;

 if(pll_type==PLL_NONE||read_pll(pll_data))
  return(-1);
 if(hz==RESTORE_FREQ)
 {
  if((freq=prev_freq)==0)
   return(0);                           /* OK, nothing has been set so far */
 }
 if((freq=get_divisor(hz))==0)
  return(-1);
 prev_freq=freq;
 b1=freq&0xFF;
 b2=(freq&0xFF00)>>8;
 switch(pll_type)
 {
  case PLL_ICS94215:
   pll_data[ICS94215_REF]=b1;
   pll_data[ICS94215_VCO]=b2;
   pll_data[ICS942XX_BYTE_COUNT]=ICS942XX_BYTES;
   pll_data[ICS94215_CAPS]|=ICS94215_ENABLE;
   pll_data[ICS94215_VCO_CTRL]|=ICS94215_VCO_ENABLE;
   break;
  case PLL_ICS94228:
   pll_data[ICS94228_REF]=b1;
   pll_data[ICS94228_VCO]=b2;
   pll_data[ICS942XX_BYTE_COUNT]=ICS942XX_BYTES;
   pll_data[ICS94228_CAPS]|=ICS94228_ENABLE;
   break;
  default:
   return(-1);
 }
 write_pll(pll_data);
 return(0);
}

/* Initializes the PLL */

int init_pll(int t)
{
 char buf[PLL_BYTES];

 switch(t)
 {
  case PLL_ICS94215:
  case PLL_ICS94228:
   pll_dev=ICS942XX_DEV;
   pll_bytes=ICS942XX_BYTES;
   break;
  case PLL_NONE:
   pll_type=t;
   return(0);
  default:
   return(-1);
 }
 /* Check the PLL configuration by reading it */
 if(read_pll(buf))
  return(-1);
 pll_type=t;
 return(0);
}

/* Returns the PLL type, if any */

int query_pll()
{
 return(pll_type);
}

/* Hibernation support */

void reconfigure_pll()
{
 set_pll_freq(RESTORE_FREQ);
}
