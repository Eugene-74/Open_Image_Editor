@echo off
setlocal enabledelayedexpansion

cd ..

mkdir build
cd build

if exist OpenImageEditor.exe del OpenImageEditor.exe

cmake -G "Ninja" ..

cmake --build .

OpenImageEditor.exe

endlocal
