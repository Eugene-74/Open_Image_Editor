#include "ImageData.hpp"

#include <QDebug>
#include <filesystem>

#include "Const.hpp"
#include "GPS_Conversion.hpp"
#include "Verification.hpp"

namespace fs = std::filesystem;

/**
 * @brief Default constructor for the ImageData class
 * @details Initializes the folders, cropSizes, orientation, and date members.
 */
ImageData::ImageData()
    : folders(Folders()), cropSizes(), orientation(), date() {
    orientation = Const::Orientation::UNDEFINED;
}

/**
 * @brief Constructor for the ImageData class with a Folders object
 * @param folders Folders object to initialize the ImageData object
 */
ImageData::ImageData(const Folders folders)
    : folders(folders) {
    orientation = Const::Orientation::UNDEFINED;
}

/**
 * @brief Constructor for the ImageData class with a string path
 * @param imagePath Path to the image
 */
ImageData::ImageData(std::string imagePath)
    : folders(Folders(imagePath)) {
    orientation = Const::Orientation::UNDEFINED;
}

/**
 * @brief Copy constructor for the ImageData class
 * @param other The ImageData object to copy from
 * @details Initializes the folders, metaData, cropSizes, orientation, date, and detectionStatus members.
 */
ImageData::ImageData(const ImageData& other)
    : folders(other.folders), metaData(other.metaData), cropSizes(other.cropSizes), orientation(other.orientation), date(other.date), detectionStatus(other.detectionStatus) {}

/**
 * @brief Assignment operator for the ImageData class
 * @param other The ImageData object to assign from
 * @return Reference to the current object
 * @details Copies the folders, metaData, cropSizes, orientation, date, detectedObjects, and detectionStatus members from the other object.
 */
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

/**
 * @brief Equality operator for the ImageData class
 * @param other The ImageData object to compare with
 * @return True if the folders names are equal, false otherwise
 * @details Compares the names of the folders in both ImageData objects.
 */
bool ImageData::operator==(const ImageData& other) const {
    return this->folders.getName() == other.folders.getName();
}

/**
 * @brief Print the image data to the console
 */
void ImageData::print() const {
    qInfo() << get();
}

/**
 * @brief Check if the image data respects the given filters
 * @param filters A map of filters to check against
 * @return True if the image data respects the filters, false otherwise
 */
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

/**
 * @brief Get a string representation of the image data
 * @return A string containing the image path and folder names
 * @details This function is used for debugging purposes.
 */
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

/**
 * @brief Get a pointer to the MetaData object
 * @return A pointer to the MetaData object
 */
MetaData* ImageData::getMetaDataPtr() {
    return &metaData;
}

/**
 * @brief Get the MetaData object
 * @return The MetaData object
 */
MetaData ImageData::getMetaData() const {
    return metaData;
}

/**
 * @brief Get the folders associated with the imageData
 * @return A vector of Folders objects
 */
std::vector<Folders> ImageData::getFolders() {
    return *folders.getFolders();
}

/**
 * @brief Add a folder to the imageData
 * @param toAddFolder The folder to add
 */
void ImageData::addFolder(const std::string& toAddFolder) {
    folders.addFolder(toAddFolder);
}

/**
 * @brief Add multiple folders to the imageData
 * @param toAddFolders A vector of folders to add
 */
void ImageData::addFolders(const std::vector<std::string>& toAddFolders) {
    for (const auto& folder : toAddFolders) {
        folders.addFolder(folder);
    }
}

/**
 * @brief Get the name of the image
 * @return The name of the image
 */
std::string ImageData::getImageName() const {
    std::string imagePath = getImagePathConst();
    size_t lastSlash = imagePath.find_last_of("/\\");
    return imagePath.substr(lastSlash + 1);
}

/**
 * @brief Get the path of the image
 * @return
 */
std::string ImageData::getImagePath() {
    return folders.getName();
}

/**
 * @brief Get the path of the image (const version)
 * @return The path of the image
 */
std::string ImageData::getImagePathConst() const {
    return folders.getName();
}

/**
 * @brief Get the extension of the image
 * @return The extension of the image
 */
std::string ImageData::getImageExtension() {
    size_t dotPosition = getImagePath().find_last_of('.');
    if (dotPosition != std::string::npos) {
        return getImagePath().substr(dotPosition);
    }
    return "";
}

/**
 * @brief Set the exif metaData of the image
 * @param toAddMetaData The exif metaData
 */
void ImageData::setExifMetaData(const Exiv2::ExifData& toAddMetaData) {
    try {
        metaData.setExifData(toAddMetaData);
        saveMetaData();
    } catch (const Exiv2::Error& e) {
        qWarning() << "Exiv2 error: " << e.what();
    }
}

/**
 * @brief Load the image metadata
 */
void ImageData::loadData() {
    try {
        if (!metaData.getDataLoadedConst()) {
            metaData.loadData(getImagePath());

            setOrientation(metaData.getImageOrientation());
            setDate(metaData.getTimestamp());

            auto exifData = metaData.getExifData();
            if (metaData.getLatitude() != -1 &&
                metaData.getLongitude() != -1) {
                latitude = convertGpsCoordinateToDecimal(exifData["Exif.GPSInfo.GPSLatitude"].toString());
                longitude = convertGpsCoordinateToDecimal(exifData["Exif.GPSInfo.GPSLongitude"].toString());
            }

            metaData.setDataLoaded(true);
        }
    } catch (const Exiv2::Error& e) {
        qWarning() << "Error loading metadata for image: " << getImagePath();
        qWarning() << "Exiv2 error: " << e.what();
    }
}

/**
 * @brief Save the image metadata
 */
void ImageData::saveMetaData() {
    try {
        metaData.saveMetaData(getImagePath());
    } catch (const Exiv2::Error& e) {
        qWarning() << "saveMetaData : " << e.what();
    }
}

/**
 * @brief Get the image orientation (in metadata)
 * @return The image orientation (exif orientation(1-8))
 */
int ImageData::getImageOrientation() {
    return metaData.getImageOrientation();
}

/**
 * @brief Turn the image to the specified orientation
 * @param rotation The rotation value (1-8) to set the image orientation
 */
void ImageData::turnImage(int rotation) {
    this->orientation = rotation;
    metaData.modifyExifValue("Exif.Image.Orientation", std::to_string(rotation));
    saveMetaData();
}

/**
 * @brief Set or create the Exif data for the image
 */
void ImageData::setOrCreateExifData() {
    metaData.setOrCreateExifData(getImagePath());
}

std::map<std::string, std::vector<std::pair<cv::Rect, float>>> ImageData::getDetectedObjects() {
    return detectedObjects.getDetectedObjects();
}

/**
 * @brief Set the detected objects for the image
 * @param detectedObjects A map of detected objects with their bounding boxes and confidence scores
 */
void ImageData::setDetectedObjects(const std::map<std::string, std::vector<std::pair<cv::Rect, float>>>& detectedObjects) {
    this->detectedObjects.setDetectedObjects(detectedObjects);
}

/**
 * @brief Clear the detected objects for the image
 */
void ImageData::clearDetectedObjects() {
    detectedObjects.clear();
    setDetectionStatusNotLoaded();
}

/**
 * @brief Save the imageData to a file
 * @param out The output file stream to save the data to
 */
void ImageData::save(std::ofstream& out) const {
    out.write(reinterpret_cast<const char*>(&orientation), sizeof(orientation));
    out.write(reinterpret_cast<const char*>(&date), sizeof(date));
    out.write(reinterpret_cast<const char*>(&latitude), sizeof(latitude));
    out.write(reinterpret_cast<const char*>(&longitude), sizeof(longitude));

    folders.save(out);

    size_t cropSizesSize = cropSizes.size();
    out.write(reinterpret_cast<const char*>(&cropSizesSize), sizeof(cropSizesSize));
    for (const auto& cropSize : cropSizes) {
        size_t innerSize = cropSize.size();
        out.write(reinterpret_cast<const char*>(&innerSize), sizeof(innerSize));
        out.write(reinterpret_cast<const char*>(cropSize.data()), innerSize * sizeof(QPoint));
    }

    out.write(reinterpret_cast<const char*>(&detectionStatus), sizeof(detectionStatus));

    detectedObjects.save(out);
}

/**
 * @brief Load the imageData from a file
 * @param in The input file stream to load the data from
 */
void ImageData::load(std::ifstream& in) {
    in.read(reinterpret_cast<char*>(&orientation), sizeof(orientation));
    in.read(reinterpret_cast<char*>(&date), sizeof(date));
    in.read(reinterpret_cast<char*>(&latitude), sizeof(latitude));
    in.read(reinterpret_cast<char*>(&longitude), sizeof(longitude));

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

    in.read(reinterpret_cast<char*>(&detectionStatus), sizeof(detectionStatus));

    detectedObjects.load(in);
}

/**
 * @brief Get the crop sizes for the image
 * @return A vector of vectors of QPoint representing the crop sizes
 */
std::vector<std::vector<QPoint>> ImageData::getCropSizes() const {
    return cropSizes;
}

/**
 * @brief Set the crop sizes for the image
 * @param cropSizes A vector of vectors of QPoint representing the crop sizes
 */
void ImageData::setCropSizes(const std::vector<std::vector<QPoint>>& cropSizes) {
    this->cropSizes = cropSizes;
}

/**
 * @brief Clear the metadata of the image
 */
void ImageData::clearMetaData() {
    metaData.clear();
    metaData.setDataLoaded(false);
}

/**
 * @brief Set the detection status of the image
 * @param detectionStatus The detection status to set
 */
void ImageData::setDetectionStatus(DetectionStatus detectionStatus) {
    this->detectionStatus = detectionStatus;
}

/**
 * @brief Set the detection status to loading
 */
void ImageData::setDetectionStatusLoading() {
    setDetectionStatus(DetectionStatus::Loading);
}

/**
 * @brief Set the detection status to not loaded
 */
void ImageData::setDetectionStatusNotLoaded() {
    setDetectionStatus(DetectionStatus::NotLoaded);
}

/**
 * @brief Set the detection status to loaded
 */
void ImageData::setDetectionStatusLoaded() {
    setDetectionStatus(DetectionStatus::Loaded);
}

/**
 * @brief Check if the detection status is loading
 * @return True if the detection status is loading, false otherwise
 */
bool ImageData::isDetectionStatusLoading() {
    return detectionStatus == DetectionStatus::Loading;
}

/**
 * @brief Check if the detection status is not loaded
 * @return True if the detection status is not loaded, false otherwise
 */
bool ImageData::isDetectionStatusNotLoaded() {
    return detectionStatus == ImageData::DetectionStatus::NotLoaded;
}

/**
 * @brief Check if the detection status is loaded
 * @return True if the detection status is loaded, false otherwise
 */
bool ImageData::isDetectionStatusLoaded() {
    return detectionStatus == ImageData::DetectionStatus::Loaded;
}

/**
 * @brief Set the metaData for the image
 * @param metaData The metaData to set
 */
void ImageData::setMetaData(const MetaData& metaData) {
    this->metaData = metaData;
}

/**
 * @brief Set the date for the image
 * @param date The date to set
 */
void ImageData::setDate(long date) {
    if (date < 0) {
        qWarning() << "Date invalide : " << date;
        date = 0;
    }
    this->date = date;
}

/**
 * @brief Get the date for the image
 * @return The date of the image
 */
long ImageData::getDate() const {
    return date;
}

/**
 * @brief Set the orientation for the image
 * @param orientation The orientation to set (1-8)
 *
 */
void ImageData::setOrientation(int orientation) {
    if (orientation < 0 || orientation > 8) {
        qWarning() << "Orientation invalide : " << orientation;
        orientation = 1;
    }
    this->orientation = orientation;
}

/**
 * @brief Get the orientation for the image
 * @return The orientation of the image (1-8)
 */
int ImageData::getOrientation() const {
    return orientation;
}

/**
 * @brief Set the latitude for the image
 */
void ImageData::setLatitude(double latitude) {
    this->latitude = latitude;
    this->metaData.setLatitude(latitude);
    this->saveMetaData();
}

/**
 * @brief Get the latitude for the image
 */
double ImageData::getLatitude() const {
    return latitude;
}

/**
 * @brief Set the longitude for the image
 */
void ImageData::setLongitude(double longitude) {
    this->longitude = longitude;
    this->metaData.setLongitude(longitude);
    this->saveMetaData();
}

/**
 * @brief Get the longitude for the image
 */
double ImageData::getLongitude() const {
    return longitude;
}