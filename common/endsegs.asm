; $Id: endsegs.asm,v 1.3 2003/11/19 21:13:24 root Exp $
;
; Segments terminating the resident code/data area

		OPTION	NOSCOPED

	 	include	groups.inc

TEXTEND	SEGMENT WORD PUBLIC 'CODE'
		PUBLIC _EndOfCode
_EndOfCode:
TEXTEND	ENDS

DATAEND	SEGMENT	WORD PUBLIC 'ZD'
		PUBLIC _EndOfData
_EndOfData:
DATAEND	ENDS

        END
