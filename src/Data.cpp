#include "Data.hpp"

#include <QApplication>
#include <QFileDialog>
#include <QLabel>
#include <QMediaPlayer>
#include <QMessageBox>
#include <QObject>
#include <QPainter>
#include <QProgressDialog>
#include <QResource>
#include <QString>
#include <QThreadPool>
#include <QTimer>
#include <QWidget>
#include <opencv2/core.hpp>
#include <opencv2/core/ocl.hpp>
#include <opencv2/opencv.hpp>
#include <regex>
#include <unordered_set>

#include "Box.hpp"
#include "Const.hpp"
#include "Conversion.hpp"
#include "Data.hpp"
#include "Download.hpp"
#include "FacesRecognition.hpp"
#include "Network.hpp"
#include "ObjectRecognition.hpp"
#include "Text.hpp"
#include "Verification.hpp"

namespace fs = std::filesystem;

/**
 * @brief Constructor for the Data class
 * @details It initializes the imageCache and sets the default options
 */
Data::Data()
    : imageCache(new std::unordered_map<std::string, QImageAndPath>()) {
    options = DEFAULT_OPTIONS;
}

/**
 * @brief Move an image to the delete images
 * @param imageNbr nbr of the images in the total
 */
void Data::preDeleteImage(int imageNbr) {
    ImageData* imageData;
    imageData = this->imagesData.getImageData(imageNbr);

    this->getDeletedImagesDataPtr()->addImage(imageData);
    qInfo() << "image deleted : " << imageNbr;
}

/**
 * @brief Recover an image from the deleted images
 * @param imageNbr nbr of the images in the total
 */
void Data::unPreDeleteImage(int imageNbr) {
    const ImageData imageData = *imagesData.getImageData(imageNbr);

    getDeletedImagesDataPtr()->removeImage(imageData);
}

/**
 * @brief Recover an image from the deleted images
 * @param imageData image to recover
 */
void Data::revocerDeletedImage(ImageData& imageData) {
    imagesData.addImage(&imageData);
    getDeletedImagesDataPtr()->removeImage(imageData);
}

/**
 * @brief Recover an image from the deleted images
 * @param imageNbr nbr of the images in the deleted images list
 */
void Data::revocerDeletedImage(int imageNbr) {
    ImageData* imageData = getDeletedImagesDataPtr()->getImageData(imageNbr);
    revocerDeletedImage(*imageData);

    imagesData.addImage(imageData);
    getDeletedImagesDataPtr()->removeImage(*imageData);
}

/**
 * @brief Remove all deleted images from the imagesData
 */
void Data::removeDeletedImages() {
    for (const auto& deletedImage : *getDeletedImagesDataPtr()->get()) {
        ImageData* imageData = this->getImagesDataPtr()->getImageData(deletedImage->getImagePath());

        removeImageFromFolders(*imageData);

        auto itPtr = std::find(imagesData.getCurrent()->begin(), imagesData.getCurrent()->end(), imageData);
        if (itPtr != imagesData.getCurrent()->end()) {
            qInfo() << "remove image from currentImagesData";
            imagesData.getCurrent()->erase(itPtr);
        }
    }
    qInfo() << "All images deleted";
}

/**
 * @brief Check if an image is deleted or not
 * @param imageNbr nbr of the image in the imagesData list
 * @return true if the image is deleted false otherwise
 */
bool Data::isDeleted(int imageNbr) {
    std::string imagePath = imagesData.getImageData(imageNbr)->getImagePathConst();
    auto it = std::find_if(getDeletedImagesDataPtr()->get()->begin(),
                           getDeletedImagesDataPtr()->get()->end(),
                           [imagePath, this](ImageData* imgPtr) {
                               return imgPtr->getImagePathConst() == imagePath;
                           });

    if (it != getDeletedImagesDataPtr()->get()->end()) {
        imagesData.getImageData(imageNbr)->print();

        return true;
    }

    return false;
}

/**
 * @brief Load a frale from a video
 * @param videoPath Path to the video file
 * @param frameNumber Frame number to load
 * @return QImage object containing the frame
 */
QImage Data::loadImageFromVideo(std::string videoPath, int frameNumber) {
    cv::VideoCapture cap(videoPath);
    if (!cap.isOpened()) {
        qWarning("Impossible d'ouvrir la vidéo : %s", videoPath.c_str());
        return QImage();
    }

    cap.set(cv::CAP_PROP_POS_FRAMES, frameNumber);

    cv::Mat frame;
    if (!cap.read(frame)) {
        qWarning("Impossible de lire le frame %d", frameNumber);
        return QImage();
    }

    QImage image = CvMatToQImage(frame);

    return image;
}

/**
 * @brief Load and transform an image from a path and put it in the cache
 * @param parent Parent widget
 * @param imagePath Path to the image
 * @param size Size of the image
 * @param setSize Set if the image should be resized
 * @param thumbnail Thumbnail size (0 for no thumbnail)
 * @param rotation Set if the image should be rotated (according to metadata)
 * @param square Set if the image should be square
 * @param crop Set if the image should be cropped (according to crop information)
 * @param force Set if the image should be loaded even if it is already in the cache
 * @return QImage object containing the image data
 */
QImage Data::loadImage(QWidget* parent, std::string imagePath, QSize size,
                       bool setSize, int thumbnail, bool rotation,
                       bool square, bool crop, bool force) {
    QImage image = loadImageNormal(imagePath, thumbnail, force);

    if (setSize) {
        image = image.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    ImageData* imageData = imagesData.getImageData(imagePath);
    if (crop && imageData != nullptr && !imageData->getCropSizes().empty()) {
        std::vector<QPoint> cropPoints = imageData->getCropSizes().back();

        if (cropPoints.size() == 2) {
            QRect cropRect = QRect(cropPoints[0], cropPoints[1]).normalized();

            QRect imageRect(0, 0, image.width(), image.height());
            cropRect = cropRect.intersected(imageRect);

            if (cropRect.isValid() && !image.isNull()) {
                QImage croppedImage = image.copy(cropRect);

                image = croppedImage;
            }
        }
    }

    if (square) {
        int cropSize = std::min(image.width(), image.height());
        int xOffset = (image.width() - cropSize) / 2;
        int yOffset = (image.height() - cropSize) / 2;

        image = image.copy(xOffset, yOffset, cropSize, cropSize);
    }

    if (rotation && imagePath.at(0) != ':' && imageData != nullptr && isExif(imageData->getImageExtension())) {
        image = rotateQImage(image, imageData);
    }
    return image;
}

/**
 * @brief Load an image from a path
 * @param imagePath Path to the image
 * @param thumbnail Thumbnail size (0 for no thumbnail)
 * @param force Set if the image should be loaded even if it is already in the cache
 * @return QImage object containing the image data
 */
QImage Data::loadAnImageFromRessources(std::string imagePath, int thumbnail) {
    QImage image;
    if (getDarkMode()) {
        std::string newPath = imagePath;
        newPath.insert(newPath.find_first_of(':') + 1, "/255-255-255-255");
        if (QResource(QString::fromStdString(newPath)).isValid()) {
            imagePath = newPath;
        }

    } else {
        std::string newPath = imagePath;
        newPath.insert(newPath.find_first_of(':') + 1, "/0-0-0-255");
        if (QResource(QString::fromStdString(newPath)).isValid()) {
            imagePath = newPath;
        }
    }
    image.load(QString::fromStdString(imagePath));
    return image;
}

/**
 * @brief Load an image from a path and put it in the cache
 * @param imagePath Path to the image
 * @param thumbnail Thumbnail size (0 for no thumbnail)
 * @param force Set if the image should be loaded even if it is already in the cache
 * @return QImage object containing the image data
 */
QImage Data::loadImageNormal(std::string imagePath, int thumbnail, bool force) {
    QImage image;
    std::string thumbnailPath = imagePath;

    if (imagePath.at(0) == ':') {
        image = loadAnImageFromRessources(imagePath, thumbnail);
    } else {
        for (int size : Const::Thumbnail::THUMBNAIL_SIZES) {
            if (thumbnail == size) {
                if (hasThumbnail(imagePath, size)) {
                    thumbnailPath = getThumbnailPath(imagePath, size);
                } else {
                    for (int sizeBis : Const::Thumbnail::THUMBNAIL_SIZES) {
                        createThumbnailIfNotExists(imagePath, sizeBis);
                    }
                }
            }
        }

        image = getImageFromCacheConst(thumbnailPath);
        if (!image.isNull() && !force) {
            return image;
        } else {
            image = loadAnImage(thumbnailPath);
        }
    }

    this->addImageInCache(imagePath, thumbnailPath, image);

    return image;
}

/**
 * @brief Load an image and put it in the cache asynchronously
 * @param imagePath The path to the image
 * @param callback The callback function to call after loading the image
 * @param setSize Set if the image should be resized
 * @param size size of the image if it should be resized
 * @param force Force the image to be loaded
 */
void Data::loadInCacheAsync(std::string imagePath, std::function<void()> callback, bool setSize, QSize size, int thumbnail, bool force) {
    std::string thumbnailPath = getThumbnailPath(imagePath, thumbnail);

    if (!isInCache(thumbnailPath)) {
        auto self = std::weak_ptr<Data>(shared_from_this());
        addHeavyThreadToFront([self, callback, imagePath, setSize, size, force, thumbnail]() {
            if (auto sharedSelf = self.lock()) {
                QImage image = sharedSelf->loadImageNormal(imagePath, thumbnail, force);

                if (callback) {
                    QMetaObject::invokeMethod(QApplication::instance(), callback, Qt::QueuedConnection);
                }
            } else {
                qWarning() << "Data object is no longer valid. Skipping loadInCacheAsync.";
            }
        });
    }
}

/**
 * @brief Load an image and put it in the cache
 * @param imagePath The path to the image
 * @param setSize Set if the image should be resized
 * @param size size of the image if it should be resized
 * @param force Force the image to be loaded
 * @return if the image is well loaded
 */
bool Data::loadInCache(const std::string imagePath, bool setSize,
                       const QSize size, bool force) {
    QImage image;
    if (!force && isInCache(imagePath)) {
        return true;
    }
    try {
        loadImageNormal(imagePath, 0, true);
    } catch (const std::exception& e) {
        qCritical() << "loadInCache : " << e.what();
    }

    return true;
}

/**
 * @brief Check if an image is loaded in cache or not
 * @param imagePath The path to the image
 * @return true if the image is in cache false otherwise
 */
bool Data::isInCache(std::string imagePath) {
    {
        std::lock_guard<std::mutex> lock(imageCacheMutex);

        std::unordered_map<std::string, QImageAndPath>::iterator it;
        it = imageCache->find(imagePath);
        return it != imageCache->end();
    }
}

/**
 * @brief Get if an image is loaded in the cache and return it
 * @param imagePath Path to the image
 * @param image image to get
 * @return true if the image is in the cache false otherwise
 */
QImage* Data::getImageFromCache(std::string imagePath) {
    {
        std::lock_guard<std::mutex> lock(imageCacheMutex);

        auto it = imageCache->find(imagePath);
        if (it != imageCache->end()) {
            return &it->second.image;
        }
        return nullptr;
    }
}

/**
 * @brief Get if an image is loaded in the cache and return it
 * @param imagePath Path to the image
 * @param image image to get
 * @return true if the image is in the cache false otherwise
 */
QImage Data::getImageFromCacheConst(std::string imagePath) const {
    {
        std::lock_guard<std::mutex> lock(imageCacheMutex);

        auto it = imageCache->find(imagePath);
        if (it != imageCache->end()) {
            return it->second.image;
        }
        return QImage();
    }
}

void Data::addImageInCache(const std::string& imagePath, const std::string& thumbnailPath, const QImage& image) {
    {
        std::lock_guard<std::mutex> lock(imageCacheMutex);

        (*imageCache)[thumbnailPath].image = image;
        (*imageCache)[thumbnailPath].imagePath = imagePath;
        size_t cacheSize = 0;

        for (const auto& pair : *imageCache) {
            cacheSize += pair.second.image.sizeInBytes();
        }
        if (static_cast<double>(cacheSize) / (1024 * 1024) > 5000) {
            qCritical() << "Image cache size: " << static_cast<double>(cacheSize) / (1024 * 1024) << " MB";
        }
    }
}

/**
 * @brief Create thumbnails for a specific size
 * @param imagePaths Path to the full size images
 * @param maxDim Maximum dimension for the thumbnails
 */
void Data::createThumbnails(const std::vector<std::string>& imagePaths,
                            const int maxDim) {
    for (const auto& imagePath : imagePaths) {
        createThumbnail(imagePath, maxDim);
    }
}

/**
 * @brief Create a thumbnail for a specific size asynchronously
 * @param imagePath Path to the full size image
 * @param maxDim Maximum dimension for the thumbnail
 * @param callback Callback function to call after creating the thumbnail it give you a boolean
 *                true if the thumbnail is created false otherwise
 */
void Data::createThumbnailAsync(const std::string& imagePath, const int maxDim, std::function<void(bool)> callback) {
    addHeavyThreadToFront([this, imagePath, maxDim, callback]() {
        bool success = createThumbnailIfNotExists(imagePath, maxDim);
        unloadFromCache(imagePath);

        if (callback) {
            callback(success);
        }
    });
}

/**
 * @brief Create all thumbnails asynchronously
 * @param imagePath Path to the full size image
 * @param callback Callback function to call after creating thumbnails it give you a boolean
 *                true if the thumbnails are created false otherwise
 */
void Data::createAllThumbnailsAsync(const std::string& imagePath, std::function<void(bool)> callback, bool toFront) {
    if (toFront) {
        addHeavyThreadToFront([this, imagePath, callback]() {
            bool success = true;
            for (auto size : Const::Thumbnail::THUMBNAIL_SIZES) {
                if (!createThumbnailIfNotExists(imagePath, size)) {
                    success = false;
                    qCritical() << "Error: Could not create thumbnail for image: " << QString::fromStdString(imagePath) << " size: " << size;
                }
            }
            unloadFromCache(imagePath);

            if (callback) {
                callback(success);
            }
        });
    } else {
        addHeavyThread([this, imagePath, callback]() {
            bool success = true;
            for (auto size : Const::Thumbnail::THUMBNAIL_SIZES) {
                if (!createThumbnailIfNotExists(imagePath, size)) {
                    success = false;
                    qCritical() << "Error: Could not create thumbnail for image: " << QString::fromStdString(imagePath) << " size: " << size;
                }
            }
            unloadFromCache(imagePath);

            if (callback) {
                callback(success);
            }
        });
    }
}

/**
 * @brief Create a thumbnail for a specific size
 * @param imagePath Path to the full size image
 * @param maxDim Maximum for the thumbnail
 * @return true if the thumbnail is successfuly created false otherwise
 */
bool Data::createThumbnail(const std::string& imagePath, const int maxDim) {
    try {
        QImage image;
        if (isImage(imagePath) || isVideo(imagePath)) {
            qDebug() << "createThumbnail";
            image = loadImageNormal(imagePath, 0);
        } else {
            return false;
        }

        if (image.isNull()) {
            qCritical() << "Error: Could not load image for thumbnail: " << QString::fromStdString(imagePath);
            return false;
        }

        double scale = std::min(static_cast<double>(maxDim) / image.width(),
                                static_cast<double>(maxDim) / image.height());

        QImage thumbnail = image.scaled(image.width() * scale, image.height() * scale,
                                        Qt::KeepAspectRatio);

        std::string outputImage = getThumbnailPath(imagePath, maxDim);

        if (!fs::exists(fs::path(outputImage).parent_path())) {
            fs::create_directories(fs::path(outputImage).parent_path());
        }

        if (!thumbnail.save(QString::fromStdString(outputImage))) {
            qCritical() << "Error: Could not save thumbnail: " << outputImage;
            return false;
        }

    } catch (const std::exception& e) {
        qCritical() << "createThumbnail : " << e.what();
        return false;
    }
    return true;
}

/**
 * @brief Create a thumbnail for a specific size
 * @param imagePath Path to the full size image
 * @param maxDim Maximum dimension for the thumbnail
 * @return true if the thumbnail is successfuly created false otherwise
 */
bool Data::deleteThumbnail(const std::string& imagePath, const int maxDim) {
    std::string thumbnailPath = getThumbnailPath(imagePath, maxDim);

    if (fs::exists(thumbnailPath)) {
        return fs::remove(thumbnailPath);
    }
    return false;
}

/**
 * @brief Create thumbnails for a specific size if it doesn't exist alread
 * @param imagePaths Path to the full size images
 * @param maxDim Maximum dimension for the thumbnails
 */
void Data::createThumbnailsIfNotExists(
    const std::vector<std::string>& imagePaths, const int maxDim) {
    for (const auto& imagePath : imagePaths) {
        createThumbnailIfNotExists(imagePath, maxDim);
    }
}

/**
 * @brief Create a thumbnail for a specific size if it doesn't exist alread
 * @param imagePath Path to the full size image
 * @param maxDim Maximum dimension for the thumbnail
 * @return true if the thumbnail exist false otherwise
 */
bool Data::createThumbnailIfNotExists(const std::string& imagePath,
                                      const int maxDim) {
    if (!hasThumbnail(imagePath, maxDim)) {
        return createThumbnail(imagePath, maxDim);
    }
    return true;
}

/**
 * @brief Check if an image as a thumbnail for a specific size
 * @param imagePath Path to the full size image
 * @param maxDim The dimension to check
 * @return true if the thumbnail exists false otherwise
 */
bool Data::hasThumbnail(const std::string& imagePath, const int maxDim) {
    std::string thumbnailPath = getThumbnailPath(imagePath, maxDim);
    return fs::exists(thumbnailPath);
}

/**
 * @brief Create all thumbnails sizes smaller or equal to size if they doesn't exist
 * @param imagePath Path to the full size image
 * @param size max size to load
 */
void Data::createAllThumbnailIfNotExists(const std::string& imagePath, const int size) {
    for (int thumbnailSize : Const::Thumbnail::THUMBNAIL_SIZES) {
        if (size >= thumbnailSize) {
            createThumbnailIfNotExists(imagePath, thumbnailSize);
        }
    }
}

/**
 * @brief Create all thumbnails sizes smaller or equal to size
 * @param imagePath Path to the full size image
 * @param size max size to load
 */
void Data::createAllThumbnail(const std::string& imagePath, const int size) {
    for (int thumbnailSize : Const::Thumbnail::THUMBNAIL_SIZES) {
        if (size >= thumbnailSize) {
            createThumbnail(imagePath, thumbnailSize);
        }
    }
}

/**
 * @brief Get the thumbnail path for a specific image and size
 * @param imagePath Path to the full size image
 * @param size Size of the thumbnail
 * @return Path to the thumbnail image
 */
std::string Data::getThumbnailPath(const std::string& imagePath,
                                   const int size) {
    if (size <= 0) {
        return imagePath;
    }
    std::hash<std::string> hasher;
    size_t hashValue = hasher(fs::path(imagePath).filename().string());
    std::string extension = ".webp";

    std::string thumbnailPath = THUMBNAIL_PATH + "/" + std::to_string(size) + "/" +
                                std::to_string(hashValue) + extension;

    return thumbnailPath;
}

/**
 * @brief Unload an image from the cache
 * @param imagePath Path to the image
 * @return true if the image is unloaded from the cache false otherwise
 */
bool Data::unloadFromCache(std::string imagePath) {
    {
        std::lock_guard<std::mutex> lock(imageCacheMutex);

        if (!imageCache) {
            qWarning() << "imageCache is not initialized : " << imagePath;
            return false;
        }

        auto it = imageCache->find(imagePath);

        if (it != imageCache->end()) {
            imageCache->erase(it);
            return true;
        }
    }
    return false;
}

/**
 * @brief Export all images with the folder arborecence
 * @param exportPath Path for the export
 * @param dateInName Parameter for the image Name to use the date in the name
 */
void Data::exportImages(std::string exportPath, bool dateInName) {
    Folders* firstFolder;

    rootFolders.print();

    firstFolder = findFirstFolderWithAllImages();

    exportPath += "/" + firstFolder->getName();

    createFolders(firstFolder, exportPath);

    copyTo(rootFolders, exportPath, dateInName);
}

/**
 * @brief Find the fist folder that contain all images starting with rootFolders
 * @return First Folder with aller images
 */
Folders* Data::findFirstFolderWithAllImages() {
    Folders* firstFolder = &rootFolders;
    firstFolder = findFirstFolderWithAllImagesSub(&rootFolders);
    return firstFolder;
}

/**
 * @brief Find the fist folder that contain all images
 * @param currentF Dossier where we are looking for the first folder with all images
 */
Folders* Data::findFirstFolderWithAllImagesSub(Folders* currentF) {
    if (currentF->getFoldersConst().size() == 1 && currentF->getFilesConst().size() == 0) {
        return findFirstFolderWithAllImagesSub(currentF->getFolder(0));
    }
    return currentF;
}

/**
 * @brief Copy all images to a destination path with the folder structure
 * @param rootFolders Root folder to start the copy
 * @param destinationPath Destination path for the copy
 * @param dateInName Parameter for the image Name to use the date in the name
 * @details It will create the folders if they don't exist
 */
void Data::copyTo(Folders rootFolders, std::string destinationPath, bool dateInName) {
    std::string initialFolder = fs::path(destinationPath).filename().string();

    QProgressDialog progressDialog("Exporting images...", "Cancel", 0, imagesData.getCurrent()->size());
    progressDialog.setWindowModality(Qt::ApplicationModal);
    progressDialog.show();

    int progress = 0;

    for (auto* imageData : imagesData.getConst()) {
        for (auto& folder : imageData->getFolders()) {
            std::string fileName = fs::path(imageData->getImagePath()).filename().string();

            std::string folderName = folder.getName();

            size_t pos = folderName.find(initialFolder);
            if (pos != std::string::npos) {
                folderName = folderName.substr(pos + initialFolder.length());
            }

            std::string destinationFile;

            if (dateInName) {
                imageData->loadData();
                Exiv2::ExifData exifData = imageData->getMetaDataPtr()->getExifData();
                if (exifData["Exif.Image.DateTime"].count() != 0) {
                    std::string date = exifData["Exif.Image.DateTime"].toString();
                    std::replace(date.begin(), date.end(), ':', '-');
                    std::replace(date.begin(), date.end(), ' ', '_');
                    destinationFile = destinationPath + "/" + folderName + "/" + date + "_" + fileName;
                } else {
                    destinationFile = destinationPath + "/" + folderName + "/" + "no_date" + "_" + fileName;
                }
            } else {
                destinationFile = destinationPath + "/" + folderName + "/" + fileName;
            }

            if (!imageData->getCropSizes().empty() || imageData->hasExtension()) {
                if (imageData->hasExtension()) {
                    // Change the extension of destinationFile to imageData->getExtension()
                    std::string newExtension = imageData->getExtension();
                    std::string::size_type dotPos = destinationFile.find_last_of('.');
                    if (dotPos != std::string::npos) {
                        destinationFile = destinationFile.substr(0, dotPos) + newExtension;
                    } else {
                        destinationFile += newExtension;
                    }
                    // qDebug() << "Copying image to extension : " << QString::fromStdString(destinationFile);
                }

                QImage image = loadAnImage(imageData->getImagePath());

                if (image.isNull()) {
                    continue;
                }
                if (!imageData->getCropSizes().empty()) {
                    std::vector<QPoint> cropPoints = imageData->getCropSizes().back();
                    if (cropPoints.size() == 2) {
                        QRect cropRect = QRect(cropPoints[0], cropPoints[1]).normalized();
                        image = image.copy(cropRect);
                    }
                }

                saveAnImage(destinationFile, image);

                std::unique_ptr<Exiv2::Image> srcImage = Exiv2::ImageFactory::open(imageData->getImagePath());
                srcImage->readMetadata();
                std::unique_ptr<Exiv2::Image> destImage = Exiv2::ImageFactory::open(destinationFile);
                destImage->setExifData(srcImage->exifData());
                destImage->setIptcData(srcImage->iptcData());
                destImage->setXmpData(srcImage->xmpData());
                destImage->writeMetadata();
            }

            else {
                QFile::copy(QString::fromStdString(imageData->getImagePath()), QString::fromStdString(destinationFile));
            }
            if (progressDialog.wasCanceled()) {
                return;
            }
            progressDialog.setValue(progress++);
            QApplication::processEvents();
        }
    }
}

/**
 * @brief Rotate an image according to the metadata (in imageData)
 * @param image Image to rotate
 * @param imageData ImageData object containing the metadata
 * @return Rotated image
 * @details The rotation is done according to the Exif orientation tag
 */
QImage rotateQImage(QImage image, ImageData* imageData) {
    if (imageData != nullptr) {
        int orientation = imageData->getOrientation();

        switch (orientation) {
            case Const::Orientation::FLIP_HORIZONTAL:
                image = image.flipped(Qt::Horizontal);
                break;
            case Const::Orientation::ROTATE_180:
                image = image.transformed(QTransform().rotate(Const::Rotation::UP_SIDE_DOWN));
                break;
            case Const::Orientation::FLIP_VERTICAL:
                image = image.flipped(Qt::Vertical);
                break;
            case Const::Orientation::TRANSPOSE:
                image = image.flipped(Qt::Horizontal).transformed(QTransform().rotate(-90));
                break;
            case Const::Orientation::ROTATE_90:
                image = image.transformed(QTransform().rotate(90));
                break;
            case Const::Orientation::TRANSVERSE:
                image = image.flipped(Qt::Horizontal).transformed(QTransform().rotate(90));
                break;
            case Const::Orientation::ROTATE_270:
                image = image.transformed(QTransform().rotate(-90));
                break;
            default:
                break;
        }
    }
    return image;
}

/**
 * @brief Create folders for the images
 * @param currentFolders Current folder to create the folders in
 * @param folderPath Path to the folder to create
 */
void Data::createFolders(Folders* currentFolders, std::string folderPath) {
    std::string initialFolderPath = folderPath;
    if (!fs::exists(initialFolderPath)) {
        fs::create_directories(initialFolderPath);
    }

    for (auto folder : *currentFolders->getFolders()) {
        folderPath = initialFolderPath + "/" + folder.getName();
        if (!fs::exists(folderPath)) {
            fs::create_directories(folderPath);
        }
        createFolders(&folder, folderPath);
    }
}

/**
 * @brief Save the data to a file
 */
void Data::saveData() {
    qInfo() << "Saving data";
    std::ofstream outFile(IMAGESDATA_SAVE_PATH, std::ios::binary);
    if (!outFile) {
        if (!fs::exists(fs::path(IMAGESDATA_SAVE_PATH).parent_path())) {
            if (!fs::create_directories(fs::path(IMAGESDATA_SAVE_PATH).parent_path())) {
                qCritical() << "Couldn't create directories for save file : " << fs::path(IMAGESDATA_SAVE_PATH).parent_path();
                return;
            }
        }

        outFile.open(IMAGESDATA_SAVE_PATH, std::ios::binary);
        if (!outFile) {
            qCritical() << "Couldn't open save file : " << IMAGESDATA_SAVE_PATH;
            return;
        }
    }

    model.save(outFile);

    size_t imagesDataSize = imagesData.get()->size();
    outFile.write(reinterpret_cast<const char*>(&imagesDataSize), sizeof(imagesDataSize));
    for (auto* imageData : *imagesData.get()) {
        imageData->save(outFile);
    }

    size_t deletedImagesDataSize = deletedImagesData.get()->size();
    outFile.write(reinterpret_cast<const char*>(&deletedImagesDataSize), sizeof(deletedImagesDataSize));
    for (auto* imageData : *deletedImagesData.get()) {
        imageData->save(outFile);
    }

    rootFolders.save(outFile);

    std::string currentFolderPath = getFolderPath(currentFolder);
    size_t pathSize = currentFolderPath.size();
    outFile.write(reinterpret_cast<const char*>(&pathSize), sizeof(pathSize));
    outFile.write(currentFolderPath.c_str(), pathSize);

    // Save personIdNames map
    size_t personIdNamesSize = personIdNames.size();
    outFile.write(reinterpret_cast<const char*>(&personIdNamesSize), sizeof(personIdNamesSize));
    for (const auto& [id, name] : personIdNames) {
        outFile.write(reinterpret_cast<const char*>(&id), sizeof(id));
        size_t nameSize = name.size();
        outFile.write(reinterpret_cast<const char*>(&nameSize), sizeof(nameSize));
        outFile.write(name.c_str(), nameSize);
    }

    outFile.close();

    // Save options to a separate file
    {
        std::ofstream optionsFile(OPTIONS_SAVE_PATH, std::ios::binary);
        if (!optionsFile) {
            if (!fs::exists(fs::path(OPTIONS_SAVE_PATH).parent_path())) {
                fs::create_directories(fs::path(OPTIONS_SAVE_PATH).parent_path());
            }
            optionsFile.open(OPTIONS_SAVE_PATH, std::ios::binary);
            if (!optionsFile) {
                qCritical() << "Couldn't open options save file : " << OPTIONS_SAVE_PATH;
            }
        }
        size_t optionsSize = options.size();
        optionsFile.write(reinterpret_cast<const char*>(&optionsSize), sizeof(optionsSize));
        for (const auto& [key, option] : options) {
            size_t keySize = key.size();
            optionsFile.write(reinterpret_cast<const char*>(&keySize), sizeof(keySize));
            optionsFile.write(key.c_str(), keySize);

            size_t typeSize = option.getTypeConst().size();
            optionsFile.write(reinterpret_cast<const char*>(&typeSize), sizeof(typeSize));
            optionsFile.write(option.getTypeConst().c_str(), typeSize);

            size_t valueSize = option.getValueConst().size();
            optionsFile.write(reinterpret_cast<const char*>(&valueSize), sizeof(valueSize));
            optionsFile.write(option.getValueConst().c_str(), valueSize);
        }
        optionsFile.close();
    }
    qInfo() << "data saved";
}

/**
 * @brief Load the data from a file
 */
void Data::loadData() {
    std::ifstream inFile(IMAGESDATA_SAVE_PATH, std::ios::binary);
    if (inFile) {
        model.load(inFile);

        size_t imagesDataSize;
        inFile.read(reinterpret_cast<char*>(&imagesDataSize), sizeof(imagesDataSize));
        for (size_t i = 0; i < imagesDataSize; ++i) {
            ImageData* imageData = new ImageData();
            imageData->load(inFile);
            imagesData.get()->push_back(imageData);
        }

        int index = 0;
        for (auto* imageData : *imagesData.get()) {
            imagesData.setImageMapValue(imageData->getImagePath(), imageData);
            index++;
        }

        size_t deletedImagesDataSize;
        inFile.read(reinterpret_cast<char*>(&deletedImagesDataSize), sizeof(deletedImagesDataSize));
        for (size_t i = 0; i < deletedImagesDataSize; ++i) {
            ImageData* imageData = new ImageData();
            imageData->load(inFile);
            deletedImagesData.get()->push_back(imageData);
        }

        rootFolders.load(inFile);

        size_t pathSize;
        inFile.read(reinterpret_cast<char*>(&pathSize), sizeof(pathSize));
        std::string currentFolderPath(pathSize, '\0');
        inFile.read(&currentFolderPath[0], pathSize);
        currentFolder = findFolderByPath(rootFolders, currentFolderPath);

        // Load personIdNames map
        size_t personIdNamesSize;
        inFile.read(reinterpret_cast<char*>(&personIdNamesSize), sizeof(personIdNamesSize));
        for (size_t i = 0; i < personIdNamesSize; ++i) {
            int id;
            size_t nameSize;
            std::string name;
            inFile.read(reinterpret_cast<char*>(&id), sizeof(id));
            inFile.read(reinterpret_cast<char*>(&nameSize), sizeof(nameSize));
            name.resize(nameSize);
            inFile.read(&name[0], nameSize);
            personIdNames[id] = name;
        }

        inFile.close();
    }
    if (!options.empty()) {
        options.clear();
    }
    std::ifstream optionsFile(OPTIONS_SAVE_PATH, std::ios::binary);
    if (optionsFile) {
        size_t optionsSize;
        optionsFile.read(reinterpret_cast<char*>(&optionsSize), sizeof(optionsSize));
        for (size_t i = 0; i < optionsSize; ++i) {
            std::string key, type, value;
            size_t keySize, typeSize, valueSize;
            optionsFile.read(reinterpret_cast<char*>(&keySize), sizeof(keySize));
            key.resize(keySize);
            optionsFile.read(&key[0], keySize);

            optionsFile.read(reinterpret_cast<char*>(&typeSize), sizeof(typeSize));
            type.resize(typeSize);
            optionsFile.read(&type[0], typeSize);

            optionsFile.read(reinterpret_cast<char*>(&valueSize), sizeof(valueSize));
            value.resize(valueSize);
            optionsFile.read(&value[0], valueSize);

            options[key] = Option(type, value);
        }
        optionsFile.close();
    } else {
        options = DEFAULT_OPTIONS;
    }
}

/**
 * @brief Add an action to the undo/redo stack
 * @param unDo Action for undo
 * @param reDo Action for redo
 */
void Data::addAction(std::function<void()> unDo, std::function<void()> reDo) {
    Actions action;
    action.unDo = unDo;
    action.reDo = reDo;
    lastActions.emplace_back(action);
    if (lastActions.size() > 100) {
        lastActions.erase(lastActions.begin());
    }
}

/**
 * @brief Save an action as done
 * @param action Action to save as done
 */
void Data::addActionDone(Actions action) {
    lastActionsDone.emplace_back(action);
    if (lastActionsDone.size() > 100) {
        lastActionsDone.erase(lastActionsDone.begin());
    }
}

/**
 * @brief Do the last action in the redo stack
 */
void Data::reDoAction() {
    try {
        if (lastActionsDone.size() > 0) {
            auto action = lastActionsDone.back();
            addAction(action.unDo, action.reDo);
            lastActionsDone.pop_back();
            action.reDo();
        }
    } catch (const std::exception& e) {
        qCritical() << e.what();
    }
}

/**
 * @brief Do the last action in the undo stack
 */
void Data::unDoAction() {
    try {
        if (lastActions.size() > 0) {
            auto action = lastActions.back();
            addActionDone(action);
            lastActions.pop_back();
            action.unDo();
        }

    } catch (const std::exception& e) {
        qCritical() << e.what();
    }
}

/**
 * @brief Sort the current images data by date
 */
void Data::sortCurrentImagesData() {
    QElapsedTimer timer;
    timer.start();

    QProgressDialog progressDialog = QProgressDialog(nullptr);
    progressDialog.move(0, 0);
    progressDialog.setWindowModality(Qt::ApplicationModal);
    int progress = 0;

    progressDialog.setLabelText("Sorting ...");
    progressDialog.setValue(0);
    int n = imagesData.getCurrent()->size();

    int estimatedSteps = static_cast<int>(n * std::log(n) * 3);
    progressDialog.setMaximum(estimatedSteps);
    progressDialog.hide();

    QApplication::processEvents();

    auto& data = *imagesData.getCurrent();
    std::sort(data.begin(), data.end(), [&progress, &progressDialog, &timer](const ImageData* firstImageData, const ImageData* secondImageData) {
        progress++;
        progressDialog.setValue(progress);
        QApplication::processEvents();
        if (timer.elapsed() > 1000) {
            progressDialog.show();
            timer.invalidate();
        }
        return firstImageData->getDate() > secondImageData->getDate();
    });
    timer.invalidate();
}

/**
 * @brief Clear the image cache
 * @details This will remove all images from the cache
 */
void Data::clearCache() {
    {
        std::lock_guard<std::mutex> lock(imageCacheMutex);

        imageCache->clear();
    }
}

/**
 * @brief Rotate an image to the left
 * @param nbr Image number to rotate (index in the imagesData)
 * @param extension Image extension
 * @param reload Function to reload
 * @param action Set if the action should be added to the undo stack
 */
void Data::rotateLeft(int nbr, std::string extension, std::function<void()> reload, bool action) {
    // nbr = nbrInTotal
    if (isExif(extension)) {
        exifRotate(nbr, Const::Rotation::LEFT, reload);
        if (action) {
            addAction(
                [this, nbr, reload]() {
                    int time = 0;
                    if (imagesData.getImageNumberInTotal() != nbr) {
                        imagesData.setImageNumber(nbr);
                        reload();
                        time = TIME_UNDO_VISUALISATION;
                    }
                    QTimer::singleShot(time, [this, nbr, reload]() {
                        exifRotate(nbr, Const::Rotation::RIGHT, reload);
                    });
                },
                [this, nbr, reload]() {
                    int time = 0;
                    if (imagesData.getImageNumberInTotal() != nbr) {
                        imagesData.setImageNumber(nbr);
                        reload();
                        time = TIME_UNDO_VISUALISATION;
                    }
                    QTimer::singleShot(time, [this, nbr, reload]() {
                        exifRotate(nbr, Const::Rotation::LEFT, reload);
                    });
                });
        }
    } else if (isReal(extension)) {
        realRotate(nbr, Const::Rotation::LEFT, reload);
        if (action) {
            addAction(
                [this, nbr, reload]() {
                    int time = 0;
                    if (imagesData.getImageNumberInTotal() != nbr) {
                        imagesData.setImageNumber(nbr);
                        reload();
                        time = TIME_UNDO_VISUALISATION;
                    }
                    QTimer::singleShot(time, [this, nbr, reload]() {
                        realRotate(nbr, Const::Rotation::RIGHT, reload);
                    });
                },
                [this, nbr, reload]() {
                    int time = 0;
                    if (imagesData.getImageNumberInTotal() != nbr) {
                        imagesData.setImageNumber(nbr);
                        reload();
                        time = TIME_UNDO_VISUALISATION;
                    }
                    QTimer::singleShot(time, [this, nbr, reload]() {
                        realRotate(nbr, Const::Rotation::LEFT, reload);
                    });
                });
        }
    }
}

/**
 * @brief Rotate an image to the right
 * @param nbr Image number to rotate (index in the imagesData)
 * @param extension Image extension
 * @param reload Function to reload
 * @param action Set if the action should be added to the undo stack
 */
void Data::rotateRight(int nbr, std::string extension, std::function<void()> reload, bool action) {
    // nbr = nbrInTotal

    if (isExif(extension)) {
        exifRotate(nbr, Const::Rotation::RIGHT, reload);

        if (action) {
            addAction(
                [this, nbr, reload]() {
                    int time = 0;
                    if (imagesData.getImageNumberInTotal() != nbr) {
                        imagesData.setImageNumber(nbr);
                        reload();
                        time = TIME_UNDO_VISUALISATION;
                    }
                    QTimer::singleShot(time, [this, nbr, reload]() {
                        exifRotate(nbr, Const::Rotation::LEFT, reload);
                    });
                },
                [this, nbr, reload]() {
                    int time = 0;
                    if (imagesData.getImageNumberInTotal() != nbr) {
                        imagesData.setImageNumber(nbr);
                        reload();
                        time = TIME_UNDO_VISUALISATION;
                    }
                    QTimer::singleShot(time, [this, nbr, reload]() {
                        exifRotate(nbr, Const::Rotation::RIGHT, reload);
                    });
                });
        }
    } else if (isReal(extension)) {
        realRotate(nbr, Const::Rotation::RIGHT, reload);
        if (action) {
            addAction(
                [this, nbr, reload]() {
                    int time = 0;
                    if (imagesData.getImageNumberInTotal() != nbr) {
                        imagesData.setImageNumber(nbr);
                        reload();
                        time = TIME_UNDO_VISUALISATION;
                    }
                    QTimer::singleShot(time, [this, nbr, reload]() {
                        realRotate(nbr, Const::Rotation::LEFT, reload);
                    });
                },
                [this, nbr, reload]() {
                    int time = 0;
                    if (imagesData.getImageNumberInTotal() != nbr) {
                        imagesData.setImageNumber(nbr);
                        reload();
                        time = TIME_UNDO_VISUALISATION;
                    }
                    QTimer::singleShot(time, [this, nbr, reload]() {
                        realRotate(nbr, Const::Rotation::RIGHT, reload);
                    });
                });
        }
    }
}

/**
 * @brief Rotate an image to the left or right (real rotation not metadata rotation)
 * @param nbr Image number to rotate (index in the imagesData)
 * @param rotation Rotation to apply (in degrees)
 * @param reload Function to reload
 */
void Data::realRotate(int nbr, int rotation, std::function<void()> reload) {
    QString outputPath = QString::fromStdString(imagesData.getImageData(nbr)->getImagePath());
    QImage image = loadImage(nullptr, imagesData.getImageData(nbr)->getImagePath(), QSize(0, 0), false);
    image = image.transformed(QTransform().rotate(rotation));
    if (!image.save(outputPath)) {
        qWarning() << "Erreur lors de la sauvegarde de l'image : " << outputPath;
    }
    unloadFromCache(imagesData.getImageData(nbr)->getImagePath());
    loadInCache(imagesData.getImageData(nbr)->getImagePath());
    createAllThumbnail(imagesData.getImageData(nbr)->getImagePath(), Const::Thumbnail::HIGHT_QUALITY);
    reload();
}

/**
 * @brief Rotate an image to the left or right (metadata rotation)
 * @param nbr Image number to rotate (index in the imagesData)
 * @param rotation Rotation to apply (in degrees, 90 or -90)
 * @param reload Function to reload
 */
void Data::exifRotate(int nbr, int rotation, std::function<void()> reload) {
    ImageData* imageData = imagesData.getImageData(nbr);
    imageData = this->getImagesDataPtr()->getImageData(imageData->getImagePath());

    if (!isTurnable(imageData->getImagePath())) {
        qWarning() << "Image not turnable : " << imageData->getImagePath();
        return;
    }
    int orientation = imageData->getOrientation();
    if (rotation == Const::Rotation::LEFT) {
        switch (orientation) {
            case Const::Orientation::NORMAL:
                orientation = Const::Orientation::ROTATE_270;
                break;
            case Const::Orientation::FLIP_HORIZONTAL:
                orientation = Const::Orientation::TRANSVERSE;
                break;
            case Const::Orientation::ROTATE_180:
                orientation = Const::Orientation::ROTATE_90;
                break;
            case Const::Orientation::FLIP_VERTICAL:
                orientation = Const::Orientation::TRANSPOSE;
                break;
            case Const::Orientation::TRANSPOSE:
                orientation = Const::Orientation::FLIP_HORIZONTAL;
                break;
            case Const::Orientation::ROTATE_90:
                orientation = Const::Orientation::NORMAL;
                break;
            case Const::Orientation::TRANSVERSE:
                orientation = Const::Orientation::FLIP_VERTICAL;
                break;
            case Const::Orientation::ROTATE_270:
                orientation = Const::Orientation::ROTATE_180;
                break;
            default:
                orientation = Const::Orientation::NORMAL;
                break;
        }
    }
    if (rotation == Const::Rotation::RIGHT) {
        switch (orientation) {
            case Const::Orientation::NORMAL:
                orientation = Const::Orientation::ROTATE_90;
                break;
            case Const::Orientation::FLIP_HORIZONTAL:
                orientation = Const::Orientation::TRANSPOSE;
                break;
            case Const::Orientation::ROTATE_180:
                orientation = Const::Orientation::ROTATE_270;
                break;
            case Const::Orientation::FLIP_VERTICAL:
                orientation = Const::Orientation::TRANSVERSE;
                break;
            case Const::Orientation::TRANSPOSE:
                orientation = Const::Orientation::FLIP_VERTICAL;
                break;
            case Const::Orientation::ROTATE_90:
                orientation = Const::Orientation::ROTATE_180;
                break;
            case Const::Orientation::TRANSVERSE:
                orientation = Const::Orientation::FLIP_HORIZONTAL;
                break;
            case Const::Orientation::ROTATE_270:
                orientation = Const::Orientation::NORMAL;
                break;
            default:
                orientation = Const::Orientation::NORMAL;
                break;
        }
    }

    imageData->turnImage(orientation);
    imageData->saveMetaData();

    reload();
}

/**
 * @brief Mirror an image up/down (metadata rotation)
 * @param nbr Image number to mirror (index in the imagesData)
 * @param extension Image extension
 * @param reload Function to reload
 * @param action Set if the action should be added to the undo stack
 */
void Data::mirrorUpDown(int nbr, std::string extension, std::function<void()> reload, bool action) {
    // nbr = nbrInTotal
    if (isExif(extension)) {
        exifMirror(nbr, true, reload);

        if (action) {
            addAction(
                [this, nbr, reload]() {
                    int time = 0;
                    if (imagesData.getImageNumberInTotal() != nbr) {
                        imagesData.setImageNumber(nbr);
                        reload();
                        time = TIME_UNDO_VISUALISATION;
                    }
                    QTimer::singleShot(time, [this, nbr, reload]() {
                        exifMirror(nbr, true, reload);
                    });
                },
                [this, nbr, reload]() {
                    int time = 0;
                    if (imagesData.getImageNumberInTotal() != nbr) {
                        imagesData.setImageNumber(nbr);
                        reload();
                        time = TIME_UNDO_VISUALISATION;
                    }
                    QTimer::singleShot(time, [this, nbr, reload]() {
                        exifMirror(nbr, true, reload);
                    });
                });
        }
    } else if (isReal(extension)) {
        realMirror(nbr, true, reload);

        if (action) {
            addAction(
                [this, nbr, reload]() {
                    int time = 0;
                    if (imagesData.getImageNumberInTotal() != nbr) {
                        imagesData.setImageNumber(nbr);
                        reload();
                        time = TIME_UNDO_VISUALISATION;
                    }
                    QTimer::singleShot(time, [this, nbr, reload]() {
                        realMirror(nbr, true, reload);
                    });
                },
                [this, nbr, reload]() {
                    int time = 0;
                    if (imagesData.getImageNumberInTotal() != nbr) {
                        imagesData.setImageNumber(nbr);
                        reload();
                        time = TIME_UNDO_VISUALISATION;
                    }
                    QTimer::singleShot(time, [this, nbr, reload]() {
                        realMirror(nbr, true, reload);
                    });
                });
        }
    }
    setSaved(false);
}

/**
 * @brief Mirror an image left/right (metadata rotation)
 * @param nbr Image number to mirror (index in the imagesData)
 * @param extension Image extension
 * @param reload Function to reload
 * @param action Set if the action should be added to the undo stack
 */
void Data::mirrorLeftRight(int nbr, std::string extension, std::function<void()> reload, bool action) {
    // nbr = nbrInTotal

    if (isExif(extension)) {
        exifMirror(nbr, false, reload);

        if (action) {
            addAction(
                [this, nbr, reload]() {
                    int time = 0;
                    if (imagesData.getImageNumberInTotal() != nbr) {
                        imagesData.setImageNumber(nbr);
                        reload();
                        time = TIME_UNDO_VISUALISATION;
                    }
                    QTimer::singleShot(time, [this, nbr, reload]() {
                        exifMirror(nbr, false, reload);
                    });
                },
                [this, nbr, reload]() {
                    int time = 0;
                    if (imagesData.getImageNumberInTotal() != nbr) {
                        imagesData.setImageNumber(nbr);
                        reload();
                        time = TIME_UNDO_VISUALISATION;
                    }
                    QTimer::singleShot(time, [this, nbr, reload]() {
                        exifMirror(nbr, false, reload);
                    });
                });
        }
    } else if (isReal(extension)) {
        realMirror(nbr, false, reload);
        if (action) {
            addAction(
                [this, nbr, reload]() {
                    int time = 0;
                    if (imagesData.getImageNumberInTotal() != nbr) {
                        imagesData.setImageNumber(nbr);
                        reload();
                        time = TIME_UNDO_VISUALISATION;
                    }
                    QTimer::singleShot(time, [this, nbr, reload]() {
                        realMirror(nbr, false, reload);
                    });
                },
                [this, nbr, reload]() {
                    int time = 0;
                    if (imagesData.getImageNumberInTotal() != nbr) {
                        imagesData.setImageNumber(nbr);
                        reload();
                        time = TIME_UNDO_VISUALISATION;
                    }
                    QTimer::singleShot(time, [this, nbr, reload]() {
                        realMirror(nbr, false, reload);
                    });
                });
        }
    }
    setSaved(false);
}

/**
 * @brief Mirror an image up/down or left/right (metadata rotation)
 * @param nbr Image number to mirror (index in the imagesData)
 * @param UpDown True for up/down, false for left/right
 * @param reload Function to reload
 */
void Data::exifMirror(int nbr, bool UpDown, std::function<void()> reload) {
    ImageData* imageData = imagesData.getImageData(nbr);

    if (!isTurnable(imageData->getImagePath())) {
        return;
    }
    int orientation = imageData->getOrientation();
    if (UpDown) {
        switch (orientation) {
            case Const::Orientation::NORMAL:
                orientation = Const::Orientation::FLIP_VERTICAL;
                break;
            case Const::Orientation::ROTATE_180:
                orientation = Const::Orientation::FLIP_HORIZONTAL;
                break;
            case Const::Orientation::ROTATE_90:
                orientation = Const::Orientation::TRANSVERSE;
                break;
            case Const::Orientation::ROTATE_270:
                orientation = Const::Orientation::TRANSPOSE;
                break;
            case Const::Orientation::FLIP_HORIZONTAL:
                orientation = Const::Orientation::ROTATE_180;
                break;
            case Const::Orientation::FLIP_VERTICAL:
                orientation = Const::Orientation::NORMAL;
                break;
            case Const::Orientation::TRANSPOSE:
                orientation = Const::Orientation::ROTATE_270;
                break;
            case Const::Orientation::TRANSVERSE:
                orientation = Const::Orientation::ROTATE_90;
                break;
            default:
                break;
        }
    } else {
        switch (orientation) {
            case Const::Orientation::NORMAL:
                orientation = Const::Orientation::FLIP_HORIZONTAL;
                break;
            case Const::Orientation::ROTATE_180:
                orientation = Const::Orientation::FLIP_VERTICAL;
                break;
            case Const::Orientation::ROTATE_90:
                orientation = Const::Orientation::TRANSPOSE;
                break;
            case Const::Orientation::ROTATE_270:
                orientation = Const::Orientation::TRANSVERSE;
                break;
            case Const::Orientation::FLIP_HORIZONTAL:
                orientation = Const::Orientation::NORMAL;
                break;
            case Const::Orientation::FLIP_VERTICAL:
                orientation = Const::Orientation::ROTATE_180;
                break;
            case Const::Orientation::TRANSPOSE:
                orientation = Const::Orientation::ROTATE_90;
                break;
            case Const::Orientation::TRANSVERSE:
                orientation = Const::Orientation::ROTATE_270;
                break;
            default:
                break;
        }
    }

    imageData->turnImage(orientation);
    imageData->saveMetaData();
    reload();
}

/**
 * @brief Mirror an image up/down or left/right (real rotation)
 * @param nbr Image number to mirror (index in the imagesData)
 * @param UpDown True for up/down, false for left/right
 * @param reload Function to reload
 */
void Data::realMirror(int nbr, bool UpDown, std::function<void()> reload) {
    QString outputPath = QString::fromStdString(imagesData.getCurrentImageData()->getImagePathConst());
    QImage image = loadImage(nullptr, imagesData.getCurrentImageData()->getImagePathConst(), QSize(0, 0), false);
    if (UpDown) {
        image = image.flipped(Qt::Vertical);
    } else {
        image = image.flipped(Qt::Horizontal);
    }
    if (!image.save(outputPath)) {
        qWarning() << "Erreur lors de la sauvegarde de l'image : " << outputPath;
    }
    unloadFromCache(imagesData.getCurrentImageData()->getImagePathConst());
    loadInCache(imagesData.getCurrentImageData()->getImagePathConst());
    createAllThumbnail(imagesData.getCurrentImageData()->getImagePathConst(), Const::Thumbnail::HIGHT_QUALITY);
    reload();
}

/**
 * @brief Clear the actions stack
 * @details This will remove all actions from the stack
 */
void Data::clearActions() {
    lastActions.clear();
    lastActionsDone.clear();
}

/**
 * @brief Give you the root folders
 * @return The root folders
 */
Folders* Data::getRootFolders() {
    return &rootFolders;
}
/**
 * @brief Give you the current folder and correct it if it's not initialized
 * @return The current Folder
 */
Folders* Data::getCurrentFolders() {
    if (currentFolder == nullptr) {
        qWarning() << "currentFolder is null";
        currentFolder = findFirstFolderWithAllImages();
        qInfo() << "currentFolder is now : " << currentFolder->getName();
    }

    if (currentFolder->getName() == "") {
        qWarning() << "currentFolder is empty";
        currentFolder = findFirstFolderWithAllImages();
        qInfo() << "currentFolder is now : " << currentFolder->getName();
    }

    return currentFolder;
}

/**
 * @brief Set the current folder
 * @param folder Folder to set as current
 */
void Data::setCurrentFolders(Folders* folder) {
    this->currentFolder = folder;
}

/**
 * @brief Give you imagesData
 * @return Ptr to ImagesData
 */
ImagesData* Data::getImagesDataPtr() {
    return &imagesData;
}

/**
 * @brief Give you deletedImagesData
 * @return Ptr to deletedImagesData
 */
ImagesData* Data::getDeletedImagesDataPtr() {
    return &deletedImagesData;
}

/**
 * @brief
 * @return
 */
Sizes* Data::getSizesPtr() {
    return &sizes;
}

/**
 * @brief Remove an image from all folders
 * @param imageData ImageData object to remove from folders
 */
void Data::removeImageFromFolders(ImageData& imageData) {
    Folders* rootFolders = getRootFolders();
    for (auto& folder : imageData.getFolders()) {
        std::string folderPath = folder.getName();
        Folders* currentFolderBis = rootFolders;
        std::string folderPathBis = std::regex_replace(folderPath, std::regex("\\\\"), "/");
        std::istringstream iss(folderPathBis);
        std::string token;
        bool run = true;
        while (run && std::getline(iss, token, '/')) {
            auto it = std::find_if(currentFolderBis->getFolders()->begin(), currentFolderBis->getFolders()->end(),
                                   [&token](const Folders& f) { return f.getName() == token; });
            if (it != currentFolderBis->getFoldersConst().end()) {
                currentFolderBis = &(*it);
            } else {
                currentFolderBis = nullptr;
                run = false;
            }
        }
        if (currentFolderBis) {
            try {
                auto it = std::find(currentFolderBis->getFilesPtr()->begin(), currentFolderBis->getFilesPtr()->end(), imageData.getImagePath());
                if (it != currentFolderBis->getFilesPtr()->end()) {
                    currentFolder->getFilesPtr()->erase(it);
                }
            } catch (const std::exception& e) {
                qCritical() << "removeImageFromFolders" << e.what();
            }
        }
    }
}

/**
 * @brief Get the absolute path of a folder
 * @param folder Folder to get the path of
 * @return The absolute path of the folder
 */
std::string Data::getFolderPath(Folders* folder) {
    // TODO Cree une sous class data pour les dossiers
    if (folder == &rootFolders) {
        return "/";
    }

    std::string path;
    while (folder != &rootFolders) {
        path = "/" + folder->getName() + path;

        Folders* parent = folder->getParent();
        if (parent == nullptr) {
            qCritical() << "getFolderPath folder has no parent";
            return "/";
        }
        folder = parent;
    }
    return path;
}

/**
 * @brief Find a folder by its absolute path
 * @param root Root folder to search in
 * @param path Absolute path of the folder to find
 * @return Pointer to the found folder, or nullptr if not found
 */
Folders* Data::findFolderByPath(Folders& root, const std::string& path) {
    if (path == "/") {
        return &root;
    }

    Folders* current = &root;
    std::istringstream iss(path);
    std::string token;
    while (std::getline(iss, token, '/')) {
        if (token.empty()) {
            continue;
        }
        auto it = std::find_if(current->getFolders()->begin(), current->getFolders()->end(),
                               [&token](const Folders& f) { return f.getName() == token; });
        if (it != current->getFoldersConst().end()) {
            current = &(*it);
        } else {
            return nullptr;
        }
    }
    return current;
}

/**
 * @brief Clear all data
 * @details This will remove all images, folders, and options from the data
 */
void Data::clear() {
    manager.removeAllThreads();
    imagesData.clear();
    rootFolders.clear();
    currentFolder = nullptr;
    options.clear();
    lastActions.clear();
    lastActionsDone.clear();
    personIdNames.clear();
    deletedImagesData.clear();

    thumbnailTimer->disconnect();
    detectObjectTimer->disconnect();
    detectFacesTimer->disconnect();

    {
        std::lock_guard<std::mutex> lock(imageCacheMutex);

        imageCache->clear();
    }
    setSaved(true);
}

/**
 * @brief Add a thread to thread queue, a thread that should no be heavy to execute
 * @param job The function to execute
 */
void Data::addThread(std::function<void()> job) {
    manager.addThread(job);
}

/**
 * @brief Add a thread to front of the thread queue, a thread that should no be heavy to execute
 * @param job The function to execute
 */
void Data::addThreadToFront(std::function<void()> job) {
    manager.addThreadToFront(job);
}

/**
 * @brief Add an heavy thread to thread queue, a thread that can be heavy to execute
 * @param job The function to execute
 */
void Data::addHeavyThread(std::function<void()> job) {
    manager.addHeavyThread(job);
}

/**
 * @brief Add a heavy thread to front of the thread queue, a thread that can be heavy to execute
 * @param job The function to execute
 */
void Data::addHeavyThreadToFront(std::function<void()> job) {
    manager.addHeavyThreadToFront(job);
}

/**
 * @brief Stop all active thread and delete thread queue
 */
void Data::stopAllThreads() {
    qInfo() << "stop all threads";

    manager.removeAllThreads();
}

/**
 * @brief Load a network from a file
 * @param model The name of the model file (without extension)
 * @return The loaded network
 */
cv::dnn::Net load_net(std::string model) {
    bool is_cuda = false;
    if (cv::cuda::getCudaEnabledDeviceCount() > 0) {
        is_cuda = true;
    }

    cv::dnn::Net result;
    try {
        result = cv::dnn::readNet(APP_FILES.toStdString() + "/" + model);
    } catch (const cv::Exception& e) {
        qWarning() << "Error loading model:" << QString::fromStdString(model) << " - " << e.what();

        // showWarningMessage(nullptr, "Error with the model", "Error loading model\nThe model will be downloaded again.");
        if (!downloadModel(model, Const::Model::YoloV5::GITHUB_TAG)) {
            qWarning() << "Failed to download model: " << QString::fromStdString(model);
            // showErrorMessage(nullptr, "Error with the model", "Error downloading model\nThe model could not be downloaded.");
        }

        try {
            result = cv::dnn::readNet(APP_FILES.toStdString() + "/" + model);
        } catch (const cv::Exception& e) {
            qWarning() << "Error loading model:" << QString::fromStdString(model) << " - " << e.what();
            // showErrorMessage(nullptr, "Error with the model", "Error loading model\nThe model is broken.");
        }
    }

    if (is_cuda) {
        result.setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA);
        result.setPreferableTarget(cv::dnn::DNN_TARGET_CUDA_FP16);
    } else {
        result.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
        result.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
    }
    return result;
}

/**
 * @brief Load the class list from a file
 * @return A vector of class names
 */
std::vector<std::string> load_class_list() {
    std::vector<std::string> class_list;
    downloadModelIfNotExists(Const::Model::YoloV5::Names::CLASS, Const::Model::YoloV5::GITHUB_TAG);
    std::ifstream ifs(APP_FILES.toStdString() + "/" + Const::Model::YoloV5::Names::CLASS);
    std::string line;
    while (getline(ifs, line)) {
        class_list.push_back(line);
    }
    return class_list;
}

/**
 * @brief Format the input image for YOLOv5
 * @param source The input image
 * @return The formatted image
 */
cv::Mat format_yolov5(const cv::Mat& source) {
    int col = source.cols;
    int row = source.rows;
    int _max = MAX(col, row);
    cv::Mat result = cv::Mat::zeros(_max, _max, CV_8UC3);

    cv::Mat source_converted;
    if (source.channels() == 1) {
        cv::cvtColor(source, source_converted, cv::COLOR_GRAY2BGR);
    } else if (source.channels() == 4) {
        cv::cvtColor(source, source_converted, cv::COLOR_BGRA2BGR);
    } else {
        source_converted = source;
    }

    source_converted.copyTo(result(cv::Rect(0, 0, col, row)));
    return result;
}

/**
 * @brief Detect objects in an image using a YOLOv5 model
 * @param imagePath Path to the image file
 * @param image The image to process
 * @param model The name of the model file
 * @return A DetectedObjects object containing the detected objects
 */
DetectedObjects* Data::detect(std::string imagePath, QImage image, std::string model) {
    cv::Mat mat = QImageToCvMat(image);
    if (mat.empty()) {
        qWarning() << "Image is empty. Check the file path or loading process.";
        return nullptr;
    }

    cv::dnn::Net net = load_net(model);
    if (net.empty()) {
        qWarning() << "Failed to load the model";
        this->setCenterText(Text::Error::failedLoadModel().toStdString());

        return nullptr;
    }

    if (this->model.getClassNames().empty()) {
        this->model.setClassNames(load_class_list());
    }
    std::vector<std::string> className = this->model.getClassNames();

    cv::Mat blob;

    auto input_image = format_yolov5(mat);

    cv::dnn::blobFromImage(input_image, blob, 1. / 255., cv::Size(Const::Yolo::INPUT_WIDTH, Const::Yolo::INPUT_HEIGHT), cv::Scalar(), true, false);
    net.setInput(blob);
    std::vector<cv::Mat> outputs;

    auto start = std::chrono::high_resolution_clock::now();
    net.forward(outputs, net.getUnconnectedOutLayersNames());
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> detectionTime = end - start;
    if (detectionTime.count() > 3000) {
        qWarning() << "Detection time with " << QString::fromStdString(model) << ":" << detectionTime.count() << " seconds";
    } else {
        qInfo() << "Detection time with " << QString::fromStdString(model) << ":" << detectionTime.count() << " seconds";
    }

    float x_factor = input_image.cols / Const::Yolo::INPUT_WIDTH;
    float y_factor = input_image.rows / Const::Yolo::INPUT_HEIGHT;

    float* data = (float*)outputs[0].data;

    const int dimensions = 85;
    const int rows = 25200;

    std::vector<int> class_ids;
    std::vector<float> confidences;
    std::vector<cv::Rect> boxes;

    for (int i = 0; i < rows; ++i) {
        float confidence = data[4];
        if (confidence >= this->model.getConfidence()) {
            float* classes_scores = data + 5;
            cv::Mat scores(1, className.size(), CV_32FC1, classes_scores);
            cv::Point class_id;
            double max_class_score;
            minMaxLoc(scores, nullptr, &max_class_score, nullptr, &class_id);
            if (max_class_score > Const::Yolo::SCORE_THRESHOLD) {
                confidences.push_back(confidence);

                class_ids.push_back(class_id.x);

                float x = data[0];
                float y = data[1];
                float w = data[2];
                float h = data[3];
                int left = int((x - 0.5 * w) * x_factor);
                int top = int((y - 0.5 * h) * y_factor);
                int width = int(w * x_factor);
                int height = int(h * y_factor);
                boxes.push_back(cv::Rect(left, top, width, height));
            }
        }

        data += 85;
    }
    DetectedObjects* detectedObjects = new DetectedObjects();
    std::map<std::string, std::vector<std::pair<cv::Rect, float>>> detectedObjectsMap;

    std::vector<int> nms_result;
    cv::dnn::NMSBoxes(boxes, confidences, Const::Yolo::SCORE_THRESHOLD, this->model.getConfidence(), nms_result);

    for (int i = 0; i < nms_result.size(); i++) {
        int idx = nms_result[i];
        std::string class_name = className[class_ids[idx]];
        if (detectedObjectsMap.find(class_name) == detectedObjectsMap.end()) {
            detectedObjectsMap[class_name] = std::vector<std::pair<cv::Rect, float>>();
        }
        detectedObjectsMap[class_name].emplace_back(boxes[idx], confidences[idx]);
    }
    detectedObjects->setDetectedObjects(detectedObjectsMap);
    detectedObjects->setModelUsed(model);
    return detectedObjects;
}

/**
 * @brief Check if images are loaded in the cache and unload them if not
 * @param center Center index of the images to check
 * @param radius Radius around the center index to check (it unload radius*2 images on each side)
 */
void Data::checkToUnloadImages(int center, int radius) {
    std::unordered_set<std::string> loadedImages;

    int lowerBound = center - radius * 2;
    int upperBound = center + radius * 2;

    for (int i = lowerBound; i <= upperBound; ++i) {
        if (i >= 0 && i < imagesData.getCurrent()->size()) {
            loadedImages.insert(imagesData.getImageDataInCurrent(i)->getImagePath());
        }
    }

    std::vector<std::string> toUnload;
    {
        std::lock_guard<std::mutex> lock(imageCacheMutex);

        for (const auto& cache : *imageCache) {
            const std::string& imagePath = cache.second.imagePath;
            if (loadedImages.find(imagePath) == loadedImages.end()) {
                toUnload.push_back(cache.first);
            }
        }
    }

    for (const auto& imagePath : toUnload) {
        unloadFromCache(imagePath);
    }
}

/**
 * @brief Check if images are loaded in the cache and load them if not
 * @param center Center index of the images to check
 * @param radius Radius around the center index to check (it load radius images on each side)
 * @param thumbnailSize Size of the thumbnail to load
 */
void Data::checkToLoadImages(int center, int radius, int thumbnailSize) {
    if (getImagesDataPtr()->getCurrent()->size() == 0) {
        return;
    }

    std::unordered_set<std::string> loadedImages;

    int lowerBound = std::max(center - radius, 0);
    int upperBound = std::min(center + radius, static_cast<int>(getImagesDataPtr()->getCurrent()->size() - 1));

    for (int i = lowerBound; i <= upperBound; ++i) {
        loadInCacheAsync(this->getImagesDataPtr()->getImageDataInCurrent(i)->getImagePath(), nullptr, false, QSize(0, 0), thumbnailSize);
    }
}

/**
 * @brief Set the center text label
 * @param centerTextLabel QLabel to set the center text
 */
void Data::setCenterTextLabel(QLabel* centerTextLabel) {
    this->centerTextLabel = centerTextLabel;
}

/**
 * @brief Set the center text
 * @param text Text to set in the center
 */
void Data::setCenterText(std::string text) {
    this->centerTextLabel->setText(QString::fromStdString(text));
}

/**
 * @brief Get the model (const)
 * @return The model
 */
DetectObjectsModel Data::getModelConst() const {
    return this->model;
}

/**
 * @brief Get the model (ptr)
 * @return The model
 */
DetectObjectsModel* Data::getModelPtr() {
    return &this->model;
}

/**
 * @brief Set the model
 * @param model The model to set
 */
void Data::setModel(DetectObjectsModel model) {
    this->model = model;
}

/**
 * @brief Get the options (const)
 * @return The options
 */
std::map<std::string, Option> Data::getOptionsConst() const {
    return this->options;
}

/**
 * @brief Get the options (ptr)
 * @return The options
 */
std::map<std::string, Option>* Data::getOptionsPtr() {
    return &this->options;
}

/**
 * @brief Set the options
 * @param options The options to set
 */
void Data::setOptions(std::map<std::string, Option> options) {
    this->options = options;
}

/**
 * @brief Get the saved status
 * @return The saved status
 */
bool Data::getSaved() {
    return this->saved;
}

/**
 * @brief Set the saved status
 * @param saved The saved status to set
 */
void Data::setSaved(bool saved) {
    this->saved = saved;
}

/**
 * @brief Get the dark mode status
 * @return The dark mode status
 */
bool Data::getDarkMode() {
    return this->darkMode;
}

/**
 * @brief Set the dark mode status
 * @param darkMode The dark mode status to set
 */
void Data::setDarkMode(bool darkMode) {
    this->darkMode = darkMode;
}

/**
 * @brief Get the images selected (ptr)
 * @return The images selected
 */
std::vector<int>* Data::getImagesSelectedPtr() {
    return &this->imagesSelected;
}

bool Data::checkConnection() {
    if (hasConnection()) {
        if (!getConnectionEnabled()) {
            setConnectionEnabled(true);
        }
        return true;
    }
    return false;
}

/**
 * @brief Get the connection enabled status
 * @return The connection enabled status
 */
bool Data::getConnectionEnabled() {
    return this->connectionEnabled;
}

/**
 * @brief Set the connection enabled status
 * @param connectionEnabled The connection enabled status to set
 */
void Data::setConnectionEnabled(bool connectionEnabled) {
    this->connectionEnabled = connectionEnabled;
}

/**
 * @brief Get the person ID names (const)
 * @return The person ID names
 */
std::map<int, std::string> Data::getPersonIdNames() const {
    return this->personIdNames;
}
/**
 * @brief Get the person ID names (ptr)
 * @return The person ID names
 */
std::map<int, std::string>* Data::getPersonIdNamesPtr() {
    return &this->personIdNames;
}

/**
 * @brief Set the person ID names
 * @param personIdNames The person ID names to set
 */
void Data::setPersonIdNames(std::map<int, std::string> personIdNames) {
    this->personIdNames = personIdNames;
}

/**
 * @brief Check if the faces has been detected and detect them if not
 */
void Data::checkDetectFaces() {
    if (!this->getConnectionEnabled() && (!downloadModelIfNotExists(Const::Model::Arcface::NAME, Const::Model::Arcface::GITHUB_TAG) || !downloadModelIfNotExists(Const::Model::Haarcascade::NAME, Const::Model::Haarcascade::GITHUB_TAG))) {
        qInfo() << "arcface could not be downloaded cheking in 1 min";
        this->setCenterText(Text::Error::failedDownloadModel().toStdString());

        QTimer::singleShot(Const::Time::MIN_1, [this]() {
            checkDetectFaces();
        });

        return;
    }

    for (auto imageData : this->getImagesDataPtr()->getConst()) {
        if (imageData) {
            if (imageData->getFaceDetectionStatusPtr()->isStatusLoading()) {
                imageData->getFaceDetectionStatusPtr()->setStatusNotLoaded();
            }
            if (!imageData->getFaceDetectionStatusPtr()->isStatusLoaded()) {
                hasNotBeenDetectedFaces.push_back(imageData);
            } else {
                this->getDetectionProgressBarPtr()->setValue(this->getDetectionProgressBarPtr()->value() + 1);
            }
        } else {
            qWarning() << "ImageData is null in checkDetectFaces";
        }
    }

    if (hasNotBeenDetectedFaces.size() != 0) {
        this->getDetectionProgressBarPtr()->show();
        this->getDetectionProgressBarPtr()->setMinimum(0);
        this->getDetectionProgressBarPtr()->setMaximum(this->getImagesDataPtr()->getConst().size());
        this->getDetectionProgressBarPtr()->setValue(0);
        this->getDetectionProgressBarPtr()->setStyleSheet("QProgressBar::chunk { background-color: #00FF00; }");
        // TODO translate
        this->getDetectionProgressBarPtr()->setToolTip("Detecting faces...");
    }
    QObject::connect(detectFacesTimer, &QTimer::timeout, [this]() {
        if (hasNotBeenDetectedFaces.size() == 0) {
            if (detectionFacesWorking == 0) {
                detectFacesTimer->stop();
                qInfo() << "all faces detection done";
                this->getDetectionProgressBarPtr()->hide();
            }
        } else {
            while (detectionFacesWorking < Const::MAX_WORKING_DETECTION && hasNotBeenDetectedFaces.size() > 0) {
                detectionFacesWorking++;
                ImageData* imageData = hasNotBeenDetectedFaces.front();
                addHeavyThread([this, imageData]() {
                    auto start = std::chrono::high_resolution_clock::now();
                    this->detectAndRecognizeFaces(imageData);
                    auto end = std::chrono::high_resolution_clock::now();
                    std::chrono::duration<double> elapsed = end - start;
                    qInfo() << "detectAndRecognizeFaces execution time:" << elapsed.count() << "seconds";
                    detectionFacesWorking--;
                    this->getDetectionProgressBarPtr()->setValue(this->getDetectionProgressBarPtr()->value() + 1);
                });

                hasNotBeenDetectedFaces.pop_front();
            }
        }
    });
    detectFacesTimer->start(0);
    detectFacesTimer->setInterval(Const::WORKING_DETECTION_TIME);
}

/**
 * @brief Check if the objetcs has been detected and detect them if not
 */
void Data::checkDetectObjects() {
    if (!this->getConnectionEnabled() && !downloadModelIfNotExists(Const::Model::YoloV5::Names::N, Const::Model::YoloV5::GITHUB_TAG)) {
        qInfo() << "yolov5n could not be downloaded cheking in 1 min";
        this->setCenterText(Text::Error::failedDownloadModel().toStdString());

        QTimer::singleShot(Const::Time::MIN_1, [this]() {
            checkDetectObjects();
        });

        return;
    }
    for (auto imageData : this->getImagesDataPtr()->getConst()) {
        if (imageData->isDetectionStatusLoading()) {
            imageData->setDetectionStatusNotLoaded();
        }
        if (!imageData->isDetectionStatusLoaded()) {
            hasNotBeenDetected.push_back(imageData);
        } else {
            this->getDetectionProgressBarPtr()->setValue(this->getDetectionProgressBarPtr()->value() + 1);
        }
    }
    if (hasNotBeenDetected.size() != 0) {
        this->getDetectionProgressBarPtr()->show();
        this->getDetectionProgressBarPtr()->setMinimum(0);
        this->getDetectionProgressBarPtr()->setMaximum(this->getImagesDataPtr()->getConst().size());
        this->getDetectionProgressBarPtr()->setValue(0);
        this->getDetectionProgressBarPtr()->setStyleSheet("QProgressBar::chunk { background-color: #FF0000; } QProgressBar { text-align: center; }");
        // TODO translate
        this->getDetectionProgressBarPtr()->setToolTip("Detecting objects...");
    }
    QObject::connect(detectObjectTimer, &QTimer::timeout, [this]() {
        if (hasNotBeenDetected.size() == 0) {
            if (detectionWorking == 0) {
                detectObjectTimer->stop();
                qInfo() << "all detection done";
                this->getDetectionProgressBarPtr()->hide();

                checkDetectFaces();
            }
        } else {
            while (detectionWorking < Const::MAX_WORKING_DETECTION && hasNotBeenDetected.size() > 0) {
                detectionWorking++;
                ImageData* imageData = hasNotBeenDetected.front();
                addHeavyThread([this, imageData]() {
                    if (imageData) {
                        std::string imagePath = imageData->getImagePath();
                        qDebug() << "checkDetectObjects";

                        QImage image = this->loadImageNormal(imagePath, 0, true);
                        image = rotateQImage(image, imageData);
                        std::string modelName = Const::Model::YoloV5::Names::N;

                        DetectedObjects* detectedObjects = this->detect(imagePath, image, modelName);
                        if (detectedObjects) {
                            imageData->setDetectedObjects(detectedObjects->getDetectedObjects());
                            imageData->setDetectionStatusLoaded();
                        } else {
                            imageData->setDetectionStatusNotLoaded();
                        }
                        if (!unloadFromCache(imagePath)) {
                            qWarning() << "Error unloading image from cache: " << QString::fromStdString(imagePath);
                        }
                    }
                    detectionWorking--;
                    this->getDetectionProgressBarPtr()->setValue(this->getDetectionProgressBarPtr()->value() + 1);
                });

                hasNotBeenDetected.pop_front();
            }
        }
    });
    detectObjectTimer->start(0);
    detectObjectTimer->setInterval(Const::WORKING_DETECTION_TIME);
}

/**
 * @brief Check if the thumbnail exists and create it if not And detect objects if needed
 */
void Data::checkThumbnailAndDetectObjects() {
    for (auto imageData : this->getImagesDataPtr()->getConst()) {
        bool hasThumbnail = true;
        int i = 0;
        while (i < Const::Thumbnail::THUMBNAIL_SIZES.size() && hasThumbnail) {
            if (!this->hasThumbnail(imageData->getImagePath(), Const::Thumbnail::THUMBNAIL_SIZES[i])) {
                hasThumbnail = false;
            }
            i++;
        }

        if (!hasThumbnail) {
            hasNoThumbnail.push_back(imageData->getImagePath());
        }
    }
    QObject::connect(thumbnailTimer, &QTimer::timeout, [this]() {
        if (hasNoThumbnail.size() == 0) {
            thumbnailTimer->stop();

            qInfo() << "all thumbnails created";
            checkDetectObjects();
        } else {
            while (thumbnailWorking < Const::MAX_WORKING_THUMBNAIL) {
                thumbnailWorking++;

                createAllThumbnailsAsync(hasNoThumbnail.front(), [this](bool done) { thumbnailWorking--; }, false);

                hasNoThumbnail.pop_front();

                if (hasNoThumbnail.size() == 0) {
                    thumbnailTimer->stop();

                    qInfo() << "all thumbnails created";
                    checkDetectObjects();
                    return;
                }
            }
        }
    });
    thumbnailTimer->start(0);
    thumbnailTimer->setInterval(Const::WORKING_THUMBNAIL_TIME);
}

void Data::detectAndRecognizeFaces(ImageData* imageData) {
    if (!downloadModelIfNotExists(Const::Model::Arcface::NAME, Const::Model::Arcface::GITHUB_TAG) || !downloadModelIfNotExists(Const::Model::Haarcascade::NAME, Const::Model::Haarcascade::GITHUB_TAG)) {
        qWarning() << "arcface or haarcascade could not be downloaded, check your connection";
        return;
    }

    imageData->getFaceDetectionStatusPtr()->setStatusLoading();
    imageData->detectFaces();

    {
        std::lock_guard<std::mutex> lock(detectAndRecognizeFacesMutex);
        // Keep detecting sync

        auto* faces = imageData->getDetectedFacesPtr();
        if (faces && !faces->empty()) {
            const auto& allImages = this->getImagesDataPtr()->getConst();
            for (size_t i = 0; i < faces->size(); ++i) {
                const cv::Mat& embedding1 = *(*faces)[i].getEmbeddingPtr();
                for (const auto* otherImage : allImages) {
                    if (otherImage == imageData) continue;
                    auto otherFaces = otherImage->getDetectedFacesConst();
                    if (otherFaces.empty()) continue;
                    for (size_t j = 0; j < otherFaces.size(); ++j) {
                        const cv::Mat& embedding2 = *otherFaces[j].getEmbeddingPtr();
                        if (!embedding1.empty() && !embedding2.empty()) {
                            double similarity = cosineSimilarity(embedding1, embedding2);
                            if (similarity > 0.6) {
                                if (*((*faces)[i].getPersonIdPtr()) == -1) {
                                    *((*faces)[i].getPersonIdPtr()) = *otherFaces[j].getPersonIdPtr();
                                }
                            }
                        }
                    }
                }
                if (*((*faces)[i].getPersonIdPtr()) == -1) {
                    *((*faces)[i].getPersonIdPtr()) = this->getPersonIdNames().size();
                    auto* personIdNames = this->getPersonIdNamesPtr();

                    int unknownIndex = static_cast<int>(this->getPersonIdNames().size());
                    std::string baseName = "Unknown n°";
                    std::string newName;
                    bool exists = true;
                    while (exists) {
                        newName = baseName + std::to_string(unknownIndex);
                        exists = false;
                        for (const auto& [id, name] : *personIdNames) {
                            if (name == newName) {
                                exists = true;
                                ++unknownIndex;
                                break;
                            }
                        }
                    }
                    (*personIdNames)[unknownIndex] = newName;
                }
            }
        }
        imageData->getFaceDetectionStatusPtr()->setStatusLoaded();
    }
}

/**
 * @brief Load an image from a path with it's exif rotation
 * @param imagePath Path to the image
 * @param thumbnail Thumbnail size (0 for no thumbnail)
 * @param force Set if the image should be loaded even if it is already in the cache
 * @return QImage object containing the image data
 */
QImage loadAnImageWithRotation(ImageData imageData, int thumbnail) {
    std::string imagePath = imageData.getImagePathConst();
    // TODO get thumbnail path
    QImage image = loadAnImage(imagePath);
    image = rotateQImage(image, &imageData);
    return image;
}

/**
 * @brief Load an image from a path
 * @param imagePath Path to the image
 * @param thumbnail Thumbnail size (0 for no thumbnail)
 * @return QImage object containing the image data
 */
QImage loadAnImage(std::string imagePath) {
    std::string imagePathbis = imagePath;

    QImage image;

    if (!fs::exists(imagePath)) {
        qCritical() << "Error: The specified path does not exist: " << QString::fromStdString(imagePath);
        image.load(Const::ImagePath::ERROR_PATH);
        return image;
    }

    if (isHeicOrHeif(imagePathbis)) {
        image = readHeicAndHeif(imagePathbis);

        // } else if (isRaw(imagePathbis)) {
        //     if (thumbnail == 0) {
        //         image = readRaw(imagePathbis);
        //     } else {
        //         qWarning() << "Thumbnail not supported for RAW images";
        //     }
    } else {
        image.load(QString::fromStdString(imagePathbis));
    }

    if (image.isNull()) {
        qCritical() << "Could not open or find the image : " << imagePathbis;
        image.load(Const::ImagePath::ERROR_PATH);
        return image;
    }
    return image;
}

/**
 * @brief Save an image to a destination path
 * @param destinationPath Path where the image should be saved
 * @param image QImage object containing the image data
 * @return true if the image was saved successfully, false otherwise
 */
bool saveAnImage(std::string destinationPath, QImage image) {
    if (isHeicOrHeif(destinationPath)) {
        writeHeicAndHeif(image, destinationPath);
        return true;
    } else {
        if (image.save(QString::fromStdString(destinationPath))) {
            return true;
        }
    }
    qWarning() << "Could not write the image : " << QString::fromStdString(destinationPath);

    return false;
}

/**
 * @brief Get the detection progress bar pointer
 * @return Pointer to the detection progress bar
 */
AsyncProgressBar* Data::getDetectionProgressBarPtr() {
    return detectionProgressBar;
}

/**
 * @brief Set the detection progress bar pointer
 * @param detectionProgressBar Pointer to the detection progress bar
 */
void Data::setDetectionProgressBarPtr(AsyncProgressBar* detectionProgressBar) {
    this->detectionProgressBar = detectionProgressBar;
}