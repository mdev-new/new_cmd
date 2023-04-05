@echo off
nasm -fbin -o extension.tmp extension.asm
"C:\Users\Zdenda\git\new_cmd\build\compressor.exe" extension.tmp extension.bin 2
pause
del extension.tmp