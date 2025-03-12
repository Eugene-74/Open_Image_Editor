#include "LoadImage.hpp"

void ThumbnailTask::run() {
    for (int i = start; i < end; ++i) {
        ImageData* imageData = data->getImagesData()->getImageData(i);

        data->createThumbnailIfNotExists(imageData->getImagePath(), 16);
        data->createThumbnailIfNotExists(imageData->getImagePath(), 128);
        data->createThumbnailIfNotExists(imageData->getImagePath(), 256);
        data->createThumbnailIfNotExists(imageData->getImagePath(), 512);
        data->unloadFromCache(imageData->getImagePath());
    }
}

void addImagesFromFolder(Data* data, QWidget* parent) {
    QProgressDialog progressDialog(parent);
    progressDialog.setWindowModality(Qt::ApplicationModal);
    progressDialog.setAutoClose(false);

    // Necessaire : sinon s'affiche tout seul au bout de 5 s
    progressDialog.show();
    progressDialog.hide();

    if (!addSelectedFilesToFolders(data, parent, progressDialog)) {
        qCritical() << "Loading canceled";

        return;
    }
    if (data->imagesData.get()->size() <= 0) {
        qCritical() << "No images found in the selected folder";
        return;
    }

    // auto images = data->getImagesData()->get();

    // ensure that it's well loaded
    data->saveData();
    data->loadData();
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

    data->imagesData = ImagesData(std::vector<ImageData*>{});

    qDebug() << "adding to tree: " << data->getRootFolders()->getFolders()->size();

    progressDialog.setLabelText("Scaning for images : ");
    progressDialog.setCancelButtonText("Cancel");
    progressDialog.setRange(0, 0);
    progressDialog.setParent(parent);
    progressDialog.show();
    QApplication::processEvents();

    // TODO cancel marche pas (le faire marcher)
    ImagesData* imagesData = new ImagesData(std::vector<ImageData*>{});
    if (!addFilesToTree(&data->rootFolders, imagesData, imagePaths, nbrImage, progressDialog)) {
        return false;
    }

    qDebug() << "Root folders: " << data->getRootFolders()->getFolders()->size();
    qDebug() << "Number of images: " << nbrImage;

    data->imagesData = *imagesData;

    progressDialog.setValue(0);
    progressDialog.setMaximum(data->getImagesData()->get()->size());
    progressDialog.setLabelText("Loading images thumbnail ...");
    progressDialog.show();
    QApplication::processEvents();

    if (!loadImagesThumbnail(data, progressDialog)) {
        return false;
    }
    return true;
}

bool loadImagesThumbnail(Data* data, QProgressDialog& progressDialog) {
    qDebug() << "Loading images thumbnail ...";
    try {
        int totalImages = data->getImagesData()->get()->size();
        int numThreads = QThreadPool::globalInstance()->maxThreadCount();
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

                        ThumbnailTask* task = new ThumbnailTask(data, start, end);
                        QThreadPool::globalInstance()->start(task);
                    }
                };

                for (int i = 0; i < numThreads; ++i) {
                    processImages();
                }

            } catch (const std::exception& e) {
                qCritical() << "loadImagesThumbnail" << e.what();
            }
        });

        watcher.setFuture(future);

        while (QThreadPool::globalInstance()->activeThreadCount() > 0 && imageIndices.size() > 0) {
            if (progressDialog.wasCanceled()) {
                QThreadPool::globalInstance()->clear();
                return false;
            }
            qDebug() << "starting count" << imageIndices.size();
            for (int index : imageIndices) {
                ImageData* imageData = data->getImagesData()->getImageData(index);
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
        qCritical() << "loadImagesThumbnail" << e.what();
        return false;
    }
    qDebug() << "images thumbnail loaded";
}

std::string readFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        qCritical() << "Could not open file: " << QString::fromStdString(filePath);
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

// Charges concrètement dans un imagesData toutes les données des images dans un dossier et ses sous dossier
// bool loadImagesFromFolder(const std::string initialPath, const std::string path, ImagesData* imagesData, int& nbrImage, QProgressDialog& progressDialog) {
//     for (const auto& entry : fs::directory_iterator(path)) {
//         if (progressDialog.wasCanceled()) {
//             return false;
//         }
//         if (fs::is_regular_file(entry.status())) {
//             if (isImage(entry.path().string())) {

//             }
//         } else if (fs::is_directory(entry.status())) {
//             loadImagesFromFolder(initialPath, entry.path().string(), imagesData, nbrImage, progressDialog);
//         }
//     }
//     return true;
// }

bool loadImagesMetaDataOfGoogle(ImagesData* imagesData, QProgressDialog& progressDialog) {
    // TODO laod persons
    return true;
}

bool addFilesToTree(Folders* currentFolder, ImagesData* imagesData, const std::string& path, int& nbrImage, QProgressDialog& progressDialog) {
    fs::path fsPath(path);

    for (const auto& part : fsPath) {
        if (progressDialog.wasCanceled()) {
            return false;
        }
        if (part == "/")
            continue;

        std::string folderName = part.string();
        if (!getIfExist(currentFolder, folderName)) {
            currentFolder->addFolder(folderName);
            currentFolder = &currentFolder->getFolders()->back();
        }
    }
    if (progressDialog.wasCanceled()) {
        return false;
    }

    if (!addSubfolders(*currentFolder, imagesData, path, nbrImage, progressDialog)) {
        return false;
    }
    return true;
}

bool addSubfolders(Folders& rootFolder, ImagesData* imagesData, const std::string& path, int& nbrImage, QProgressDialog& progressDialog) {
    auto subDirectories = fs::directory_iterator(path);

    for (const auto& entry : subDirectories) {
        if (progressDialog.wasCanceled()) {
            return false;
        }
        if (entry.is_directory()) {
            if (containImage(entry.path().string())) {
                std::string folderName = entry.path().filename().string();
                rootFolder.addFolder(folderName);
                addSubfolders(rootFolder.getFolders()->back(), imagesData, entry.path().string(), nbrImage, progressDialog);
            }
        } else {
            if (isImage(entry.path().filename().string())) {
                Folders folders = Folders(entry.path().string());
                rootFolder.addFile(entry.path().string());
                folders.addFolder(fs::absolute(entry.path()).parent_path().string());
                ImageData* imageData = new ImageData(folders);
                imageData->loadData();
                imageData->clearMetaData();

                imagesData->get()->push_back(imageData);

                nbrImage += 1;
                progressDialog.setLabelText(QString("Scaning for images : %1").arg(nbrImage));
                QApplication::processEvents();
            }
        }
    }
    return true;
}