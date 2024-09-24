#ifndef IMAGESDATA_H
#define IMAGESDATA_H

#include <iostream>
#include <vector>
#include "../imageData/imageData.h"



class ImagesData
{
public:
    std::vector<ImageData> imagesData;

    ImagesData(const std::vector<ImageData> a) : imagesData(a) {}

    void print() const;

    void addImage(ImageData& imageD);

    void removeImage(const ImageData& image);

    ImageData* getImageData(int id);

    std::vector<ImageData>  get();
};


#endif
