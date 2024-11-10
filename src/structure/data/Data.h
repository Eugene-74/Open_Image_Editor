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
#include <QFileDialog>


class Data
{
public:
    ImagesData* imagesData;
    ImagesData* deletedImagesData;
    std::map<std::string, QImage>* imageCache = nullptr;

    void preDeleteImage(int imageNbr);
    void unPreDeleteImage(int imageNbr);


    void revocerDeletedImage(int imageNbr);
    void revocerDeletedImage(ImageData& imageData);

    void removeDeletedImages();
    bool isDeleted(int imageNbr);

    QImage loadImage(QWidget* parent, std::string imagePath, QSize size, bool setSize);
};