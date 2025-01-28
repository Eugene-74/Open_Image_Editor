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
copy ..\start.bat .

mkdir bin
cd bin

copy ..\..\build\%EXECUTABLE% .

windeployqt6.exe %EXECUTABLE%

@REM NON trouver jsp pk mais necessaire
xcopy C:\msys64\mingw64\bin\libjpeg-8.dll .

C:\mingw-bundledlls\mingw-bundledlls C:\Users\eugen\Documents\MesDocuments\git\Open_Image_Editor\release\bin\%EXECUTABLE% > dependencies.txt

:copy_dependencies
for /f "tokens=*" %%i in ('type dependencies.txt') do (
    echo Copying %%i
    xcopy /Y "%%i" .
    C:\mingw-bundledlls\mingw-bundledlls %%i > temp_dependencies.txt
    for /f "tokens=*" %%j in (temp_dependencies.txt) do (
        if not exist "%%j" (
            echo Copying sub-dependency %%j
            xcopy /Y "%%j" .
        )
    )
)
del dependencies.txt
del temp_dependencies.txt
exit /b

call :copy_dependencies

endlocal