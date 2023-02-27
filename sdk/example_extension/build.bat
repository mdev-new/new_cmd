@echo off
gcc -fpic -O2 -s -shared -fpermissive -w -o example.dll -I.. main.cpp -Wl,-e,
objcopy -O binary example.dll example.dll.bin
..\dlltool.exe example.dll example.dll.bin example.bin 2
del example.dll
del example.dll.bin
pause
