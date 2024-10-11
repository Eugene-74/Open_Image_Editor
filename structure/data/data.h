#pragma once

#include "../imagesData/imagesData.h"


class Data
{
public:
    ImagesData imagesData;
    ImagesData deletedImagesData;

    void preDeleteImage(int imageNbr);

    void revocerDeletedImage(int imageNbr);
    void removeDeletedImages();

};