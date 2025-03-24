#pragma once
#include <string>

auto isImage(const std::string& path) -> bool;
auto isVideo(const std::string& path) -> bool;
auto isMedia(const std::string& path) -> bool;

auto isTurnable(const std::string& cheminFichier) -> bool;
auto isMirrorable(const std::string& path) -> bool;

auto isExif(const std::string& extension) -> bool;
auto isReal(const std::string& extension) -> bool;

auto isExifPath(const std::string& imagePath) -> bool;
auto isRealPath(const std::string& imagePath) -> bool;

auto isHeicOrHeif(const std::string& path) -> bool;
auto isRaw(const std::string& path) -> bool;
