unset GTK_PATH
# Créer un répertoire de build et y naviguer
mkdir build
cd build

# Configurer le projet avec CMake
cmake ..

# Compiler le projet
cmake --build .

cpack