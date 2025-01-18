#include "LoadImage.h"

// Fonction pour ajouter des fichiers sélectionnés à la liste des dossiers
ImagesData addSelectedFilesToFolders(Data* data, QWidget* parent){
    ImagesData imagesData = ImagesData(std::vector<ImageData>{});

    fileSelector fileSelector;

    QStringList selectedFiles;

    selectedFiles = fileSelector.openDirectoryDialog();

    for (const QString& fileName : selectedFiles){
        startLoadingImagesFromFolder(data, fileName.toStdString(), &imagesData);
    }

    return imagesData;
}

std::string getDirectoryFromUser(QWidget* parent)
{
    QFileDialog dialog(parent);
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setOption(QFileDialog::ShowDirsOnly, true);

    if (dialog.exec())
    {
        QStringList selectedDirectories = dialog.selectedFiles();
        if (!selectedDirectories.isEmpty())
        {
            return selectedDirectories.first().toStdString();
        }
    }
    return "";
}

#include <QProgressDialog>
// Charges dans un imagesData toutes les données des images dans un dossier et ses sous dossier
void startLoadingImagesFromFolder(Data* data, const std::string imagePaths, ImagesData* imagesData)
{

    int nbrImage = 0;

    addFilesToTree(&data->rootFolders, imagePaths);

    countImagesFromFolder(imagePaths, nbrImage);

    data->rootFolders.print();

    std::cerr << "nombre d'image à charger : " << nbrImage << std::endl;

    loadImagesFromFolder(imagePaths, imagePaths, imagesData, nbrImage);

    loadImagesMetaData(imagesData);

    loadImagesMetaDataOfGoogle(imagesData);
}

std::string readFile(const std::string& filePath)
{
    std::ifstream file(filePath);
    if (!file.is_open())
    {
        throw std::runtime_error("Could not open file: " + filePath);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// std::map<std::string, std::string> parseJsonToMap(const std::string& jsonString) {

std::map<std::string, std::string> parseJsonToMap(const std::string& jsonString)
{
    std::map<std::string, std::string> resultMap;
    std::string key, value;
    std::istringstream jsonStream(jsonString);
    char ch;

    while (jsonStream >> ch)
    {
        if (ch == '"')
        {
            std::getline(jsonStream, key, '"');
            jsonStream >> ch; // skip ':'
            jsonStream >> ch; // skip space or '"'
            if (ch == '"')
            {
                std::getline(jsonStream, value, '"');
            } else
            {
                jsonStream.putback(ch);
                std::getline(jsonStream, value, ',');
                value.pop_back(); // remove trailing comma
            }
            resultMap[key] = value;
        }
    }

    return resultMap;
}
// }

std::map<std::string, std::string> openJsonFile(std::string filePath)
{
    std::string jsonString = readFile(filePath);
    std::map<std::string, std::string> jsonMap = parseJsonToMap(jsonString);
    return jsonMap;
}

// Conte toutes les images dans un dossier et ses sous dossier
void countImagesFromFolder(const std::string path, int& nbrImage)
{

    int i = 0;
    for (const auto& entry : fs::directory_iterator(path))
    {
        if (fs::is_regular_file(entry.status()))
        {

            if (isImage(entry.path().string()))
            {
                nbrImage += 1;
            }
        } else if (fs::is_directory(entry.status())) {

            countImagesFromFolder(entry.path().string(), nbrImage);

            i += 1;
        }
    }
}

// Charges concrètement dans un imagesData toutes les données des images dans un dossier et ses sous dossier
void loadImagesFromFolder(const std::string initialPath, const std::string path, ImagesData* imagesData, int& nbrImage){
    for (const auto& entry : fs::directory_iterator(path))
    {
        if (fs::is_regular_file(entry.status()))
        {
            if (isImage(entry.path().string()))
            {

                fs::path relativePath = fs::relative(entry.path(), fs::path(initialPath).parent_path());

                Folders folders;
                ImageData* imageData = imagesData->getImageData(entry.path().string());
                if (imageData != nullptr){
                    folders = imageData->folders;
                } else{
                    folders = Folders(fs::absolute(entry.path()).parent_path().string());
                }
                folders.files.push_back(relativePath.parent_path().filename().string());

                ImageData imageD(entry.path().string(), folders);
                imagesData->addImage(imageD);

                nbrImage -= 1;
                std::cerr << "Image restante : " << nbrImage << std::endl;
            }
        } else if (fs::is_directory(entry.status())){
            loadImagesFromFolder(initialPath, entry.path().string(), imagesData, nbrImage);
        }
    }
}

void loadImagesMetaData(ImagesData* imagesData)
{
    for (int i = 0; i < imagesData->get()->size(); ++i){
        imagesData->getImageData(i)->loadData();
    }
}

void loadImagesMetaDataOfGoogle(ImagesData* imagesData)
{
    for (int i = 0; i < imagesData->get()->size(); ++i)
    {
        ImageData* imageData = imagesData->getImageData(i);
        std::string jsonFilePath = imageData->getImagePath() + ".json";

        if (fs::exists(jsonFilePath))
        {
            std::cerr << "json found" << std::endl;
            std::map<std::string, std::string> jsonMap = openJsonFile(jsonFilePath);

            for (const auto& [key, value] : jsonMap)
            {
                std::cerr << key << " : " << value << std::endl;
                if (!key.empty() && !value.empty())
                {
                    std::string exifKey = mapJsonKeyToExifKey(key);
                    if (exifKey != "")
                    {
                        imageData->metaData.modifyExifValue(exifKey, value);
                    }
                }
            }
        } else{
            // displayExifData(imageData->metaData.exifMetaData);
        }
    }
}

std::string mapJsonKeyToExifKey(const std::string& jsonKey)
{
    static const std::map<std::string, std::string> keyMap = {
        // TODO mettre tous les nécessaires
        // {"title", "Exif.Image.ImageDescription"}, // inutile
        {"description", "Exif.Image.ImageDescription"},
        {"photoTakenTime.timestamp", "Exif.Photo.DateTimeOriginal"},
        {"geoData.latitude", "Exif.GPSInfo.GPSLatitude"},
        {"geoData.longitude", "Exif.GPSInfo.GPSLongitude"},
        {"geoData.altitude", "Exif.GPSInfo.GPSAltitude"},
        {"googlePhotosOrigin.mobileUpload.deviceModel", "Exif.Image.Model"},
        {"googlePhotosOrigin.mobileUpload.deviceType", "Exif.Image.Make"},
        {"googlePhotosOrigin.mobileUpload.deviceSoftwareVersion", "Exif.Image.Software"} };

    std::string lowerJsonKey = jsonKey;
    std::transform(lowerJsonKey.begin(), lowerJsonKey.end(), lowerJsonKey.begin(), ::tolower);

    auto it = keyMap.find(lowerJsonKey);
    if (it != keyMap.end())
    {
        return it->second;
    }
    return "";
}