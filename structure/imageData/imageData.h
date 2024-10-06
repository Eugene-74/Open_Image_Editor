#pragma once

#include <iostream>
#include <vector>
#include <filesystem>

#include "../folders/folders.h"
#include "../metaData/metaData.h"

class ImageData
{
public:
    std::string imagePath;
    Folders folders;
    MetaData metaData;

    ImageData() : imagePath(""), folders(Folders()), metaData(MetaData()) {}

    ImageData(const ImageData& other)
        : imagePath(other.imagePath), folders(other.folders), metaData(other.metaData) {}

    ImageData(std::string a, const Folders& c) : imagePath(a), folders(c) {}

    // Opérateur d'affectation
    ImageData& operator=(const ImageData& other);

    void print() const;

    std::string get() const;

    MetaData* getMetaData();

    std::vector<std::string> getFolders();

    void addFolder(const std::string& toAddFolder);

    void addFolders(const std::vector<std::string>& toAddFolders);




    std::string getImageName() const;

    bool operator==(const ImageData& other) const;

    std::string getImagePath();



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

};


