#include "LoadImage.h"


// Fonction pour ajouter des fichiers sélectionnés à la liste des dossiers
ImagesData addSelectedFilesToFolders(Data* data, QWidget* parent) {
    // Data data;
    ImagesData imagesData = ImagesData(std::vector<ImageData>{});;
    fileSelector fileSelector;

    QStringList selectedFiles;

    selectedFiles = fileSelector.openDirectoryDialog();

    for (const QString& fileName : selectedFiles) {
        startLoadingImagesFromFolder(data, fileName.toStdString(), &imagesData);
    }


    return imagesData;


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
void startLoadingImagesFromFolder(Data* data, const std::string imagePaths, ImagesData* imagesData) {
    int nbrImage = 0;

    addFilesToTree(&data->rootFolders, imagePaths);

    countImagesFromFolder(imagePaths, nbrImage);

    data->rootFolders.print();

    std::cerr << "nombre d'image à charger : " << nbrImage << std::endl;

    loadImagesFromFolder(imagePaths, imagePaths, imagesData, nbrImage);

    loadImagesMetaData(imagesData);



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
    std::map<std::string, std::string> jsonMap;
    size_t pos = 0;

    while ((pos = jsonString.find("{", pos)) != std::string::npos) {
        size_t endPos = jsonString.find("}", pos);
        std::string jsonObject = jsonString.substr(pos, endPos - pos + 1);

        size_t keyPos = 0;
        while ((keyPos = jsonObject.find("\"", keyPos)) != std::string::npos) {
            size_t keyEnd = jsonObject.find("\"", keyPos + 1);
            std::string key = jsonObject.substr(keyPos + 1, keyEnd - keyPos - 1);

            size_t valuePos = jsonObject.find(":", keyEnd) + 1;
            size_t valueEnd = jsonObject.find(",", valuePos);
            if (valueEnd == std::string::npos) {
                valueEnd = jsonObject.find("}", valuePos);
            }
            std::string value = jsonObject.substr(valuePos, valueEnd - valuePos);
            value.erase(remove_if(value.begin(), value.end(), isspace), value.end());

            jsonMap[key] = value;

            keyPos = keyEnd + 1;
        }

        pos = endPos + 1;
    }

    return jsonMap;
}


std::map<std::string, std::string> openJsonFile(std::string filePath) {
    std::string jsonString = readFile(filePath);
    std::map<std::string, std::string> jsonMap = parseJsonToMap(jsonString);
    return jsonMap;
}

// Conte toutes les images dans un dossier et ses sous dossier
void countImagesFromFolder(const std::string path, int& nbrImage) {

    int i = 0;
    for (const auto& entry : fs::directory_iterator(path)) {
        if (fs::is_regular_file(entry.status())) {

            if (isImage(entry.path())) {
                nbrImage += 1;

            }
        }
        else if (fs::is_directory(entry.status())) {

            countImagesFromFolder(entry.path(), nbrImage);

            i += 1;
        }
    }
}

// Charges concrètement dans un imagesData toutes les données des images dans un dossier et ses sous dossier
void loadImagesFromFolder(const std::string initialPath, const std::string path, ImagesData* imagesData, int& nbrImage) {
    for (const auto& entry : fs::directory_iterator(path)) {
        if (fs::is_regular_file(entry.status())) {
            if (isImage(entry.path())) {

                fs::path relativePath = fs::relative(entry.path(), fs::path(initialPath).parent_path());

                Folders folders;
                ImageData* imageData = imagesData->getImageData(entry.path());
                if (imageData != nullptr) {
                    folders = imageData->folders;
                }
                else {
                    folders = Folders(relativePath.parent_path().string());

                }
                folders.files.push_back(relativePath.parent_path().filename().string());

                ImageData imageD(entry.path(), folders);
                imagesData->addImage(imageD);

                nbrImage -= 1;
                std::cerr << "Image restante : " << nbrImage << std::endl;
            }
        }
        else if (fs::is_directory(entry.status())) {
            loadImagesFromFolder(initialPath, entry.path(), imagesData, nbrImage);
        }
    }
}

void loadImagesMetaData(ImagesData* imagesData) {
    for (int i = 0; i < imagesData->get()->size(); ++i) {
        imagesData->getImageData(i)->loadData();
    }
}

void loadImagesMetaDataOfGoogle(ImagesData* imagesData) {
    for (int i = 0; i < imagesData->get()->size(); ++i) {
        ImageData* imageData = imagesData->getImageData(i);
        std::string jsonFilePath = imageData->getImagePath() + ".json";

        if (fs::exists(jsonFilePath)) {
            std::cerr << "json found" << std::endl;
            std::map<std::string, std::string> jsonMap = openJsonFile(jsonFilePath);

            for (const auto& [key, value] : jsonMap) {
                std::cerr << key << " : " << value << std::endl;
                // if (key.find("Exif") != std::string::npos) {
                //     imageData->metaData.modifyExifValue(key, value);
                // } else if (key.find("Xmp") != std::string::npos) {
                //     imageData->metaData.modifyXmpValue(key, value);
                // } else if (key.find("Iptc") != std::string::npos) {
                //     imageData->metaData.modifyIptcValue(key, value);
                // }
            }
        }
    }
}