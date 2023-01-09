@echo off
gcc -O2 -s -shared -fpermissive -w -o example.dll -I.. main.cpp
..\dlltool.exe example.dll example.bin 2
pause