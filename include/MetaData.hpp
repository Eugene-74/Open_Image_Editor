#pragma once

#include <exiv2/exiv2.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <QObject>
class MetaData {
   public:
    MetaData() = default;

    MetaData(const MetaData& other)
        : xmpMetaData(other.xmpMetaData),
          exifMetaData(other.exifMetaData),
          iptcMetaData(other.iptcMetaData) {}

    MetaData& operator=(const MetaData& other);
    bool operator==(const MetaData& other) const;

    void setExif(const Exiv2::ExifData& toAddMetaData);

    void saveMetaData(const std::string& imagePath);

    int getImageOrientation();
    long getTimestamp();
    void setTimestamp(long timestamp);

    QString getTimestampString();

    void setLatitude(double latitude);
    double getLatitude() const;

    void setLongitude(double longitude);
    double getLongitude() const;

    int getOrientation() const;
    void setOrientation(int orientation);

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

    bool getDataLoadedConst() const;
    void setDataLoaded(bool dataLoaded);

    Exiv2::ExifData getExifMetaDataConst() const;
    Exiv2::XmpData getXmpMetaDataConst() const;
    Exiv2::IptcData getIptcMetaDataConst() const;

   private:
    bool dataLoaded = false;

    Exiv2::ExifData exifMetaData;
    Exiv2::XmpData xmpMetaData;
    Exiv2::IptcData iptcMetaData;
};

bool saveExifData(const std::string& imagePath, const Exiv2::ExifData& exifData);
bool saveXmpData(const std::string& imagePath, const Exiv2::XmpData& exifData);
bool saveIptcData(const std::string& imagePath, const Exiv2::IptcData& exifData);

void displayData(const MetaData metaData);
void displayExifData(const Exiv2::ExifData& data);
void displayXmpData(const Exiv2::XmpData& data);
void displayIptcData(const Exiv2::IptcData& data);
