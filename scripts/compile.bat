@echo off
setlocal enabledelayedexpansion

for /f "tokens=1,2 delims==" %%a in (%~dp0..\AppConfig.txt) do (
    set %%a=%%b
)

mkdir build
cd build

if exist %APP_NAME%-%APP_VERSION%.exe del %APP_NAME%-%APP_VERSION%.exe

@REM cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Build ..
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Build ..


@REM cmake --build .
mingw32-make -j %NUMBER_OF_PROCESSORS%

%APP_NAME%-%APP_VERSION%.exe

endlocal