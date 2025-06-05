@echo off

xcopy "%USERPROFILE%\Downloads\docs" . /E /I /Y

python "%~dp0update_versions_json.py"

git add .
git commit -m "Update documentation"
git push origin gh-pages