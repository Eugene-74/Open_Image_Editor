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
int ImagesData::getImageNumber() {
    return imageNumber;
}

void ImagesData::print() const {
    qDebug() << "ImagesData : \n";
    for (const ImageData valeur : imagesData) {
        qDebug() << valeur.get();
    }
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

    imagesData.push_back(imageD);
    imageMap[imageD.getImagePath()] = &imageD;
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
    auto it = std::find(imagesData.begin(), imagesData.end(), image);

    if (it != imagesData.end()) {
        imagesData.erase(it);
    }
}

ImageData* ImagesData::getImageData(int id) {
    if (id < 0 || id >= imagesData.size()) {
        throw std::out_of_range("getImageData :: Index hors limites" + std::to_string(id));
    }
    return &imagesData.at(id);
}

ImageData* ImagesData::getImageDataInCurrent(int id) {
    if (id < 0 || id >= currentImagesData.size()) {
        throw std::out_of_range("getImageData current :: Index hors limites" + std::to_string(id));
    }
    return currentImagesData.at(id);
}

ImageData* ImagesData::getImageData(std::string imagePath) {
    auto start = std::chrono::high_resolution_clock::now();
    auto it = imageMap.find(imagePath);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    if (it != imageMap.end()) {
        return it->second;
    }
    return nullptr;
}

ImageData* ImagesData::getCurrentImageData() {
    if (currentImagesData.size() <= 0 || imageNumber >= currentImagesData.size()) {
        return nullptr;
    }

    return currentImagesData.at(imageNumber);
}
std::vector<ImageData>* ImagesData::get() {
    return &imagesData;
}
std::vector<ImageData*>* ImagesData::getCurrent() {
    return &currentImagesData;
}

std::vector<ImageData> ImagesData::getConst() const {
    return imagesData;
}

int ImagesData::getImageDataId(std::string imagePath) {
    // qDebug() << "getImageDataId :: " << imagePath;
    auto it = std::find_if(currentImagesData.begin(), currentImagesData.end(),
                           [&imagePath](const ImageData* imgD) {
                               return imgD->getImagePathConst() == imagePath;
                           });

    if (it != currentImagesData.end()) {
        return std::distance(currentImagesData.begin(), it);
    } else {
        return -1;
    }
}
