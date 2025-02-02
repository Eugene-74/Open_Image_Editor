#include "LoadImage.h"


void ThumbnailTask::run() {
    for (int i = start; i < end; ++i) {
        // if (progressDialog->wasCanceled()) {
        //     return;
        // }
        ImageData* imageData = imagesData->getImageData(i);
        data->createThumbnailIfNotExists(imageData->getImagePath(), 128);
        data->createThumbnailIfNotExists(imageData->getImagePath(), 256);
        data->createThumbnailIfNotExists(imageData->getImagePath(), 512);
        data->unloadFromCache(imageData->getImagePath());
        // progressDialog->setValue(i);
        // QMetaObject::invokeMethod(QApplication::instance(), "processEvents", Qt::QueuedConnection);
    }
}

void addImagesFromFolder(Data* data, QWidget* parent){

    QProgressDialog progressDialog(parent);
    // Necessaire : sinon s'affiche tout seul au bout de 5 s
    progressDialog.show();
    progressDialog.hide();

    addSelectedFilesToFolders(data, parent, progressDialog);
    if (data->imagesData.get()->size() <= 0){
        return;
    }

    auto start = std::chrono::high_resolution_clock::now();


    data->sortImagesData(progressDialog);


    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Sorting images took " << elapsed.count() << " seconds." << std::endl;

    data->saveData();
}


// Fonction pour ajouter des fichiers sélectionnés à la liste des dossiers
void addSelectedFilesToFolders(Data* data, QWidget* parent, QProgressDialog& progressDialog){
    fileSelector fileSelector;

    QStringList selectedFiles;

    selectedFiles = fileSelector.openDirectoryDialog();

    for (const QString& fileName : selectedFiles){
        startLoadingImagesFromFolder(parent, data, fileName.toStdString(), progressDialog);
    }

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
bool startLoadingImagesFromFolder(QWidget* parent, Data* data, const std::string imagePaths, QProgressDialog& progressDialog){

    int nbrImage = 0;

    data->rootFolders = Folders("");

    data->imagesData = ImagesData(std::vector<ImageData>{});

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

    ImagesData* imagesData = new  ImagesData(std::vector<ImageData>{});

    if (!loadImagesFromFolder(imagePaths, imagePaths, imagesData, loaded, progressDialog)){
        qDebug() << "loadImagesFromFolder failed : " << imagePaths;
        return false;
    }

    data->imagesData = *imagesData;

    progressDialog.setValue(0);
    progressDialog.show();
    progressDialog.setLabelText("Loading images googleData...");
    if (!loadImagesMetaDataOfGoogle(imagesData, progressDialog)){
        return false;
    }

    progressDialog.setValue(0);
    progressDialog.show();
    progressDialog.setLabelText("Loading images thumbnail... (this will lag ~ 50 image / s)");
    QApplication::processEvents();
    int totalImages = imagesData->get()->size();
    int numThreads = QThreadPool::globalInstance()->maxThreadCount();
    int imagesPerThread = totalImages / numThreads;

    int numOperation = totalImages / 100;

    // ThumbnailTask* task = new ThumbnailTask(data, imagesData, 0, imagesData->get()->size(), &progressDialog);
    // QThreadPool::globalInstance()->start(task);

    for (int i = 0; i < numThreads; ++i) {
        int start = i * imagesPerThread;
        int end = (i == numThreads - 1) ? totalImages : start + imagesPerThread;
        ThumbnailTask* task = new ThumbnailTask(data, imagesData, start, end
            // , &progressDialog
        );
        QThreadPool::globalInstance()->start(task);
    }

    QThreadPool::globalInstance()->waitForDone();


    // for (int i = 0; i < imagesData->get()->size(); ++i) {
    //     // TODO cree une fonction et utiliser le threadpool si possible
    //     if (progressDialog.wasCanceled()) {
    //         return false;
    //     }
    //     ImageData* imageData = imagesData->getImageData(i);
    //     data->createThumbnailIfNotExists(imageData->getImagePath(), 128);
    //     data->createThumbnailIfNotExists(imageData->getImagePath(), 256);
    //     data->createThumbnailIfNotExists(imageData->getImagePath(), 512);

    //     data->unloadFromCache(imageData->getImagePath());
    //     progressDialog.setValue(i);
    //     QApplication::processEvents();
    // }

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


bool loadImagesMetaDataOfGoogle(ImagesData* imagesData, QProgressDialog& progressDialog){
    progressDialog.setMaximum(imagesData->get()->size());

    for (int i = 0; i < imagesData->get()->size(); ++i){
        if (progressDialog.wasCanceled()){
            return false;
        }
        ImageData* imageData = imagesData->getImageData(i);
        std::string jsonFilePath = imageData->getImagePath() + ".json";

        if (fs::exists(jsonFilePath)){
            qDebug() << "json found : " << jsonFilePath.c_str();
            std::map<std::string, std::string> jsonMap = openJsonFile(jsonFilePath);

            for (const auto& [key, value] : jsonMap){
                if (!key.empty() && !value.empty())
                {
                    std::string exifKey = mapJsonKeyToExifKey(key);
                    if (exifKey != "")
                    {
                        imageData->metaData.modifyExifValue(exifKey, value);
                    }
                }
            }
            imageData->saveMetaData();
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