#ifndef MAIN_H
#define MAIN_H

#include <iostream>
#include <vector>
#include <exiv2/exiv2.hpp>
#include <filesystem>

#include "structure/folders/folders.h"
#include "structure/metaData/metaData.h"
#include "structure/imageData/imageData.h"
#include "structure/imagesData/imagesData.h"


const std::string THUMBNAIL_PATH = "/home/eugene/.cache/thumbnails";

bool estImage(const std::string& cheminFichier);



void countImagesFromFolder(const std::string path, int& nbrImage);
void loadImagesFromFolder(const std::string initialPath, const std::string path, ImagesData& imagesData, int& nbrImage);
void startLoadingImagesFromFolder(const std::string imagePaths, ImagesData& imagesData);
void loadImagesMetaData(ImagesData& imagesData);
void nextImage(int nbr);



#endif
