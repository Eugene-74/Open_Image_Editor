@echo off
setlocal enabledelayedexpansion

for /f "tokens=1,2 delims==" %%a in (%~dp0..\AppConfig.txt) do (
    set %%a=%%b
)

mkdir build
cd build

if exist %APP_NAME%-%APP_VERSION%.exe del %APP_NAME%-%APP_VERSION%.exe

cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Test -DCMAKE_C_COMPILER_LAUNCHER=ccache -DCMAKE_CXX_COMPILER_LAUNCHER=ccache -DCMAKE_CUDA_COMPILER=nvcc -DCMAKE_CUDA_ARCHITECTURES=all ..

cmake --build .

ctest --output-on-failure
if %errorlevel% neq 0 (
    echo Tests failed, aborting execution.
    exit /b %errorlevel%
)
endlocal