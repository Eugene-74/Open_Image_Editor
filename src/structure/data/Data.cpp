#include "Data.h"

namespace fs = std::filesystem;  // Alias pour simplifier le code


void Data::preDeleteImage(int imageNbr) {
    ImageData* imageData;
    imageData = imagesData.getImageData(imageNbr);

    deletedImagesData.addImage(*imageData);
    std::cerr << "image deleted : " << imageNbr << std::endl;
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

    for (const auto& deletedImage : *deletedImagesData.get()) {

        // Find the image in imagesData
        auto it = std::find(imagesData.get()->begin(), imagesData.get()->end(), deletedImage);
        // If it exists, remove it from imagesData
        if (it != imagesData.get()->end()) {
            // imagesData.removeImage(*imagesData.getImageData(it));
            imagesData.get()->erase(it);
            deletedImage.print();
        }

    }

    std::cerr << "All images deleted" << std::endl;


}


bool Data::isDeleted(int imageNbr) {

    // Find the image in deletedImagesData


    std::string imagePath = imagesData.getImageData(imageNbr)->imagePath;

    auto it = std::find_if(deletedImagesData.get()->begin(), deletedImagesData.get()->end(),
        [imagePath, this](const ImageData img) {

            return img.imagePath == imagePath;
        });

    if (it != deletedImagesData.get()->end()) {
        imagesData.getImageData(imageNbr)->print();

        return true;
    }

    return false;
}
QImage Data::loadImage(QWidget* parent, std::string imagePath, QSize size, bool setSize, int thumbnail, bool rotation, bool square) {
    if (square){
        return loadImageSquare(parent, imagePath, size, setSize, thumbnail, rotation);

    }
    else{
        return loadImageNormal(parent, imagePath, size, setSize, thumbnail, rotation);

    }
}


QImage Data::loadImageSquare(QWidget* parent, std::string imagePath, QSize size, bool setSize, int thumbnail, bool rotation) {
    // Calculate the center crop dimensions
    QImage image = loadImageNormal(parent, imagePath, size, setSize, thumbnail, rotation);
    // std::cerr << "imageloaded " << std::endl;
    int cropSize = std::min(image.width(), image.height());
    int xOffset = (image.width() - cropSize) / 2;
    int yOffset = (image.height() - cropSize) / 2;

    // Crop the image to a square
    image = image.copy(xOffset, yOffset, cropSize, cropSize);
    return image;
}
QImage Data::loadImageNormal(QWidget* parent, std::string imagePath, QSize size, bool setSize, int thumbnail, bool rotation) {


    // Check if the image is in the imageCache
    std::map<std::string, QImageAndPath>* cache = imageCache;


    auto it = cache->find(imagePath);
    if (it != cache->end()) {
        // std::cerr << "image trouve dans le cache" << std::endl;
        return it->second.image;
    }

    it = cache->find(getThumbnailPath(imagePath, 512));
    if (it != cache->end()) {
        // std::cerr << "image trouve dans le cache" << std::endl;
        return it->second.image;
    }

    it = cache->find(getThumbnailPath(imagePath, 256));
    if (it != cache->end()) {
        // std::cerr << "image trouve dans le cache" << std::endl;
        return it->second.image;
    }

    it = cache->find(getThumbnailPath(imagePath, 128));
    if (it != cache->end()) {
        // std::cerr << "image trouve dans le cache" << std::endl;
        return it->second.image;
    }


    std::string imagePathbis = imagePath;


    QResource ressource(QString::fromStdString(imagePath));

    QImage image;

    if (ressource.isValid()){
        image.load(QString::fromStdString(imagePathbis));

    }
    else{
        if (thumbnail == 128){
            if (hasThumbnail(imagePath, 128)){
                imagePathbis = getThumbnailPath(imagePath, 128);
            }
            else{

                createThumbnail(imagePath, 128);
                createThumbnailIfNotExists(imagePath, 256);
                createThumbnailIfNotExists(imagePath, 512);
            }
        }
        else  if (thumbnail == 256){
            if (hasThumbnail(imagePath, 256)){
                imagePathbis = getThumbnailPath(imagePath, 256);
            }
            else{
                createThumbnailIfNotExists(imagePath, 128);
                createThumbnail(imagePath, 256);
                createThumbnailIfNotExists(imagePath, 512);


            }
        }
        else if (thumbnail == 512){
            if (hasThumbnail(imagePath, 512)){
                imagePathbis = getThumbnailPath(imagePath, 512);
            }
            else{
                createThumbnailIfNotExists(imagePath, 128);
                createThumbnailIfNotExists(imagePath, 256);
                createThumbnail(imagePath, 512);


            }
        }

        image.load(QString::fromStdString(imagePathbis));
        if (image.isNull()) {



            return QImage();
        }

        if (setSize) {
            image = image.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
        if (rotation){
            ImageData* imageData = imagesData.getImageData(imagePath);
            if (imageData != nullptr){
                // std::cerr << "imageData" << std::endl;
                Exiv2::ExifData exifData = imageData->getMetaData()->getExifData();
                if (exifData.empty()) {
                    // std::cerr << "empty" << std::endl;
                }
                else {
                    // std::cerr << "not empty" << std::endl;
                    if (exifData["Exif.Image.Orientation"].count() != 0) {
                        int orientation = exifData["Exif.Image.Orientation"].toInt64();
                        // std::cerr << "orientation : " << orientation << " :: " << imagePathbis << std::endl;
                        switch (orientation) {
                        case 1:
                            // No transformation needed
                            break;
                        case 2:
                            image = image.mirrored(true, false); // Horizontal mirror
                            break;
                        case 3:
                            image = image.transformed(QTransform().rotate(180));
                            break;
                        case 4:
                            image = image.mirrored(false, true); // Vertical mirror
                            break;
                        case 5:
                            image = image.mirrored(true, false).transformed(QTransform().rotate(90)); // Horizontal mirror + 90 degrees
                            break;
                        case 6:
                            image = image.transformed(QTransform().rotate(90));
                            break;
                        case 7:
                            image = image.mirrored(true, false).transformed(QTransform().rotate(-90)); // Horizontal mirror + -90 degrees
                            break;
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
        }
    }

    // Add the image to the imageCache



    (*cache)[imagePathbis].image = image;
    (*cache)[imagePathbis].imagePath = imagePath;
    return image;
}

bool Data::loadInCache(std::string imagePath, bool setSize, QSize size, bool force) {
    QImage image;
    if (force && isInCache(imagePath)){
        return true;
    }
    image.load(QString::fromStdString(imagePath));
    if (image.isNull()) {
        return false;
    }

    if (setSize) {
        image = image.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    ImageData* imageData = imagesData.getImageData(imagePath);
    if (imageData != nullptr){
        Exiv2::ExifData exifData = imageData->getMetaData()->getExifData();
        if (exifData.empty()) {
        }
        else {
            if (exifData["Exif.Image.Orientation"].count() != 0) {
                int orientation = exifData["Exif.Image.Orientation"].toInt64();
                switch (orientation) {
                case 1:
                    // No transformation needed
                    break;
                case 2:
                    image = image.mirrored(true, false); // Horizontal mirror
                    break;
                case 3:
                    image = image.transformed(QTransform().rotate(180));
                    break;
                case 4:
                    image = image.mirrored(false, true); // Vertical mirror
                    break;
                case 5:
                    image = image.mirrored(true, false).transformed(QTransform().rotate(90)); // Horizontal mirror + 90 degrees
                    break;
                case 6:
                    image = image.transformed(QTransform().rotate(90));
                    break;
                case 7:
                    image = image.mirrored(true, false).transformed(QTransform().rotate(-90)); // Horizontal mirror + -90 degrees
                    break;
                case 8:
                    image = image.transformed(QTransform().rotate(-90));
                    break;
                default:
                    break;
                }
            }
        }
    }
    // Add the image to the imageCache
    (*imageCache)[imagePath].image = image;
    (*imageCache)[imagePath].imagePath = imagePath;

    createThumbnailIfNotExists(imagePath, 128);
    createThumbnailIfNotExists(imagePath, 256);
    createThumbnailIfNotExists(imagePath, 512);

    return true;
}

bool Data::isInCache(std::string imagePath){
    return imageCache->find(imagePath) != imageCache->end();
}

bool Data::getLoadedImage(std::string imagePath, QImage& image){
    auto it = imageCache->find(imagePath);
    if (it != imageCache->end()) {
        image = it->second.image;
        return true;
    }
    return false;
}


void Data::rotateImageCache(std::string imagePath, int rotation){
    QImage image;
    auto it = imageCache->find(imagePath);
    if (it != imageCache->end()) {
        image = it->second.image;
        // Perform operations with the found image
    }
    image = image.transformed(QTransform().rotate(rotation));
    imageCache->operator[](imagePath).image = image;

}

void Data::mirrorImageCache(std::string imagePath, int upDown){
    QImage image;
    auto it = imageCache->find(imagePath);
    if (it != imageCache->end()) {
        image = it->second.image;
        // Perform operations with the found image
    }
    if (upDown == true){
        image = image.mirrored(false, true);
    }
    else if (upDown == false){
        image = image.mirrored(true, false);
    }
    imageCache->operator[](imagePath).image = image;

}

void Data::createThumbnails(const std::vector<std::string>& imagePaths, const int maxDim) {
    for (const auto& imagePath : imagePaths) {
        createThumbnail(imagePath, maxDim);
    }
}

void Data::createThumbnail(const std::string& imagePath, const int maxDim) {
    QImage image = loadImage(nullptr, imagePath, QSize(maxDim, maxDim), false, 0, false);


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
        createThumbnail(imagePath, 128);
    }
    if (size > 256) {
        createThumbnail(imagePath, 256);
    }
    if (size > 512) {
        createThumbnail(imagePath, 512);
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

bool Data::unloadFromCache(std::string imagePath){
    if (!imageCache) {
        std::cerr << "imageCache is not initialized" << std::endl;
        return false;
    }
    auto it = imageCache->find(imagePath);

    if (it != imageCache->end()) {
        imageCache->erase(it);
        return true;
    }
    return false;
}
void Data::exportImages(std::string exportPath) {
    copyImages(&rootFolders, exportPath);
}


void Data::copyImages(Folders* currentFolders, std::string path) const {
    for (Folders& folder : currentFolders->folders) {
        std::string folderPath = path + "/" + folder.folderName;
        if (!fs::exists(folderPath)) {
            fs::create_directories(folderPath);
        }
        else{
        }
        copyTo(folder.folderName, path);
        copyImages(&folder, folderPath);
    }

}

void Data::copyTo(std::string filePath, std::string destinationPath) const{

    for (const auto& imageData : imagesData.imagesData) {
        for (const auto& file : imageData.folders.files) {
            if (file == filePath) {
                fs::copy(imageData.imagePath, destinationPath + "/" + file, fs::copy_options::overwrite_existing);
            }
        }
    }
}