@echo off
setlocal enabledelayedexpansion

mkdir build
cd build

cmake -G "Ninja" ..
    
cmake --build .

OpenImageEditor.exe

endlocal
