@echo off
rem Launch this from MSVC-enabled console

setlocal enabledelayedexpansion enableextensions

set CFLAGS=/W4 /std:c++17 /FC /TP /EHsc /nologo
set INCLUDES=/I "include"

set LIB=toiletdb.lib
set EXE=toiletdb.exe

set OBJDIR=obj
set BINDIR=build

set SRC=src

if not exist %OBJDIR% mkdir %OBJDIR%
if not exist %BINDIR% mkdir %BINDIR%

for %%f in (%SRC/*.cpp) do (
    cl.exe /c %CFLAGS% /Fo%OBJDIR%\%%~nf.obj %SRC%\%%f
)

set OBJECTS=

for %%x in (%OBJDIR%\*.obj) do set OBJECTS=!OBJECTS! %%x
set OBJECTS=%OBJECTS:~1%

lib.exe /OUT:"%BINDIR%\%LIB%" /nologo %OBJECTS%

echo %OBJECTS%

cl.exe %CFLAGS% %INCLUDES% /Fe%BINDIR%\%EXE% cli\main.cpp cli\cli.cpp /link %BINDIR%\%LIB% -SUBSYSTEM:windows
