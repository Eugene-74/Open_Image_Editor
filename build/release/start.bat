@echo off
setlocal

type "AppConfig.txt"
for /f "tokens=1,2 delims==" %%a in (AppConfig.txt) do (
    set %%a=%%b
)

cd bin
set PATH=%PATH%;%CD%

REM Utiliser un appel de sous-routine pour ajouter les sous-répertoires au PATH
call :add_subdirectories_to_path

start "" /b "%CD%\%APP_NAME%-%APP_VERSION%.exe" 


endlocal
exit /b

:add_subdirectories_to_path
for /D %%d in (*) do (
    echo "Adding %%d to PATH"
    set "PATH=%PATH%;%CD%\%%d"
)
exit /b