#include <iostream>
#include "imagesData.h"


void ImagesData::print() const {
    std::cout << "ImagesData : \n";
    for (const ImageData& valeur : imagesData) {
        std::cout << valeur.get();
    }
    std::cout << std::endl;
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

        imagesData.push_back(imageD);
    }
    else {
        imagesData.push_back(imageD);
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
        throw std::out_of_range("Index hors limites");
    }
    return &imagesData.at(id);
    // renvoie un pointeur ce qui permet une modification automatique dans ImagesData
}
std::vector<ImageData>  ImagesData::get() {
    return imagesData;
}
