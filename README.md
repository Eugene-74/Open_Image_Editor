For all other informations go to [wiki](https://github.com/Eugene-74/Open_Image_Editor/wiki) :
# Instalation

## Simple instalation

Just start the .exe in the last release and follow the instruction (there is no microsoft license for now so you must accept to start the .exe even if windows show a warning).

## Compilation

I you want to lose time you can build the project your self. For that you must :

### Install all dependencies

I am using Msys2 with MinGW64 so you need to install Msys2.
Then you need to install all the dependencies :

```
pacman -Syu

pacman -S base-devel mingw-w64-x86_64-toolchain

pacman -S mingw-w64-x86_64-opencv

pacman -S mingw-w64-x86_64-qt6

pacman -S mingw-w64-x86_64-exiv2

pacman -S mingw-w64-x86_64-libheif

pacman -S mingw-w64-x86_64-curl

pacman -S mingw-w64-x86_64-jsoncpp
```

When all in well installed just launch edit\compile.bat
