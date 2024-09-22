# moc main.cpp -o moc_main.cpp
g++ -fPIC main.cpp -o main.exe `pkg-config --cflags --libs Qt5Widgets opencv4` -lexiv2
