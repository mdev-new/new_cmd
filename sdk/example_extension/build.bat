@echo off
gcc -O2 -s -shared -o example.dll -I.. main.cpp
..\dlltool.exe example.dll example.bin 2