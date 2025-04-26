@echo off
setlocal enabledelayedexpansion

for /f "tokens=1,2 delims==" %%a in (%~dp0..\AppConfig.txt) do (
    set %%a=%%b
)

mkdir build
cd build

if exist %APP_NAME%-%APP_VERSION%.exe del %APP_NAME%-%APP_VERSION%.exe

@REM cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Build -DCMAKE_C_COMPILER_LAUNCHER=ccache -DCMAKE_CXX_COMPILER_LAUNCHER=ccache ..
@REM cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Build -DCMAKE_C_COMPILER_LAUNCHER=ccache -DCMAKE_CXX_COMPILER_LAUNCHER=ccache -DCMAKE_CUDA_COMPILER=nvcc -DCMAKE_CUDA_ARCHITECTURES=all ..
cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_C_COMPILER_LAUNCHER=ccache -DCMAKE_CXX_COMPILER_LAUNCHER=ccache -DCMAKE_CUDA_COMPILER=nvcc -DCMAKE_CUDA_ARCHITECTURES=all ..

cmake --build .

if "%1"=="gdb" (
    gdb %APP_NAME%-%APP_VERSION%.exe
)else (
    %APP_NAME%-%APP_VERSION%.exe
)

endlocal