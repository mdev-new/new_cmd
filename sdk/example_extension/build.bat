@echo off
nasm -fbin -o extension.tmp extension.asm
"D:\new_cmd_build\compressor.exe" extension.tmp extension.bin 2
pause
del extension.tmp