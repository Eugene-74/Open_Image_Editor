@echo off
setlocal enabledelayedexpansion

for /f "tokens=1,2 delims==" %%a in (%~dp0..\AppConfig.txt) do (
    set %%a=%%b
)

mkdir build
cd build

cmake -G "Ninja" ..
    
cmake --build . --config Release


rmdir /s /q release

mkdir release
cd release

set EXECUTABLE=%APP_NAME%.exe
copy ..\..\install\start.bat .
copy ..\..\install\icon.ico .
copy ..\..\install\launch.vbs .
copy ..\..\LICENSE .
copy ..\..\README.md .




mkdir bin
cd bin

copy ..\..\%EXECUTABLE% .

windeployqt6.exe %EXECUTABLE%

@REM NON trouver jsp pk mais necessaire
@REM xcopy C:\msys64\mingw64\bin\libjpeg-8.dll .

C:\mingw-bundledlls\mingw-bundledlls C:\Users\eugen\Documents\MesDocuments\git\Open_Image_Editor\build\release\bin\%EXECUTABLE% > dependencies.txt

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

@REM TODO add
cd ..
cd ..
cd ..

del /q *.exe

cd install 

makensis installateur.nsi

git tag v%APP_VERSION%
git push origin v%APP_VERSION%


exit /b

call :copy_dependencies

endlocal