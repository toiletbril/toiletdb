@echo on

setlocal enabledelayedexpansion enableextensions

set BT2022="C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
set BT2019="C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
set VS2022="C:\Program Files (x86)\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
set VS2019="C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"

if not defined VS (
  if exist %VS2022% (call %VS2022%) else if exist %VS2019% (call %VS2019%) else if exist %BT2022% (call %BT2022%) else if exist %BT2019% (call %BT2019%)
)

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

cl.exe %CFLAGS% %INCLUDES% /Fe%BINDIR%\%EXE% cli\main.cpp cli\cli.cpp /link %BINDIR%\%LIB% /subsystem:windows
