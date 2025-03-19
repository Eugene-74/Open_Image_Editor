For all other informations go to [wiki](https://github.com/Eugene-74/Open_Image_Editor/wiki)

For docs go to [docs](https://eugene-74.github.io/Open_Image_Editor/index.html)

For sonarCloud stats :

[![Quality Gate Status](https://sonarcloud.io/api/project_badges/measure?project=Eugene-74_Open_Image_Editor&metric=alert_status)](https://sonarcloud.io/dashboard?id=Eugene-74_Open_Image_Editor)
[![Coverage](https://sonarcloud.io/api/project_badges/measure?project=Eugene-74_Open_Image_Editor&metric=coverage)](https://sonarcloud.io/dashboard?id=Eugene-74_Open_Image_Editor)
[![Maintainability Rating](https://sonarcloud.io/api/project_badges/measure?project=Eugene-74_Open_Image_Editor&metric=sqale_rating)](https://sonarcloud.io/dashboard?id=Eugene-74_Open_Image_Editor)
[![Reliability Rating](https://sonarcloud.io/api/project_badges/measure?project=Eugene-74_Open_Image_Editor&metric=reliability_rating)](https://sonarcloud.io/dashboard?id=Eugene-74_Open_Image_Editor)
[![Security Rating](https://sonarcloud.io/api/project_badges/measure?project=Eugene-74_Open_Image_Editor&metric=security_rating)](https://sonarcloud.io/dashboard?id=Eugene-74_Open_Image_Editor)

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

pacman -S mingw-w64-x86_64-cmake

pacman -S mingw-w64-x86_64-opencv

pacman -S mingw-w64-x86_64-qt6

pacman -S mingw-w64-x86_64-exiv2

pacman -S mingw-w64-x86_64-libheif

pacman -S mingw-w64-x86_64-curl

pacman -S mingw-w64-x86_64-jsoncpp

pacman -S mingw-w64-x86_64-dlib

pacman -S mingw-w64-x86_64-gtest

pacman -S mingw-w64-x86_64-ccache


```

When all in well installed just launch scripts\compile.bat
