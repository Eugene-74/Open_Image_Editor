#include "Data.h"

void Data::preDeleteImage(int imageNbr) {
    ImageData* imageData;
    imageData = imagesData.getImageData(imageNbr);

    deletedImagesData.addImage(*imageData);
    std::cerr << "image deleted" << imageNbr << std::endl;
    deletedImagesData.print();

}
void Data::unPreDeleteImage(int imageNbr) {
    ImageData* imageData;
    imageData = imagesData.getImageData(imageNbr);

    deletedImagesData.removeImage(*imageData);

}

void Data::revocerDeletedImage(ImageData& imageData) {
    imagesData.addImage(imageData);
    deletedImagesData.removeImage(imageData);
}

void Data::revocerDeletedImage(int imageNbr) {
    ImageData* imageData;
    imageData = deletedImagesData.getImageData(imageNbr);
    revocerDeletedImage(*imageData);

    // imagesData.addImage(*imageData);
    // deletedImagesData.removeImage(*imageData);

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


bool Data::isDeleted(int imageNbr) {

    // Find the image in deletedImagesData
    auto it = std::find_if(deletedImagesData.get().begin(), deletedImagesData.get().end(),
        [imageNbr, this](const ImageData& img) {
            return img == *imagesData.getImageData(imageNbr);
        });

    if (it != deletedImagesData.get().end()) {
        imagesData.getImageData(imageNbr)->print();
        return true;
    }

    return false;
}
