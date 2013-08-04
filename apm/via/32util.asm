; $Id: 32util.asm,v 1.6 2003/11/19 20:04:00 root Exp $
;
; Utility routines

	OPTION	OLDSTRUCTS
	OPTION	SEGMENT:USE16

.386p
		
 	include	groups.inc
	include	sas.inc

	public	_locate_krnl_mte
	public	_install_hook, _uninstall_hook, _apm_idle_hook
	public	_sys1503_dead_end

	extern	_acpi_c2_port:word, _acpi_c3_port:word
	extern	_pf_OutMessage:fword

_TEXT32 SEGMENT	WORD PUBLIC USE32 'FLATCODE'
	assume	ds:DGROUP

; Locate the kernel MTE through System Anchor Segment
		
_locate_krnl_mte	PROC
		push	es
		mov	ax, SAS_selector
		mov	es, ax
		mov	bx, es:[SAS_vm_data]
		mov	eax, dword ptr es:[bx+SAS_vm_krnl_mte]
		pop	es
		retn
_locate_krnl_mte	endp

; Update the kernel APM hook address

_install_hook	proc
		push	ebp
		mov	ebp, esp
dest	equ	dword ptr [ebp+8]
laddr	equ	dword ptr [ebp+12]
		mov	ebx, dest
		mov	eax, laddr
; Write the segment last - the kernel checks its value to see if the
; hook is installed
		mov	dword ptr ds:[ebx], eax
		mov	word ptr ds:[ebx+4], cs
		pop	ebp
		ret
_install_hook	endp

; Remove the hook

_uninstall_hook	proc
		push	ebp
		mov	ebp, esp
dest	equ	dword ptr [ebp+8]
		mov	ebx, dest
		sub	eax, eax
; Remove the segment first, then purge the offset
		mov	word ptr ds:[ebx+4], ax
		mov	dword ptr ds:[ebx], eax
		pop	ebp
		ret
_uninstall_hook	endp

; APM idle hook

_apm_idle_hook	proc	far
		or	ax, ax
		jnz	done		; If asked to "wake up" then just don't
					; "sleep"
		pusha
		push	ds
		mov	bx, seg DGROUP
		mov	ds, bx
		mov	dx, ds:_acpi_c2_port
		pop	ds
		in	al, dx		; Disconnect the CPU and go to sleep
		popa
done:
		retf			; 16:32 routine!
_apm_idle_hook	endp

; SYS1503 dead-end. SYS1503 doesn't require to process Alt+Ctrl+Delete, nor
; can it be taken out with REIPL=OFF (see below). Therefore we are allowed to
; sleep deeper than usual and go into ACPI C3 (shutdown the CPU).

_sys1503_dead_end proc
		mov	bx, seg DGROUP
		mov	ds, bx
		db	0FFh, 01Dh
		dd	offset ds:_pf_OutMessage
prepare_dead_end::
		mov	bx, seg DGROUP
		mov	ds, bx
dead_end:
; Attempt ACPI C3 first
		mov	dx, word ptr ds:_acpi_c3_port
		in	al, dx
; If for some reason it has failed, try ACPI C2
		mov	dx, word ptr ds:_acpi_c2_port
		in	al, dx
		jmp	short dead_end	; Retry ACPI C3
_sys1503_dead_end endp

; ReIPL for SYS1503

_sys1503_reipl	proc
		mov	bx, seg DGROUP
		mov	ds, bx
		db	0FFh, 01Dh
		dd	offset ds:_pf_OutMessage
; Reboot the system by means of KBC.
		mov	al, 0FEh
		out	64h, al
; Again, save the CPU power if the reboot didn't work for some reason
		jmp	prepare_dead_end
_sys1503_reipl	endp

; Returns the code segment.

_get_cs		proc
		mov	ax, cs
		ret
_get_cs		endp
	
_TEXT32 ENDS

        END
