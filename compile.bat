mkdir build
cd build

cmake -G "Ninja" ..
    
cmake --build .

OpenImageEditor.exe

cd ..