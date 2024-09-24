#ifndef MAIN_H
#define MAIN_H

#include <iostream>
#include <vector>
#include <exiv2/exiv2.hpp>
#include <filesystem>

#include "structure/date/date.h"
#include "structure/folders/folders.h"
#include "structure/metaData/metaData.h"
#include "structure/imageData/imageData.h"
#include "structure/imagesData/imagesData.h"





// bool saveExifData(const std::string& imagePath, const Exiv2::ExifData& exifData);
// Exiv2::ExifData loadExifData(const std::string& imagePath);
// void displayExifData(const Exiv2::ExifData& exifData);
Date timestampToDate(time_t timestamp);



bool estImage(const std::string& cheminFichier);
void listerContenu(const std::string initialPath, const std::string chemin, ImagesData& imagesData);
void loadImagesMetaData(ImagesData& imagesData);


#endif
