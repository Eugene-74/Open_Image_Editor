#pragma once
#include <QObject>
#include <QRunnable>
#include <string>
#include <vector>

#include "Folders.hpp"
class QProgressDialog;
class QWidget;
class Data;
class ImagesData;

bool addSelectedFilesToFolders(Data* data, QWidget* parent, QProgressDialog& progressDialog);

bool startLoadingImagesFromFolder(QWidget* parent, Data* data, const std::string imagePaths, QProgressDialog& progressDialog);
bool addFilesToTree(Folders* currentFolder, ImagesData* imagesData, const std::string& path, int& nbrImage, QProgressDialog& progressDialog);
bool addSubfolders(Folders& rootFolder, ImagesData* imagesData, const std::string& path, int& nbrImage, QProgressDialog& progressDialog);
bool loadImagesThumbnail(Data* data, QProgressDialog& progressDialog);

void addImagesFromFolder(Data* data, QWidget* parent);

