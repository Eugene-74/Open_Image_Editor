# Compile all the library

#!/bin/bash

# Compile individual files
g++ -I./functions -I./structure -I./display `pkg-config --cflags --libs opencv4` `pkg-config --cflags --libs exiv2` -c src/structure/folders/folders.cpp -o lib/folders.o
g++ -I./functions -I./structure -I./display `pkg-config --cflags --libs opencv4` `pkg-config --cflags --libs exiv2` -c src/structure/metaData/metaData.cpp -o lib/metaData.o
g++ -I./functions -I./structure -I./display `pkg-config --cflags --libs opencv4` `pkg-config --cflags --libs exiv2` -c src/structure/imageData/imageData.cpp -o lib/imageData.o
g++ -I./functions -I./structure -I./display `pkg-config --cflags --libs opencv4` `pkg-config --cflags --libs exiv2` -c src/structure/imagesData/imagesData.cpp -o lib/imagesData.o
g++ -I./functions -I./structure -I./display `pkg-config --cflags --libs opencv4` `pkg-config --cflags --libs exiv2` -c src/structure/data/data.cpp -o lib/data.o

# Compile the rest of the files similarly...

# Archive everything into a static library
# ar rcs lib/libfolders.a lib/*.o



g++ -I./functions -I./structure -I./display -c src/functions/vector/vector.cpp -o lib/vector.o
g++ -I./functions -I./structure -I./display -c src/functions/clickableLabel/clickableLabel.cpp -fPIC `pkg-config --cflags --libs Qt5Widgets` -o lib/clickableLabel.o

g++ -I./functions -I./structure -I./display `pkg-config --cflags --libs opencv4` -c src/functions/thumbnail/thumbnail.cpp -o lib/thumbnail.o





moc -o lib/moc_imageEditor.cpp src/display/imageEditor/imageEditor.h
g++ -I./functions -I./structure -I./display -fPIC `pkg-config --cflags --libs opencv4` `pkg-config --cflags --libs Qt5Widgets`-c src/display/imageEditor/imageEditor.cpp -o lib/imageEditor.o
g++ -c lib/moc_imageEditor.cpp -fPIC `pkg-config --cflags --libs opencv4` `pkg-config --cflags --libs Qt5Widgets` -o lib/moc_imageEditor.o



ar rcs lib/libfolders.a lib/*.o


# sh compileLib.sh