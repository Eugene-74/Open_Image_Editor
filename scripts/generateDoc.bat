@echo off


xcopy "%USERPROFILE%\Downloads\docs" docs /E /I /Y

git checkout main