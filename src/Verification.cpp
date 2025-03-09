#include "Verification.hpp"

namespace fs = std::filesystem;

bool isImage(const std::string& path) {
    if (fs::path(path).filename().string().front() == '.') {
        return false;
    }
    std::vector<std::string> extensionsImages = IMAGE_EXTENSIONS;
    std::string extension = fs::path(path).extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    return std::find(extensionsImages.begin(), extensionsImages.end(), extension) != extensionsImages.end();
}

bool isTurnable(const std::string& path) {
    std::vector<std::string> extensionsImages = TURNABLE_IMAGE_EXTENSIONS;
    std::string extension = fs::path(path).extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    return std::find(extensionsImages.begin(), extensionsImages.end(), extension) != extensionsImages.end();
}

bool isMirrorable(const std::string& path) {
    std::vector<std::string> extensionsImages = MIRRORABLE_IMAGE_EXTENSIONS;
    std::string extension = fs::path(path).extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    return std::find(extensionsImages.begin(), extensionsImages.end(), extension) != extensionsImages.end();
}

bool isExif(const std::string& extension) {
    std::string ext = extension;
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return std::find(METADA_ACTION_IMAGE_EXTENSIONS.begin(), METADA_ACTION_IMAGE_EXTENSIONS.end(), ext) != METADA_ACTION_IMAGE_EXTENSIONS.end();
}

bool isReal(const std::string& extension) {
    std::string ext = extension;
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return std::find(REAL_ACTION_IMAGE_EXTENSIONS.begin(), REAL_ACTION_IMAGE_EXTENSIONS.end(), ext) != REAL_ACTION_IMAGE_EXTENSIONS.end();
}

bool isRealPath(const std::string imagePath) {
    std::string extension = fs::path(imagePath).extension().string();
    return isReal(extension);
}

bool isExifPath(const std::string& imagePath) {
    std::string extension = fs::path(imagePath).extension().string();
    return isExif(extension);
}

bool isHeicOrHeif(const std::string& path) {
    std::vector<std::string> extensionsImages = {".heic", ".heif"};
    std::string extension = fs::path(path).extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    return std::find(extensionsImages.begin(), extensionsImages.end(), extension) != extensionsImages.end();
}

bool isRaw(const std::string& path) {
    std::vector<std::string> rawExtensions = {".cr2", ".nef", ".arw", ".dng", ".rw2", ".orf", ".raf", ".srw"};
    std::string extension = fs::path(path).extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    return std::find(rawExtensions.begin(), rawExtensions.end(), extension) != rawExtensions.end();
}