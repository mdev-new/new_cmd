[bits 64]
[org 0x00]

jmp start

start:
	mov rax, rcx
	mov ecx, 20
	call [rax]
	mov rax, 10
	retn

counter dw 0
test_command:
	add dword [counter], 1
	mov eax, dword [counter]
	retn