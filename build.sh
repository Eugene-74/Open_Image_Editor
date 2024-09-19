# g++ -o main.exe main.cpp `pkg-config --cflags --libs Qt5Widgets opencv4`
g++ -fPIC main.cpp -o main.exe  `pkg-config --cflags --libs Qt5Widgets opencv4`
