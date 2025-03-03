@echo off
setlocal enabledelayedexpansion

for /f "tokens=1,2 delims==" %%a in (%~dp0..\AppConfig.txt) do (
    set %%a=%%b
)

git tag -s v%APP_VERSION%
git push origin v%APP_VERSION%
gh release create v%APP_VERSION% %INSTALLER_APP_NAME%-%APP_VERSION%.exe --notes "Version %APP_VERSION%"


endlocal