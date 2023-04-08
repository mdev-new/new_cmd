@echo off
nasm -fbin -o extension.tmp extension.asm
compressor.exe extension.tmp extension.bin 2
pause
del extension.tmp
