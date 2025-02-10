@echo off
setlocal enabledelayedexpansion

mkdir build
cd build


cmake -G "Ninja" ..

rm -rf CMakeCache.txt CMakeFiles
cmake --build .

ctest

endlocal
