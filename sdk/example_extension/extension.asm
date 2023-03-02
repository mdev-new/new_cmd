[bits 64]
[org 0x00]

jmp start

data dq 0

start:
	pop rbx
	call [rbx]
	retn

counter dw 0
test_command:
	add dword [counter], 1
	mov eax, [counter]
	ret