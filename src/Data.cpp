#include "Data.hpp"

namespace fs = std::filesystem;  // Alias pour simplifier le code

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

// Supprime de imagesData les images dans deletedImagesData
void Data::removeDeletedImages() {
    for (const auto& deletedImage : *deletedImagesData.get()) {
        auto it = std::find(imagesData.get()->begin(), imagesData.get()->end(),
                            deletedImage);
        if (it != imagesData.get()->end()) {
            imagesData.get()->erase(it);
            deletedImage.print();
        }
    }

    qDebug() << "All images deleted";
}

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
    std::string extension = imagesData.getCurrentImageData()->getImageExtension();

    if (crop) {
        ImageData* imageData = imagesData.getImageData(imagePath);
        if (imageData != nullptr) {
            ImageData* imageData = imagesData.getImageData(imagePath);
            if (imageData != nullptr && !imageData->cropSizes.empty()) {
                std::vector<QPoint> cropPoints = imageData->cropSizes.back();
                if (cropPoints.size() == 2) {
                    QRect cropRect = QRect(cropPoints[0], cropPoints[1]).normalized();

                    // Vérifier que le rectangle de découpe est dans les limites de l'image
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
    if (rotation) {
        if (isExifTurnOrMirror(extension)) {
            image = rotateQImage(image, imagePath);
        }
    }
    return image;
}

QImage Data::loadImageNormal(QWidget* parent, std::string imagePath, QSize size,
                             bool setSize, int thumbnail, bool force) {
    auto it = imageCache->find(imagePath);
    if (it != imageCache->end()) {
        return it->second.image;
    }
    if (imagePath.at(0) == ':') {
        if (darkMode) {
            imagePath.insert(imagePath.find_first_of(':') + 1, "/255-255-255-255");
        } else {
            imagePath.insert(imagePath.find_first_of(':') + 1, "/0-0-0-255");
        }
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
    qDebug() << "Image cache size: " << static_cast<double>(cacheSize) / (1024 * 1024) << " MB";

    return image;
}

void Data::loadInCacheAsync(std::string imagePath, std::function<void()> callback, bool setSize, QSize size, bool force) {
    LoadImageTask* task = new LoadImageTask(this, imagePath, setSize, size, force, callback);
    QThreadPool::globalInstance()->start(task);
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
    if (maxDim == 128) {
        outputImage = THUMBNAIL_PATH + "/normal/" +
                      std::to_string(hashValue) + extension;
    } else if (maxDim == 256) {
        outputImage = THUMBNAIL_PATH + "/large/" +
                      std::to_string(hashValue) + extension;
    } else if (maxDim == 512) {
        outputImage = THUMBNAIL_PATH + "/x-large/" +
                      std::to_string(hashValue) + extension;
    }

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
        qDebug() << "Creating thumbnail: " << imagePath;
        createThumbnail(imagePath, maxDim);
    }
}
bool Data::hasThumbnail(const std::string& imagePath, const int maxDim) {
    std::hash<std::string> hasher;
    size_t hashValue = hasher(fs::path(imagePath).filename().string());

    std::string extension = ".png";

    std::string outputImage;

    if (maxDim == 128) {
        outputImage = THUMBNAIL_PATH + "/normal/" +
                      std::to_string(hashValue) + extension;
    } else if (maxDim == 256) {
        outputImage = THUMBNAIL_PATH + "/large/" +
                      std::to_string(hashValue) + extension;
    } else if (maxDim == 512) {
        outputImage = THUMBNAIL_PATH + "/x-large/" +
                      std::to_string(hashValue) + extension;
    }
    return fs::exists(outputImage);
}

void Data::createAllThumbnailIfNotExists(const std::string& imagePath, const int size) {
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

    // std::string extension = fs::path(imagePath).extension().string();
    std::string extension = ".png";

    std::string outputImage;

    if (size == 128) {
        outputImage = THUMBNAIL_PATH + "/normal/" +
                      std::to_string(hashValue) + extension;
    } else if (size == 256) {
        outputImage = THUMBNAIL_PATH + "/large/" +
                      std::to_string(hashValue) + extension;
    } else if (size == 512) {
        outputImage = THUMBNAIL_PATH + "/x-large/" +
                      std::to_string(hashValue) + extension;
    }
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
        // TODO ajouter le clear des metaDAta et juste sauvegarder la rotation

        return true;
    }
    return false;
}

bool Data::unloadFromFutures(std::string imagePath) {
    // auto it = futures.find(QString::fromStdString(imagePath));
    // if (it != futures.end())
    // {
    //     futures.erase(it);
    //     return true;
    // }
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
    progressDialog.setWindowModality(Qt::WindowModal);
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

                // Copy metadata
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
        // Vérifiez si le répertoire parent existe
        if (!fs::exists(fs::path(filePath).parent_path())) {
            // Essayez de créer les répertoires nécessaires
            if (!fs::create_directories(fs::path(filePath).parent_path())) {
                qDebug() << "Couldn't create directories for save file : " << fs::path(filePath).parent_path();
                return;
            }
        }

        // Essayez d'ouvrir le fichier à nouveau après avoir créé les répertoires
        outFile.open(filePath, std::ios::binary);
        if (!outFile) {
            qDebug() << "Couldn't open save file : " << filePath;
            return;
        }
    }

    // Serialize imagesData

    size_t imagesDataSize = imagesData.get()->size();
    outFile.write(reinterpret_cast<const char*>(&imagesDataSize),
                  sizeof(imagesDataSize));
    for (const auto& imageData : *imagesData.get()) {
        imageData.save(outFile);
    }

    // Serialize deletedImagesData
    size_t deletedImagesDataSize = deletedImagesData.get()->size();
    outFile.write(reinterpret_cast<const char*>(&deletedImagesDataSize),
                  sizeof(deletedImagesDataSize));
    for (const auto& imageData : *deletedImagesData.get()) {
        imageData.save(outFile);
    }

    // Serialize options
    size_t optionsSize = options.size();
    outFile.write(reinterpret_cast<const char*>(&optionsSize),
                  sizeof(optionsSize));
    for (const auto& [key, option] : options) {
        size_t keySize = key.size();
        outFile.write(reinterpret_cast<const char*>(&keySize), sizeof(keySize));
        outFile.write(key.c_str(), keySize);
        size_t valueSize = option.value.size();
        outFile.write(reinterpret_cast<const char*>(&valueSize), sizeof(valueSize));
        outFile.write(option.value.c_str(), valueSize);
    }

    // Serialize rootFolders
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
        std::string key, value;
        size_t keySize, valueSize;
        inFile.read(reinterpret_cast<char*>(&keySize), sizeof(keySize));
        key.resize(keySize);
        inFile.read(&key[0], keySize);
        inFile.read(reinterpret_cast<char*>(&valueSize), sizeof(valueSize));
        value.resize(valueSize);
        inFile.read(&value[0], valueSize);
        options[key] = Option("string", value);
    }
    rootFolders.load(inFile);

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
    if (!lastActionsDone.empty()) {
        std::cerr << "reDoAction" << std::endl;
        auto action = lastActionsDone.back();
        addAction(action.unDo, action.reDo);
        lastActionsDone.pop_back();
        action.reDo();
    }
}

void Data::unDoAction() {
    if (!lastActions.empty()) {
        std::cerr << "unDoAction" << std::endl;
        auto action = lastActions.back();
        addActionDone(action);
        lastActions.pop_back();
        action.unDo();
    }
}
void Data::clearActions() {
    lastActions.clear();
}

void Data::sortImagesData(QProgressDialog& progressDialog) {
    progressDialog.setLabelText("Loading imagesData ...");
    progressDialog.setValue(0);
    progressDialog.setMaximum(imagesData.get()->size());
    QApplication::processEvents();

    int progress = 0;

    for (auto& imageData : *imagesData.get()) {
        imageData.loadData();
        imageData.clearMetaData();
        progress++;
        progressDialog.setValue(progress);
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

void Data::rotateLeft(int nbr, std::string extension, std::function<void()> reload) {
    bool savedBefore = saved;

    if (isExifTurnOrMirror(extension)) {
        // rotateLeftJpg();
        exifRotate(nbr, 90, reload);

        addAction(
            [this, nbr, savedBefore, reload]() {
                int time = 0;
                if (imagesData.imageNumber != nbr) {
                    imagesData.imageNumber = nbr;
                    reload();
                    time = TIME_UNDO_VISUALISATION;
                }
                if (savedBefore) {
                    saved = true;
                }
                QTimer::singleShot(time, [this, nbr, reload]() {
                    // rotateRightJpg();
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
                saved = false;
                QTimer::singleShot(time, [this, nbr, reload]() {
                    exifRotate(nbr, 90, reload);

                    // rotateLeftJpg();
                });
            });
    } else if (isRealTurnOrMirror(extension)) {
        // rotateLeftPng();
        realRotate(nbr, 90, reload);

        addAction(
            [this, nbr, savedBefore, reload]() {
                int time = 0;
                if (imagesData.imageNumber != nbr) {
                    imagesData.imageNumber = nbr;
                    reload();
                    time = TIME_UNDO_VISUALISATION;
                }
                if (savedBefore) {
                    saved = true;
                }
                QTimer::singleShot(time, [this, nbr, reload]() {
                    // rotateRightPng();
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
                saved = false;
                QTimer::singleShot(time, [this, nbr, reload]() {
                    // rotateLeftPng();
                    realRotate(nbr, 90, reload);
                });
            });
    }
    saved = false;
}

void Data::rotateRight(int nbr, std::string extension, std::function<void()> reload) {
    bool savedBefore = saved;

    if (isExifTurnOrMirror(extension)) {
        // rotateRightJpg();
        exifRotate(nbr, -90, reload);

        addAction(
            [this, nbr, savedBefore, reload]() {
                int time = 0;
                if (imagesData.imageNumber != nbr) {
                    imagesData.imageNumber = nbr;
                    reload();
                    time = TIME_UNDO_VISUALISATION;
                }
                if (savedBefore) {
                    saved = true;
                }
                QTimer::singleShot(time, [this, nbr, reload]() {
                    // rotateLeftJpg();
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
                saved = false;
                QTimer::singleShot(time, [this, nbr, reload]() {
                    // rotateRightJpg();
                    exifRotate(nbr, -90, reload);
                });
            });
    } else if (isRealTurnOrMirror(extension)) {
        // rotateRightPng();
        realRotate(nbr, -90, reload);

        addAction(
            [this, nbr, savedBefore, reload]() {
                int time = 0;
                if (imagesData.imageNumber != nbr) {
                    imagesData.imageNumber = nbr;
                    reload();
                    time = TIME_UNDO_VISUALISATION;
                }
                if (savedBefore) {
                    saved = true;
                }
                QTimer::singleShot(time, [this, nbr, reload]() {
                    realRotate(nbr, 90, reload);

                    // rotateLeftJpg();
                });
            },
            [this, nbr, reload]() {
                int time = 0;
                if (imagesData.imageNumber != nbr) {
                    imagesData.imageNumber = nbr;
                    reload();
                    time = TIME_UNDO_VISUALISATION;
                }
                saved = false;
                QTimer::singleShot(time, [this, nbr, reload]() {
                    // rotateRightPng();
                    realRotate(nbr, -90, reload);
                });
            });
    }
    saved = false;
}

void Data::realRotate(int nbr, int rotation, std::function<void()> reload) {
    QString outputPath = QString::fromStdString(imagesData.getCurrentImageData()->folders.name);
    QImage image = loadImage(nullptr, imagesData.getCurrentImageData()->folders.name, QSize(0, 0), false);
    image = image.transformed(QTransform().rotate(rotation));
    if (!image.save(outputPath)) {
        qDebug() << "Erreur lors de la sauvegarde de l'image : " << outputPath;
    }
    unloadFromCache(imagesData.getCurrentImageData()->folders.name);
    loadInCache(imagesData.getCurrentImageData()->folders.name);
    createAllThumbnail(imagesData.getCurrentImageData()->folders.name, 512);
    reload();
}

void Data::exifRotate(int nbr, int rotation, std::function<void()> reload) {
    ImageData* imageData = imagesData.getImageData(nbr);

    if (!isTurnable(imageData->getImagePath())) {
        return;
    }
    int orientation = imageData->orientation;
    if (rotation == 90) {
        qDebug() << "rotateLeftJpg";
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
        qDebug() << "rotateRightJpg";

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

void Data::mirrorUpDown(int nbr, std::string extension, std::function<void()> reload) {
    bool savedBefore = saved;

    if (isExifTurnOrMirror(extension)) {
        // mirrorUpDownJpg();
        realMirror(nbr, true, reload);
        addAction(
            [this, nbr, savedBefore, reload]() {
                int time = 0;
                if (imagesData.imageNumber != nbr) {
                    imagesData.imageNumber = nbr;
                    reload();
                    time = TIME_UNDO_VISUALISATION;
                }
                if (savedBefore) {
                    saved = true;
                }
                QTimer::singleShot(time, [this, nbr, reload]() {
                    // mirrorUpDownJpg();
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
                saved = false;
                QTimer::singleShot(time, [this, nbr, reload]() {
                    // mirrorUpDownJpg();
                    realMirror(nbr, true, reload);
                });
            });
    } else if (isRealTurnOrMirror(extension)) {
        // mirrorUpDownPng();
        realMirror(nbr, true, reload);

        addAction(
            [this, nbr, savedBefore, reload]() {
                int time = 0;
                if (imagesData.imageNumber != nbr) {
                    imagesData.imageNumber = nbr;
                    reload();
                    time = TIME_UNDO_VISUALISATION;
                }
                if (savedBefore) {
                    saved = true;
                }
                QTimer::singleShot(time, [this, nbr, reload]() {
                    // mirrorUpDownPng();
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
                saved = false;
                QTimer::singleShot(time, [this, nbr, reload]() {
                    // mirrorUpDownPng();
                    realMirror(nbr, true, reload);
                });
            });
    }
    saved = false;
}

void Data::mirrorLeftRight(int nbr, std::string extension, std::function<void()> reload) {
    bool savedBefore = saved;

    if (isExifTurnOrMirror(extension)) {
        // mirrorLeftRightJpg();
        exifMirror(nbr, false, reload);
        addAction(
            [this, nbr, savedBefore, reload]() {
                int time = 0;
                if (imagesData.imageNumber != nbr) {
                    imagesData.imageNumber = nbr;
                    reload();
                    time = TIME_UNDO_VISUALISATION;
                }
                if (savedBefore) {
                    saved = true;
                }
                QTimer::singleShot(time, [this, nbr, reload]() {
                    // mirrorLeftRightJpg();
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
                saved = false;
                QTimer::singleShot(time, [this, nbr, reload]() {
                    // mirrorLeftRightJpg();
                    exifMirror(nbr, false, reload);
                });
            });
    } else if (isRealTurnOrMirror(extension)) {
        // mirrorLeftRightPng();
        realMirror(nbr, false, reload);
        addAction(
            [this, nbr, savedBefore, reload]() {
                int time = 0;
                if (imagesData.imageNumber != nbr) {
                    imagesData.imageNumber = nbr;
                    reload();
                    time = TIME_UNDO_VISUALISATION;
                }
                if (savedBefore) {
                    saved = true;
                }
                QTimer::singleShot(time, [this, nbr, reload]() {
                    // mirrorLeftRightPng();
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
                saved = false;
                QTimer::singleShot(time, [this, nbr, reload]() {
                    // mirrorLeftRightPng();
                    realMirror(nbr, false, reload);
                });
            });
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