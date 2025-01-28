@echo off
setlocal

cd bin

set PATH=%PATH%;%CD%

for /D %%d in ("%CD%\*") do (
    set PATH=%PATH%;%%d
)

start "" "%CD%\OpenImageEditor.exe"

endlocal