#include "Data.hpp"

namespace fs = std::filesystem;

void Data::preDeleteImage(int imageNbr) {
    ImageData* imageData;
    imageData = imagesData.getImageData(imageNbr);

    deletedImagesData.addImage(*imageData);
    qDebug() << "image deleted : " << imageNbr;
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

    imagesData.addImage(*imageData);
    deletedImagesData.removeImage(*imageData);
}

// Delete in imagesData images that are also in deletedImagesData
void Data::removeDeletedImages() {
    qDebug() << "Deleting images";
    for (const auto& deletedImage : *deletedImagesData.get()) {
        qDebug() << "Deleting images 1";

        auto it = std::find(imagesData.get()->begin(), imagesData.get()->end(),
                            deletedImage);
        // auto it = imagesData.imageMap.find(deletedImage.getImagePathConst());
        qDebug() << "Deleting images 2";

        removeImageFromFolders(*it);

        if (it != imagesData.get()->end()) {
            imagesData.get()->erase(it);
            deletedImage.print();
            imagesData.imageMap.erase(deletedImage.getImagePathConst());
        }
    }
    qDebug() << "All images deleted";
}

// Check if the imageNumber is deleted or not (!! imageNumber in imagesData!!)
bool Data::isDeleted(int imageNbr) {
    std::string imagePath = imagesData.getImageData(imageNbr)->folders.name;
    auto it = std::find_if(deletedImagesData.get()->begin(),
                           deletedImagesData.get()->end(),
                           [imagePath, this](const ImageData img) {
                               return img.folders.name == imagePath;
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

    if (crop) {
        ImageData* imageData = imagesData.getImageData(imagePath);

        if (imageData != nullptr) {
            if (!imageData->cropSizes.empty()) {
                std::vector<QPoint> cropPoints = imageData->cropSizes.back();

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
        }
    }

    if (square) {
        int cropSize = std::min(image.width(), image.height());
        int xOffset = (image.width() - cropSize) / 2;
        int yOffset = (image.height() - cropSize) / 2;

        image = image.copy(xOffset, yOffset, cropSize, cropSize);
    }

    if (rotation && imagePath.at(0) != ':') {
        // TODO pas ouf ça
        std::string extension = imagesData.getCurrentImageData()->getImageExtension();
        if (isExifTurnOrMirror(extension)) {
            // TODO change imagePath to imageData
            image = rotateQImage(image, imagePath);
        }
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
            qDebug() << "Could not open or find the image : " << imagePathbis;

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
        qDebug() << "loadInCacheAsync" << e.what();
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

void Data::createThumbnail(const std::string& imagePath, const int maxDim) {
    QImage image =
        loadImageNormal(nullptr, imagePath, QSize(maxDim, maxDim), false, 0);

    double scale = std::min(static_cast<double>(maxDim) / image.width(),
                            static_cast<double>(maxDim) / image.height());

    QImage thumbnail = image.scaled(image.width() * scale, image.height() * scale,
                                    Qt::KeepAspectRatio);

    std::hash<std::string> hasher;
    size_t hashValue = hasher(fs::path(imagePath).filename().string());

    std::string extension = ".png";

    std::string outputImage;
    // if (maxDim == 128) {
    outputImage = THUMBNAIL_PATH + "/" + std::to_string(maxDim) + "/" +
                  std::to_string(hashValue) + extension;
    // } else if (maxDim == 256) {
    //     outputImage = THUMBNAIL_PATH + "/large/" +
    //                   std::to_string(hashValue) + extension;
    // } else if (maxDim == 512) {
    //     outputImage = THUMBNAIL_PATH + "/x-large/" +
    //                   std::to_string(hashValue) + extension;
    // }

    if (!fs::exists(fs::path(outputImage).parent_path())) {
        fs::create_directories(fs::path(outputImage).parent_path());
    }
    if (!thumbnail.save(QString::fromStdString(outputImage))) {
        qDebug() << "Error: Could not save thumbnail: " << outputImage;
    }
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
    std::hash<std::string> hasher;
    size_t hashValue = hasher(fs::path(imagePath).filename().string());

    std::string extension = ".png";

    std::string outputImage;

    // if (maxDim == 128) {
    outputImage = THUMBNAIL_PATH + "/" + std::to_string(maxDim) + "/" +
                  std::to_string(hashValue) + extension;
    // } else if (maxDim == 256) {
    //     outputImage = THUMBNAIL_PATH + "/large/" +
    //                   std::to_string(hashValue) + extension;
    // } else if (maxDim == 512) {
    //     outputImage = THUMBNAIL_PATH + "/x-large/" +
    //                   std::to_string(hashValue) + extension;
    // }
    return fs::exists(outputImage);
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
    std::string extension = ".png";

    std::string outputImage;

    // if (size == 128) {
    outputImage = THUMBNAIL_PATH + "/" + std::to_string(size) + "/" +
                  std::to_string(hashValue) + extension;
    // } else if (size == 256) {
    //     outputImage = THUMBNAIL_PATH + "/large/" +
    //                   std::to_string(hashValue) + extension;
    // } else if (size == 512) {
    //     outputImage = THUMBNAIL_PATH + "/x-large/" +
    //                   std::to_string(hashValue) + extension;
    // }
    return outputImage;
}

bool Data::unloadFromCache(std::string imagePath) {
    if (!imageCache) {
        qDebug() << "imageCache is not initialized : " << imagePath;
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

    firstFolder = findFirstFolderWithAllImages(imagesData, rootFolders);

    exportPath += "/" + firstFolder->name;

    createFolders(firstFolder, exportPath);

    copyTo(rootFolders, exportPath, dateInName);
}

Folders* Data::findFirstFolderWithAllImages(const ImagesData& imagesData, const Folders& currentFolder) const {
    if (currentFolder.folders.size() > 1) {
        return const_cast<Folders*>(&currentFolder);
    }
    for (const auto& folder : currentFolder.folders) {
        for (ImageData imageData : imagesData.imagesData) {
            for (Folders folderBis : imageData.folders.folders) {
                if (folderBis.name == folder.name) {
                    return const_cast<Folders*>(&currentFolder);
                }
            }
        }
        return findFirstFolderWithAllImages(imagesData, folder);
    }

    return const_cast<Folders*>(&currentFolder);
}

void Data::copyTo(Folders rootFolders, std::string destinationPath, bool dateInName) {
    std::string initialFolder = fs::path(destinationPath).filename().string();

    QProgressDialog progressDialog("Exporting images...", "Cancel", 0, imagesData.imagesData.size());
    progressDialog.setWindowModality(Qt::ApplicationModal);
    progressDialog.show();

    int progress = 0;

    for (auto& imageData : imagesData.imagesData) {
        for (auto& folder : imageData.folders.folders) {
            std::string fileName = fs::path(imageData.folders.name).filename().string();

            std::string folderName = folder.name;

            size_t pos = folderName.find(initialFolder);
            if (pos != std::string::npos) {
                folderName = folderName.substr(pos + initialFolder.length());
            }

            std::string destinationFile;

            if (dateInName) {
                imageData.loadData();
                Exiv2::ExifData exifData = imageData.getMetaData()->getExifData();
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
            if (!imageData.cropSizes.empty()) {
                QImage image(QString::fromStdString(imageData.folders.name));
                if (!image.isNull()) {
                    std::vector<QPoint> cropPoints = imageData.cropSizes.back();
                    if (cropPoints.size() == 2) {
                        QRect cropRect = QRect(cropPoints[0], cropPoints[1]).normalized();
                        image = image.copy(cropRect);
                    }
                }
                image.save(QString::fromStdString(destinationFile));

                Exiv2::Image::AutoPtr srcImage = Exiv2::ImageFactory::open(imageData.folders.name);
                srcImage->readMetadata();
                Exiv2::Image::AutoPtr destImage = Exiv2::ImageFactory::open(destinationFile);
                destImage->setExifData(srcImage->exifData());
                destImage->setIptcData(srcImage->iptcData());
                destImage->setXmpData(srcImage->xmpData());
                destImage->writeMetadata();
            } else {
                QFile::copy(QString::fromStdString(imageData.folders.name), QString::fromStdString(destinationFile));
            }
            if (progressDialog.wasCanceled()) {
                return;
            }
            progressDialog.setValue(progress++);
            QApplication::processEvents();
        }
    }
}

QImage Data::rotateQImage(QImage image, std::string imagePath) {
    ImageData* imageData = imagesData.getImageData(imagePath);

    if (imageData != nullptr) {
        int orientation = imageData->orientation;

        switch (orientation) {
            case 2:
                image = image.mirrored(true, false);
                break;
            case 3:
                image = image.transformed(QTransform().rotate(180));
                break;
            case 4:
                image = image.mirrored(false, true);
                break;
            case 5:
                image = image.mirrored(true, false).transformed(QTransform().rotate(90));
                break;
            case 6:
                image = image.transformed(QTransform().rotate(90));
                break;
            case 7:
                image = image.mirrored(true, false).transformed(QTransform().rotate(-90));
                break;
            case 8:
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
        folderPath = initialFolderPath + "/" + folder.name;
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
                qDebug() << "Couldn't create directories for save file : " << fs::path(filePath).parent_path();
                return;
            }
        }

        outFile.open(filePath, std::ios::binary);
        if (!outFile) {
            qDebug() << "Couldn't open save file : " << filePath;
            return;
        }
    }

    size_t imagesDataSize = imagesData.get()->size();
    outFile.write(reinterpret_cast<const char*>(&imagesDataSize),
                  sizeof(imagesDataSize));
    for (const auto& imageData : *imagesData.get()) {
        imageData.save(outFile);
    }

    size_t deletedImagesDataSize = deletedImagesData.get()->size();
    outFile.write(reinterpret_cast<const char*>(&deletedImagesDataSize),
                  sizeof(deletedImagesDataSize));
    for (const auto& imageData : *deletedImagesData.get()) {
        imageData.save(outFile);
    }

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

    outFile.close();
}

void Data::loadData() {
    std::string filePath = IMAGESDATA_SAVE_DATA_PATH;
    std::ifstream inFile(filePath, std::ios::binary);
    if (!inFile) {
        qDebug() << "Couldn't open load file : " << filePath;
        return;
    }

    size_t imagesDataSize;
    inFile.read(reinterpret_cast<char*>(&imagesDataSize), sizeof(imagesDataSize));
    for (size_t i = 0; i < imagesDataSize; ++i) {
        ImageData imageData;
        imageData.load(inFile);

        imagesData.get()->push_back(imageData);
    }

    // Update imageMap with pointers to ImageData in imagesData
    for (auto& imageData : *imagesData.get()) {
        imagesData.imageMap[imageData.getImagePath()] = &imageData;
    }

    size_t deletedImagesDataSize;
    inFile.read(reinterpret_cast<char*>(&deletedImagesDataSize),
                sizeof(deletedImagesDataSize));
    for (size_t i = 0; i < deletedImagesDataSize; ++i) {
        ImageData imageData;
        imageData.load(inFile);
        deletedImagesData.addImage(imageData);
    }

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
    // TODO save currentFolder

    inFile.close();
}

void Data::cancelTasks() {
    // TODO handle
    // qThreadPool->clear();
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
    if (lastActionsDone.size() > 0) {
        auto action = lastActionsDone.back();
        addAction(action.unDo, action.reDo);
        lastActionsDone.pop_back();
        action.reDo();
    }
}

void Data::unDoAction() {
    if (lastActions.size() > 0) {
        auto action = lastActions.back();
        addActionDone(action);
        lastActions.pop_back();
        action.unDo();
    }
}

void Data::sortImagesData(QProgressDialog& progressDialog) {
    progressDialog.setLabelText("Loading imagesData ...");
    progressDialog.setValue(0);
    progressDialog.setMaximum(imagesData.get()->size());
    progressDialog.show();
    QApplication::processEvents();

    int progress = 0;

    for (auto& imageData : *imagesData.get()) {
        imageData.loadData();
        imageData.clearMetaData();
        progress++;
        progressDialog.setValue(progress);
        QApplication::processEvents();
    }

    progressDialog.setLabelText("Sorting ...");
    progressDialog.setValue(0);
    int n = imagesData.get()->size();
    // TODO mieux estimer X( estimatedSteps
    int estimatedSteps = static_cast<int>(n * std::log(n) * 3);
    progressDialog.setMaximum(estimatedSteps);
    progressDialog.show();
    QApplication::processEvents();

    auto& data = *imagesData.get();
    std::sort(data.begin(), data.end(), [&progress, &progressDialog](const ImageData& a, const ImageData& b) {
        progress++;
        progressDialog.setValue(progress);
        QApplication::processEvents();

        return a.date > b.date;
    });
    for (auto& imageData : *imagesData.get()) {
        imageData.metaData.clear();
    }
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
    if (isExifTurnOrMirror(extension)) {
        exifRotate(nbr, 90, reload);
        if (action) {
            addAction(
                [this, nbr, reload]() {
                    int time = 0;
                    if (imagesData.imageNumber != nbr) {
                        imagesData.imageNumber = nbr;
                        reload();
                        time = TIME_UNDO_VISUALISATION;
                    }
                    QTimer::singleShot(time, [this, nbr, reload]() {
                        exifRotate(nbr, -90, reload);
                    });
                },
                [this, nbr, reload]() {
                    int time = 0;
                    if (imagesData.imageNumber != nbr) {
                        imagesData.imageNumber = nbr;
                        reload();
                        time = TIME_UNDO_VISUALISATION;
                    }
                    QTimer::singleShot(time, [this, nbr, reload]() {
                        exifRotate(nbr, 90, reload);
                    });
                });
        }
    } else if (isRealTurnOrMirror(extension)) {
        realRotate(nbr, 90, reload);
        if (action) {
            addAction(
                [this, nbr, reload]() {
                    int time = 0;
                    if (imagesData.imageNumber != nbr) {
                        imagesData.imageNumber = nbr;
                        reload();
                        time = TIME_UNDO_VISUALISATION;
                    }
                    QTimer::singleShot(time, [this, nbr, reload]() {
                        realRotate(nbr, -90, reload);
                    });
                },
                [this, nbr, reload]() {
                    int time = 0;
                    if (imagesData.imageNumber != nbr) {
                        imagesData.imageNumber = nbr;
                        reload();
                        time = TIME_UNDO_VISUALISATION;
                    }
                    QTimer::singleShot(time, [this, nbr, reload]() {
                        realRotate(nbr, 90, reload);
                    });
                });
        }
    }
}

void Data::rotateRight(int nbr, std::string extension, std::function<void()> reload, bool action) {
    if (isExifTurnOrMirror(extension)) {
        exifRotate(nbr, -90, reload);

        if (action) {
            addAction(
                [this, nbr, reload]() {
                    int time = 0;
                    if (imagesData.imageNumber != nbr) {
                        imagesData.imageNumber = nbr;
                        reload();
                        time = TIME_UNDO_VISUALISATION;
                    }
                    QTimer::singleShot(time, [this, nbr, reload]() {
                        exifRotate(nbr, 90, reload);
                    });
                },
                [this, nbr, reload]() {
                    int time = 0;
                    if (imagesData.imageNumber != nbr) {
                        imagesData.imageNumber = nbr;
                        reload();
                        time = TIME_UNDO_VISUALISATION;
                    }
                    QTimer::singleShot(time, [this, nbr, reload]() {
                        exifRotate(nbr, -90, reload);
                    });
                });
        }
    } else if (isRealTurnOrMirror(extension)) {
        realRotate(nbr, -90, reload);
        if (action) {
            addAction(
                [this, nbr, reload]() {
                    int time = 0;
                    if (imagesData.imageNumber != nbr) {
                        imagesData.imageNumber = nbr;
                        reload();
                        time = TIME_UNDO_VISUALISATION;
                    }
                    QTimer::singleShot(time, [this, nbr, reload]() {
                        realRotate(nbr, 90, reload);
                    });
                },
                [this, nbr, reload]() {
                    int time = 0;
                    if (imagesData.imageNumber != nbr) {
                        imagesData.imageNumber = nbr;
                        reload();
                        time = TIME_UNDO_VISUALISATION;
                    }
                    QTimer::singleShot(time, [this, nbr, reload]() {
                        realRotate(nbr, -90, reload);
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
    ImageData* imageData = imagesData.getImageData(nbr);

    if (!isTurnable(imageData->getImagePath())) {
        return;
    }
    int orientation = imageData->orientation;
    if (rotation == 90) {
        switch (orientation) {
            case 1:
                orientation = 8;
                break;
            case 2:
                orientation = 7;
                break;
            case 3:
                orientation = 6;
                break;
            case 4:
                orientation = 5;
                break;
            case 5:
                orientation = 2;
                break;
            case 6:
                orientation = 1;
                break;
            case 7:
                orientation = 4;
                break;
            case 8:
                orientation = 3;
                break;
            default:
                break;
        }
    }
    if (rotation == -90) {
        switch (orientation) {
            case 1:
                orientation = 6;
                break;
            case 2:
                orientation = 5;
                break;
            case 3:
                orientation = 8;
                break;
            case 4:
                orientation = 7;
                break;
            case 5:
                orientation = 4;
                break;
            case 6:
                orientation = 3;
                break;
            case 7:
                orientation = 2;
                break;
            case 8:
                orientation = 1;
                break;
            default:
                break;
        }
    }

    imageData->turnImage(orientation);

    imageData->saveMetaData();

    reload();
}

void Data::mirrorUpDown(int nbr, std::string extension, std::function<void()> reload, bool action) {
    if (isExifTurnOrMirror(extension)) {
        realMirror(nbr, true, reload);

        if (action) {
            addAction(
                [this, nbr, reload]() {
                    int time = 0;
                    if (imagesData.imageNumber != nbr) {
                        imagesData.imageNumber = nbr;
                        reload();
                        time = TIME_UNDO_VISUALISATION;
                    }
                    QTimer::singleShot(time, [this, nbr, reload]() {
                        realMirror(nbr, true, reload);
                    });
                },
                [this, nbr, reload]() {
                    int time = 0;
                    if (imagesData.imageNumber != nbr) {
                        imagesData.imageNumber = nbr;
                        reload();
                        time = TIME_UNDO_VISUALISATION;
                    }
                    QTimer::singleShot(time, [this, nbr, reload]() {
                        realMirror(nbr, true, reload);
                    });
                });
        }
    } else if (isRealTurnOrMirror(extension)) {
        realMirror(nbr, true, reload);

        if (action) {
            addAction(
                [this, nbr, reload]() {
                    int time = 0;
                    if (imagesData.imageNumber != nbr) {
                        imagesData.imageNumber = nbr;
                        reload();
                        time = TIME_UNDO_VISUALISATION;
                    }
                    QTimer::singleShot(time, [this, nbr, reload]() {
                        realMirror(nbr, true, reload);
                    });
                },
                [this, nbr, reload]() {
                    int time = 0;
                    if (imagesData.imageNumber != nbr) {
                        imagesData.imageNumber = nbr;
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
    if (isExifTurnOrMirror(extension)) {
        exifMirror(nbr, false, reload);

        if (action) {
            addAction(
                [this, nbr, reload]() {
                    int time = 0;
                    if (imagesData.imageNumber != nbr) {
                        imagesData.imageNumber = nbr;
                        reload();
                        time = TIME_UNDO_VISUALISATION;
                    }
                    QTimer::singleShot(time, [this, nbr, reload]() {
                        exifMirror(nbr, false, reload);
                    });
                },
                [this, nbr, reload]() {
                    int time = 0;
                    if (imagesData.imageNumber != nbr) {
                        imagesData.imageNumber = nbr;
                        reload();
                        time = TIME_UNDO_VISUALISATION;
                    }
                    QTimer::singleShot(time, [this, nbr, reload]() {
                        exifMirror(nbr, false, reload);
                    });
                });
        }
    } else if (isRealTurnOrMirror(extension)) {
        realMirror(nbr, false, reload);
        if (action) {
            addAction(
                [this, nbr, reload]() {
                    int time = 0;
                    if (imagesData.imageNumber != nbr) {
                        imagesData.imageNumber = nbr;
                        reload();
                        time = TIME_UNDO_VISUALISATION;
                    }
                    QTimer::singleShot(time, [this, nbr, reload]() {
                        realMirror(nbr, false, reload);
                    });
                },
                [this, nbr, reload]() {
                    int time = 0;
                    if (imagesData.imageNumber != nbr) {
                        imagesData.imageNumber = nbr;
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
    int orientation = imageData->orientation;
    if (UpDown) {
        switch (orientation) {
            case 1:
                orientation = 4;
                break;
            case 3:
                orientation = 2;
                break;
            case 6:
                orientation = 5;
                break;
            case 8:
                orientation = 7;
                break;
            case 2:
                orientation = 3;
                break;
            case 4:
                orientation = 1;
                break;
            case 5:
                orientation = 6;
                break;
            case 7:
                orientation = 8;
                break;
            default:
                break;
        }
    } else {
        switch (orientation) {
            case 1:
                orientation = 2;
                break;
            case 3:
                orientation = 4;
                break;
            case 6:
                orientation = 7;
                break;
            case 8:
                orientation = 5;
                break;
            case 2:
                orientation = 1;
                break;
            case 4:
                orientation = 3;
                break;
            case 5:
                orientation = 8;
                break;
            case 7:
                orientation = 6;
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
    QString outputPath = QString::fromStdString(imagesData.getCurrentImageData()->folders.name);
    QImage image = loadImage(nullptr, imagesData.getCurrentImageData()->folders.name, QSize(0, 0), false);
    if (UpDown) {
        image = image.mirrored(false, true);
    } else {
        image = image.mirrored(true, false);
    }
    if (!image.save(outputPath)) {
        qDebug() << "Erreur lors de la sauvegarde de l'image : " << outputPath;
    }
    unloadFromCache(imagesData.getCurrentImageData()->folders.name);
    loadInCache(imagesData.getCurrentImageData()->folders.name);
    createAllThumbnail(imagesData.getCurrentImageData()->folders.name, 512);
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
    return currentFolder;
}

ImagesData* Data::getImagesData() {
    return &imagesData;
}

void Data::removeImageFromFolders(ImageData& imageData) {
    Folders* rootFolders = getRootFolders();
    for (auto& folder : imageData.getFolders()) {
        std::string folderPath = *folder.getName();
        Folders* currentFolder = rootFolders;
        std::istringstream iss(folderPath);
        std::string token;
        bool run = true;
        while (run && std::getline(iss, token, '/')) {
            auto it = std::find_if(currentFolder->folders.begin(), currentFolder->folders.end(),
                                   [&token](const Folders& f) { return f.name == token; });
            if (it != currentFolder->folders.end()) {
                currentFolder = &(*it);
            } else {
                currentFolder = nullptr;
                // break;
                run = false;
            }
        }
        currentFolder->files.erase(std::remove(currentFolder->files.begin(), currentFolder->files.end(), imageData.getImagePath()), currentFolder->files.end());

        qDebug() << "Remove image from folder : " << *currentFolder->getName();
    }
}