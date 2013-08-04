; $Id: viaprtio.asm,v 1.1 2003/11/19 21:13:24 root Exp $
;
; Low-level 32-bit port I/O library

		.386

		OPTION	NOSCOPED
		OPTION	SEGMENT:USE16

		public	_VIA_inpb, _VIA_inpd, _VIA_outpb, _VIA_outpd

		include	groups.inc

		_TEXT	SEGMENT WORD PUBLIC 'CODE'
		assume	ds:DGROUP

;
; 8-bit read
; 

_VIA_inpb	proc near

inpb_port	= word ptr  4

		push	bp
		mov	bp, sp
		mov	dx, [bp+inpb_port]
		in	al, dx
		pop	bp
		retn	
_VIA_inpb	endp

;
; 32-bit read
; 

_VIA_inpd	proc near

ind_value	= dword	ptr -4
ind_port	= word ptr  4

		enter	4, 0
		push	eax
		mov	dx, [bp+ind_port]
		in	eax, dx
		mov	[bp+ind_value], eax
		pop	eax
		mov	ax, word ptr [bp+ind_value]
		mov	dx, word ptr [bp+ind_value+2]
		leave	
		retn	
_VIA_inpd	endp

;
; 8-bit write
;

_VIA_outpb	proc near

outpb_port	= word ptr  4
outpb_value	= word ptr  6

		push	bp
		mov	bp, sp
		push	ax
		mov	dx, [bp+outpb_port]
		mov	ax, [bp+outpb_value]
		out	dx, al
		pop	ax
		leave	
		retn	
_VIA_outpb	endp

;
; 32-bit write
;

_VIA_outpd	proc near

outd_port	= word ptr  4
outd_value	= dword	ptr  6

		push	bp
		mov	bp, sp
		push	eax
		mov	dx, [bp+outd_port]
		mov	eax, [bp+outd_value]
		out	dx, eax
		pop	eax
		leave	
		retn	
_VIA_outpd	endp

		_TEXT	ENDS
		END
