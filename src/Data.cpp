#include "Data.hpp"

#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QPainter>
#include <QProgressDialog>
#include <QResource>
#include <QThreadPool>
#include <QTimer>
#include <QWidget>
#include <regex>

#include "Box.hpp"
#include "Const.hpp"
#include "Conversion.hpp"
#include "Data.hpp"
#include "Verification.hpp"

namespace fs = std::filesystem;

Data::Data()
    : imageCache(new std::map<std::string, QImageAndPath>()) {
    options = DEFAULT_OPTIONS;
}

/**
 * @brief
 * @param imageNbr
 */
void Data::preDeleteImage(int imageNbr) {
    ImageData* imageData;
    imageData = this->imagesData.getImageData(imageNbr);

    this->deletedImagesData.addImage(*imageData);
    qInfo() << "image deleted : " << imageNbr;
    this->deletedImagesData.print();
}

/**
 * @brief
 * @param imageNbr
 */
void Data::unPreDeleteImage(int imageNbr) {
    const ImageData imageData = *imagesData.getImageData(imageNbr);

    deletedImagesData.removeImage(imageData);
}

void Data::revocerDeletedImage(ImageData& imageData) {
    imagesData.addImage(imageData);
    deletedImagesData.removeImage(imageData);
}

void Data::revocerDeletedImage(int imageNbr) {
    ImageData imageData = *deletedImagesData.getImageData(imageNbr);
    revocerDeletedImage(imageData);

    imagesData.addImage(imageData);
    deletedImagesData.removeImage(imageData);
}

// Delete in imagesData images that are also in deletedImagesData
void Data::removeDeletedImages() {
    for (const auto& deletedImage : *deletedImagesData.get()) {
        // auto it = std::find(imagesData.get()->begin(), imagesData.get()->end(),
        //                     deletedImage);
        // TODO faire sans mais ça marche pas ...
        ImageData* imageData = getImagesData()->getImageData(deletedImage->getImagePath());

        removeImageFromFolders(*imageData);

        auto itPtr = std::find(imagesData.getCurrent()->begin(), imagesData.getCurrent()->end(), imageData);
        if (itPtr != imagesData.getCurrent()->end()) {
            qInfo() << "remove image from currentImagesData";
            imagesData.getCurrent()->erase(itPtr);
        }
    }
    qInfo() << "All images deleted";
}

// Check if the imageNumber is deleted or not (!! imageNumber in imagesData!!)
bool Data::isDeleted(int imageNbr) {
    std::string imagePath = imagesData.getImageData(imageNbr)->getImagePathConst();
    auto it = std::find_if(deletedImagesData.get()->begin(),
                           deletedImagesData.get()->end(),
                           [imagePath, this](ImageData* imgPtr) {
                               return imgPtr->getImagePathConst() == imagePath;
                           });

    if (it != deletedImagesData.get()->end()) {
        imagesData.getImageData(imageNbr)->print();

        return true;
    }

    return false;
}

QImage Data::loadImage(QWidget* parent, std::string imagePath, QSize size,
                       bool setSize, int thumbnail, bool rotation,
                       bool square, bool crop, bool force) {
    QImage image = loadImageNormal(parent, imagePath, size, setSize, thumbnail, force);

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

QImage Data::loadImageNormal(QWidget* parent, std::string imagePath, QSize size,
                             bool setSize, int thumbnail, bool force) {
    if (imagePath.at(0) == ':') {
        if (darkMode) {
            imagePath.insert(imagePath.find_first_of(':') + 1, "/255-255-255-255");
        } else {
            imagePath.insert(imagePath.find_first_of(':') + 1, "/0-0-0-255");
        }
    }
    auto it = imageCache->find(imagePath);
    if (it != imageCache->end()) {
        return it->second.image;
    }

    if (!force) {
        it = imageCache->find(getThumbnailPath(imagePath, 512));
        if (it != imageCache->end()) {
            return it->second.image;
        }

        it = imageCache->find(getThumbnailPath(imagePath, 256));
        if (it != imageCache->end()) {
            return it->second.image;
        }

        it = imageCache->find(getThumbnailPath(imagePath, 128));
        if (it != imageCache->end()) {
            return it->second.image;
        }
    }

    std::string imagePathbis = imagePath;

    QImage image;

    if (imagePath.at(0) == ':') {
        QResource ressource(QString::fromStdString(imagePath));
        if (ressource.isValid()) {
            image.load(QString::fromStdString(imagePathbis));
        }

    } else {
        if (thumbnail == 128) {
            if (hasThumbnail(imagePath, 128)) {
                imagePathbis = getThumbnailPath(imagePath, 128);
            } else {
                createThumbnail(imagePath, 128);
                createThumbnailIfNotExists(imagePath, 256);
                createThumbnailIfNotExists(imagePath, 512);
            }
        } else if (thumbnail == 256) {
            if (hasThumbnail(imagePath, 256)) {
                imagePathbis = getThumbnailPath(imagePath, 256);
            } else {
                createThumbnailIfNotExists(imagePath, 128);
                createThumbnail(imagePath, 256);
                createThumbnailIfNotExists(imagePath, 512);
            }
        } else if (thumbnail == 512) {
            if (hasThumbnail(imagePath, 512)) {
                imagePathbis = getThumbnailPath(imagePath, 512);
            } else {
                createThumbnailIfNotExists(imagePath, 128);
                createThumbnailIfNotExists(imagePath, 256);
                createThumbnail(imagePath, 512);
            }
        }

        if (isHeicOrHeif(imagePathbis)) {
            if (thumbnail == 0) {
                image = readHeicAndHeif(imagePathbis);
            }

        } else if (isRaw(imagePathbis)) {
            if (thumbnail == 0) {
                image = readRaw(imagePathbis);
            }
        } else {
            image.load(QString::fromStdString(imagePathbis));
        }

        if (image.isNull()) {
            qCritical() << "Could not open or find the image : " << imagePathbis;

            return QImage();
        }

        if (setSize) {
            image = image.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
    }
    (*imageCache)[imagePathbis].image = image;
    (*imageCache)[imagePathbis].imagePath = imagePath;

    size_t cacheSize = 0;

    for (const auto& pair : *imageCache) {
        cacheSize += pair.second.image.sizeInBytes();
    }
    if (static_cast<double>(cacheSize) / (1024 * 1024) > 5000) {
        qCritical() << "Image cache size: " << static_cast<double>(cacheSize) / (1024 * 1024) << " MB";
    }

    return image;
}

void Data::loadInCacheAsync(std::string imagePath, std::function<void()> callback, bool setSize, QSize size, bool force) {
    LoadImageTask* task = new LoadImageTask(this, imagePath, setSize, size, force, callback);
    try {
        QThreadPool::globalInstance()->start(task);
    } catch (const std::exception& e) {
        qCritical() << "loadInCacheAsync" << e.what();
    }
}

bool Data::loadInCache(const std::string imagePath, bool setSize,
                       const QSize size, bool force) {
    QImage image;
    if (!force && isInCache(imagePath)) {
        return true;
    }

    loadImageNormal(nullptr, imagePath, size, setSize, 0, true);

    createThumbnailIfNotExists(imagePath, 128);
    createThumbnailIfNotExists(imagePath, 256);
    createThumbnailIfNotExists(imagePath, 512);

    return true;
}

bool Data::isInCache(std::string imagePath) {
    return imageCache->find(imagePath) != imageCache->end();
}

bool Data::getLoadedImage(std::string imagePath, QImage& image) {
    auto it = imageCache->find(imagePath);
    if (it != imageCache->end()) {
        image = it->second.image;
        return true;
    }
    return false;
}

void Data::createThumbnails(const std::vector<std::string>& imagePaths,
                            const int maxDim) {
    for (const auto& imagePath : imagePaths) {
        createThumbnail(imagePath, maxDim);
    }
}

bool Data::createThumbnail(const std::string& imagePath, const int maxDim) {
    try {
        // qDebug() << "createThumbnail : " << imagePath << " : " << maxDim;
        QImage image = loadImageNormal(nullptr, imagePath, QSize(maxDim, maxDim), false, 0);
        // qDebug() << "createThumbnail : imageLoaded";
        double scale = std::min(static_cast<double>(maxDim) / image.width(),
                                static_cast<double>(maxDim) / image.height());

        QImage thumbnail = image.scaled(image.width() * scale, image.height() * scale,
                                        Qt::KeepAspectRatio);
        // qDebug() << "createThumbnail : scaled";

        std::string outputImage = getThumbnailPath(imagePath, maxDim);

        if (!fs::exists(fs::path(outputImage).parent_path())) {
            fs::create_directories(fs::path(outputImage).parent_path());
        }
        // qDebug() << "createThumbnail : folder created if not exist";

        if (!thumbnail.save(QString::fromStdString(outputImage))) {
            qCritical() << "Error: Could not save thumbnail: " << outputImage;
            return false;
        }
        // qDebug() << "createThumbnail : saved";

    } catch (const std::exception& e) {
        qCritical() << "createThumbnail : " << e.what();
        return false;
    }
    return true;
}

bool Data::deleteThumbnail(const std::string& imagePath, const int maxDim) {
    std::string thumbnailPath = getThumbnailPath(imagePath, maxDim);

    if (fs::exists(thumbnailPath)) {
        return fs::remove(thumbnailPath);
    }
    return false;
}

void Data::createThumbnailsIfNotExists(
    const std::vector<std::string>& imagePaths, const int maxDim) {
    for (const auto& imagePath : imagePaths) {
        createThumbnailIfNotExists(imagePath, maxDim);
    }
}

void Data::createThumbnailIfNotExists(const std::string& imagePath,
                                      const int maxDim) {
    if (!hasThumbnail(imagePath, maxDim)) {
        createThumbnail(imagePath, maxDim);
    }
}
bool Data::hasThumbnail(const std::string& imagePath, const int maxDim) {
    std::string thumbnailPath = getThumbnailPath(imagePath, maxDim);

    return fs::exists(thumbnailPath);
}

void Data::createAllThumbnailIfNotExists(const std::string& imagePath, const int size) {
    if (size > 16) {
        createThumbnailIfNotExists(imagePath, 16);
    }
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

void Data::createAllThumbnail(const std::string& imagePath, const int size) {
    if (size > 16) {
        createThumbnail(imagePath, 16);
    }
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

std::string Data::getThumbnailPath(const std::string& imagePath,
                                   const int size) {
    std::hash<std::string> hasher;
    size_t hashValue = hasher(fs::path(imagePath).filename().string());
    std::string extension = ".webp";

    std::string thumbnailPath = THUMBNAIL_PATH + "/" + std::to_string(size) + "/" +
                                std::to_string(hashValue) + extension;

    return thumbnailPath;
}

bool Data::unloadFromCache(std::string imagePath) {
    if (!imageCache) {
        qWarning() << "imageCache is not initialized : " << imagePath;
        return false;
    }
    auto it = imageCache->find(imagePath);

    if (it != imageCache->end()) {
        imageCache->erase(it);
        return true;
    }
    return false;
}

void Data::exportImages(std::string exportPath, bool dateInName) {
    Folders* firstFolder;

    rootFolders.print();

    firstFolder = findFirstFolderWithAllImages();

    exportPath += "/" + firstFolder->getName();

    createFolders(firstFolder, exportPath);

    copyTo(rootFolders, exportPath, dateInName);
}

Folders* Data::findFirstFolderWithAllImages() const {
    Folders currentF = rootFolders;
    return findFirstFolderWithAllImagesSub(currentF);
}

Folders* Data::findFirstFolderWithAllImagesSub(const Folders& currentF) const {
    if (currentF.folders.size() > 1) {
        return const_cast<Folders*>(&currentF);
    }
    for (const auto& folder : currentF.folders) {
        if (folder.getFilesConst().size() > 0) {
            return const_cast<Folders*>(&folder);
        }
        return findFirstFolderWithAllImagesSub(folder);
    }

    return const_cast<Folders*>(&currentF);
}

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
            if (!imageData->getCropSizes().empty()) {
                QImage image(QString::fromStdString(imageData->getImagePath()));
                if (!image.isNull()) {
                    std::vector<QPoint> cropPoints = imageData->getCropSizes().back();
                    if (cropPoints.size() == 2) {
                        QRect cropRect = QRect(cropPoints[0], cropPoints[1]).normalized();
                        image = image.copy(cropRect);
                    }
                }
                image.save(QString::fromStdString(destinationFile));

                Exiv2::Image::AutoPtr srcImage = Exiv2::ImageFactory::open(imageData->getImagePath());
                srcImage->readMetadata();
                Exiv2::Image::AutoPtr destImage = Exiv2::ImageFactory::open(destinationFile);
                destImage->setExifData(srcImage->exifData());
                destImage->setIptcData(srcImage->iptcData());
                destImage->setXmpData(srcImage->xmpData());
                destImage->writeMetadata();
            } else {
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

QImage Data::rotateQImage(QImage image, ImageData* imageData) {
    if (imageData != nullptr) {
        int orientation = imageData->getOrientation();

        switch (orientation) {
            case Const::Orientation::FLIP_HORIZONTAL:
                image = image.mirrored(true, false);
                break;
            case Const::Orientation::ROTATE_180:
                image = image.transformed(QTransform().rotate(Const::Rotation::UP_SIDE_DOWN));
                break;
            case Const::Orientation::FLIP_VERTICAL:
                image = image.mirrored(false, true);
                break;
            case Const::Orientation::TRANSPOSE:
                image = image.mirrored(true, false).transformed(QTransform().rotate(-90));
                break;
            case Const::Orientation::ROTATE_90:
                image = image.transformed(QTransform().rotate(90));
                break;
            case Const::Orientation::TRANSVERSE:
                image = image.mirrored(true, false).transformed(QTransform().rotate(90));
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

void Data::createFolders(Folders* currentFolders, std::string folderPath) {
    std::string initialFolderPath = folderPath;
    if (!fs::exists(initialFolderPath)) {
        fs::create_directories(initialFolderPath);
    }

    for (auto& folder : currentFolders->folders) {
        folderPath = initialFolderPath + "/" + folder.getName();
        if (!fs::exists(folderPath)) {
            fs::create_directories(folderPath);
        }
        createFolders(&folder, folderPath);
    }
}

void Data::saveData() {
    qDebug() << "Saving data";

    std::string filePath = IMAGESDATA_SAVE_DATA_PATH;
    std::ofstream outFile(filePath, std::ios::binary);
    if (!outFile) {
        if (!fs::exists(fs::path(filePath).parent_path())) {
            if (!fs::create_directories(fs::path(filePath).parent_path())) {
                qCritical() << "Couldn't create directories for save file : " << fs::path(filePath).parent_path();
                return;
            }
        }

        outFile.open(filePath, std::ios::binary);
        if (!outFile) {
            qCritical() << "Couldn't open save file : " << filePath;
            return;
        }
    }

    size_t imagesDataSize = imagesData.get()->size();
    outFile.write(reinterpret_cast<const char*>(&imagesDataSize), sizeof(imagesDataSize));
    for (auto* imageData : *imagesData.get()) {
        // if (imageData->getpersons().size() > 0) {
        //     qDebug() << "save person 1 : " << imageData->getImagePath();
        // }
        imageData->save(outFile);
    }

    // size_t deletedImagesDataSize = deletedImagesData.get()->size();
    // outFile.write(reinterpret_cast<const char*>(&deletedImagesDataSize), sizeof(deletedImagesDataSize));
    // for (const auto* imageData : *deletedImagesData.get()) {
    //     imageData->save(outFile);
    // }

    size_t optionsSize = options.size();
    outFile.write(reinterpret_cast<const char*>(&optionsSize),
                  sizeof(optionsSize));
    for (const auto& [key, option] : options) {
        size_t keySize = key.size();
        outFile.write(reinterpret_cast<const char*>(&keySize), sizeof(keySize));
        outFile.write(key.c_str(), keySize);

        size_t typeSize = option.type.size();
        outFile.write(reinterpret_cast<const char*>(&typeSize), sizeof(typeSize));
        outFile.write(option.type.c_str(), typeSize);

        size_t valueSize = option.value.size();
        outFile.write(reinterpret_cast<const char*>(&valueSize), sizeof(valueSize));
        outFile.write(option.value.c_str(), valueSize);
    }

    rootFolders.save(outFile);

    std::string currentFolderPath = getFolderPath(currentFolder);
    size_t pathSize = currentFolderPath.size();
    outFile.write(reinterpret_cast<const char*>(&pathSize), sizeof(pathSize));
    outFile.write(currentFolderPath.c_str(), pathSize);

    outFile.close();
    qDebug() << "data saved";
}

void Data::loadData() {
    std::string filePath = IMAGESDATA_SAVE_DATA_PATH;
    std::ifstream inFile(filePath, std::ios::binary);
    if (!inFile) {
        qCritical() << "Couldn't open load file : " << filePath;
        return;
    }

    size_t imagesDataSize;
    inFile.read(reinterpret_cast<char*>(&imagesDataSize), sizeof(imagesDataSize));
    for (size_t i = 0; i < imagesDataSize; ++i) {
        ImageData* imageData = new ImageData();
        imageData->load(inFile);
        if (imageData->getpersons().size() > 0) {
            qDebug() << "load person 1 : " << imageData->getImagePath();
        }
        imagesData.get()->push_back(imageData);
    }

    // Update imageMap with pointers to ImageData in imagesData
    int index = 0;
    for (auto* imageData : *imagesData.get()) {
        if (imageData->getpersons().size() > 0) {
            qDebug() << "load person 2 : " << imageData->getImagePath();
        }
        imagesData.setImageMapValue(imageData->getImagePath(), imageData);
        imagesData.setImageMapIntValue(index, imageData);
        index++;
    }

    // size_t deletedImagesDataSize;
    // inFile.read(reinterpret_cast<char*>(&deletedImagesDataSize), sizeof(deletedImagesDataSize));
    // for (size_t i = 0; i < deletedImagesDataSize; ++i) {
    //     ImageData* imageData = new ImageData();
    //     imageData->load(inFile);
    //     // deletedImagesData.push_back(*imageData);
    // }

    options.clear();
    size_t optionsSize;
    inFile.read(reinterpret_cast<char*>(&optionsSize), sizeof(optionsSize));
    for (size_t i = 0; i < optionsSize; ++i) {
        std::string key, type, value;
        size_t keySize, typeSize, valueSize;
        inFile.read(reinterpret_cast<char*>(&keySize), sizeof(keySize));
        key.resize(keySize);
        inFile.read(&key[0], keySize);

        inFile.read(reinterpret_cast<char*>(&typeSize), sizeof(typeSize));
        type.resize(typeSize);
        inFile.read(&type[0], typeSize);

        inFile.read(reinterpret_cast<char*>(&valueSize), sizeof(valueSize));
        value.resize(valueSize);
        inFile.read(&value[0], valueSize);

        options[key] = Option(type, value);
    }
    rootFolders.load(inFile);

    // Load the path of currentFolder
    size_t pathSize;
    inFile.read(reinterpret_cast<char*>(&pathSize), sizeof(pathSize));
    std::string currentFolderPath(pathSize, '\0');
    inFile.read(&currentFolderPath[0], pathSize);
    currentFolder = findFolderByPath(rootFolders, currentFolderPath);
    qDebug() << "currentFolder : " << currentFolderPath;

    inFile.close();
    // todo mettre autre part
    // if (!currentFolder) {
    //     currentFolder = findFirstFolderWithAllImages();
    // }
}

void Data::addAction(std::function<void()> unDo, std::function<void()> reDo) {
    Actions action;
    action.unDo = unDo;
    action.reDo = reDo;
    lastActions.emplace_back(action);
    if (lastActions.size() > 100) {
        lastActions.erase(lastActions.begin());
    }
}

void Data::addActionDone(Actions action) {
    lastActionsDone.emplace_back(action);
    if (lastActionsDone.size() > 100) {
        lastActionsDone.erase(lastActionsDone.begin());
    }
}

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
    // TODO mieux estimer X( estimatedSteps
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

void Data::clearCache() {
    imageCache->clear();
}

void LoadImageTask::run() {
    data->loadInCache(imagePath, setSize, size, force);
    if (callback) {
        QMetaObject::invokeMethod(QApplication::instance(), callback,
                                  Qt::QueuedConnection);
    }
}

void Data::rotateLeft(int nbr, std::string extension, std::function<void()> reload, bool action) {
    if (isExif(extension)) {
        exifRotate(nbr, Const::Rotation::LEFT, reload);
        if (action) {
            addAction(
                [this, nbr, reload]() {
                    int time = 0;
                    if (imagesData.getImageNumber() != nbr) {
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
                    if (imagesData.getImageNumber() != nbr) {
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
                    if (imagesData.getImageNumber() != nbr) {
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
                    if (imagesData.getImageNumber() != nbr) {
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

void Data::rotateRight(int nbr, std::string extension, std::function<void()> reload, bool action) {
    if (isExif(extension)) {
        exifRotate(nbr, Const::Rotation::RIGHT, reload);

        if (action) {
            addAction(
                [this, nbr, reload]() {
                    int time = 0;
                    if (imagesData.getImageNumber() != nbr) {
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
                    if (imagesData.getImageNumber() != nbr) {
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
                    if (imagesData.getImageNumber() != nbr) {
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
                    if (imagesData.getImageNumber() != nbr) {
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

void Data::realRotate(int nbr, int rotation, std::function<void()> reload) {
    QString outputPath = QString::fromStdString(imagesData.getImageData(nbr)->getImagePath());
    QImage image = loadImage(nullptr, imagesData.getImageData(nbr)->getImagePath(), QSize(0, 0), false);
    image = image.transformed(QTransform().rotate(-rotation));
    if (!image.save(outputPath)) {
        qDebug() << "Erreur lors de la sauvegarde de l'image : " << outputPath;
    }
    unloadFromCache(imagesData.getImageData(nbr)->getImagePath());
    loadInCache(imagesData.getImageData(nbr)->getImagePath());
    createAllThumbnail(imagesData.getImageData(nbr)->getImagePath(), 512);
    reload();
}

void Data::exifRotate(int nbr, int rotation, std::function<void()> reload) {
    // qDebug() << "Exif rotate :" << nbr;
    ImageData* imageData = imagesData.getImageData(nbr);
    // TODO faire sans mais ça marche pas ...
    imageData = getImagesData()->getImageData(imageData->getImagePath());

    if (!isTurnable(imageData->getImagePath())) {
        qWarning() << "Image not turnable : " << imageData->getImagePath();
        return;
    }
    int orientation = imageData->getOrientation();
    if (rotation == Const::Rotation::RIGHT) {
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
    if (rotation == Const::Rotation::LEFT) {
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
    // qDebug() << "Exif rotate orientation :" << orientation;
    imageData->turnImage(orientation);

    imageData->saveMetaData();

    reload();
}

void Data::mirrorUpDown(int nbr, std::string extension, std::function<void()> reload, bool action) {
    if (isExif(extension)) {
        exifMirror(nbr, true, reload);

        if (action) {
            addAction(
                [this, nbr, reload]() {
                    int time = 0;
                    if (imagesData.getImageNumber() != nbr) {
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
                    if (imagesData.getImageNumber() != nbr) {
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
                    if (imagesData.getImageNumber() != nbr) {
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
                    if (imagesData.getImageNumber() != nbr) {
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
    saved = false;
}

void Data::mirrorLeftRight(int nbr, std::string extension, std::function<void()> reload, bool action) {
    if (isExif(extension)) {
        exifMirror(nbr, false, reload);

        if (action) {
            addAction(
                [this, nbr, reload]() {
                    int time = 0;
                    if (imagesData.getImageNumber() != nbr) {
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
                    if (imagesData.getImageNumber() != nbr) {
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
                    if (imagesData.getImageNumber() != nbr) {
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
                    if (imagesData.getImageNumber() != nbr) {
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
    saved = false;
}

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

void Data::realMirror(int nbr, bool UpDown, std::function<void()> reload) {
    QString outputPath = QString::fromStdString(imagesData.getCurrentImageData()->getImagePathConst());
    QImage image = loadImage(nullptr, imagesData.getCurrentImageData()->getImagePathConst(), QSize(0, 0), false);
    if (UpDown) {
        image = image.mirrored(false, true);
    } else {
        image = image.mirrored(true, false);
    }
    if (!image.save(outputPath)) {
        qDebug() << "Erreur lors de la sauvegarde de l'image : " << outputPath;
    }
    unloadFromCache(imagesData.getCurrentImageData()->getImagePathConst());
    loadInCache(imagesData.getCurrentImageData()->getImagePathConst());
    createAllThumbnail(imagesData.getCurrentImageData()->getImagePathConst(), 512);
    reload();
}

void Data::clearActions() {
    lastActions.clear();
    lastActionsDone.clear();
}

Folders* Data::getRootFolders() {
    return &rootFolders;
}
Folders* Data::getCurrentFolders() {
    if (currentFolder == nullptr || currentFolder->getName() == "") {
        // qDebug() << "currentFolder is null 0 ";
        currentFolder = findFirstFolderWithAllImages();
        // qWarning() << "currentFolder is null 1";
    }
    return currentFolder;
}

ImagesData* Data::getImagesData() {
    return &imagesData;
}

void Data::removeImageFromFolders(ImageData& imageData) {
    Folders* rootFolders = getRootFolders();
    for (auto& folder : imageData.getFolders()) {
        std::string folderPath = folder.getName();
        Folders* currentFolderBis = rootFolders;
        std::string folderPathBis = std::regex_replace(folderPath, std::regex("\\\\"), "/");
        std::istringstream iss(folderPathBis);
        std::string token;
        bool run = true;
        qDebug() << "Remove image from folder :  1" << folderPath;
        while (run && std::getline(iss, token, '/')) {
            auto it = std::find_if(currentFolderBis->folders.begin(), currentFolderBis->folders.end(),
                                   [&token](const Folders& f) { return f.getName() == token; });
            if (it != currentFolderBis->folders.end()) {
                currentFolderBis = &(*it);
            } else {
                currentFolderBis = nullptr;
                run = false;
            }
        }
        if (currentFolderBis) {
            try {
                // qDebug() << "Remove image from folder :  2" << folderPath;
                // qDebug() << "Current folder files:";
                // for (const auto& file : currentFolderBis->files) {
                //     qDebug() << QString::fromStdString(file);
                // }
                qDebug() << "Image to find: " << QString::fromStdString(imageData.getImagePath());
                auto it = std::find(currentFolderBis->getFilesPtr()->begin(), currentFolderBis->getFilesPtr()->end(), imageData.getImagePath());
                if (it != currentFolderBis->getFilesPtr()->end()) {
                    qDebug() << "find to remove";
                    // currentFolderBis->files.erase(it);
                    currentFolder->getFilesPtr()->erase(it);
                }
            } catch (const std::exception& e) {
                qCritical() << "removeImageFromFolders" << e.what();
            }
        }

        qDebug() << "Remove image from folder : " << currentFolder->getName();
    }
}

std::string Data::getFolderPath(Folders* folder) {
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
        auto it = std::find_if(current->folders.begin(), current->folders.end(),
                               [&token](const Folders& f) { return f.getName() == token; });
        if (it != current->folders.end()) {
            current = &(*it);
        } else {
            return nullptr;
        }
    }
    return current;
}

void Data::clear() {
    imagesData.clear();
    rootFolders.clear();
    currentFolder = nullptr;
    options.clear();
    lastActions.clear();
    lastActionsDone.clear();
    imageCache->clear();
    saved = true;
}