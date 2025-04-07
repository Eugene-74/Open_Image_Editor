#include "ImageData.hpp"

#include <QDebug>
#include <filesystem>

#include "Const.hpp"
#include "Verification.hpp"

namespace fs = std::filesystem;

ImageData::ImageData()
    : folders(Folders()), cropSizes(), orientation(), date() {
    orientation = Const::Orientation::UNDEFINED;
}

ImageData::ImageData(const Folders folders)
    : folders(folders) {
    orientation = Const::Orientation::UNDEFINED;
}

ImageData::ImageData(std::string imagePath)
    : folders(Folders(imagePath)) {
    orientation = Const::Orientation::UNDEFINED;
}

ImageData::ImageData(const ImageData& other)
    : folders(other.folders), metaData(other.metaData), cropSizes(other.cropSizes), orientation(other.orientation), date(other.date)
      // , persons(other.persons)
      ,
      detectionStatus(other.detectionStatus) {}

ImageData& ImageData::operator=(const ImageData& other) {
    if (this != &other) {
        folders.getName() = other.folders.getName();
        metaData = other.metaData;
        cropSizes = other.cropSizes;
        orientation = other.orientation;
        detectedObjects = other.detectedObjects;
        detectionStatus = other.detectionStatus;
        date = other.date;
    }
    return *this;
}

bool ImageData::operator==(const ImageData& other) const {
    return this->folders.getName() == other.folders.getName();
}

void ImageData::print() const {
    qInfo() << get();
}

bool ImageData::respectFilters(const std::map<std::string, bool>& filters) const {
    if (std::all_of(filters.begin(), filters.end(), [](const auto& filter) {
            return !filter.second || filter.first == "image" || filter.first == "video";
        })) {
        return true;
    }
    if (filters.at("image") && isImage(folders.getName())) {
        for (const auto& filter : filters) {
            if (filter.second && detectedObjects.getDetectedObjectsConst().count(filter.first)) {
                return true;
            }
        }
    } else if (filters.at("video") && isVideo(folders.getName())) {
        return true;
    }
    return false;
}

std::string ImageData::get() const {
    std::string name;
    name += "Image : " + getImagePathConst() + " folders : ";
    for (const auto& file : folders.getFoldersConst()) {
        name += " ";
        name += file.getName();
    }
    name += "\n";
    return name;
}

MetaData* ImageData::getMetaDataPtr() {
    return &metaData;
}

MetaData ImageData::getMetaData() const {
    return metaData;
}

std::vector<Folders> ImageData::getFolders() {
    return *folders.getFolders();
}


void ImageData::addFolder(const std::string& toAddFolder) {
    folders.addFolder(toAddFolder);
}

void ImageData::addFolders(const std::vector<std::string>& toAddFolders) {
    for (const auto& folder : toAddFolders) {
        folders.addFolder(folder);
    }
}

std::string ImageData::getImageName() const {
    std::filesystem::path filePath(getImagePathConst());
    return filePath.filename().string();
}

std::string ImageData::getImagePath() {
    return folders.getName();
}

std::string ImageData::getImagePathConst() const {
    return folders.getName();
}

std::string ImageData::getImageExtension() {
    return fs::path(getImagePath()).extension().string();
}

void ImageData::setExifMetaData(const Exiv2::ExifData& toAddMetaData) {
    try {
        metaData.setExifData(toAddMetaData);
        saveMetaData();
    } catch (const Exiv2::Error& e) {
        qWarning() << "Exiv2 error: " << e.what();
    }
}

void ImageData::loadData() {
    try {
        if (!metaData.dataLoaded) {
            metaData.loadData(getImagePath());
            orientation = metaData.getImageOrientation();
            date = metaData.getTimestamp();

            metaData.dataLoaded = true;
        }
    } catch (const Exiv2::Error& e) {
        qWarning() << "Error loading metadata for image: " << getImagePath();
        qWarning() << "Exiv2 error: " << e.what();
    }
}

void ImageData::saveMetaData() {
    try {
        metaData.saveMetaData(getImagePath());
    } catch (const Exiv2::Error& e) {
        qWarning() << "saveMetaData : " << e.what();
    }
}

int ImageData::getImageWidth() {
    return metaData.getImageWidth();
}
int ImageData::getImageHeight() {
    return metaData.getImageHeight();
}
int ImageData::getImageOrientation() {
    return metaData.getImageOrientation();
}

void ImageData::turnImage(int rotation) {
    this->orientation = rotation;
    metaData.modifyExifValue("Exif.Image.Orientation", std::to_string(rotation));
    // metaData.modifyExifValue("Exif.Thumbnail.Orientation", std::to_string(rotation));
    // metaData.modifyXmpValue("Xmp.tiff.Orientation", std::to_string(rotation));
}

void ImageData::setOrCreateExifData() {
    metaData.setOrCreateExifData(getImagePath());
}

std::map<std::string, std::vector<std::pair<cv::Rect, float>>> ImageData::getDetectedObjects() {
    return detectedObjects.getDetectedObjects();
}

void ImageData::setDetectedObjects(const std::map<std::string, std::vector<std::pair<cv::Rect, float>>>& detectedObjects) {
    this->detectedObjects.setDetectedObjects(detectedObjects);
}
void ImageData::clearDetectedObjects() {
    detectedObjects.clear();
    setDetectionStatusNotLoaded();
}

void ImageData::save(std::ofstream& out) const {
    out.write(reinterpret_cast<const char*>(&orientation), sizeof(orientation));
    out.write(reinterpret_cast<const char*>(&date), sizeof(date));

    folders.save(out);

    size_t cropSizesSize = cropSizes.size();
    out.write(reinterpret_cast<const char*>(&cropSizesSize), sizeof(cropSizesSize));
    for (const auto& cropSize : cropSizes) {
        size_t innerSize = cropSize.size();
        out.write(reinterpret_cast<const char*>(&innerSize), sizeof(innerSize));
        out.write(reinterpret_cast<const char*>(cropSize.data()), innerSize * sizeof(QPoint));
    }

    // TODO save doesn't work
    // out.write(reinterpret_cast<const char*>(&detectionStatus), sizeof(detectionStatus));

    detectedObjects.save(out);
}

void ImageData::load(std::ifstream& in) {
    in.read(reinterpret_cast<char*>(&orientation), sizeof(orientation));
    in.read(reinterpret_cast<char*>(&date), sizeof(date));

    folders.load(in);

    size_t cropSizesSize;
    in.read(reinterpret_cast<char*>(&cropSizesSize), sizeof(cropSizesSize));
    if (cropSizesSize >= 1) {
        cropSizes.resize(cropSizesSize);
        for (size_t i = 0; i < cropSizesSize; ++i) {
            size_t innerSize;
            in.read(reinterpret_cast<char*>(&innerSize), sizeof(innerSize));
            cropSizes[i].resize(innerSize);
            in.read(reinterpret_cast<char*>(cropSizes[i].data()), innerSize * sizeof(QPoint));
        }
    }
    // TODO save doesn't work
    // in.read(reinterpret_cast<char*>(&detectionStatus), sizeof(detectionStatus));
    // // TODO mettre autre part
    // if (isDetectionStatusLoading()) {
    //     setDetectionStatusNotLoaded();
    // }

    detectedObjects.load(in);
}

std::vector<std::vector<QPoint>> ImageData::getCropSizes() const {
    return cropSizes;
}

void ImageData::setCropSizes(const std::vector<std::vector<QPoint>>& cropSizes) {
    this->cropSizes = cropSizes;
}

void ImageData::clearMetaData() {
    metaData.clear();
    metaData.dataLoaded = false;
}

// DetectionStatus ImageData::getDetectionStatus() const {
//     return detectionStatus;
// }

void ImageData::setDetectionStatus(DetectionStatus detectionStatus) {
    this->detectionStatus = detectionStatus;
}

void ImageData::setDetectionStatusLoading() {
    setDetectionStatus(DetectionStatus::Loading);
}

void ImageData::setDetectionStatusNotLoaded() {
    setDetectionStatus(DetectionStatus::NotLoaded);
}

void ImageData::setDetectionStatusLoaded() {
    setDetectionStatus(DetectionStatus::Loaded);
}

bool ImageData::isDetectionStatusLoading() {
    return detectionStatus == DetectionStatus::Loading;
}

bool ImageData::isDetectionStatusNotLoaded() {
    return detectionStatus == ImageData::DetectionStatus::NotLoaded;
}

bool ImageData::isDetectionStatusLoaded() {
    return detectionStatus == ImageData::DetectionStatus::Loaded;
}

void ImageData::setMetaData(const MetaData& metaData) {
    this->metaData = metaData;
}

void ImageData::setDate(long date) {
    if (date < 0) {
        qWarning() << "Date invalide : " << date;
        date = 0;
    }
    this->date = date;
}

long ImageData::getDate() const {
    return date;
}

void ImageData::setOrientation(int orientation) {
    if (orientation < 0 || orientation > 8) {
        qWarning() << "Orientation invalide : " << orientation;
        orientation = 1;
    }
    this->orientation = orientation;
}

int ImageData::getOrientation() const {
    return orientation;
}