# Compile all the library

g++ -I./functions -I./structure -c structure/date/date.cpp -o lib/date.o
g++ -I./functions -I./structure -c structure/folders/folders.cpp -o lib/folders.o
g++ -I./functions -I./structure -c structure/metaData/metaData.cpp -o lib/metaData.o
g++ -I./functions -I./structure -c structure/imageData/imageData.cpp -o lib/imageData.o
g++ -I./functions -I./structure -c structure/imagesData/imagesData.cpp -o lib/imagesData.o


g++ -I./functions -I./structure -c functions/vector/vector.cpp -o lib/vector.o



ar rcs lib/libfolders.a lib/vector.o lib/folders.o lib/date.o lib/metaData.o lib/imageData.o lib/imagesData.o

# sh compileLib.sh