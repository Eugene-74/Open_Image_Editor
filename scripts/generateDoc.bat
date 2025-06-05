@echo off

xcopy "%USERPROFILE%\Downloads\docs" . /E /I /Y
for /f "usebackq tokens=*" %%v in ('dir /b /ad docs') do (
    findstr /c:"\"%%v\"" versions.json >nul
    if errorlevel 1 (
        powershell -Command "(Get-Content versions.json | ConvertFrom-Json) + '%%v' | Set-Content versions.json"
        powershell -Command "((Get-Content versions.json | ConvertFrom-Json) | Sort-Object) | ConvertTo-Json | Set-Content versions.json"
    )
)
git add .
git commit -m "Update documentation"
git push origin gh-pages