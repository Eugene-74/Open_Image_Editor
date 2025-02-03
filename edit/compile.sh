unset GTK_PATH

# cd ..


# Créer un répertoire de build et y naviguer
mkdir -p build
cd build

# Configurer le projet avec CMake en utilisant Ninja
cmake -G Ninja ..

# Compiler le projet avec Ninja
ninja -j8

# Lancer l'exécutable
./OpenImageEditor