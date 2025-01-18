#pragma once
#include "../../structure/imagesData/ImagesData.h"
#include "../../structure/data/Data.h"
#include <QWidget>
#include "../../functions/fileSelector/FileSelector.h"
#include "../box/Box.h"

#include "../../Main.h"
#include <QStringList>
#include <QProgressDialog>

#include <string>
#include <vector>

// ImagesData* addSelectedFilesToFolders(Data* data, QWidget* parent, QProgressDialog& progressDialog);
ImagesData addSelectedFilesToFolders(Data* data, QWidget* parent);
std::string getDirectoryFromUser(QWidget* parent);


void startLoadingImagesFromFolder(QWidget* parent, Data* data, const std::string imagePaths, ImagesData* imagesData, QProgressDialog& progressDialog);
void countImagesFromFolder(const std::string path, int& nbrImage);
bool loadImagesFromFolder(const std::string initialPath, const std::string path, ImagesData* imagesData, int& nbrImage, QProgressDialog& progressDialog);
bool loadImagesMetaData(ImagesData* imagesData, QProgressDialog& progressDialog);
bool loadImagesMetaDataOfGoogle(ImagesData* imagesData, QProgressDialog& progressDialog);


std::string mapJsonKeyToExifKey(const std::string& jsonKey);


std::map<std::string, std::string> openJsonFile(std::string filePath);
std::string readFile(const std::string& filePath);
std::map<std::string, std::string> parseJsonToMap(const std::string& jsonString);
