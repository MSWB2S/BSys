@echo off
REM Temporary solution until we can self-host BSys.
setlocal

set root=%~dp0
set bin=%root%Bin
set obj=%bin%\Intermediate

mkdir %bin%
mkdir %obj%
cl WBSys.cpp /EHsc /O2 /std:c++17 /Fe:WBSys.exe

move WBSys.exe %bin%
move WBSys.obj %obj%