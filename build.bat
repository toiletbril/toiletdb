@echo off
set ENTRY=main.cpp
set FLAGS=-O2 -Wall -Wextra -pedantic
set SRC=src
set OUT=bin
set OUTFILE=toiletdb.exe

if not exist %OUT% mkdir %OUT%

@echo on
cc %FLAGS% %SRC%/%ENTRY% -o %OUT%/%OUTFILE%
