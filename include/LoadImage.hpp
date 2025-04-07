#pragma once
#include <QObject>
#include <QRunnable>
#include <string>
#include <vector>

#include "Folders.hpp"
// Forward
class QProgressDialog;
class QWidget;
class Data;
class ImagesData;

bool addSelectedFilesToFolders(Data* data, QWidget* parent, QProgressDialog& progressDialog);
std::string getDirectoryFromUser(QWidget* parent);

bool startLoadingImagesFromFolder(QWidget* parent, Data* data, const std::string imagePaths, QProgressDialog& progressDialog);
bool addFilesToTree(Folders* currentFolder, ImagesData* imagesData, const std::string& path, int& nbrImage, QProgressDialog& progressDialog);
bool addSubfolders(Folders& rootFolder, ImagesData* imagesData, const std::string& path, int& nbrImage, QProgressDialog& progressDialog);
bool loadImagesThumbnail(Data* data, QProgressDialog& progressDialog);

std::map<std::string, std::string> openJsonFile(std::string filePath);
std::string readFile(const std::string& filePath);
std::map<std::string, std::string> parseJsonToMap(const std::string& jsonString);

void addImagesFromFolder(Data* data, QWidget* parent);

