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

class ThumbnailTask : public QRunnable {
public:
    ThumbnailTask(Data* data, ImagesData* imagesData, int start, int end
        // , QProgressDialog* progressDialog
    )
        : data(data), imagesData(imagesData), start(start), end(end)
        // , progressDialog(progressDialog)
    {
    }

    void run() override;

private:
    Data* data;
    ImagesData* imagesData;
    int start;
    int end;
    // QProgressDialog* progressDialog;
};

void addSelectedFilesToFolders(Data* data, QWidget* parent, QProgressDialog& progressDialog);
std::string getDirectoryFromUser(QWidget* parent);


bool startLoadingImagesFromFolder(QWidget* parent, Data* data, const std::string imagePaths, QProgressDialog& progressDialog);
void countImagesFromFolder(const std::string path, int& nbrImage);
bool loadImagesFromFolder(const std::string initialPath, const std::string path, ImagesData* imagesData, int& nbrImage, QProgressDialog& progressDialog);
bool loadImagesMetaDataOfGoogle(ImagesData* imagesData, QProgressDialog& progressDialog);


std::string mapJsonKeyToExifKey(const std::string& jsonKey);


std::map<std::string, std::string> openJsonFile(std::string filePath);
std::string readFile(const std::string& filePath);
std::map<std::string, std::string> parseJsonToMap(const std::string& jsonString);

void addImagesFromFolder(Data* data, QWidget* parent);
