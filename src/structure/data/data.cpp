#include "data.h"

void Data::preDeleteImage(int imageNbr) {
    ImageData* imageData;
    imageData = imagesData.getImageData(imageNbr);

    deletedImagesData.addImage(*imageData);
    std::cerr << "image deleted" << imageNbr << std::endl;
    // removeImage(*imageData);
    deletedImagesData.print();

}
void Data::revocerDeletedImage(int imageNbr) {
    ImageData* imageData;
    imageData = deletedImagesData.getImageData(imageNbr);

    imagesData.addImage(*imageData);
    deletedImagesData.removeImage(*imageData);

}

// Supprime de imagesData les images dans deletedImagesData
void Data::removeDeletedImages() {

    for (const auto& deletedImage : deletedImagesData.get()) {

        // Find the image in imagesData
        auto it = std::find(imagesData.get().begin(), imagesData.get().end(), deletedImage);
        // If it exists, remove it from imagesData
        if (it != imagesData.get().end()) {
            // imagesData.removeImage(*imagesData.getImageData(it));
            imagesData.get().erase(it);
            deletedImage.print();
        }

    }

    std::cerr << "All images deleted" << std::endl;


}

