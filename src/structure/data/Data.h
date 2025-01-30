#pragma once

#include "../imagesData/ImagesData.h"
#include <opencv2/opencv.hpp>
#include <QImage>
#include <QFile>
#include <QSize>
#include <QResource>
#include <QtConcurrent/QtConcurrent>
#include <QProgressDialog>


#include <iostream>
#include <fstream>
#include "../../display/box/Box.h"
#include "../../display/conversion/Conversion.h"

#include "../../structure/folders/Folders.h"
#include "../../structure/sizes/Sizes.h"
#include "../../structure/threadPool/ThreadPool.h"


#include <QFileDialog>

class QImageAndPath {
public:

    QImage image;
    std::string imagePath;
};

class Actions {
public:
    Actions() = default;
    std::function<void()> unDo;
    std::function<void()> reDo;

};


class Data {
public:
    ImagesData imagesData;
    ImagesData deletedImagesData;


    // Folders rootFolders = Folders("/");

#ifdef _WIN32
    Folders rootFolders = Folders("");
#else
    Folders rootFolders = Folders("/");
#endif

    Sizes sizes;
    std::map<std::string, Option> options = DEFAULT_OPTIONS;
    std::map<std::string, QImageAndPath>* imageCache = nullptr;
    bool saved = true;

    unsigned int numThreads = std::max(1u, std::thread::hardware_concurrency() / 2); // Reserve 2 threads for UI
    ThreadPool threadPool = ThreadPool(numThreads);
    std::map<QString, std::future<void>> futures;


    std::vector<int> imagesSelected;



    Data() : imageCache(new std::map<std::string, QImageAndPath>()) {}


    void preDeleteImage(int imageNbr);
    void unPreDeleteImage(int imageNbr);

    void revocerDeletedImage(int imageNbr);
    void revocerDeletedImage(ImageData& imageData);

    void removeDeletedImages();

    QImage loadImage(QWidget* parent, std::string imagePath, QSize size, bool setSize, int thumbnail = 0, bool rotation = true, bool square = false, bool crop = true, bool force = false);
    QImage loadImageNormal(QWidget* parent, std::string imagePath, QSize size, bool setSize, int thumbnail = 0, bool force = false);

    bool loadInCache(std::string imagePath, bool setSize = false, QSize size = QSize(0, 0), bool force = false);
    void loadInCacheAsync(std::string imagePath, std::function<void()> callback, bool setSize = false, QSize size = QSize(0, 0), bool force = false);

    bool unloadFromCache(std::string imagePath);
    bool unloadFromFutures(std::string imagePath);

    bool isInCache(std::string imagePath);
    bool getLoadedImage(std::string imagePath, QImage& image);

    void createThumbnails(const std::vector<std::string>& imagePaths, const int maxDim);
    void createThumbnail(const std::string& imagePath, const int maxDim);

    void createThumbnailIfNotExists(const std::string& imagePath, const int maxDim);
    void createThumbnailsIfNotExists(const std::vector<std::string>& imagePaths, const int maxDim);

    void createAllThumbnail(const std::string& imagePath, const int maxDim);
    void createAllThumbnailIfNotExists(const std::string& imagePath, const int maxDim);

    bool hasThumbnail(const std::string& imagePath, const int maxDim);

    std::string getThumbnailPath(const std::string& imagePath, const int size);

    void exportImages(std::string exportPath, bool dateInName);

    void saveData();
    void loadData();

    void cancelTasks();
    bool isDeleted(int imageNbr);

    void addAction(std::function<void()> unDo, std::function<void()> reDo);
    void addActionDone(Actions action);

    void unDoAction();
    void reDoAction();

    void clearActions();

    void sortImagesData(QProgressDialog& progressDialog);

private:
    std::vector<Actions> lastActions = {};
    std::vector<Actions> lastActionsDone = {};


    void loadImageTask(std::string imagePath, bool setSize, QSize size, bool force, std::function<void()> callback);
    QImage rotateQImage(QImage image, std::string imagePath);

    Folders* findFirstFolderWithAllImages(const ImagesData& imagesData, const Folders& currentFolder) const;
    void createFolders(Folders* currentFolders, std::string path);
    void copyTo(Folders rootFolders, std::string destinationPath, bool dateInName);

};




