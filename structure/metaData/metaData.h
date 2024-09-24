#ifndef METADATA_H
#define METADATA_H

#include <iostream>
#include <vector>

#include <exiv2/exiv2.hpp>

#include "../../structure/date/date.h"


class MetaData
{
public:
    Exiv2::ExifData metaData;

    Exiv2::ExifData get();

    void set(const Exiv2::ExifData& toAddMetaData);

    void load(const std::string& imagePath);

    void save(const std::string& imageName);

    int getImageWidth();

    int getImageHeight();

    int getImageOrientation();

    Date getImageDate();

    bool modifyExifValue(const std::string& key, const std::string& newValue);
};


Exiv2::ExifData loadExifData(const std::string& imagePath);

bool saveExifData(const std::string& imagePath, const Exiv2::ExifData& exifData);

void displayExifData(const Exiv2::ExifData& exifData);

#endif
