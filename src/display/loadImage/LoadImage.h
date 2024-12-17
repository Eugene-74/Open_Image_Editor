#pragma once
#include "../../structure/imagesData/ImagesData.h"
#include "../../structure/data/Data.h"
#include <QWidget>
#include "../../functions/fileSelector/FileSelector.h"
#include "../box/Box.h"

#include "../../Main.h"
#include <QStringList>
#include <string>
#include <vector>

ImagesData addSelectedFilesToFolders(Data* data, QWidget* parent);

std::string getDirectoryFromUser(QWidget* parent);

void startLoadingImagesFromFolder(Data* data, const std::string imagePaths, ImagesData* imagesData);
void countImagesFromFolder(const std::string path, int& nbrImage);
void loadImagesFromFolder(const std::string initialPath, const std::string path, ImagesData* imagesData, int& nbrImage);
void loadImagesMetaData(ImagesData* imagesData);

std::map<std::string, std::string> openJsonFile(std::string filePath);
std::string readFile(const std::string& filePath);
std::map<std::string, std::string> parseJsonToMap(const std::string& jsonString);
