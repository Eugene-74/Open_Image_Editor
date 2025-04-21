#include "ImagesData.hpp"

#include "Const.hpp"

/**
 * @brief Assignment operator for ImagesData class
 * @param other The ImagesData object to assign from
 * @return Reference to the current object
 * @details This function assigns the values of the other ImagesData object to the current object.
 */
ImagesData& ImagesData::operator=(const ImagesData& other) {
    if (this != &other) {
        imagesData = other.imagesData;
        currentImagesData = other.currentImagesData;
        imageMap = other.imageMap;
        imageNumber = other.imageNumber;
    }
    return *this;
}

/**
 * @brief Set the image number to the given value. If the value is out of range, it will be clamped to the valid range.
 * @param nbr The new image number to set.
 * @details This function sets the image number to the specified value. If the value is less than 0, it will be set to 0. If it exceeds the size of currentImagesData, it will be set to the last index of currentImagesData.
 */
void ImagesData::setImageNumber(int nbr) {
    if (currentImagesData.size() == 0) {
        nbr = 0;
    } else {
        nbr = std::max(0, nbr);
        nbr = std::min(nbr, (int)currentImagesData.size() - 1);
    }

    imageNumber = nbr;
}

/**
 * @brief Get the current image number.
 * @return The current image number.
 * @details This function returns the current image number. It is the index of the image in the currentImagesData vector.
 */
int ImagesData::getImageNumber() const {
    return imageNumber;
}

/**
 * @brief Get the image number in imagesData.
 * @return The image number in imagesData.
 * @details This function returns the image number in imagesData. It is the index of the image in the imagesData vector.
 */
int ImagesData::getImageNumberInTotal() {
    return getImageNumberInTotal(imageNumber);
}

/**
 * @brief Get the image number in imagesData from the image number in currentImagesData.
 * @param imageNbrInCurrent The image number in currentImagesData.
 * @return The image number in imagesData.
 * @details This function returns the image number in imagesData from the image number in currentImagesData. It is the index of the image in the imagesData vector.
 */
int ImagesData::getImageNumberInTotal(int imageNbrInCurrent) {
    return getImageDataId(getImageDataInCurrent(imageNbrInCurrent)->getImagePathConst());
}

/**
 * @brief Get the image number in currentImagesData from the image number in imagesData.
 * @param imageNbrInTotal The image number in imagesData.
 * @return The image number in currentImagesData.
 * @details This function returns the image number in currentImagesData from the image number in imagesData. It is the index of the image in the currentImagesData vector.
 */
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

/**
 * @brief Add an image to the imagesData vector and set the imageMap value.
 * @param imageData The image data to add.
 */
void ImagesData::addImage(ImageData* imageData) {
    std::string imagePath = imageData->getImagePath();
    this->get()->push_back(imageData);
    this->setImageMapValue(imagePath, this->get()->back());
}

/**
 * @brief Remove an image from the imagesData.
 * @param image The image data to remove.
 */
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
 * @brief Get the imageData in imagesData at the given index.
 * @param id The index of the imageData to retrieve.
 * @return The imageData at the given index.
 * @details This function retrieves the imageData at the specified index from imagesData. If the index is out of range, it returns nullptr.
 */
ImageData* ImagesData::getImageData(int id) {
    ImageData* imageData = getImageData(imagesData.at(id)->getImagePath());
    if (imageData == nullptr) {
        qWarning() << "getImageData :: Index hors limites" + std::to_string(id);
    }
    return imageData;
}

/**
 * @brief Get the imageData in currentImagesData at the given index.
 * @param id The index of the imageData to retrieve.
 * @return The imageData at the given index.
 * @details This function retrieves the imageData at the specified index from currentImagesData. If the index is out of range, it throws an exception.
 */
ImageData* ImagesData::getImageDataInCurrent(int id) {
    if (id < 0 || id >= currentImagesData.size()) {
        throw std::out_of_range("getImageData current :: Index hors limites : " + std::to_string(id));
    }
    return currentImagesData.at(id);
}

/**
 * @brief Get the imageData in imagesData by its path.
 * @param imagePath The path of the imageData to retrieve.
 * @return The imageData at the given path.
 * @details This function retrieves the imageData at the specified path from imagesData. If the path is empty, it returns nullptr.
 */
ImageData* ImagesData::getImageData(std::string imagePath) {
    if (imagePath.empty()) {
        return nullptr;
    }
    auto foundImageData = imageMap.find(imagePath);

    if (foundImageData != imageMap.end()) {
        ImageData* imageData = foundImageData->second;
        return imageData;
    }
    return nullptr;
}

/**
 * @brief Get the current image data.
 * @return The current image data.
 * @details This function retrieves the current image data from currentImagesData. If the currentImagesData is empty or the imageNumber is out of range, it returns nullptr.
 */
ImageData* ImagesData::getCurrentImageData() {
    if (currentImagesData.size() <= 0 || imageNumber >= currentImagesData.size()) {
        return nullptr;
    }

    return currentImagesData.at(imageNumber);
}

/**
 * @brief Get the imagesData vector (ptr version).
 * @return A pointer to the imagesData vector.
 */
std::vector<ImageData*>* ImagesData::get() {
    return &imagesData;
}

/**
 * @brief Get the imagesData vector as a constant reference (const version).
 * @return A constant reference to the imagesData vector.
 */
std::vector<ImageData*> ImagesData::getConst() const {
    return imagesData;
}

/**
 * @brief Get the current imagesData vector (ptr version).
 * @return A pointer to the current imagesData vector.
 */
std::vector<ImageData*>* ImagesData::getCurrent() {
    return &currentImagesData;
}

/**
 * @brief Get the imageData index in currentImagesData by its path.
 * @param imagePath The path of the imageData to retrieve.
 * @return The index of the imageData in currentImagesData.
 * @details This function retrieves the index of the imageData at the specified path from currentImagesData. If the path is not found, it returns -1.
 */
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

/**
 * @brief Get the imageData index in imagesData by its path.
 * @param imagePath The path of the imageData to retrieve.
 * @return The index of the imageData in imagesData.
 * @details This function retrieves the index of the imageData at the specified path from imagesData. If the path is not found, it returns -1.
 */
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

/**
 * @brief Get the imageMap (ptr version).
 * @return A pointer to the imageMap.
 * @details This function retrieves the imageMap. It is a map that associates image paths with their corresponding ImageData objects.
 */
std::unordered_map<std::string, ImageData*>* ImagesData::getImageMap() {
    return &imageMap;
}

/**
 * @brief Set the imageMap value for a given image path.
 * @param imagePath The path of the image.
 * @param imageData The ImageData object to associate with the image path.
 * @details This function sets the imageMap value for a given image path. It associates the image path with the corresponding ImageData object.
 */
void ImagesData::setImageMapValue(std::string imagePath, ImageData* imageData) {
    imageMap[imagePath] = imageData;
}

/**
 * @brief Clear all imagesData and reset the image number.
 * @details This function clears all imagesData and resets the image number to 0. It also clears the imageMap and currentImagesData vectors.
 */
void ImagesData::clear() {
    imagesData.clear();
    imageMap.clear();
    currentImagesData.clear();
    imageNumber = 0;
}

/**
 * @brief Get the filters map.
 * @return The filters map.
 */
std::map<std::string, bool> ImagesData::getFilters() {
    return filters;
}

/**
 * @brief Set the filters map.
 * @param filters The filters map to set.
 * @return true if the filters were set successfully, false otherwise.
 */
bool ImagesData::setFilters(std::map<std::string, bool> filters) {
    if (filters.empty()) {
        return false;
    }
    this->filters = filters;
    return true;
}

/**
 * @brief Set the value of a specific filter.
 * @param filterName The name of the filter to set.
 * @param filterValue The value to set for the filter.
 * @return true if the filter value was set successfully, false otherwise.
 */
bool ImagesData::setFiltersValue(std::string filterName, bool filterValue) {
    if (filters.empty()) {
        return false;
    }
    if (filters.find(filterName) == filters.end()) {
        return false;
    }

    filters[filterName] = filterValue;
    return true;
}
