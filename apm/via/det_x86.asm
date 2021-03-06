;*
;* $Id: det_x86.asm,v 1.1 2002/06/27 20:14:39 root Stab $
;* ---------------------------------------------------------------------------
;* This code uses the  classic way  of determining what Intel x86 CPU is used.
;* The exact author is unknown.
;*

public          _detect_x86

		_TEXT	SEGMENT BYTE PUBLIC "CODE"
		ASSUME	CS:_TEXT

;*
;* Returns one of the following values for corresponding CPU types:
;*
;*    0x0086 -> 8086/8088
;*    0x0186 -> 80186/80188
;*    0x0286 -> 80286
;*    0x0386 -> 80386 and higher
;*

_detect_x86     proc
		pushf
		xor	ax, ax
		push	ax
		popf
		pushf
		pop	ax
		and	ax, 0F000h
		cmp	ax, 0F000h
		jnz	test_386
		push	cx
		mov	ax, 0FFFFh
		mov	cl, 21h
		shl	ax, cl
		pop	cx
		jnz	l186
		mov	ax, 86h
		popf
		jmp	short done
l186:
		mov	ax, 186h
		popf
		jmp	short done
test_386:
		mov	ax, 7000h
		push	ax
		popf
		pushf
		pop	ax
		and	ax, 7000h
		jnz	l386
		mov	ax, 286h
		popf
		jmp	short done
l386:
		mov	ax, 386h
		popf
		jmp	short $+2
done:
                ret
_detect_x86     endp

		_TEXT	ENDS

        	end
