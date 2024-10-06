unset GTK_PATH
sh compileLib.sh
    # -L./opencv/build/lib -lopencv_ts \
    # -I/usr/include/opencv4 \
    # -I/libraries/Exiv2/exiv2/include/exiv2 -L/libraries/Exiv2/install/lib -lexiv2 \
    # -I/libraries/opencv/include -L/libraries/opencv/build/lib -lopencv_core -lopencv_imgcodecs -lopencv_highgui -lopencv_imgproc\

g++ -g main.cpp ressources/qrc_ressources.cpp \
    -fPIC \
    -L./lib -lfolders -o main.exe  `pkg-config --cflags --libs Qt5Widgets Qt5Gui Qt5Core opencv4` -lexiv2
    # opencv4

# g++ main.cpp ressources/qrc_resources.cpp \
#     lib/vector.o \
#     lib/thumbnail.o \
#     lib/imageEditor.o \
#     lib/folders.o \
#     lib/metaData.o \
#     lib/imageData.o \
#     lib/imagesData.o \
#     -fPIC -o main.exe  `pkg-config --cflags --libs Qt5Widgets Qt5Gui Qt5Core  opencv4` -lexiv2

# g++ main.cpp ressources/qrc_resources.cpp \
#     functions/vector/vector.cpp \
#     functions/thumbnail/thumbnail.cpp \
#     display/imageEditor/imageEditor.cpp \
#     structure/folders/folders.cpp \
#     structure/metaData/metaData.cpp \
#     structure/imageData/imageData.cpp \
#     structure/imagesData/imagesData.cpp \
#     -fPIC -o main.exe  `pkg-config --cflags --libs Qt5Widgets Qt5Gui Qt5Core  opencv4` -lexiv2
# essai : 

./main.exe