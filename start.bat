@echo off
setlocal

cd bin
set PATH=%PATH%;%CD%
echo "here"

REM Utiliser un appel de sous-routine pour ajouter les sous-répertoires au PATH
call :add_subdirectories_to_path

start "" "%CD%\OpenImageEditor.exe"

endlocal
exit /b

:add_subdirectories_to_path
for /D %%d in (*) do (
    echo "Adding %%d to PATH"
    set "PATH=%PATH%;%CD%\%%d"
)
exit /b