#pragma once

#include <iostream>
#include <vector>

#include "FaceRecognition.hpp"
#include "Folders.hpp"
#include "MetaData.hpp"

// Forward declaration
class QPoint;

// class MetaData;

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
    int orientation;
    long date = 0;

   public:
    ImageData();
    ImageData(const Folders folders);

    ImageData(std::string imagePath);

    ImageData(const ImageData& other);

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
