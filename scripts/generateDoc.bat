@echo off
setlocal enabledelayedexpansion

doxygen Doxyfile

git stash push -m "Stash before switching to github-pages"

@REM git checkout github-pages



@REM git checkout main

@REM git stash pop



endlocal
