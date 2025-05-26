@echo off
setlocal enabledelayedexpansion

for /f "tokens=1,2 delims==" %%a in (%~dp0..\AppConfig.txt) do (
    set %%a=%%b
)

mkdir build
cd build

cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER_LAUNCHER=ccache -DCMAKE_CXX_COMPILER_LAUNCHER=ccache -DCMAKE_CUDA_COMPILER=nvcc -DCMAKE_CUDA_ARCHITECTURES=all ..

cmake --build . 

@REM TODO remettre les tests (marche pas)
@REM ctest --output-on-failure
@REM if %errorlevel% neq 0 (
@REM     echo Tests failed, aborting execution.
@REM     exit /b %errorlevel%
@REM )

rmdir /s /q release

mkdir release
cd release

set EXECUTABLE=%APP_NAME%-%APP_VERSION%.exe
@REM copy ..\..\install\start.bat .
copy ..\..\install\icon.ico .
copy ..\..\install\cacert.pem .
copy ..\..\LICENSE .
copy ..\..\README.md .
copy ..\..\AppConfig.txt .




mkdir bin
cd bin

copy ..\..\%EXECUTABLE% .

windeployqt6.exe --release --qmldir ..\..\..\src\ressources %EXECUTABLE%

C:\mingw-bundledlls\mingw-bundledlls C:\Users\eugen\Documents\MesDocuments\git\Open_Image_Editor\build\release\bin\%EXECUTABLE% > dependencies.txt

:copy_dependencies
for /f "tokens=*" %%i in ('type dependencies.txt') do (
    xcopy /Y "%%i" .
    C:\mingw-bundledlls\mingw-bundledlls %%i > temp_dependencies.txt
    for /f "tokens=*" %%j in (temp_dependencies.txt) do (
        if not exist "%%j" (
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