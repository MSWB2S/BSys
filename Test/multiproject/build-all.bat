@echo off
setlocal

echo === [1/3] Building mathlib.dll ===
wbsys mathlib\mathlib.ini build
if errorlevel 1 (
    echo mathlib build failed
    exit /b 1
)

echo === [2/3] Building app.exe ===
wbsys app\app.ini build
if errorlevel 1 (
    echo app build failed
    exit /b 1
)

echo === [3/3] Staging mathlib.dll next to app.exe ===
copy /Y mathlib\build\mathlib.dll app\build\mathlib.dll >nul

echo Build complete: app\build\app.exe
endlocal
