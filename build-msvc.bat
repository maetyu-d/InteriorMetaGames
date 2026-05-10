@echo off
setlocal
where cl >nul 2>nul
if errorlevel 1 (
    if exist "C:\BuildTools\Common7\Tools\VsDevCmd.bat" (
        call "C:\BuildTools\Common7\Tools\VsDevCmd.bat" -arch=x64 -host_arch=x64
    )
)
if not exist build mkdir build
cl /nologo /EHsc /O2 /std:c++17 src\DeadChannel.cpp /Fe:build\DeadChannel.exe user32.lib gdi32.lib opengl32.lib winmm.lib
cl /nologo /EHsc /O2 /std:c++17 src\TheWhiteFigure.cpp /Fe:build\TheWhiteFigure.exe user32.lib gdi32.lib opengl32.lib winmm.lib
cl /nologo /EHsc /O2 /std:c++17 src\EndlessAirportGate.cpp /Fe:build\EndlessAirportGate.exe user32.lib gdi32.lib opengl32.lib winmm.lib
endlocal
