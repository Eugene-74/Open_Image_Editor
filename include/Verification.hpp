#pragma once
#include <string>

bool isImage(const std::string& cheminFichier);
bool isVideo(const std::string& path);

bool isTurnable(const std::string& cheminFichier);
bool isMirrorable(const std::string& path);

bool isExif(const std::string& extension);
bool isReal(const std::string& extension);

bool isExifPath(const std::string& imagePath);
bool isRealPath(const std::string& imagePath);

bool isHeicOrHeif(const std::string& path);
bool isRaw(const std::string& path);
