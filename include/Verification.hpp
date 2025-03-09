#pragma once

#include <QApplication>
#include <filesystem>
#include <iostream>
#include <vector>

#include "Const.hpp"

bool isImage(const std::string& cheminFichier);
bool isTurnable(const std::string& cheminFichier);
bool isMirrorable(const std::string& path);

bool isExif(const std::string& extension);
bool isReal(const std::string& extension);

bool isExifPath(const std::string& imagePath);
bool isRealPath(const std::string& imagePath);

bool isHeicOrHeif(const std::string& path);
bool isRaw(const std::string& path);
