#include "ImagesData.hpp"

ImagesData& ImagesData::operator=(const ImagesData& other) {
    if (this != &other) {
        imagesData = other.imagesData;  // Utiliser l'opérateur d'affectation de std::vector
        imageNumber = other.imageNumber;
    }
    return *this;
}

void ImagesData::setImageNumber(int nbr) {
    while (nbr < 0) {
        nbr += 1;
    }
    while (nbr >= imagesData.size()) {
        nbr -= 1;
    }

    imageNumber = nbr;
}
int ImagesData::getImageNumber() {
    return imageNumber;
}

void ImagesData::print() const {
    qDebug() << "ImagesData : \n";
    for (const ImageData& valeur : imagesData) {
        qDebug() << valeur.get();
    }
}

void ImagesData::addImage(ImageData& imageD) {
    auto it = std::find_if(imagesData.begin(), imagesData.end(),
                           [&imageD](const ImageData& imgD) {
                               return imgD == imageD;
                           });

    if (it != imagesData.end()) {
        ImageData lastImageD = *it;
        imagesData.erase(it);
        imageD.addFolders(lastImageD.getFolders());
        imageD.setCropSizes(lastImageD.getCropSizes());

        imagesData.push_back(imageD);
    } else {
        imagesData.push_back(imageD);
        for (const auto& crop : imageD.getCropSizes()) {
            for (const auto& point : crop) {
                qDebug() << point;
            }
        }
    }
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
    // renvoie un pointeur ce qui permet une modification automatique dans ImagesData
}

ImageData* ImagesData::getImageData(std::string imagePath) {
    auto it = std::find_if(imagesData.begin(), imagesData.end(),
                           [&imagePath](const ImageData& imgD) {
                               return imgD.folders.name == imagePath;
                           });

    if (it != imagesData.end()) {
        return &(*it);
    }
    return nullptr;
}

ImageData* ImagesData::getCurrentImageData() {
    if (imagesData.size() <= 0 || imageNumber >= imagesData.size()) {
        return nullptr;
        throw std::out_of_range("getCurrentImageData :: Index hors limites");
    }

    return &imagesData.at(imageNumber);
}
std::vector<ImageData>* ImagesData::get() {
    return &imagesData;
}

int ImagesData::getImageIdByName(std::string imagePath) {
    auto it = std::find_if(imagesData.begin(), imagesData.end(),
                           [&imagePath](const ImageData& imgD) {
                               return imgD.folders.name == imagePath;
                           });

    if (it != imagesData.end()) {
        return std::distance(imagesData.begin(), it);
    } else {
        return -1;  // Return -1 if the image is not found
    }
}
