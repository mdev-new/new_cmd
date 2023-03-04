[bits 64]
[org 0x00]

jmp start

cmd: db "test", 0

counter dw 0
test_command:
	add dword [counter], 1
	mov eax, dword [counter]
	retn

start:
	mov rax, rcx
	mov rax, test_command
	mov rcx, cmd
	call [rax]
	mov rax, 10
	retn