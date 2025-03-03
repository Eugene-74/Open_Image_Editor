#pragma once

#include <QApplication>
#include <filesystem>
#include <iostream>
#include <vector>

#include "Const.hpp"

bool isImage(const std::string& cheminFichier);
bool isTurnable(const std::string& cheminFichier);
bool isMirrorable(const std::string& path);

bool isExifTurnOrMirror(const std::string& extension);
bool isRealTurnOrMirror(const std::string& extension);

bool isHeicOrHeif(const std::string& path);
bool isRaw(const std::string& path);
