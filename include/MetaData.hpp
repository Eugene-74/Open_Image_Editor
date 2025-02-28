#pragma once

#include <QDateTime>
#include <QDebug>
#include <codecvt>
#include <ctime>
#include <exiv2/exiv2.hpp>
#include <fstream>
#include <iostream>
#include <vector>

class MetaData {
   public:
    bool dataLoaded = false;

    Exiv2::ExifData exifMetaData;  // JPEG, TIFF, mais pas PNG
    Exiv2::XmpData xmpMetaData;
    Exiv2::IptcData iptcMetaData;
    //

    // Constructeur par défaut
    MetaData() = default;

    // Constructeur de copie
    MetaData(const MetaData& other)
        : exifMetaData(other.exifMetaData) {}

    // Opérateur d'affectation
    MetaData& operator=(const MetaData& other);

    void setExif(const Exiv2::ExifData& toAddMetaData);

    void saveMetaData(const std::string& imageName);

    int getImageWidth();

    int getImageHeight();

    int getImageOrientation();
    long getTimestamp();

    bool modifyExifValue(const std::string& key, const std::string& newValue);
    bool modifyXmpValue(const std::string& key, const std::string& newValue);
    bool modifyIptcValue(const std::string& key, const std::string& newValue);

    void setOrCreateExifData(std::string imagePath);

    void setExifData(const Exiv2::ExifData data);
    void setXmpData(const Exiv2::XmpData data);
    void setIptcData(const Exiv2::IptcData data);

    Exiv2::ExifData getExifData();
    Exiv2::XmpData getXmpData();
    Exiv2::IptcData getIptcData();

    void displayMetaData();

    void loadData(const std::string& imagePath);
    void clear();
};

bool saveExifData(const std::string& imagePath, const Exiv2::ExifData& exifData);
bool saveXmpData(const std::string& imagePath, const Exiv2::XmpData& exifData);
bool saveIptcData(const std::string& imagePath, const Exiv2::IptcData& exifData);

void displayData(const MetaData metaData);
void displayExifData(const Exiv2::ExifData& data);
void displayXmpData(const Exiv2::XmpData& data);
void displayIptcData(const Exiv2::IptcData& data);
