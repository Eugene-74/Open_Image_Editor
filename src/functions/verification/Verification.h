#pragma once

#include <iostream>
#include <vector>
#include <QApplication>
#include <filesystem>


bool isImage(const std::string& cheminFichier);
bool isTurnable(const std::string& cheminFichier);
bool isMirrorable(const std::string& path);
