#pragma once

#include <iostream>
#include <vector>
#include <filesystem>

#include "../folders/Folders.h"
#include "../metaData/MetaData.h"
#include <filesystem>
namespace fs = std::filesystem;

class ImageData
{
public:
    std::string imagePath;
    Folders folders;
    MetaData metaData;
    std::vector<std::vector<QPoint>> cropSizes;

    ImageData() : imagePath(""), folders(Folders()), metaData(MetaData()), cropSizes() {}

    ImageData(const ImageData& other)
        : imagePath(other.imagePath), folders(other.folders), metaData(other.metaData), cropSizes(other.cropSizes) {
    }

    ImageData(std::string a, const Folders c) : imagePath(a), folders(c) {}

    // Opérateur d'affectation
    // ImageData& operator=(const ImageData& other);
    ImageData& operator=(const ImageData& other);

    void print() const;

    std::string get() const;

    MetaData* getMetaData();
    MetaData getMetaData()const;

    std::vector<std::string> getFolders();

    void addFolder(const std::string& toAddFolder);

    void addFolders(const std::vector<std::string>& toAddFolders);




    std::string getImageName() const;

    bool operator==(const ImageData& other) const;

    std::string getImagePath();
    std::string getImageExtension();




    void setExifMetaData(const Exiv2::ExifData& toAddMetaData);

    void loadData();

    void saveMetaData();

    int getImageWidth();

    int getImageHeight();

    int getImageOrientation();


    void turnImage(int rotation);


    void save(std::ofstream& out) const;

    void load(std::ifstream& in);

    void setOrCreateExifData();

    void handleExiv2Error(const Exiv2::Error& e);

    std::vector<std::vector<QPoint>> getCropSizes() const;

    void setCropSizes(const std::vector<std::vector<QPoint>>& cropSizes);

};


