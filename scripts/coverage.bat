mkdir build
cd build
mkdir coverage

gcovr -r ..  --exclude ".*\.hpp$" --exclude ".*moc_.*\.cpp$" --html --html-details -o coverage/coverage.html
gcovr -r ..  --exclude ".*\.hpp$" --exclude ".*moc_.*\.cpp$" --sonarqube -o coverage/coverage-sonarqube.xml


cd ..