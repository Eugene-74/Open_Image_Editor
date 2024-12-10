#pragma once

#include "../imagesData/ImagesData.h"
#include <opencv2/opencv.hpp>
#include <QImage>
#include <QFile>
#include <QSize>
#include <QResource>

#include <iostream>
#include <fstream> 
#include "../../display/box/Box.h"
#include "../../structure/folders/Folders.h"
#include "../../structure/sizes/Sizes.h"




#include <QFileDialog>


class QImageAndPath
{
public:
    QImage image;
    std::string imagePath;
};

class Data
{
public:
    ImagesData imagesData;
    ImagesData deletedImagesData;
    Folders rootFolders = Folders("/");
    Sizes sizes;
    std::map<std::string, Option> options = DEFAULT_OPTIONS;



    std::map<std::string, QImageAndPath>* imageCache = nullptr;



    void preDeleteImage(int imageNbr);
    void unPreDeleteImage(int imageNbr);


    void revocerDeletedImage(int imageNbr);
    void revocerDeletedImage(ImageData& imageData);

    void removeDeletedImages();
    bool isDeleted(int imageNbr);

    QImage loadImage(QWidget* parent, std::string imagePath, QSize size, bool setSize, int thumbnail = 0, bool rotation = true, bool square = false);
    QImage loadImageNormal(QWidget* parent, std::string imagePath, QSize size, bool setSize, int thumbnail = 0);
    QImage loadImageSquare(QWidget* parent, std::string imagePath, QSize size, bool setSize, int thumbnail = 0);

    bool loadInCache(std::string imagePath, bool setSize = false, QSize size = QSize(0, 0), bool force = false);
    bool unloadFromCache(std::string imagePath);


    bool isInCache(std::string imagePath);
    bool getLoadedImage(std::string imagePath, QImage& image);

    void rotateImageCache(std::string imagePath, int rotation);
    void mirrorImageCache(std::string imagePath, int upDown);


    void createThumbnails(const std::vector<std::string>& imagePaths, const int maxDim);
    void createThumbnail(const std::string& imagePath, const int maxDim);


    void createThumbnailsIfNotExists(const std::vector<std::string>& imagePaths, const int maxDim);

    void createThumbnailIfNotExists(const std::string& imagePath, const int maxDim);

    bool hasThumbnail(const std::string& imagePath, const int maxDim);

    void createAllThumbnail(const std::string& path, const int maxDim);

    std::string getThumbnailPath(const std::string& imagePath, const int size);

    void exportImages(std::string exportPath, bool dateInName);
    void copyImages(Folders* currentFolders, std::string path, bool dateInName) const;
    // void copyTo(std::string path);
    void copyTo(std::string filePath, std::string destinationPath, bool dateInName)const;

    QImage rotateQImage(QImage image, std::string imagePath);

    void saveData();
    void loadData();

};


