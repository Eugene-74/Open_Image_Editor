unset GTK_PATH
# Créer un répertoire de build et y naviguer
mkdir buildLinux
cd buildLinux

# Configurer le projet avec CMake
cmake ..

# Compiler le projet
cmake --build .

# Lancer l'exécutable
./EasyImageEditor