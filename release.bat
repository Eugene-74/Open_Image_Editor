mkdir build
cd build

cmake -G "Ninja" ..
    
cmake --build .

cpack
cd ..