#include "ImagesData.hpp"

#include "Const.hpp"

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
        nbr = std::max(0, nbr);
        nbr = std::min(nbr, (int)currentImagesData.size() - 1);
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
    return imageNbrInCurrent - 1;
}

void ImagesData::addImage(ImageData* imageData) {
    std::string imagePath = imageData->getImagePath();
    // if (getImageData(imagePath) != nullptr) {

    //     return;
    // }
    this->get()->push_back(imageData);
    this->setImageMapValue(imagePath, this->get()->back());
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

/**
 * @brief Give you the ImageData of the image at the index id
 * @param id The id of the image
 * @return The imageData if the id is valid, nullptr otherwise
 */
ImageData* ImagesData::getImageData(int id) {
    // if (id < 0 || id >= imageMapInt.size()) {
    //     throw std::out_of_range("getImageData :: Index hors limites" + std::to_string(id));
    // }
    // auto foundImageData = imageMapInt.find(id);
    // if (foundImageData != imageMapInt.end()) {
    //     return foundImageData->second;
    // }
    ImageData* imageData = getImageData(imagesData.at(id)->getImagePath());
    if (imageData == nullptr) {
        qWarning() << "getImageData :: Index hors limites" + std::to_string(id);
    }
    return imageData;
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
    imageMap[imagePath] = imageData;
}

void ImagesData::setImageMapIntValue(int index, ImageData* imageData) {
    imageMapInt[index] = imageData;
}

void ImagesData::clear() {
    imagesData.clear();
    imageMap.clear();
    imageMapInt.clear();
    currentImagesData.clear();
    imageNumber = 0;
}

std::map<std::string, bool> ImagesData::getFilters() {
    return filters;
}

bool ImagesData::setFilters(std::map<std::string, bool> filters) {
    this->filters = filters;
    return true;
}

bool ImagesData::setFiltersValue(std::string filterName, bool filterValue) {
    filters[filterName] = filterValue;
    return true;
}
