@echo off
setlocal

set build=bin
set bin=sandbox.exe
set src=src\main.c

if exist "%build%" rmdir /s /q "%build%"
mkdir "%build%"

echo Compiling %src% -> %build%\%bin%

zig cc %src% ^
    -o %build%\%bin% ^
    -std=c11 ^

endlocal
