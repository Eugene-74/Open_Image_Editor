@echo off


xcopy "%USERPROFILE%\Downloads\docs" docs /E /I /Y


@REM cd docs
@REM git add .
@REM git commit -m "Update documentation"
@REM git push origin gh-pages
@REM cd ..


@REM git checkout main