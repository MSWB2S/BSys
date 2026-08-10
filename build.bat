@echo off
REM Temporary solution until we can self-host BSys.
setlocal

set root=%~dp0

cd %root%

if not exist build mkdir build

cl /nologo /EHsc /std:c++17 /Iinclude ^
    /Fe:build\WBSys.exe ^
    src\main.cpp ^
    src\builder.cpp ^
    src\manifest.cpp ^
    src\process.cpp ^
    src\utility.cpp