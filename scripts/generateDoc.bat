@echo off


xcopy "%USERPROFILE%\Downloads\docs" docs /E /I /Y


git add .
git commit -m "Update documentation"
git push origin gh-pages


@REM git checkout main