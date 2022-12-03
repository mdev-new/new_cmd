@echo off
if not exist dist mkdir dist
if not exist bin mkdir bin
g++ -o dist/cmd.exe src/*.cpp -fpermissive