#pragma once

#include <iostream>
#include <vector>

#include "Folders.hpp"
#include "LoadingStatus.hpp"
#include "MetaData.hpp"
#include "ObjectRecognition.hpp"

// Forward declaration
class QPoint;

// class MetaData;

class ImageData {
    enum class DetectionStatus {
        NotLoaded,
        Loading,
        Loaded
    };

   public:
    ImageData();
    ImageData(const Folders folders);

    ImageData(std::string imagePath);

    ImageData(const ImageData& other);

    ImageData& operator=(const ImageData& other);
    bool operator==(const ImageData& other) const;

    bool respectFilters(const std::map<std::string, bool>& filters) const;

    void print() const;
    
    std::string get() const;

    std::map<std::string, std::vector<std::pair<cv::Rect, float>>> getDetectedObjects();
    std::vector<DetectedFaces>* getDetectedFacesPtr();
    std::vector<DetectedFaces> getDetectedFacesConst() const;
    void setDetectedObjects(const std::map<std::string, std::vector<std::pair<cv::Rect, float>>>& detectedObjects);
    void clearDetectedObjects();
    void detectFaces();

    void setDate(long date);
    long getDate() const;
    
    void setOrientation(int orientation);
    int getOrientation() const;

    void setLatitude(double latitude);
    double getLatitude() const;

    void setLongitude(double longitude);
    double getLongitude() const;

    // DetectionStatus getDetectionStatus() const;
    void setDetectionStatus(DetectionStatus detectionStatus);
    void setDetectionStatusLoading();
    void setDetectionStatusNotLoaded();
    void setDetectionStatusLoaded();
    bool isDetectionStatusLoading();
    bool isDetectionStatusNotLoaded();
    bool isDetectionStatusLoaded();

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

    int getImageOrientation();

    void turnImage(int rotation);

    void save(std::ofstream& out) const;
    void load(std::ifstream& in);

    void setOrCreateExifData();

    std::vector<std::vector<QPoint>> getCropSizes() const;
    void setCropSizes(const std::vector<std::vector<QPoint>>& cropSizes);

    void clearMetaData();

    LoadingStatus getFaceDetectionStatus() const;

   private:
    MetaData metaData;
    DetectedObjects detectedObjects;

    Folders folders;
    DetectionStatus detectionStatus = DetectionStatus::NotLoaded;
    LoadingStatus faceDetectionStatus;

    std::vector<std::vector<QPoint>> cropSizes;
    int orientation;
    long date = 0;
    double latitude = 0.0;
    double longitude = 0.0;
};
