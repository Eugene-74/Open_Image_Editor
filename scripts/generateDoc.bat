@echo off
setlocal enabledelayedexpansion

for /f "tokens=1,2 delims==" %%a in (%~dp0..\AppConfig.txt) do (
    set %%a=%%b
)

REM Update Doxyfile with APP_VERSION
set "APP_VERSION=%APP_VERSION%"
(for /f "delims=" %%i in (Doxyfile) do (
    set "line=%%i"
    if "!line:~0,12!"=="PROJECT_NAME" (
        echo PROJECT_NAME = %App_name%
    ) else if "!line:~0,14!"=="PROJECT_NUMBER" (
        echo PROJECT_NUMBER = %APP_VERSION%
    ) else (
        echo !line!
    )
)) > Doxyfile.tmp
move /Y Doxyfile.tmp Doxyfile

doxygen Doxyfile
endlocal