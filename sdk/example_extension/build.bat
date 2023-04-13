@echo off

set path=..\..\build;%PATH%

<nul set/p"=Compiling"

for %%x in (extension1) do (
	nasm -fbin %%x.asm -o %%x.tmp
	if %errorlevel% neq 1 (pause & exit/b)
	compressor.exe %%x.tmp %%x.bin 2
	if "%1"=="doPause" pause
	del %%x.tmp
	<nul set/p"=."
)
