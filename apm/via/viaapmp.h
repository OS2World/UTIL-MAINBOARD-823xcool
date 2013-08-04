/* $Id: viaapmp.h,v 1.3 2005/01/06 11:56:13 root Exp $ */

/* VIAAPM private data (subject to change) */

#ifndef VIAAPMP_INCLUDED
#define VIAAPMP_INCLUDED

#define RC_BUG  (STATUS_DONE|STERR|ERROR_I24_INVALID_PARAMETER) /* RC on error */

/* PLL section */

#define PLL_NONE                   0
#define PLL_ICS94215               1
#define PLL_ICS94228               2

/* Throttling timers */

#define TT_FAST                    0    /*    7.5/  120 us cycle */
#define TT_MEDIUM                  1    /*   30  /  480 us cycle */
#define TT_SLOW                    2    /* 1000  /16000 us cycle */

#define TT_DEFAULT             32767    /* None set */

#endif
