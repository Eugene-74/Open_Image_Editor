@echo off
setlocal enabledelayedexpansion

mkdir build
cd build


cmake -G "MinGW Makefiles" ..

rm -rf CMakeCache.txt CMakeFiles
cmake --build .

ctest --verbose

endlocal
