@echo off


xcopy "%USERPROFILE%\Downloads\docs" docs /E /I /Y


cd docs
git add .
git commit -m "Update documentation"
git push origin gh-pages
cd ..


git checkout main