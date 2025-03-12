@echo off

xcopy "%USERPROFILE%\Downloads\docs" . /E /I /Y

git add .
git commit -m "Update documentation"
git push origin gh-pages