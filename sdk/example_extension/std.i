; vim: set syntax=asm
%macro push_regs 0
	push rbx
	push rcx
	push rdx
	push r8 
	push r9 
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15
%endmacro

%macro pop_regs 0
	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop r9 
	pop r8 
	pop rdx
	pop rcx
	pop rbx
%endmacro

; indexes in DllMainData structure
; !!! KEEP THIS IN SYNC WITH src/extension_api/extension_api.hh:DllMainData !!!
registerCmd  equ (0*8) ; always (idx*8)
sleep        equ (1*8)
setEnvVar    equ (2*8)
createThread equ (3*8)
getProcAddr  equ (4*8)
baseAddress  equ (5*8)
