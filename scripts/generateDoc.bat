@echo off

xcopy "%USERPROFILE%\Downloads\docs" . /E /I /Y

REM Ajout automatique des dossiers de version à versions.json
powershell -NoProfile -ExecutionPolicy Bypass -Command ^
    "$versionsPath = 'versions.json'; $dirs = Get-ChildItem -Directory | Where-Object { $_.Name -match '^[0-9]+\.[0-9]+\.[0-9]+$' } | ForEach-Object { $_.Name }; if (Test-Path $versionsPath) { $json = Get-Content $versionsPath | ConvertFrom-Json } else { $json = @() }; foreach ($v in $dirs) { if ($json -notcontains $v) { $json += $v } }; $json = $json | Sort-Object; $json | ConvertTo-Json | Set-Content $versionsPath -Encoding UTF8"

git add .
git commit -m "Update documentation"
git push origin gh-pages