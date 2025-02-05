@echo off
setlocal enabledelayedexpansion

for /f "tokens=1,2 delims==" %%a in (%~dp0..\AppConfig.txt) do (
    set %%a=%%b
)

mkdir build
cd build

if exist %APP_NAME%-%APP_VERSION%.exe del %APP_NAME%-%APP_VERSION%.exe

cmake -G "Ninja" ..

cmake --build .

%APP_NAME%-%APP_VERSION%.exe

endlocal
