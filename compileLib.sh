# Compile all the library

#!/bin/bash

# Compile individual files
g++ -I./functions -I./structure -I./display `pkg-config --cflags --libs opencv4` `pkg-config --cflags --libs exiv2` -c structure/date/date.cpp -o lib/date.o
g++ -I./functions -I./structure -I./display `pkg-config --cflags --libs opencv4` `pkg-config --cflags --libs exiv2` -c structure/folders/folders.cpp -o lib/folders.o
g++ -I./functions -I./structure -I./display `pkg-config --cflags --libs opencv4` `pkg-config --cflags --libs exiv2` -c structure/metaData/metaData.cpp -o lib/metaData.o
g++ -I./functions -I./structure -I./display `pkg-config --cflags --libs opencv4` `pkg-config --cflags --libs exiv2` -c structure/imageData/imageData.cpp -o lib/imageData.o
g++ -I./functions -I./structure -I./display `pkg-config --cflags --libs opencv4` `pkg-config --cflags --libs exiv2` -c structure/imagesData/imagesData.cpp -o lib/imagesData.o

# Compile the rest of the files similarly...

# Archive everything into a static library
ar rcs lib/libfolders.a lib/*.o



g++ -I./functions -I./structure -I./display -c functions/vector/vector.cpp -o lib/vector.o
g++ -I./functions -I./structure -I./display -c functions/clickableLabel/ClickableLabel.cpp -fPIC `pkg-config --cflags --libs Qt5Widgets` -o lib/clickableLabel.o

g++ -I./functions -I./structure -I./display `pkg-config --cflags --libs opencv4` -c functions/thumbnail/thumbnail.cpp -o lib/thumbnail.o





moc -o moc_imageEditor.cpp display/imageEditor/imageEditor.h
g++ -I./functions -I./structure -I./display -fPIC `pkg-config --cflags --libs Qt5Widgets`-c display/imageEditor/imageEditor.cpp -o lib/imageEditor.o
g++ -c moc_imageEditor.cpp -fPIC `pkg-config --cflags --libs Qt5Widgets` -o lib/moc_imageEditor.o



ar rcs lib/libfolders.a lib/*.o


# sh compileLib.sh