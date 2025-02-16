#include "LoadImage.hpp"

void ThumbnailTask::run() {
    for (int i = start; i < end; ++i) {
        ImageData* imageData = imagesData->getImageData(i);
        data->createThumbnailIfNotExists(imageData->getImagePath(), 128);
        data->createThumbnailIfNotExists(imageData->getImagePath(), 256);
        data->createThumbnailIfNotExists(imageData->getImagePath(), 512);
        data->unloadFromCache(imageData->getImagePath());
    }
}

void addImagesFromFolder(Data* data, QWidget* parent) {
    QProgressDialog progressDialog(parent);
    // Necessaire : sinon s'affiche tout seul au bout de 5 s
    progressDialog.show();
    progressDialog.hide();

    if (!addSelectedFilesToFolders(data, parent, progressDialog)) {
        return;
    }
    if (data->imagesData.get()->size() <= 0) {
        return;
    }

    auto start = std::chrono::high_resolution_clock::now();

    data->sortImagesData(progressDialog);

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    qDebug() << "Sorting images took " << elapsed.count() << " seconds.";

    data->saveData();
}

// Fonction pour ajouter des fichiers sélectionnés à la liste des dossiers
bool addSelectedFilesToFolders(Data* data, QWidget* parent, QProgressDialog& progressDialog) {
    fileSelector fileSelector;

    QStringList selectedFiles;

    selectedFiles = fileSelector.openDirectoryDialog();

    if (selectedFiles.isEmpty()) {
        return false;
    }

    for (const QString& fileName : selectedFiles) {
        startLoadingImagesFromFolder(parent, data, fileName.toStdString(), progressDialog);
    }
    return true;
}

std::string getDirectoryFromUser(QWidget* parent) {
    QFileDialog dialog(parent);
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setOption(QFileDialog::ShowDirsOnly, true);

    if (dialog.exec()) {
        QStringList selectedDirectories = dialog.selectedFiles();
        if (!selectedDirectories.isEmpty()) {
            return selectedDirectories.first().toStdString();
        }
    }
    return "";
}

// Charges dans un imagesData toutes les données des images dans un dossier et ses sous dossier
bool startLoadingImagesFromFolder(QWidget* parent, Data* data, const std::string imagePaths, QProgressDialog& progressDialog) {
    int nbrImage = 0;

    data->rootFolders = Folders("");

    data->imagesData = ImagesData(std::vector<ImageData>{});

    qDebug() << "adding to tree: " << data->rootFolders.folders.size();

    addFilesToTree(&data->rootFolders, imagePaths);

    qDebug() << "Root folders: " << data->rootFolders.folders.size();

    countImagesFromFolder(imagePaths, nbrImage);

    qDebug() << "Number of images: " << nbrImage;

    progressDialog.setLabelText("Loading images...");
    progressDialog.setCancelButtonText("Cancel");
    progressDialog.setRange(0, nbrImage);
    progressDialog.setParent(parent);
    progressDialog.setWindowModality(Qt::WindowModal);
    progressDialog.show();

    int loaded = 0;

    ImagesData* imagesData = new ImagesData(std::vector<ImageData>{});

    if (!loadImagesFromFolder(imagePaths, imagePaths, imagesData, loaded, progressDialog)) {
        qDebug() << "loadImagesFromFolder failed : " << imagePaths;
        return false;
    }

    data->imagesData = *imagesData;

    progressDialog.setValue(0);
    progressDialog.show();
    progressDialog.setLabelText("Loading images googleData...");
    if (!loadImagesMetaDataOfGoogle(imagesData, progressDialog)) {
        return false;
    }

    progressDialog.setValue(0);
    progressDialog.show();
    progressDialog.setLabelText("Loading images thumbnail...");
    QApplication::processEvents();

    try {
        // TODO ne marche pas bien si il y a deja des thumbnail

        // TODO utilise le threadPool mais fait beug l'application
        int totalImages = imagesData->get()->size();
        int numThreads = QThreadPool::globalInstance()->maxThreadCount();
        // int imagesPerThread = totalImages / numThreads;
        int imagesPerThread = 10;

        int thumbnailsCreated = 0;

        // List with number from 0 to totalImages
        std::vector<int> imageIndices(totalImages);
        std::iota(imageIndices.begin(), imageIndices.end(), 0);

        QFutureWatcher<void> watcher;

        QObject::connect(&watcher, &QFutureWatcher<void>::progressValueChanged, &progressDialog, &QProgressDialog::setValue);
        QObject::connect(&watcher, &QFutureWatcher<void>::finished, &progressDialog, &QProgressDialog::reset);

        QFuture<void> future = QtConcurrent::run([&]() {
            try {
                std::atomic<int> currentIndex(0);

                auto processImages = [&]() {
                    while (true) {
                        int start = currentIndex.fetch_add(imagesPerThread);
                        if (start >= totalImages) break;
                        int end = std::min(start + imagesPerThread, totalImages);

                        ThumbnailTask* task = new ThumbnailTask(data, imagesData, start, end);
                        QThreadPool::globalInstance()->start(task);
                    }
                };

                for (int i = 0; i < numThreads; ++i) {
                    processImages();
                }

            } catch (const std::exception& e) {
                std::cerr << e.what() << '\n';
            }
        });

        watcher.setFuture(future);

        while (QThreadPool::globalInstance()->activeThreadCount() > 0 && imageIndices.size() > 0) {
            qDebug() << "starting count" << imageIndices.size();
            QThread::msleep(1000);
            for (int index : imageIndices) {
                ImageData* imageData = imagesData->getImageData(index);
                if (data->hasThumbnail(imageData->getImagePath(), 128) &&
                    data->hasThumbnail(imageData->getImagePath(), 256) &&
                    data->hasThumbnail(imageData->getImagePath(), 512)) {
                    ++thumbnailsCreated;
                    imageIndices.erase(std::remove(imageIndices.begin(), imageIndices.end(), index), imageIndices.end());
                }
            }
            progressDialog.setValue(thumbnailsCreated);
            QCoreApplication::processEvents();
            qDebug() << "Number of thumbnails created: " << thumbnailsCreated << "/" << totalImages;
            qDebug() << "Number of active thread: " << QThreadPool::globalInstance()->activeThreadCount();
        }
        return true;
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return false;
    }
}

std::string readFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + filePath);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::map<std::string, std::string> parseJsonToMap(const std::string& jsonString) {
    std::map<std::string, std::string> resultMap;
    std::string key, value;
    std::istringstream jsonStream(jsonString);
    char ch;

    while (jsonStream >> ch) {
        if (ch == '"') {
            std::getline(jsonStream, key, '"');
            jsonStream >> ch;
            jsonStream >> ch;
            if (ch == '"') {
                std::getline(jsonStream, value, '"');
            } else {
                jsonStream.putback(ch);
                std::getline(jsonStream, value, ',');
                value.pop_back();
            }
            resultMap[key] = value;
        }
    }

    return resultMap;
}

std::map<std::string, std::string> openJsonFile(std::string filePath) {
    std::string jsonString = readFile(filePath);
    std::map<std::string, std::string> jsonMap = parseJsonToMap(jsonString);
    return jsonMap;
}

// Count images in a folder and its sub folders
void countImagesFromFolder(const std::string path, int& nbrImage) {
    qDebug() << "countImagesFromFolder : " << path.c_str();
    int i = 0;

    for (const auto& entry : fs::directory_iterator(path)) {
        if (fs::is_regular_file(entry.status())) {
            if (isImage(entry.path().string())) {
                nbrImage += 1;
            }
        } else if (fs::is_directory(entry.status())) {
            countImagesFromFolder(entry.path().string(), nbrImage);

            i += 1;
        }
    }
}

// Charges concrètement dans un imagesData toutes les données des images dans un dossier et ses sous dossier
bool loadImagesFromFolder(const std::string initialPath, const std::string path, ImagesData* imagesData, int& nbrImage, QProgressDialog& progressDialog) {
    for (const auto& entry : fs::directory_iterator(path)) {
        if (progressDialog.wasCanceled()) {
            return false;
        }
        if (fs::is_regular_file(entry.status())) {
            if (isImage(entry.path().string())) {
                fs::path relativePath = fs::relative(entry.path(), fs::path(initialPath).parent_path());

                Folders folders;
                ImageData* imageData = imagesData->getImageData(entry.path().string());
                if (imageData != nullptr) {
                    folders = imageData->folders;
                } else {
                    folders = Folders(entry.path().string());
                }
                folders.addFolder(fs::absolute(entry.path()).parent_path().string());

                ImageData imageD(folders);

                imagesData->addImage(imageD);

                nbrImage += 1;
                progressDialog.setValue(nbrImage);
                QApplication::processEvents();
            }
        } else if (fs::is_directory(entry.status())) {
            loadImagesFromFolder(initialPath, entry.path().string(), imagesData, nbrImage, progressDialog);
        }
    }
    return true;
}

bool loadImagesMetaDataOfGoogle(ImagesData* imagesData, QProgressDialog& progressDialog) {
    progressDialog.setMaximum(imagesData->get()->size());
    try {
        for (int i = 0; i < imagesData->get()->size(); ++i) {
            // TODO not working for now
            if (progressDialog.wasCanceled()) {
                return false;
            }
            ImageData* imageData = imagesData->getImageData(i);
            std::string jsonFilePath = imageData->getImagePath() + ".json";

            if (fs::exists(jsonFilePath)) {
                qDebug() << "json found : " << jsonFilePath.c_str();
                std::map<std::string, std::string> jsonMap = openJsonFile(jsonFilePath);

                for (const auto& [key, value] : jsonMap) {
                    if (!key.empty() && !value.empty()) {
                        std::string exifKey = mapJsonKeyToExifKey(key);
                        if (exifKey != "") {
                            imageData->metaData.modifyExifValue(exifKey, value);
                        }
                    }
                }
                imageData->saveMetaData();
            } else {
                // displayExifData(imageData->metaData.exifMetaData);
            }
            progressDialog.setValue(i);
            QApplication::processEvents();
        }
        return true;
    } catch (const std::exception& e) {
        qDebug() << e.what();
        return false;
    }
}

std::string mapJsonKeyToExifKey(const std::string& jsonKey) {
    static const std::map<std::string, std::string> keyMap = {
        // TODO mettre tous les nécessaires
        {"description", "Exif.Image.ImageDescription"},
        {"photoTakenTime.timestamp", "Exif.Photo.DateTimeOriginal"},
        {"geoData.latitude", "Exif.GPSInfo.GPSLatitude"},
        {"geoData.longitude", "Exif.GPSInfo.GPSLongitude"},
        {"geoData.altitude", "Exif.GPSInfo.GPSAltitude"},
        {"googlePhotosOrigin.mobileUpload.deviceModel", "Exif.Image.Model"},
        {"googlePhotosOrigin.mobileUpload.deviceType", "Exif.Image.Make"},
        {"googlePhotosOrigin.mobileUpload.deviceSoftwareVersion", "Exif.Image.Software"}};

    std::string lowerJsonKey = jsonKey;
    std::transform(lowerJsonKey.begin(), lowerJsonKey.end(), lowerJsonKey.begin(), ::tolower);

    auto it = keyMap.find(lowerJsonKey);
    if (it != keyMap.end()) {
        return it->second;
    }
    return "";
}
