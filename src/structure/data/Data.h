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
// #include "../../functions/thumbnail/Thumbnail.h"

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
    ImagesData* imagesData;
    ImagesData* deletedImagesData;
    std::map<std::string, QImageAndPath>* imageCache = nullptr;

    void preDeleteImage(int imageNbr);
    void unPreDeleteImage(int imageNbr);


    void revocerDeletedImage(int imageNbr);
    void revocerDeletedImage(ImageData& imageData);

    void removeDeletedImages();
    bool isDeleted(int imageNbr);

    QImage loadImage(QWidget* parent, std::string imagePath, QSize size, bool setSize, int thumbnail = 0);
    bool loadInCache(std::string imagePath, bool setSize = false, QSize size = QSize(0, 0));
    bool unloadFromCache(std::string imagePath);


    bool isInCache(std::string imagePath);
    bool getLoadedImage(std::string imagePath, QImage& image);

    void rotateImageCache(std::string imagePath, int rotation);

    void createThumbnails(const std::vector<std::string>& imagePaths, const int maxDim);
    void createThumbnail(const std::string& imagePath, const int maxDim);


    void createThumbnailsIfNotExists(const std::vector<std::string>& imagePaths, const int maxDim);

    void createThumbnailIfNotExists(const std::string& imagePath, const int maxDim);

    bool hasThumbnail(const std::string& imagePath, const int maxDim);

    void createAllThumbnail(const std::string& path, const int maxDim);

    std::string getThumbnailPath(const std::string& imagePath, const int size);

};

