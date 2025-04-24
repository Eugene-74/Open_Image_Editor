mkdir build
cd build
mkdir coverage


if "%SONAR_TOKEN%"=="" (
    echo "Erreur : SONAR_TOKEN n'est pas défini. Veuillez le définir avant d'exécuter ce script. : setx SONAR_TOKEN <votre_token>"
    exit /b 1
)

gcovr -r ..  --exclude ".*\.hpp$" --exclude ".*moc_.*\.cpp$" --html --html-details -o coverage/coverage.html
gcovr -r ..  --exclude ".*\.hpp$" --exclude ".*moc_.*\.cpp$" --sonarqube -o coverage/coverage-sonarqube.xml


cd ..
sonar-scanner -Dsonar.token=%SONAR_TOKEN%