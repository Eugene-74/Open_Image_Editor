#pragma once

#include <iostream>
#include <vector>
#include <QApplication>
#include <filesystem>

#include "../../Const.h"


bool isImage(const std::string& cheminFichier);
bool isTurnable(const std::string& cheminFichier);
bool isMirrorable(const std::string& path);

bool exifTurnOrMiror(std::string extension);
bool realTurnOrMiror(std::string extension);


bool HEICOrHEIF(const std::string& path);
bool isRAW(const std::string& path);
