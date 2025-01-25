@echo off
setlocal enabledelayedexpansion

mkdir build
cd build

cmake -G "Ninja" ..
    
cmake --build . --config Release


cd ..

rmdir /s /q release

mkdir release
cd release

set EXECUTABLE=OpenImageEditor.exe

copy ..\build\%EXECUTABLE% .

windeployqt6.exe %EXECUTABLE%


@REM TODO modifier pour pas copier tout les dll mais juste les bons (mais comment les trouver XD)
xcopy C:\msys64\mingw64\bin\*.dll .

C:\mingw-bundledlls\mingw-bundledlls C:\Users\eugen\Documents\MesDocuments\git\Open_Image_Editor\build\export\%EXECUTABLE% > dependencies.txt

:copy_dependencies
for /f "tokens=*" %%i in ('type dependencies.txt') do (
    echo Copying %%i
    xcopy "%%i" .
    C:\mingw-bundledlls\mingw-bundledlls %%i > temp_dependencies.txt
    for /f "tokens=*" %%j in (temp_dependencies.txt) do (
        if not exist "%%j" (
            echo Copying sub-dependency %%j
            xcopy "%%j" .
        )
    )
)
exit /b


call :copy_dependencies




endlocal