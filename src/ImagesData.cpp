#include "ImagesData.hpp"

ImagesData& ImagesData::operator=(const ImagesData& other) {
    if (this != &other) {
        imagesData = other.imagesData;
        imageNumber = other.imageNumber;
    }
    return *this;
}

void ImagesData::setImageNumber(int nbr) {
    if (currentImagesData.size() == 0) {
        nbr = 0;
    } else {
        while (nbr < 0) {
            nbr += 1;
        }
        while (nbr >= currentImagesData.size()) {
            nbr -= 1;
        }
    }

    imageNumber = nbr;
}
int ImagesData::getImageNumber() const {
    return imageNumber;
}

int ImagesData::getImageNumberInTotal() {
    return getImageNumberInTotal(imageNumber);
}

int ImagesData::getImageNumberInTotal(int imageNbrInCurrent) {
    return getImageDataId(getImageDataInCurrent(imageNbrInCurrent)->getImagePathConst());
}

int ImagesData::getImageNumberInCurrent(int imageNbrInTotal) {
    int imageNbrInCurrent = 0;
    bool run = true;
    while (imageNbrInCurrent < currentImagesData.size() && run) {
        if (getImageDataInCurrent(imageNbrInCurrent)->getImagePath() == getImageData(imageNbrInTotal)->getImagePath()) {
            run = false;
        }
        imageNbrInCurrent++;
    }
    return imageNbrInCurrent;
}

void ImagesData::print() const {
    // qDebug() << "ImagesData : \n";
    // for (const ImageData valeur : imagesData) {
    //     qDebug() << valeur.get();
    // }
}

void ImagesData::addImage(ImageData& imageD) {
    // auto it = std::find_if(imagesData.begin(), imagesData.end(),
    //                        [&imageD](const ImageData& imgD) {
    //                            return imgD == imageD;
    //                        });

    // if (it != imagesData.end()) {
    // ImageData lastImageD = *it;
    // imagesData.erase(it);
    // imageD.addFolders(lastImageD.getFolders());
    // imageD.setCropSizes(lastImageD.getCropSizes());

    imagesData.push_back(&imageD);
    // } else {
    //     imagesData.push_back(imageD);
    //     for (const auto& crop : imageD.getCropSizes()) {
    //         for (const auto& point : crop) {
    //             qDebug() << point;
    //         }
    //     }
    // }
}

void ImagesData::removeImage(const ImageData& image) {
    auto foundImageData = std::find_if(imagesData.begin(), imagesData.end(),
                                       [&image](ImageData* imgPtr) {
                                           return *imgPtr == image;
                                       });

    if (foundImageData != imagesData.end()) {
        imagesData.erase(foundImageData);
    }
}

ImageData* ImagesData::getImageData(int id) {
    if (id < 0 || id >= imageMapInt.size()) {
        throw std::out_of_range("getImageData :: Index hors limites" + std::to_string(id));
    }
    auto foundImageData = imageMapInt.find(id);
    if (foundImageData != imageMapInt.end()) {
        return foundImageData->second;
    }
    return nullptr;
}

ImageData* ImagesData::getImageDataInCurrent(int id) {
    if (id < 0 || id >= currentImagesData.size()) {
        throw std::out_of_range("getImageData current :: Index hors limites : " + std::to_string(id));
    }
    return currentImagesData.at(id);
}

ImageData* ImagesData::getImageData(std::string imagePath) {
    auto foundImageData = imageMap.find(imagePath);

    if (foundImageData != imageMap.end()) {
        ImageData* imageData = foundImageData->second;
        return imageData;
    }
    qDebug() << "not foundImageData : " << imagePath.c_str();

    return nullptr;
}

ImageData* ImagesData::getCurrentImageData() {
    if (currentImagesData.size() <= 0 || imageNumber >= currentImagesData.size()) {
        return nullptr;
    }

    return currentImagesData.at(imageNumber);
}
std::vector<ImageData*>* ImagesData::get() {
    return &imagesData;
}

std::vector<ImageData*> ImagesData::getConst() const {
    return imagesData;
}

std::vector<ImageData*>* ImagesData::getCurrent() {
    return &currentImagesData;
}

int ImagesData::getImageDataIdInCurrent(std::string imagePath) {
    auto foundImageData = std::find_if(currentImagesData.begin(), currentImagesData.end(),
                                       [&imagePath](ImageData* imgPtr) {
                                           return imgPtr->getImagePathConst() == imagePath;
                                       });

    if (foundImageData != currentImagesData.end()) {
        return std::distance(currentImagesData.begin(), foundImageData);
    } else {
        return -1;
    }
}

int ImagesData::getImageDataId(std::string imagePath) {
    auto foundImageData = std::find_if(imagesData.begin(), imagesData.end(),
                                       [&imagePath](ImageData* imgPtr) {
                                           return imgPtr->getImagePathConst() == imagePath;
                                       });

    if (foundImageData != imagesData.end()) {
        return std::distance(imagesData.begin(), foundImageData);
    } else {
        return -1;
    }
}

std::unordered_map<std::string, ImageData*>* ImagesData::getImageMap() {
    return &imageMap;
}

void ImagesData::setImageMapValue(std::string imagePath, ImageData* imageData) {
    // qDebug() << "setImageMapValue : " << imagePath.c_str();
    // if (imageData->getpersons().size() > 0) {
    //     qDebug() << "load person 3 : " << imageData->getImagePath();
    // }
    imageMap[imagePath] = imageData;
    // if (imageMap[imagePath]->getpersons().size() > 0) {
    //     qDebug() << "load person 4 : " << imageData->getImagePath();
    // }
}

void ImagesData::setImageMapIntValue(int index, ImageData* imageData) {
    imageMapInt[index] = imageData;
}
