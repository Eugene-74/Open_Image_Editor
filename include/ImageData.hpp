#pragma once

#include <filesystem>
#include <iostream>
#include <vector>

#include "Folders.hpp"
#include "MetaData.hpp"
namespace fs = std::filesystem;

class ImageData {
   public:
    // std::string imagePath;
    Folders folders;
    MetaData metaData;
    std::vector<std::vector<QPoint>> cropSizes;

    int orientation = 0;
    long date = 0;

    ImageData()
        : folders(Folders()), metaData(MetaData()), cropSizes(), orientation() {}

    // !! necessaire sinon push_back ne fonctionne pas
    ImageData(const ImageData& other)
        : folders(other.folders), metaData(other.metaData), cropSizes(other.cropSizes), orientation(other.orientation) {
    }

    ImageData(const Folders c)
        : folders(c) {}

    // Opérateur d'affectation
    ImageData& operator=(const ImageData& other);

    void print() const;

    std::string get() const;

    MetaData* getMetaData();
    MetaData getMetaData() const;

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
    void clearMetaData();

   private:
};
