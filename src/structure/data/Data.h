#pragma once

#include "../imagesData/ImagesData.h"


class Data
{
public:
    ImagesData imagesData;
    ImagesData deletedImagesData;

    void preDeleteImage(int imageNbr);
    void unPreDeleteImage(int imageNbr);


    void revocerDeletedImage(int imageNbr);
    void revocerDeletedImage(ImageData& imageData);

    void removeDeletedImages();
    bool isDeleted(int imageNbr);
};