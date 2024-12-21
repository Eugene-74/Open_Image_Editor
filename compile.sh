unset GTK_PATH
# Créer un répertoire de build et y naviguer
mkdir -p buildLinux
cd buildLinux

# Configurer le projet avec CMake en utilisant Ninja
cmake -G Ninja ..

# Compiler le projet avec Ninja
ninja

# Lancer l'exécutable
./EasyImageEditor