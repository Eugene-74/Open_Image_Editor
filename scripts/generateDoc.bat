@echo off

xcopy "%USERPROFILE%\Downloads\docs" . /E /I /Y

@REM TODO faire versio auto

git add .
git commit -m "Update documentation"
git push origin gh-pages