

unset GTK_PATH
sh src/compileLib.sh
    # -L./opencv/build/lib -lopencv_ts \
    # -I/usr/include/opencv4 \
    # -I/libraries/Exiv2/exiv2/include/exiv2 -L/libraries/Exiv2/install/lib -lexiv2 \
    # -I/libraries/opencv/include -L/libraries/opencv/build/lib -lopencv_core -lopencv_imgcodecs -lopencv_highgui -lopencv_imgproc\

g++ -g src/main.cpp src/ressources/qrc_ressources.cpp \
    -fPIC \
    -L./lib -lfolders -o build/main.exe  `pkg-config --cflags --libs Qt5Widgets Qt5Gui Qt5Core opencv4` -lexiv2


cd build
./main.exe