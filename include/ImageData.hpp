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
   private:
    enum class PersonStatus {
        NotLoaded,
        Loading,
        Loaded
    };

    MetaData metaData;

    Folders folders;
    std::vector<Person> persons;
    PersonStatus personStatus = PersonStatus::NotLoaded;

    std::vector<std::vector<QPoint>> cropSizes;
    int orientation = Const::Orientation::UNDEFINED;
    long date = 0;

   public:
    ImageData()
        : folders(Folders()), cropSizes(), orientation(), date() {}

    ImageData(const Folders folders)
        : folders(folders) {}

    ImageData(std::string imagePath)
        : folders(Folders(imagePath)) {}

    ImageData(const ImageData& other)
        : folders(other.folders), metaData(other.metaData), cropSizes(other.cropSizes), orientation(other.orientation), date(other.date), persons(other.persons), personStatus(other.personStatus) {}

    ImageData& operator=(const ImageData& other);
    bool operator==(const ImageData& other) const;

    void print() const;
    
    std::string get() const;
    
    void setDate(long date);
    long getDate() const;
    
    void setOrientation(int orientation);
    int getOrientation() const;

    std::vector<Person> getpersons() const;
    void setpersons(const std::vector<Person>& persons);
    
    PersonStatus getPersonStatus() const;
    void setPersonStatus(PersonStatus personStatus);
    void setPersonStatusLoading();
    void setPersonStatusNotLoaded();
    void setPersonStatusLoaded();
    bool isPersonStatusLoading();
    bool isPersonStatusNotLoaded();
    bool isPersonStatusLoaded();

    MetaData* getMetaDataPtr();
    MetaData getMetaData() const;
    void setMetaData(const MetaData& metaData);

    std::vector<Folders> getFolders();

    void addFolder(const std::string& toAddFolder);

    void addFolders(const std::vector<std::string>& toAddFolders);

    std::string getImageName() const;

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
};
