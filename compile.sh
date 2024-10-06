unset GTK_PATH
sh compileLib.sh
g++ -g main.cpp ressources/qrc_resources.cpp \
    -fPIC -L./lib -lfolders -o main.exe  `pkg-config --cflags --libs Qt5Widgets Qt5Gui Qt5Core  opencv4` -lexiv2

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