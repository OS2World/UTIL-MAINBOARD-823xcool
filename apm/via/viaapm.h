/* $Id: viaapm.h,v 1.15 2004/12/02 04:25:37  Exp $ */

/* Common VIA APM constants */

#ifndef VIAAPM_INCLUDED
#define VIAAPM_INCLUDED

/* APM device driver identification */

#define APM_DRIVER       "\\DEV\\VIAAPM$"

/* Custom IOCTL commands. The SET/QUERY function code differentiation (0x20)
   should be pursued strictly - the data areas are only validated based on the
   function code. */

#define IOCTL_CAT_VIAAPM             0xA0
#define VIAAPM_CMD_SET_HOOK          0xC0       /* Init-time only: install the
                                                   kernel hook */
#define VIAAPM_CMD_QUERY_HOOK        0xE0       /* Query if the hook was
                                                   installed */
#define VIAAPM_CMD_RESET             0xC1       /* Reset the chip */
#define VIAAPM_CMD_SET_THROTTLING    0xC2       /* Set CPU thottling */
#define VIAAPM_CMD_QUERY_THROTTLING  0xE2       /* Query CPU thottling */
#define VIAAPM_CMD_QUERY_CHIPSET     0xE3       /* Query chipset type, one of
                                                   CHIP_* */
/* HWMon removed -- AAB 19/11/2004 */
#define VIAAPM_CMD_SET_FSB           0xC5       /* ** Undocumented ** */
#define VIAAPM_CMD_QUERY_FSB         0xE5       /* ** Undocumented ** */
#define VIAAPM_CMD_SET_CPU_FREQ      0xC6       /* ** Undocumented ** */
#define VIAAPM_CMD_QUERY_CPU_FREQ    0xE6       /* ** Undocumented ** */

/* Chip sets */

#define CHIP_UNKNOWN               0
#define CHIP_KT133                 1
#define CHIP_KT266                 2            /* KT266/333 (VT8233 family) */
#define CHIP_KX133                 3
#define CHIP_KT400                 4            /* KT333/400, KT600? */
#define CHIP_KN266                 5            /* KN266 */
#define CHIP_KT880                 6

/* Exported array for chipset types. NOTE: VT8367 (KT333) has the same PCI
   ID as VT8366[A], and they can't be distinguished by their revisions
   either. */

#define CHIPSET_TYPES char *chipset_types[]= \
{ \
 "Unknown", \
 "KT133", \
 "KT266/KT333", \
 "KX133", \
 "KT600/KT400/KT333", \
 "KN266", \
 "KT880", \
 NULL \
}

/* Configuration return codes */   
                                   
#define CONF_OK                    0
#define CONF_UNKNOWN_CHIP          1    /* Unsupported chip */
#define CONF_BAD_ACPI              2    /* Invalid ACPI address */

/* Hook installation results */

#define HOOK_OK                    0
#define HOOK_NO_MTE                1    /* Failed to locate the kernel
                                           MTE */
#define HOOK_NO_OBJTAB             2    /* No object table */
#define HOOK_BAD_OTE               3    /* Bad objtbl entries */
#define HOOK_NO_ANCHOR             4    /* Anchor block not found */
#define HOOK_NO_APM_AREA           5    /* APM sig area not found */
#define HOOK_NO_APM_SIG            6    /* APM sig not found */
#define HOOK_NO_TRAP_SCRN          7    /* Trap screen dead-end area not
                                           found */
#define HOOK_NO_SYS1503            8    /* SYS1503 dead-end area not found */

#endif
