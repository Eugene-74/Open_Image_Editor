#pragma once
#include <QProgressDialog>
#include <QStringList>
#include <QWidget>
#include <string>
#include <vector>

#include "Box.hpp"
#include "Data.hpp"
#include "FileSelector.hpp"
#include "ImagesData.hpp"
#include "Main.hpp"

class ThumbnailTask : public QObject, public QRunnable {
    Q_OBJECT

   public:
    ThumbnailTask(Data* data, ImagesData* imagesData, int start, int end)
        : data(data), imagesData(imagesData), start(start), end(end) {
    }

    void run() override;

   private:
    Data* data;
    ImagesData* imagesData;
    int start;
    int end;
};

bool addSelectedFilesToFolders(Data* data, QWidget* parent, QProgressDialog& progressDialog);
std::string getDirectoryFromUser(QWidget* parent);

bool startLoadingImagesFromFolder(QWidget* parent, Data* data, const std::string imagePaths, QProgressDialog& progressDialog);
bool loadImagesFromFolder(const std::string initialPath, const std::string path, ImagesData* imagesData, int& nbrImage, QProgressDialog& progressDialog);
bool loadImagesMetaDataOfGoogle(ImagesData* imagesData, QProgressDialog& progressDialog);

std::string mapJsonKeyToExifKey(const std::string& jsonKey);

std::map<std::string, std::string> openJsonFile(std::string filePath);
std::string readFile(const std::string& filePath);
std::map<std::string, std::string> parseJsonToMap(const std::string& jsonString);

void addImagesFromFolder(Data* data, QWidget* parent);
