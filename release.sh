unset GTK_PATH
# Créer un répertoire de build et y naviguer
mkdir -p buildReleaseLinux
cd buildReleaseLinux

# Configurer le projet avec CMake en utilisant Ninja
cmake -G Ninja .. -DCMAKE_BUILD_TYPE=Release

# Compiler le projet avec Ninja
ninja -j8

# Lancer l'exécutable
./OpenImageEditor