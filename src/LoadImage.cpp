#include "LoadImage.hpp"

#include <QApplication>
#include <QDebug>
#include <QFileDialog>
#include <QProgressDialog>
#include <QThreadPool>
#include <QTimer>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <map>
#include <mutex>
#include <numeric>
#include <queue>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "Const.hpp"
#include "Data.hpp"
#include "FileSelector.hpp"
#include "Folders.hpp"
#include "ImagesData.hpp"
#include "Verification.hpp"

namespace fs = std::filesystem;

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

    // auto images = data->getImagesData()->get();

    // ensure that it's well loaded
    qInfo() << "Opening folder, with " << data->getCurrentFolders()->getFilesPtr()->size() << " images" << " and " << data->getCurrentFolders()->getFolders()->size() << " folders";
    qInfo() << "saving data after loading";
    data->saveData();
    data->clear();
    data->loadData();
    qInfo() << "All loading done";
    qInfo() << "Opening folder, with " << data->getCurrentFolders()->getFilesPtr()->size() << " images" << " and " << data->getCurrentFolders()->getFolders()->size() << " folders";
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

    progressDialog.setLabelText("Scaning for images : ");
    progressDialog.setCancelButtonText("Cancel");
    progressDialog.setRange(0, 0);
    progressDialog.setParent(parent);
    progressDialog.show();
    QApplication::processEvents();

    ImagesData* imagesData = data->getImagesData();
    Folders* rootFolder = data->getRootFolders();

    if (!addFilesToTree(rootFolder, imagesData, imagePaths, nbrImage, progressDialog)) {
        return false;
    }

    data->imagesData = *imagesData;

    progressDialog.setValue(0);
    progressDialog.setMaximum(data->getImagesData()->get()->size());
    progressDialog.setLabelText("Loading images thumbnail ...");
    progressDialog.show();
    QApplication::processEvents();

    if (data->imagesData.get()->size() <= 0) {
        qCritical() << "No images found in the selected folder";
        return false;
    }

    if (!loadImagesThumbnail(data, progressDialog)) {
        return false;
    }

    if (QMessageBox::question(parent, "Pre-detection", "Do you want to perform object pre-detection on all images?") == QMessageBox::Yes) {
        progressDialog.setLabelText("Detecting objects in images...");
        progressDialog.setValue(0);
        progressDialog.setMaximum(data->getImagesData()->get()->size());
        progressDialog.show();
        QApplication::processEvents();

        int detectedImages = 0;
        int batchSize = IMAGE_PER_THREAD;
        int totalImages = data->getImagesData()->get()->size();

        for (int start = 0; start < totalImages; start += batchSize) {
            int end = std::min(start + batchSize, totalImages);

            data->addHeavyThread([start, end, data, &detectedImages]() {
                for (int i = start; i < end; ++i) {
                    ImageData* imageData = data->getImagesData()->get()->at(i);
                    if (imageData->isDetectionStatusNotLoaded()) {
                        QImage qImage(QString::fromStdString(imageData->getImagePath()));
                        auto detectedObjects = data->detect(imageData->getImagePath(), qImage, "yolov5n").getDetectedObjects();
                        imageData->setDetectedObjects(detectedObjects);
                        imageData->setDetectionStatusLoaded();
                    }
                    detectedImages++;
                }
            });
        }
        while (QThreadPool::globalInstance()->activeThreadCount() > 0) {
            progressDialog.setValue(detectedImages);
            QCoreApplication::processEvents();
            if (progressDialog.wasCanceled()) {
                break;
            }
        }
    }

    return true;
}
bool loadImagesThumbnail(Data* data, QProgressDialog& progressDialog) {
    int totalImages = data->getImagesData()->get()->size();
    int imagesPerThread = IMAGE_PER_THREAD;
    int thumbnailsCreated = 0;

    std::mutex cacheMutex;

    for (int start = 0; start < totalImages; start += imagesPerThread) {
        int end = std::min(start + imagesPerThread, totalImages);
        data->addThread([start, end, data]() {
            for (int i = start; i < end; ++i) {
                ImageData* imageData = data->getImagesData()->get()->at(i);
                data->createThumbnailIfNotExists(imageData->getImagePath(), 16);
                data->createThumbnailIfNotExists(imageData->getImagePath(), 128);
                data->createThumbnailIfNotExists(imageData->getImagePath(), 256);
                data->createThumbnailIfNotExists(imageData->getImagePath(), 512);

                try {
                    data->unloadFromCache(imageData->getImagePath());
                } catch (const std::exception& e) {
                    qCritical() << "Exception during unloadFromCache: " << e.what();
                }
            }
        });
    }

    std::vector<int> imageIndices(totalImages);
    std::iota(imageIndices.begin(), imageIndices.end(), 0);

    QTimer timer;
    QObject::connect(&timer, &QTimer::timeout, [&]() {
        qInfo() << "starting count" << imageIndices.size();
        try {
            for (int index : imageIndices) {
                ImageData* imageData = data->getImagesData()->getImageData(index);
                data->hasThumbnail(imageData->getImagePath(), 128);

                if (data->hasThumbnail(imageData->getImagePath(), 128) &&
                    data->hasThumbnail(imageData->getImagePath(), 256) &&
                    data->hasThumbnail(imageData->getImagePath(), 512)) {
                    ++thumbnailsCreated;

                    imageIndices.erase(std::remove(imageIndices.begin(), imageIndices.end(), index), imageIndices.end());
                }
            }
        } catch (const std::exception& e) {
            qCritical() << e.what();
        }
        qInfo() << "ending count" << imageIndices.size();

        progressDialog.setValue(thumbnailsCreated);
        qInfo() << "Number of thumbnails created: " << thumbnailsCreated << "/" << totalImages;
        qInfo() << "Number of active thread: " << QThreadPool::globalInstance()->activeThreadCount();
        if (QThreadPool::globalInstance()->activeThreadCount() > 0 && imageIndices.size() > 0) {
            timer.start(1000);
        }
    });
    timer.start(1000);
    while (QThreadPool::globalInstance()->activeThreadCount() > 0) {
        QCoreApplication::processEvents();
        if (progressDialog.wasCanceled()) {
            QThreadPool::globalInstance()->clear();
            return false;
        }
    }
    return true;
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

bool addFilesToTree(Folders* currentFolder, ImagesData* imagesData, const std::string& path, int& nbrImage, QProgressDialog& progressDialog) {
    fs::path fsPath(path);

    for (const auto& part : fsPath) {
        if (progressDialog.wasCanceled()) {
            return false;
        }
        if (part == "/")
            continue;

        std::string folderName = part.string();

        currentFolder->addFolder(folderName);
        auto it = std::find_if(currentFolder->getFolders()->begin(), currentFolder->getFolders()->end(),
                               [folderName](Folders& folder) { return folder.getName() == folderName; });
        if (it != currentFolder->getFolders()->end()) {
            currentFolder = &(*it);
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
            if (containMedia(entry.path().string())) {
                std::string folderName = entry.path().filename().string();
                Folders* subFolder = rootFolder.getFolder(rootFolder.addFolder(folderName));
                addSubfolders(*subFolder, imagesData, entry.path().string(), nbrImage, progressDialog);
            }
        } else {
            if (isImage(entry.path().filename().string()) || isVideo(entry.path().filename().string())) {
                // TODO vérifie tres mal et si on ajoute un dossier au dessus elle se copie 2 fois

                std::string imagePath = entry.path().string();
                std::replace(imagePath.begin(), imagePath.end(), '\\', '/');
                if (imagesData->getImageData(imagePath) == nullptr) {
                    // qDebug() << imagesData->getImageMap()->begin()->first;
                    qDebug() << "Adding image: " << imagePath;
                    Folders folders = Folders(imagePath);
                    rootFolder.addFile(imagePath);
                    folders.addFolder(fs::absolute(entry.path()).parent_path().string());
                    ImageData* imageData = new ImageData(folders);
                    imageData->loadData();
                    imageData->clearMetaData();
                    imagesData->addImage(imageData);
                }
                nbrImage += 1;
                progressDialog.setLabelText(QString("Scaning for images : %1").arg(nbrImage));
                QApplication::processEvents();
            }
        }
    }
    return true;
}