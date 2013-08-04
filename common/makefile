# $Id: makefile,v 1.7 2004/07/04 20:36:12 root Exp $
#
# Common routines for PCI drivers. NMAKE with:
#
#   COPYMS = 1   to build a private version of the library incorporating
#                several MS C routines to facilitate linkage.

# MASM v 6.0 and DDK tools

DDK_BASE = ..\..\..
MASM_OPT = -c -I$(DDK_BASE)\INC
MASM = $(DDK_BASE)\tools\ml.exe
LIB = $(DDK_BASE)\tools\lib.exe

CC = cl.exe
COPT = /c /Asfu /Zlp /G2s /Os

!ifdef COPYMS
PRIVATE = 1
MS_OBJS = .anulrem .anuldiv .anfalmul .anfaldiv .anfauldi .anlmul .anldiv .anlshl .anlshr .anulshr
MS_LIB = $(DDK_BASE)\lib\slibcep.lib
!endif

# Rules

.asm.obj:
 @echo * $<
 @$(MASM) $(MASM_OPT) $<

.c.obj:
 @echo * $<
 $(CC) $(COPT) -Fo$@ $<

all: $(DDK_BASE)\lib\libpcidr.lib

clean:
 -@del *.?bj 2>NUL
 -@del *.lib 2>NUL
 -@del *.crf 2>NUL
 -@del *.lst 2>NUL

DEPOBJS = libvia.obj viaprtio.obj endsegs.obj vsprintf.obj kprintf.obj parsearg.obj
OBJS = $(DEPOBJS) $(MS_OBJS:.=)

$(DDK_BASE)\lib\libpcidr.lib: $(DEPOBJS)
 @echo * $@
 @if exist $@ del $@
!ifdef COPYMS
 @$(LIB) /NOLOGO $(MS_LIB) $(MS_OBJS:.=*),nul
!endif
 @$(LIB) /NOLOGO $@ $(OBJS),nul
!ifdef PRIVATE
 @echo A private version has been created -- $@
 @echo Please do not distribute it.
!endif
