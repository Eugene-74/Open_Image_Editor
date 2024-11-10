#include "Data.h"

namespace fs = std::filesystem;  // Alias pour simplifier le code


void Data::preDeleteImage(int imageNbr) {
    ImageData* imageData;
    imageData = imagesData->getImageData(imageNbr);

    deletedImagesData->addImage(*imageData);
    std::cerr << "image deleted" << imageNbr << std::endl;
    deletedImagesData->print();

}
void Data::unPreDeleteImage(int imageNbr) {
    ImageData* imageData;
    imageData = imagesData->getImageData(imageNbr);


    deletedImagesData->removeImage(*imageData);

}

void Data::revocerDeletedImage(ImageData& imageData) {
    imagesData->addImage(imageData);
    deletedImagesData->removeImage(imageData);
}

void Data::revocerDeletedImage(int imageNbr) {
    ImageData* imageData;
    imageData = deletedImagesData->getImageData(imageNbr);
    revocerDeletedImage(*imageData);

    // imagesData.addImage(*imageData);
    // deletedImagesData.removeImage(*imageData);

}

// Supprime de imagesData les images dans deletedImagesData
void Data::removeDeletedImages() {

    for (const auto& deletedImage : deletedImagesData->get()) {

        // Find the image in imagesData
        auto it = std::find(imagesData->get().begin(), imagesData->get().end(), deletedImage);
        // If it exists, remove it from imagesData
        if (it != imagesData->get().end()) {
            // imagesData.removeImage(*imagesData.getImageData(it));
            imagesData->get().erase(it);
            deletedImage.print();
        }

    }

    std::cerr << "All images deleted" << std::endl;


}


bool Data::isDeleted(int imageNbr) {

    // Find the image in deletedImagesData
    auto it = std::find_if(deletedImagesData->get().begin(), deletedImagesData->get().end(),
        [imageNbr, this](const ImageData& img) {
            return img == *imagesData->getImageData(imageNbr);
        });

    if (it != deletedImagesData->get().end()) {
        imagesData->getImageData(imageNbr)->print();
        return true;
    }

    return false;
}


QImage Data::loadImage(QWidget* parent, std::string imagePath, QSize size, bool setSize) {


    // Check if the image is in the imageCache
    auto it = imageCache->find(imagePath);
    if (it != imageCache->end()) {
        // std::cerr << "Image found in cache: " << imagePath << std::endl;
        return it->second;
    }

    QImage image(QString::fromStdString(imagePath));
    // std::cerr << "load Image " << imagePath << std::endl;

    QResource ressource(QString::fromStdString(imagePath));

    if (ressource.isValid()){
        // std::cerr << "ressource valide" << std::endl;
    }
    else{
        // std::cerr << "ressource non valide" << std::endl;
        if (image.isNull()) {
            // QMessageBox::warning(parent, "Load Image", "The image could not be loaded.");
            return QImage();
        }

        if (setSize) {
            image = image.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }

        ImageData* imageData = imagesData->getImageData(imagePath);
        if (imageData != nullptr){
            Exiv2::ExifData exifData = imageData->getMetaData()->getExifData();
            if (exifData.empty()) {
                std::cerr << "No EXIF data found in image!" << std::endl;
            }
            else {
                std::cerr << imagePath << "EXIF data found in image! " << exifData["Exif.Image.Orientation"].toInt64() << std::endl;
                if (exifData["Exif.Image.Orientation"].count() != 0) {
                    int orientation = exifData["Exif.Image.Orientation"].toInt64();

                    // Rotate the image based on the EXIF orientation
                    switch (orientation) {
                    case 3:
                        image = image.transformed(QTransform().rotate(180));
                        break;
                    case 6:
                        image = image.transformed(QTransform().rotate(90));
                        break;
                    case 8:
                        image = image.transformed(QTransform().rotate(-90));
                        break;
                    }
                }
            }
        }
    }

    // Add the image to the imageCache
    (*imageCache)[imagePath] = image;


    return image;
}
