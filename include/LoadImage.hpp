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

bool addSelectedFilesToFolders(std::shared_ptr<Data> data, QWidget* parent, QProgressDialog& progressDialog);

bool startLoadingImagesFromFolder(QWidget* parent, std::shared_ptr<Data> data, const std::string imagePaths, QProgressDialog& progressDialog);
bool addFilesToTree(Folders* currentFolder, ImagesData* imagesData, const std::string& path, int& nbrImage, QProgressDialog& progressDialog);
bool addSubfolders(Folders& rootFolder, ImagesData* imagesData, const std::string& path, int& nbrImage, QProgressDialog& progressDialog);
bool loadImagesThumbnail(std::shared_ptr<Data> data, QProgressDialog& progressDialog);

void addImagesFromFolder(std::shared_ptr<Data> data, QWidget* parent);
