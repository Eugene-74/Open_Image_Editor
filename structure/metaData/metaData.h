#ifndef METADATA_H
#define METADATA_H

#include <iostream>
#include <vector>
#include <fstream>
#include <exiv2/exiv2.hpp>
// #include "../imagesData/imagesData.h"



#include "../../structure/date/date.h"


class MetaData
{
public:
    Exiv2::ExifData exifMetaData; // JPEG, TIFF, mais pas PNG
    Exiv2::XmpData xmpMetaData;
    Exiv2::IptcData iptcMetaData;
    // 

        // Constructeur par défaut
    MetaData() = default;

    // Constructeur de copie
    MetaData(const MetaData& other) : exifMetaData(other.exifMetaData) {}

    // Opérateur d'affectation
    MetaData& operator=(const MetaData& other) {
        if (this != &other) {
            exifMetaData = other.exifMetaData; // Utiliser l'opérateur d'affectation de std::vector
            xmpMetaData = other.xmpMetaData; // Utiliser l'opérateur d'affectation de std::vector
            iptcMetaData = other.iptcMetaData; // Utiliser l'opérateur d'affectation de std::vector

        }
        return *this;
    }

    // Exiv2::ExifData& getExif();

    void setExif(const Exiv2::ExifData& toAddMetaData);

    // void loadExifMetaData(const std::string& imagePath);
    // void loadXmpMetaData(const std::string& imagePath);
    // void loadIptcMetaData(const std::string& imagePath);


    void saveMetaData(const std::string& imageName);

    int getImageWidth();

    int getImageHeight();

    int getImageOrientation();


    bool modifyExifValue(const std::string& key, const std::string& newValue);
    bool modifyXmpValue(const std::string& key, const std::string& newValue);
    bool modifyIptcValue(const std::string& key, const std::string& newValue);

    // Sauvegarder en binaire
    void save(std::ofstream& out) const;

    // Charger à partir d'un fichier binaire
    void load(std::ifstream& in);

    void setOrCreateExifData(std::string& imagePath);


    void setExifData(const Exiv2::ExifData data);
    void setXmpData(const Exiv2::XmpData data);
    void setIptcData(const Exiv2::IptcData data);

    Exiv2::ExifData getExifData();
    Exiv2::XmpData getXmpData();
    Exiv2::IptcData getIptcData();

    void displayMetaData();


    void loadData(const std::string& imagePath);
};



bool saveExifData(const std::string& imagePath, const Exiv2::ExifData& exifData);
bool saveXmpData(const std::string& imagePath, const Exiv2::XmpData& exifData);
bool saveIptcData(const std::string& imagePath, const Exiv2::IptcData& exifData);


void displayData(const MetaData metaData);
void displayExifData(const Exiv2::ExifData& data);
void displayXmpData(const Exiv2::XmpData& data);
void displayIptcData(const Exiv2::IptcData& data);



#endif
