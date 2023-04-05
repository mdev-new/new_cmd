%include "std.i"
	section .text

[bits 64]
[org 0x00]
	jmp start

	section .data

baseAddr dq 0
cmd db "test", 0
counter dw 0

	section .text

test_command:
	xor rax, rax ; make sure upper half if empty
	add dword [rel counter], 1
	mov eax, dword [rel counter]
	retn

start:
	push_regs
	mov r15, rcx ; r15 = &DllMainData
	mov r14, [r15+baseAddress] ; r14 = this module's base address
	mov [r14+baseAddr], r14

	lea rcx, [r14+cmd]
	lea rdx, [r14+test_command]
	call [r15+registerCmd]

	pop_regs

	mov rax, 0
	retn
