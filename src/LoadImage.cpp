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

#include "Box.hpp"
#include "Const.hpp"
#include "Data.hpp"
#include "FileSelector.hpp"
#include "Folders.hpp"
#include "ImagesData.hpp"
#include "Verification.hpp"

namespace fs = std::filesystem;

/**
 * @brief Add images from a folder to the data structure
 * @param data Pointer to the Data object
 * @param parent Pointer to the parent QWidget (usually the main window)
 * @details This function opens a file dialog to select a folder and adds all images from that folder and it's sub folders to the data structure.
 */
void addImagesFromFolder(std::shared_ptr<Data> data, QWidget* parent) {
    if (data->getImagesDataPtr()->get()->size() > 0) {
        std::map<std::string, std::string> option = showOptionsDialog(parent, "Add images from folder",
                                                                      {
                                                                          // TODO translate
                                                                          {"Keep old save", Option("bool", "true")},
                                                                      });
        if (option["Keep old save"] == "false") {
            std::remove(IMAGESDATA_SAVE_PATH.c_str());
            data->loadData();  // ouvre les options par exemple
        }
    }
    data->stopAllThreads();

    QProgressDialog progressDialog(parent);
    progressDialog.setWindowModality(Qt::ApplicationModal);
    progressDialog.setAutoClose(false);

    // Necessaire : sinon s'affiche tout seul au bout de 5 s
    progressDialog.show();
    progressDialog.hide();

    if (!addSelectedFilesToFolders(data, parent, progressDialog)) {
        qCritical() << "Loading canceled";
        data->clear();
        data->loadData();
        return;
    }

    // ensure that it's well loaded
    qInfo() << "Opening folder, with " << data->getCurrentFolders()->getFilesPtr()->size() << " images" << " and " << data->getCurrentFolders()->getFolders()->size() << " folders";
    qInfo() << "saving data after loading";
    data->saveData();
    data->clear();
    data->loadData();
    qInfo() << "All loading done";
    qInfo() << "Opening folder, with " << data->getCurrentFolders()->getFilesPtr()->size() << " images" << " and " << data->getCurrentFolders()->getFolders()->size() << " folders";
    data->checkThumbnailAndDetectObjects();
}

/**
 * @brief Add selected files to folders
 * @param data Pointer to the Data object
 * @param parent Pointer to the parent QWidget (usually the main window)
 * @param progressDialog Reference to the QProgressDialog object
 * @return true if files were added successfully, false otherwise
 * @details This function opens a file dialog to select files and adds them to the folders in the data structure.
 */
bool addSelectedFilesToFolders(std::shared_ptr<Data> data, QWidget* parent, QProgressDialog& progressDialog) {
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

/**
 * @brief Load images from a folder and its subfolders into the data structure and create a tree of folders
 * @param parent Pointer to the parent QWidget (usually the main window)
 * @param data Pointer to the Data object
 * @param imagePaths The path of the folder to load images from
 * @param progressDialog Reference to the QProgressDialog object
 * @return true if images were loaded successfully, false otherwise
 */
bool startLoadingImagesFromFolder(QWidget* parent, std::shared_ptr<Data> data, const std::string imagePaths, QProgressDialog& progressDialog) {
    int nbrImage = 0;

    progressDialog.setLabelText("Scaning for images : ");
    progressDialog.setCancelButtonText("Cancel");
    progressDialog.setRange(0, 0);
    progressDialog.setParent(parent);
    progressDialog.show();
    QApplication::processEvents();

    ImagesData* imagesData = data->getImagesDataPtr();
    Folders* rootFolder = data->getRootFolders();

    if (!addFilesToTree(rootFolder, imagesData, imagePaths, nbrImage, progressDialog)) {
        return false;
    }

    *data->getImagesDataPtr() = *imagesData;

    // progressDialog.setValue(0);
    // progressDialog.setMaximum(data->getImagesDataPtr()->get()->size());
    // progressDialog.setLabelText("Loading images thumbnail ...");
    // progressDialog.show();
    // QApplication::processEvents();

    // if (!loadImagesThumbnail(data, progressDialog)) {
    //     return false;
    // }
    if (data->getImagesDataPtr()->get()->size() <= 0) {
        qCritical() << "No images found in the selected folder";
        return false;
    }

    return true;
}

/**
 * @brief Add files to the tree structure (of folders)
 * @param currentFolder Pointer to the current folder
 * @param imagesData Pointer to the ImagesData object
 * @param path The path of the folder to add
 * @param nbrImage Reference to the number of images added
 * @param progressDialog Reference to the QProgressDialog object
 * @return true if files were added successfully, false otherwise
 */
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

/**
 * @brief Add subfolders to the tree structure (of folders)
 * @param rootFolder Pointer to the root folder
 * @param imagesData Pointer to the ImagesData object
 * @param path The path of the folder to add
 * @param nbrImage Reference to the number of images added
 * @param progressDialog Reference to the QProgressDialog object
 * @return true if subfolders were added successfully, false otherwise
 */
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
            if (isImage(entry.path().filename().string())
                // || isVideo(entry.path().filename().string())
            ) {
                std::string imagePath = entry.path().string();
                std::replace(imagePath.begin(), imagePath.end(), '\\', '/');
                if (imagesData->getImageData(imagePath) == nullptr) {
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