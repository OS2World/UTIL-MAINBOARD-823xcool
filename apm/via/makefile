# $Id: makefile,v 1.22 2004/12/02 04:25:37  Exp $
#
# VIA ACPI/APM drivers.

H = ..\..\..\..\H
CMN_H = ..\..\COMMON

# Tools:
#
# OS/2: MS C v 6.0 from DDK + MS C/386 from DDK + OpenWatcom v 11.0c.
# DOS : MS C v 6.0 from DDK + EXE->COM convertors and packers
#       (one may just take the last step out with NMAKE NO_PACK=1).
#
# Note: we build the DOS driver for /Asfu rather than /AT, that allows for a
#       unified build (same object code for OS/2 and DOS) and lets the OS/2
#       DDK users, though contradictory to the DDK license, to compile the
#       DOS driver as well - CRTCOM.LIB is absent from DDK.

ASM = ml.exe
ASMOPT = -c -I$(CMN_H)

CC = wcc386.exe
!ifdef EXPERIMENTAL
# No more experimental options as of 19/11/2004
!endif
!ifdef LIBC
ADD_COPT = -i=$(LIBCPATH) -D__EXTENDED__ -zl -D_LNK_CONV=_System
!else
!error Static linking is not supported with Watcom, please specify LIBC
!endif
!ifndef LIBCPATH
!error LIBCPATH must be set, e.g. LIBCPATH=E:\TOOLKIT\H\LIBC
!endif
COPT = -os -5s -Dmktemp=_mktemp -wcd=107 -s -i=. $(DEBUG_COPT) $(ADD_COPT) $(EXP_COPT) -ze
LINKLIB = OWATCOMS+LIBCSS+OS2386.LIB
ADD_LINKOPT = /NOD

CC286 = cl.exe
C286OPT = /c /Asfu /I$(H) /I$(CMN_H) /Zlp /G2s /Olrg $(EXP_C286OPT) /B2 C2L

CC386 = cl386.exe
C386OPT = /c /Zalfep /Asnw /I$(H) /G3sd /NT_TEXT32 /ND_DATA32

!include "$(H)\version.mak"

FIRSTOBJ = 16first.obj
LASTOBJ = 
OBJS = $(FIRSTOBJ) 16init.obj 16chcfg.obj 16args.obj 16smbus.obj \
       16pll.obj externs.obj 16to32.obj $(LASTOBJ) install.obj \
       32util.obj patchram.obj
LIBS = ..\..\..\..\lib\libpcidr.lib+..\..\..\..\lib\clibcep.lib
LIBS_DOS = ..\..\..\..\lib\libpcidr.lib+..\..\..\..\lib\clibce.lib

all: viaapm.sys apmctrl.exe msdos\viaapm.com

# VIA driver

viaapm.sys: $(OBJS) makefile
        Rem Create DEF file <<$(@B).def
PHYSICAL DEVICE VIAAPM

DESCRIPTION "$(FILEVER)  VIA ACPI/APM add-on driver"

CODE PRELOAD EXECUTEREAD
DATA PRELOAD

SEGMENTS
 _DDHDR       CLASS 'DATA' PRELOAD NONSHARED
 CONST        CLASS 'CONST' PRELOAD NONSHARED
 _DATA        CLASS 'DATA' PRELOAD NONSHARED
 _BSS         CLASS 'BSS' PRELOAD NONSHARED
 FAR_BSS      CLASS 'FAR_BSS' PRELOAD NONSHARED
 DATAEND      CLASS 'ZD' PRELOAD NONSHARED
 _TEXT        CLASS 'CODE' PRELOAD EXECUTEREAD NONSHARED
 "CODE"       CLASS 'CODE' PRELOAD EXECUTEREAD NONSHARED
 TEXTEND      CLASS 'CODE' PRELOAD EXECUTEREAD NONSHARED
 RMCode       CLASS 'CODE' PRELOAD EXECUTEREAD NONSHARED
 CONST32      CLASS 'FLATDATA' PRELOAD NONSHARED
 CONST232     CLASS 'FLATDATA' PRELOAD NONSHARED
 _DATA32      CLASS 'FLATDATA' PRELOAD NONSHARED
 _BSS32       CLASS 'FLATBSS' PRELOAD NONSHARED
 _TEXT32      CLASS 'FLATCODE' PRELOAD EXECUTEREAD NONSHARED

<<keep
 LINK386 /MAP:FULL /EXEPACK /NOE $(OBJS),$@,$(@B).map,OS2286+..\..\..\..\LIB\VDH+..\..\..\..\lib\DHCALLS.LIB+contrib\apmcalls.lib+$(LIBS),$(@B).def;
 del $(@B).def
 mapsym $(@B).map
!ifndef NO_PACK
 lxlite /CS /BDX- /ZS $@
!endif

# Control program

apmctrl.exe: apmctrl.obj
        Rem Create DEF file <<$(@B).def
NAME APMCTRL WINDOWCOMPAT

DESCRIPTION "$(FILEVER)  VIA ACPI/APM control program"

<<keep
 LINK386 $(ADD_LINKOPT) /PMTYPE:VIO /MAP:FULL /STACK:24576 $**,$@,$(@B).map,$(LINKLIB),$(@B).def;
 mapsym $(@B).map
!ifndef NO_PACK
 lxlite /CS /BDX- /ZS $@
 -packexe $@
!endif

# DOS TSR

msdos\viaapm.com: msdos.obj det_x86.obj 16chcfg.obj 16smbus.obj 16pll.obj externs.obj
 LINK /STACK:8192 /NOE /MAP:FULL $**,$(@R).exe,$(@R).map,$(LIBS_DOS);
 cd msdos
 mapsym $(@B).map
!ifndef NO_PACK
 start /dos /c execonv $(@B).exe /V /D
!endif
 cd ..

.asm.obj:
 $(ASM) $(ASMOPT) $<

16first.obj:
 $(CC286) $(C286OPT) -Fo$@ %s

16init.obj:
 $(CC286) $(C286OPT) -Fo$@ %s

16chcfg.obj:
 $(CC286) $(C286OPT) -Fo$@ %s

16args.obj:
 $(CC286) $(C286OPT) -Fo$@ %s

16smbus.obj:
 $(CC286) $(C286OPT) -Fo$@ %s

16pll.obj:
 $(CC286) $(C286OPT) -Fo$@ %s

msdos.obj:
 $(CC286) $(C286OPT) -Fo$@ %s

externs.obj:
 $(CC286) $(C286OPT) -Fo$@ %s

install.obj:
 $(CC386) $(C386OPT) -Fo$@ %s

patchram.obj:
 $(CC386) $(C386OPT) -Fo$@ %s

apmctrl.obj:
 $(CC) $(COPT) %s

clean:
 -del *.obj *.map *.sys *.sym

install: all
 copy viaapm*.sy? c:\os2\boot

# Distribution

DISTPATH = dist\src\DDK\base\src\dev\apm\via

dist: all
 -rm -rf dist
 -md dist
 -md dist\msdos
 -md dist\src
 -md dist\src\DDK
 -md dist\src\DDK\base
 -md dist\src\DDK\base\lib
 -md dist\src\DDK\base\src
 -md dist\src\DDK\base\src\dev
 -md dist\src\DDK\base\src\dev\common
 -md dist\src\DDK\base\src\dev\apm
 -md dist\src\DDK\base\src\dev\apm\via
 -md dist\src\DDK\base\src\dev\apm\via\contrib
 for %%i in (msdos\*.exe msdos\*.com msdos\*.sym msdos\*.txt) do copy %%i dist\msdos
 for %%i in (..\generic\apm_nh.sy* *.sy* doc\* *.exe) do copy %%i dist
 for %%i in (contrib\*) do copy %%i $(DISTPATH)\contrib
 for %%i in (*.asm *.inc *.c *.h changelog makefile *.def) do copy %%i $(DISTPATH)
 copy $(CMN_H)\* dist\src\DDK\base\src\dev\common
 -del dist\src\DDK\base\src\dev\common\*.?bj
 cd dist\src
 arj a -+ -j$$ -r -s ..\src
 cd ../..
 rm -rf dist/src
