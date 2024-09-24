# moc main.cpp -o moc_main.cpp
# g++ -L. -lfolders -fPIC main.cpp -o main.exe `pkg-config --cflags --libs Qt5Widgets opencv4` -lexiv2
# g++ -I./structure -fPIC main.cpp -o main.exe `pkg-config --cflags --libs Qt5Widgets opencv4` -lexiv2
# g++ -I./structure -I./functions -fPIC main.cpp  functions/vector/vector.cpp structure/folders/folders.cpp -o main.exe `pkg-config --cflags --libs Qt5Widgets opencv4` -lexiv2

g++ main.cpp -L./lib -lfolders -o main.exe `pkg-config --cflags --libs Qt5Widgets opencv4` -lexiv2
