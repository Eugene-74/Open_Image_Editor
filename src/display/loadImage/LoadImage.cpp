#include "LoadImage.h"

void addImagesFromFolder(Data* data, QWidget* parent){

    QProgressDialog progressDialog(parent);
    // Necessaire : sinon s'affiche tout seul au bout de 5 s
    progressDialog.show();
    progressDialog.hide();

    ImagesData imagesData = addSelectedFilesToFolders(data, parent, progressDialog);
    if (imagesData.get()->size() <= 0){
        return;
    }
    // TODO *& necessaire sinon data ressort sans imagesData ...
    data->imagesData = *&imagesData;

    auto start = std::chrono::high_resolution_clock::now();
    data->sortImagesData();
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Sorting images took " << elapsed.count() << " seconds." << std::endl;

    auto saveStart = std::chrono::high_resolution_clock::now();
    data->saveData();
    auto saveEnd = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> saveElapsed = saveEnd - saveStart;
    std::cout << "Saving data took " << saveElapsed.count() << " seconds." << std::endl;
}


// Fonction pour ajouter des fichiers sélectionnés à la liste des dossiers
ImagesData addSelectedFilesToFolders(Data* data, QWidget* parent, QProgressDialog& progressDialog){
    ImagesData imagesData = ImagesData(std::vector<ImageData>{});

    fileSelector fileSelector;

    QStringList selectedFiles;

    selectedFiles = fileSelector.openDirectoryDialog();

    for (const QString& fileName : selectedFiles){
        if (!startLoadingImagesFromFolder(parent, data, fileName.toStdString(), &imagesData, progressDialog)){
            return ImagesData(std::vector<ImageData>{});
        }
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

// Charges dans un imagesData toutes les données des images dans un dossier et ses sous dossier
bool startLoadingImagesFromFolder(QWidget* parent, Data* data, const std::string imagePaths, ImagesData* imagesData, QProgressDialog& progressDialog){

    int nbrImage = 0;

    addFilesToTree(&data->rootFolders, imagePaths);
    // data->rootFolders.print();
    // return false;

    countImagesFromFolder(imagePaths, nbrImage);


    progressDialog.setLabelText("Loading images...");
    progressDialog.setCancelButtonText("Cancel");
    progressDialog.setRange(0, nbrImage);
    progressDialog.setParent(parent);
    progressDialog.setWindowModality(Qt::WindowModal);
    progressDialog.show();

    int loaded = 0;
    if (!loadImagesFromFolder(imagePaths, imagePaths, imagesData, loaded, progressDialog)){
        std::cerr << "loadImagesFromFolder failed" << std::endl;
        return false;
    }


    progressDialog.setValue(0);
    progressDialog.show();
    progressDialog.setLabelText("Loading images metaData...");
    if (!loadImagesMetaData(imagesData, progressDialog)){
        return false;
    }


    progressDialog.setValue(0);
    progressDialog.show();
    progressDialog.setLabelText("Loading images googleData...");
    if (!loadImagesMetaDataOfGoogle(imagesData, progressDialog)){
        return false;
    }

    progressDialog.setValue(0);
    progressDialog.show();
    progressDialog.setLabelText("Loading images thumbnail...");


    for (int i = 0; i < imagesData->get()->size(); ++i) {
        if (progressDialog.wasCanceled()) {
            return false;
        }
        ImageData* imageData = imagesData->getImageData(i);
        // TODO cree une fonction et utiliser le threadpool si possible
        data->createThumbnailIfNotExists(imageData->getImagePath(), 128);
        data->createThumbnailIfNotExists(imageData->getImagePath(), 256);
        data->createThumbnailIfNotExists(imageData->getImagePath(), 512);

        data->unloadFromCache(imageData->getImagePath());
        progressDialog.setValue(i);
        QApplication::processEvents();
    }
    progressDialog.setLabelText("sorting...");
    QApplication::processEvents();



    return true;
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
bool loadImagesFromFolder(const std::string initialPath, const std::string path, ImagesData* imagesData, int& nbrImage, QProgressDialog& progressDialog){
    for (const auto& entry : fs::directory_iterator(path)){
        if (progressDialog.wasCanceled()){
            return false;
        }
        if (fs::is_regular_file(entry.status())){
            if (isImage(entry.path().string())){

                fs::path relativePath = fs::relative(entry.path(), fs::path(initialPath).parent_path());

                Folders folders;
                ImageData* imageData = imagesData->getImageData(entry.path().string());
                if (imageData != nullptr){
                    folders = imageData->folders;
                } else{
                    folders = Folders(entry.path().string());
                }
                folders.addFolder(fs::absolute(entry.path()).parent_path().string());
                std::cerr << "folders.name : " << fs::absolute(entry.path()).parent_path().string() << std::endl;

                ImageData imageD(folders);

                imagesData->addImage(imageD);

                nbrImage += 1;
                progressDialog.setValue(nbrImage);
                QApplication::processEvents();
            }
        } else if (fs::is_directory(entry.status())){
            loadImagesFromFolder(initialPath, entry.path().string(), imagesData, nbrImage, progressDialog);
        }
    }
    return true;
}

bool loadImagesMetaData(ImagesData* imagesData, QProgressDialog& progressDialog){
    int currentImage = 0;
    for (int i = 0; i < imagesData->get()->size(); ++i){
        if (progressDialog.wasCanceled()){
            return false;
        }
        imagesData->getImageData(i)->loadData();
        currentImage += 1;
        progressDialog.setValue(currentImage);
        QApplication::processEvents();
    }
    return true;
}

bool loadImagesMetaDataOfGoogle(ImagesData* imagesData, QProgressDialog& progressDialog){
    progressDialog.setMaximum(imagesData->get()->size());

    for (int i = 0; i < imagesData->get()->size(); ++i){
        if (progressDialog.wasCanceled()){
            return false;
        }
        ImageData* imageData = imagesData->getImageData(i);
        std::string jsonFilePath = imageData->getImagePath() + ".json";

        if (fs::exists(jsonFilePath)){
            std::cerr << "json found" << std::endl;
            std::map<std::string, std::string> jsonMap = openJsonFile(jsonFilePath);

            for (const auto& [key, value] : jsonMap){
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
        progressDialog.setValue(i);
        QApplication::processEvents();
    }
    return true;
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