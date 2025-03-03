#pragma once

#include <filesystem>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>

#include "FaceRecognition.hpp"
#include "Folders.hpp"
#include "MetaData.hpp"

namespace fs = std::filesystem;

class ImageData {
    // TODO netoyer

   public:
    enum class Status {
        NotLoaded,
        Loading,
        Loaded
    };
    Folders folders;
    MetaData metaData;
    std::vector<std::vector<QPoint>> cropSizes;
    std::vector<Person> persons;
    Status status = Status::NotLoaded;

    int orientation = Const::Orientation::UNDEFINED;
    long date = 0;

    ImageData()
        : folders(Folders()), metaData(MetaData()), cropSizes(), orientation(), date(), persons(), status() {}

    // !! necessaire sinon push_back ne fonctionne pas
    ImageData(const ImageData& other)
        : folders(other.folders), metaData(other.metaData), cropSizes(other.cropSizes), orientation(other.orientation), date(other.date), persons(other.persons), status(other.status) {
    }

    ImageData(const Folders c)
        : folders(c) {}

    // Opérateur d'affectation
    ImageData& operator=(const ImageData& other);

    void print() const;

    std::string get() const;

    MetaData* getMetaData();
    MetaData getMetaData() const;

    std::vector<Folders> getFolders();

    void addFolder(const std::string& toAddFolder);

    void addFolders(const std::vector<std::string>& toAddFolders);

    std::string getImageName() const;

    bool operator==(const ImageData& other) const;

    std::string getImagePath();
    std::string getImagePathConst() const;

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

    std::vector<std::vector<QPoint>> getCropSizes() const;

    void setCropSizes(const std::vector<std::vector<QPoint>>& cropSizes);
    void clearMetaData();

   private:
};
