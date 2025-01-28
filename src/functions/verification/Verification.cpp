#include "Verification.h"
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
    std::vector<std::string> extensionsImages = MIRORABLE_IMAGE_EXTENSIONS;
    std::string extension = fs::path(path).extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    return std::find(extensionsImages.begin(), extensionsImages.end(), extension) != extensionsImages.end();
}

bool exifTurnOrMiror(std::string extension){
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    if (std::find(METADA_ACTION_IMAGE_EXTENSIONS.begin(), METADA_ACTION_IMAGE_EXTENSIONS.end(), extension) != METADA_ACTION_IMAGE_EXTENSIONS.end()) {
        return true;
    }
    return false;
}

bool realTurnOrMiror(std::string extension){
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    if (std::find(REAL_ACTION_IMAGE_EXTENSIONS.begin(), REAL_ACTION_IMAGE_EXTENSIONS.end(), extension) != REAL_ACTION_IMAGE_EXTENSIONS.end()) {
        return true;
    }
    return false;
}

bool HEICOrHEIF(const std::string& path) {
    std::vector<std::string> extensionsImages = { ".heic", ".heif" };
    std::string extension = fs::path(path).extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    return std::find(extensionsImages.begin(), extensionsImages.end(), extension) != extensionsImages.end();
}

bool isRAW(const std::string& path) {
    std::vector<std::string> rawExtensions = { ".cr2", ".nef", ".arw", ".dng", ".rw2", ".orf", ".raf", ".srw" };
    std::string extension = fs::path(path).extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    return std::find(rawExtensions.begin(), rawExtensions.end(), extension) != rawExtensions.end();
}