; $Id: 16to32.asm,v 1.2 2002/03/04 20:46:22 root Stab $
;
; Thunking layer

	OPTION	OLDSTRUCTS
	OPTION	SEGMENT:USE16

.386p
		
	extrn	_init32:FAR32

 	include	groups.inc

	public	_do_init32

_TEXT   SEGMENT WORD PUBLIC 'CODE'

; Pass control to the 32-bit segment for further initialization in ring 0

_do_init32	proc near
		push	ds
		call	far32 ptr FLAT:_init32 ; _loadds -> _DATA32
		pop	ds
		ret
_do_init32	ENDP

_TEXT   ENDS

        END
