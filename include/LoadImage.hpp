#pragma once
#include <QProgressDialog>
#include <QWidget>
#include <string>
#include <vector>

#include "Data.hpp"
#include "ImagesData.hpp"

class ThumbnailTask : public QObject, public QRunnable {
    Q_OBJECT

   public:
    ThumbnailTask(Data* data, int start, int end)
        : data(data), start(start), end(end) {
    }

    void run() override;

   private:
    Data* data;
    int start;
    int end;
};

bool addSelectedFilesToFolders(Data* data, QWidget* parent, QProgressDialog& progressDialog);
std::string getDirectoryFromUser(QWidget* parent);

bool startLoadingImagesFromFolder(QWidget* parent, Data* data, const std::string imagePaths, QProgressDialog& progressDialog);
bool loadImagesMetaDataOfGoogle(ImagesData* imagesData, QProgressDialog& progressDialog);
bool addFilesToTree(Folders* currentFolder, ImagesData* imagesData, const std::string& path, int& nbrImage, QProgressDialog& progressDialog);
bool addSubfolders(Folders& rootFolder, ImagesData* imagesData, const std::string& path, int& nbrImage, QProgressDialog& progressDialog);
bool loadImagesThumbnail(Data* data, QProgressDialog& progressDialog);

std::map<std::string, std::string> openJsonFile(std::string filePath);
std::string readFile(const std::string& filePath);
std::map<std::string, std::string> parseJsonToMap(const std::string& jsonString);

void addImagesFromFolder(Data* data, QWidget* parent);

