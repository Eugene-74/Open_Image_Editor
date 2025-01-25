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



@REM C:\mingw-bundledlls\mingw-bundledlls C:\Users\eugen\Documents\MesDocuments\git\Open_Image_Editor\build\export\%EXECUTABLE% > dependencies.txt

@REM :copy_dependencies
@REM for /f "tokens=*" %%i in ('type dependencies.txt') do (
@REM     echo Copying %%i
@REM     xcopy "%%i" .
    @REM C:\mingw-bundledlls\mingw-bundledlls %%i > temp_dependencies.txt
    @REM for /f "tokens=*" %%j in (temp_dependencies.txt) do (
    @REM     if not exist "%%j" (
    @REM         echo Copying sub-dependency %%j
    @REM         xcopy "%%j" .
    @REM     )
    @REM )
@REM )
@REM exit /b

@REM call :copy_dependencies

xcopy C:\msys64\mingw64\bin\*.dll .

endlocal