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


QImage Data::loadImage(QWidget* parent, std::string imagePath, QSize size, bool setSize, bool thumbnail) {


    // Check if the image is in the imageCache
    auto it = imageCache->find(imagePath);
    if (it != imageCache->end()) {
        std::cerr << "image trouve dans le cache" << std::endl;
        return it->second;
    }

    std::string imagePathbis = imagePath;


    QResource ressource(QString::fromStdString(imagePath));

    QImage image;

    if (ressource.isValid()){
        image.load(QString::fromStdString(imagePathbis));

    }
    else{
        if (thumbnail){
            if (hasThumbnail(imagePath, 128)){
                imagePathbis = getThumbnailPath(imagePath, 128);
            }
            else{
                createThumbnail(imagePath, 128);
                createThumbnailIfNotExists(imagePath, 256);
                createThumbnailIfNotExists(imagePath, 512);


            }
        }

        image.load(QString::fromStdString(imagePathbis));
        if (image.isNull()) {
            return QImage();
        }

        if (setSize) {
            image = image.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
        // if (imagePathbis == imagePath){
        ImageData* imageData = imagesData->getImageData(imagePath);
        if (imageData != nullptr){
            Exiv2::ExifData exifData = imageData->getMetaData()->getExifData();
            if (exifData.empty()) {
            }
            else {
                if (exifData["Exif.Image.Orientation"].count() != 0) {
                    int orientation = exifData["Exif.Image.Orientation"].toInt64();
                    std::cerr << "orientation : " << orientation << " :: " << imagePath << std::endl;
                    switch (orientation) {
                        // case 1:
                        //     // No transformation needed
                        //     break;
                        // case 2:
                        //     image = image.mirrored(true, false); // Horizontal mirror
                        //     break;
                    case 3:
                        image = image.transformed(QTransform().rotate(180));
                        break;
                        // case 4:
                        //     image = image.mirrored(false, true); // Vertical mirror
                        //     break;
                        // case 5:
                        //     image = image.mirrored(true, false).transformed(QTransform().rotate(90)); // Horizontal mirror + 90 degrees
                        //     break;
                    case 6:
                        image = image.transformed(QTransform().rotate(90));
                        break;
                        // case 7:
                        //     image = image.mirrored(true, false).transformed(QTransform().rotate(-90)); // Horizontal mirror + -90 degrees
                        //     break;
                    case 8:
                        image = image.transformed(QTransform().rotate(-90));
                        break;
                    default:
                        // Unknown orientation, no transformation
                        break;
                    }
                }
            }
        }
        // }
    }

    // Add the image to the imageCache
    (*imageCache)[imagePathbis] = image;


    return image;
}


void Data::rotateImageCache(std::string imagePath, int rotation){
    QImage image;
    auto it = imageCache->find(imagePath);
    if (it != imageCache->end()) {
        image = it->second;
        // Perform operations with the found image
    }
    image = image.transformed(QTransform().rotate(rotation));
    imageCache->operator[](imagePath) = image;
}

void Data::createThumbnails(const std::vector<std::string>& imagePaths, const int maxDim) {
    for (const auto& imagePath : imagePaths) {
        createThumbnail(imagePath, maxDim);
    }
}

void Data::createThumbnail(const std::string& imagePath, const int maxDim) {
    QImage image = loadImage(nullptr, imagePath, QSize(maxDim, maxDim), false, false);


    // Calculate the scaling factor to maintain aspect ratio
    double scale = std::min(static_cast<double>(maxDim) / image.width(), static_cast<double>(maxDim) / image.height());

    // Resize the image
    QImage thumbnail = image.scaled(image.width() * scale, image.height() * scale, Qt::KeepAspectRatio);

    // Generate the output path for the thumbnail
    std::hash<std::string> hasher;
    size_t hashValue = hasher(imagePath);

    std::string extension = fs::path(imagePath).extension().string();

    std::string outputImage;
    if (maxDim == 128) {
        outputImage = THUMBNAIL_PATH + "/normal/" + std::to_string(hashValue) + extension;
    }
    else if (maxDim == 256) {
        outputImage = THUMBNAIL_PATH + "/large/" + std::to_string(hashValue) + extension;
    }
    else if (maxDim == 512) {
        outputImage = THUMBNAIL_PATH + "/x-large/" + std::to_string(hashValue) + extension;
    }

    // Save the thumbnail image
    if (!thumbnail.save(QString::fromStdString(outputImage))) {
        std::cerr << "Error: Could not save thumbnail: " << outputImage << std::endl;
    }
}
void Data::createThumbnailsIfNotExists(const std::vector<std::string>& imagePaths, const int maxDim) {
    for (const auto& imagePath : imagePaths) {
        createThumbnailIfNotExists(imagePath, maxDim);
    }
}


/**
 * @brief Creates a thumbnail for the given image if it does not already exist.
 *
 * This function checks if a thumbnail for the specified image exists. If it does not,
 * it creates a new thumbnail.
 *
 * @param imagePath The path to the image file for which the thumbnail should be created.
 */
void Data::createThumbnailIfNotExists(const std::string& imagePath, const int maxDim) {
    if (!hasThumbnail(imagePath, maxDim)) {

        // std::cout << "creating Thumbnail for: " << imagePath << std::endl;
        createThumbnail(imagePath, maxDim);
    }
    else {
        // std::cout << "Thumbnail already exists for: " << maxDim << " : " << imagePath << std::endl;
    }
}
bool Data::hasThumbnail(const std::string& imagePath, const int maxDim) {

    std::hash<std::string> hasher;
    size_t hashValue = hasher(imagePath);
    std::string extension = fs::path(imagePath).extension().string();
    std::string outputImage;

    if (maxDim == 128) {
        outputImage = THUMBNAIL_PATH + "/normal/" + std::to_string(hashValue) + extension;
    }
    else if (maxDim == 256) {
        outputImage = THUMBNAIL_PATH + "/large/" + std::to_string(hashValue) + extension;
    }
    else if (maxDim == 512) {
        outputImage = THUMBNAIL_PATH + "/x-large/" + std::to_string(hashValue) + extension;
    }

    // Check if the thumbnail file exists
    return fs::exists(outputImage);
}

void Data::createAllThumbnail(const std::string& imagePath, const int size) {
    std::vector<std::string> imagePaths;

    if (size > 128) {
        createThumbnailIfNotExists(imagePath, 128);
    }
    if (size > 256) {
        createThumbnailIfNotExists(imagePath, 256);
    }
    if (size > 512) {
        createThumbnailIfNotExists(imagePath, 512);
    }
}

std::string Data::getThumbnailPath(const std::string& imagePath, const int size){
    std::hash<std::string> hasher;
    size_t hashValue = hasher(imagePath);
    std::string extension = fs::path(imagePath).extension().string();
    std::string outputImage;

    if (size == 128) {
        outputImage = THUMBNAIL_PATH + "/normal/" + std::to_string(hashValue) + extension;
    }
    else if (size == 256) {
        outputImage = THUMBNAIL_PATH + "/large/" + std::to_string(hashValue) + extension;
    }
    else if (size == 512) {
        outputImage = THUMBNAIL_PATH + "/x-large/" + std::to_string(hashValue) + extension;
    }
    return outputImage;
}