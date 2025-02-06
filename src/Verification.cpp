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
    std::vector<std::string> extensionsImages = MIRORABLE_IMAGE_EXTENSIONS;
    std::string extension = fs::path(path).extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    return std::find(extensionsImages.begin(), extensionsImages.end(), extension) != extensionsImages.end();
}

bool isExifTurnOrMiror(std::string extension) {
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    if (std::find(METADA_ACTION_IMAGE_EXTENSIONS.begin(), METADA_ACTION_IMAGE_EXTENSIONS.end(), extension) != METADA_ACTION_IMAGE_EXTENSIONS.end()) {
        return true;
    }
    return false;
}

bool isRealTurnOrMiror(std::string extension) {
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    if (std::find(REAL_ACTION_IMAGE_EXTENSIONS.begin(), REAL_ACTION_IMAGE_EXTENSIONS.end(), extension) != REAL_ACTION_IMAGE_EXTENSIONS.end()) {
        return true;
    }
    return false;
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