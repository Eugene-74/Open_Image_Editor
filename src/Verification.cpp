#include "Verification.hpp"

#include <algorithm>
#include <filesystem>
#include <vector>

#include "Const.hpp"

namespace fs = std::filesystem;

/**
 * @brief Check if the file is an image
 * @param path The path to the file
 * @return true if the file is an image, false otherwise
 * @details This function checks the file extension against a list of known image extensions.
 */
bool isImage(const std::string& path) {
    if (fs::path(path).filename().string().front() == '.') {
        return false;
    }
    std::vector<std::string> extensionsImages = Const::Extension::IMAGE;
    std::string extension = fs::path(path).extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    return std::find(extensionsImages.begin(), extensionsImages.end(), extension) != extensionsImages.end();
}

/**
 * @brief Check if the file is a video
 * @param path The path to the file
 * @return true if the file is a video, false otherwise
 * @details This function checks the file extension against a list of known video extensions.
 */
bool isVideo(const std::string& path) {
    if (fs::path(path).filename().string().front() == '.') {
        return false;
    }
    std::vector<std::string> extensionsImages = Const::Extension::VIDEO;
    std::string extension = fs::path(path).extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    return std::find(extensionsImages.begin(), extensionsImages.end(), extension) != extensionsImages.end();
}

/**
 * @brief Check if the file is a media file (image or video)
 * @param path The path to the file
 * @return true if the file is a media file, false otherwise
 * @details This function checks the file extension against a list of known media extensions (image and video).
 */
bool isMedia(const std::string& path) {
    return isImage(path) || isVideo(path);
}

/**
 * @brief Check if the file is turnable (image)
 * @param path The path to the file
 * @return true if the file is turnable, false otherwise
 * @details This function checks the file extension against a list of known turnable image extensions.
 */
bool isTurnable(const std::string& path) {
    std::vector<std::string> extensionsImages = Const::Extension::TURNABLE_IMAGE;
    std::string extension = fs::path(path).extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    return std::find(extensionsImages.begin(), extensionsImages.end(), extension) != extensionsImages.end();
}

/**
 * @brief Check if the file is mirrorable (image)
 * @param path The path to the file
 * @return true if the file is mirrorable, false otherwise
 * @details This function checks the file extension against a list of known mirrorable image extensions.
 */
bool isMirrorable(const std::string& path) {
    std::vector<std::string> extensionsImages = Const::Extension::MIRRORABLE_IMAGE;
    std::string extension = fs::path(path).extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    return std::find(extensionsImages.begin(), extensionsImages.end(), extension) != extensionsImages.end();
}

/**
 * @brief Check if the file is an EXIF image
 * @param extension The file extension
 * @return true if the file is an EXIF image, false otherwise
 * @details This function checks the file extension against a list of known EXIF image extensions.
 */
bool isExif(const std::string& extension) {
    std::string ext = extension;
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return std::find(Const::Extension::METADA_ACTION_IMAGE.begin(), Const::Extension::METADA_ACTION_IMAGE.end(), ext) != Const::Extension::METADA_ACTION_IMAGE.end();
}

/**
 * @brief Check if the file is a real image (doesn't support exif)
 * @param extension The file extension
 * @return true if the file is a real image (doesn't support exif), false otherwise
 * @details This function checks the file extension against a list of known real image (doesn't support exif) extensions.
 */
bool isReal(const std::string& extension) {
    std::string ext = extension;
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return std::find(Const::Extension::REAL_ACTION_IMAGE.begin(), Const::Extension::REAL_ACTION_IMAGE.end(), ext) != Const::Extension::REAL_ACTION_IMAGE.end();
}

/**
 * @brief Check if the file is a real image (doesn't support exif)
 * @param imagePath The path to the image file
 * @return true if the file is a real image (doesn't support exif), false otherwise
 * @details This function checks the file extension against a list of known real image (doesn't support exif) extensions.
 */
bool isRealPath(const std::string imagePath) {
    std::string extension = fs::path(imagePath).extension().string();
    return isReal(extension);
}

/**
 * @brief Check if the file is an EXIF image
 * @param imagePath The path to the image file
 * @return true if the file is an EXIF image, false otherwise
 * @details This function checks the file extension against a list of known EXIF image extensions.
 */
bool isExifPath(const std::string& imagePath) {
    std::string extension = fs::path(imagePath).extension().string();
    return isExif(extension);
}

/**
 * @brief Check if the file is a HEIC or HEIF image
 * @param path The path to the file
 * @return true if the file is a HEIC or HEIF image, false otherwise
 */
bool isHeicOrHeif(const std::string& path) {
    std::vector<std::string> extensionsImages = {".heic", ".heif"};
    std::string extension = fs::path(path).extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    return std::find(extensionsImages.begin(), extensionsImages.end(), extension) != extensionsImages.end();
}

/**
 * @brief Check if the file is a RAW image
 * @param path The path to the file
 * @return true if the file is a RAW image, false otherwise
 * @details This function checks the file extension against a list of known RAW image extensions.
 */
bool isRaw(const std::string& path) {
    std::vector<std::string> rawExtensions = {".cr2", ".nef", ".arw", ".dng", ".rw2", ".orf", ".raf", ".srw"};
    std::string extension = fs::path(path).extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    return std::find(rawExtensions.begin(), rawExtensions.end(), extension) != rawExtensions.end();
}