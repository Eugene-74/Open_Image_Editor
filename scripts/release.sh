unset GTK_PATH

# cd ..

# Créer un répertoire de build et y naviguer
mkdir -p buildReleaseLinux
cd buildReleaseLinux

# Configurer le projet avec CMake en utilisant Ninja
cmake -G Ninja .. -DCMAKE_BUILD_TYPE=Release

# Compiler le projet avec Ninja
ninja -j8

../linuxdeployqt-continuous-x86_64.AppImage OpenImageEditor -unsupported-allow-new-glibc
# ../linuxdeployqt-continuous-x86_64.AppImage OpenImageEditor -unsupported-allow-new-glibc -appimage


# Check for libexiv2
if ldd OpenImageEditor | grep -q exiv2; then
    echo "libexiv2 is correctly linked."
else
    echo "libexiv2 is not linked. Please check your installation."
fi

# Check for opencv
if ldd OpenImageEditor | grep -q opencv4; then
    echo "opencv is correctly linked."
else
    echo "opencv is not linked. Please check your installation."
fi

# Creation zip file
mkdir MonApplication
cp OpenImageEditor MonApplication/
ldd OpenImageEditor | grep "=> /" | awk '{print $3}' | xargs -I '{}' cp -v '{}' MonApplication/
zip -r MonApplication.zip MonApplication


echo "Testing the application with included libraries..."
cd MonApplication
LD_LIBRARY_PATH=. ./OpenImageEditor
cd ..