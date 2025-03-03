@echo off
setlocal enabledelayedexpansion

for /f "tokens=1,2 delims==" %%a in (%~dp0..\AppConfig.txt) do (
    set %%a=%%b
)

mkdir build
cd build

cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release ..
    
mingw32-make -j %NUMBER_OF_PROCESSORS% 

rmdir /s /q release

mkdir release
cd release

set EXECUTABLE=%APP_NAME%-%APP_VERSION%.exe
copy ..\..\install\start.bat .
copy ..\..\install\icon.ico .
copy ..\..\install\launch.vbs .
copy ..\..\LICENSE .
copy ..\..\README.md .
copy ..\..\AppConfig.txt .




mkdir bin
cd bin

copy ..\..\%EXECUTABLE% .

windeployqt6.exe %EXECUTABLE%

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

cd ..
cd ..
cd ..

del /q *.exe

cd install 

makensis installateur.nsi


cd ..

gh release delete v%APP_VERSION% --yes
git tag -d v%APP_VERSION%
git push origin --delete v%APP_VERSION%

git tag v%APP_VERSION%
git push origin v%APP_VERSION%
gh release create v%APP_VERSION% %INSTALLER_APP_NAME%-%APP_VERSION%.exe --notes "Version %APP_VERSION%" --prerelease

exit /b

call :copy_dependencies

endlocal