#include "Data.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <fstream>

namespace fs = std::filesystem; // Alias pour simplifier le code

void Data::preDeleteImage(int imageNbr)
{
    ImageData* imageData;
    imageData = imagesData.getImageData(imageNbr);

    deletedImagesData.addImage(*imageData);
    std::cerr << "image deleted : " << imageNbr << std::endl;
    deletedImagesData.print();
}
void Data::unPreDeleteImage(int imageNbr)
{
    ImageData* imageData;
    imageData = imagesData.getImageData(imageNbr);

    deletedImagesData.removeImage(*imageData);
}

void Data::revocerDeletedImage(ImageData& imageData)
{
    imagesData.addImage(imageData);
    deletedImagesData.removeImage(imageData);
}

void Data::revocerDeletedImage(int imageNbr)
{
    ImageData* imageData;
    imageData = deletedImagesData.getImageData(imageNbr);
    revocerDeletedImage(*imageData);

    imagesData.addImage(*imageData);
    deletedImagesData.removeImage(*imageData);
}

// Supprime de imagesData les images dans deletedImagesData
void Data::removeDeletedImages()
{
    for (const auto& deletedImage : *deletedImagesData.get())
    {
        // Find the image in imagesData
        auto it = std::find(imagesData.get()->begin(), imagesData.get()->end(),
            deletedImage);
        // If it exists, remove it from imagesData
        if (it != imagesData.get()->end())
        {
            // imagesData.removeImage(*imagesData.getImageData(it));
            imagesData.get()->erase(it);
            deletedImage.print();
        }
    }

    std::cerr << "All images deleted" << std::endl;
}

bool Data::isDeleted(int imageNbr)
{
    // Find the image in deletedImagesData

    std::string imagePath = imagesData.getImageData(imageNbr)->imagePath;

    auto it = std::find_if(deletedImagesData.get()->begin(),
        deletedImagesData.get()->end(),
        [imagePath, this](const ImageData img)
        {
            return img.imagePath == imagePath;
        });

    if (it != deletedImagesData.get()->end())
    {
        imagesData.getImageData(imageNbr)->print();

        return true;
    }

    return false;
}
QImage Data::loadImage(QWidget* parent, std::string imagePath, QSize size,
    bool setSize, int thumbnail, bool rotation,
    bool square, bool crop, bool force)
{
    QImage image = loadImageNormal(parent, imagePath, size, setSize, thumbnail, force);

    if (crop)
    {
        int imageId = imagesData.getImageIdByName(imagePath);
        if (imageId != -1)
        {
            ImageData* imageData = imagesData.getImageData(imagePath);
            if (imageData != nullptr && !imageData->cropSizes->empty())
            {

                std::vector<QPoint> cropPoints = imageData->cropSizes->back();
                if (cropPoints.size() == 2)
                {
                    QRect cropRect = QRect(cropPoints[0], cropPoints[1]).normalized();

                    // Vérifier que le rectangle de découpe est dans les limites de l'image
                    QRect imageRect(0, 0, image.width(), image.height());
                    cropRect = cropRect.intersected(imageRect);

                    if (cropRect.isValid() && !image.isNull())
                    {
                        // Découper l'image en utilisant le rectangle de découpe
                        QImage croppedImage = image.copy(cropRect);

                        image = croppedImage;
                    }
                }
            }
        }
    }
    if (square)
    {

        int cropSize = std::min(image.width(), image.height());
        int xOffset = (image.width() - cropSize) / 2;
        int yOffset = (image.height() - cropSize) / 2;

        image = image.copy(xOffset, yOffset, cropSize, cropSize);
    }
    if (rotation)
    {
        image = rotateQImage(image, imagePath);
    }
    return image;
}

QImage Data::loadImageNormal(QWidget* parent, std::string imagePath, QSize size,
    bool setSize, int thumbnail, bool force)
{
    std::map<std::string, QImageAndPath>* cache = imageCache;

    auto it = cache->find(imagePath);
    if (it != cache->end())
    {
        return it->second.image;
    }

    if (!force)
    {
        it = cache->find(getThumbnailPath(imagePath, 512));
        if (it != cache->end())
        {
            return it->second.image;
        }

        it = cache->find(getThumbnailPath(imagePath, 256));
        if (it != cache->end())
        {
            return it->second.image;
        }

        it = cache->find(getThumbnailPath(imagePath, 128));
        if (it != cache->end())
        {
            return it->second.image;
        }
    }

    std::string imagePathbis = imagePath;

    QResource ressource(QString::fromStdString(imagePath));

    QImage image;

    if (ressource.isValid())
    {
        image.load(QString::fromStdString(imagePathbis));
    } else
    {
        if (thumbnail == 128)
        {
            if (hasThumbnail(imagePath, 128))
            {
                imagePathbis = getThumbnailPath(imagePath, 128);
            } else
            {
                createThumbnail(imagePath, 128);
                createThumbnailIfNotExists(imagePath, 256);
                createThumbnailIfNotExists(imagePath, 512);
            }
        } else if (thumbnail == 256)
        {
            if (hasThumbnail(imagePath, 256))
            {
                imagePathbis = getThumbnailPath(imagePath, 256);
            } else
            {
                createThumbnailIfNotExists(imagePath, 128);
                createThumbnail(imagePath, 256);
                createThumbnailIfNotExists(imagePath, 512);
            }
        } else if (thumbnail == 512)
        {
            if (hasThumbnail(imagePath, 512))
            {
                imagePathbis = getThumbnailPath(imagePath, 512);
            } else
            {
                createThumbnailIfNotExists(imagePath, 128);
                createThumbnailIfNotExists(imagePath, 256);
                createThumbnail(imagePath, 512);
            }
        }

        image.load(QString::fromStdString(imagePathbis));
        if (image.isNull())
        {
            return QImage();
        }

        if (setSize)
        {
            image = image.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
    }

    (*cache)[imagePathbis].image = image;
    (*cache)[imagePathbis].imagePath = imagePath;
    return image;
}

void Data::loadInCacheAsync(std::string imagePath,
    std::function<void()> callback, bool setSize,
    QSize size, bool force)
{
    futures[QString::fromStdString(imagePath)] = threadPool.enqueue(
        &Data::loadImageTask, this, imagePath, setSize, size, force, callback);
}

void Data::loadImageTask(std::string imagePath, bool setSize, QSize size,
    bool force, std::function<void()> callback)
{
    loadInCache(imagePath, setSize, size, force);
    if (callback)
    {
        QMetaObject::invokeMethod(QApplication::instance(), callback,
            Qt::QueuedConnection);
    }
}
bool Data::loadInCache(const std::string imagePath, bool setSize,
    const QSize size, bool force)
{
    QImage image;
    if (!force && isInCache(imagePath))
    {
        return true;
    }
    image.load(QString::fromStdString(imagePath));
    if (image.isNull())
    {
        return false;
    }

    if (setSize)
    {
        image = image.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    // std::cerr << "loaded in cache : " << imagePath << std::endl;
    (*imageCache)[imagePath].image = image;
    (*imageCache)[imagePath].imagePath = imagePath;

    createThumbnailIfNotExists(imagePath, 128);
    createThumbnailIfNotExists(imagePath, 256);
    createThumbnailIfNotExists(imagePath, 512);

    return true;
}

bool Data::isInCache(std::string imagePath)
{
    return imageCache->find(imagePath) != imageCache->end();
}

bool Data::getLoadedImage(std::string imagePath, QImage& image)
{
    auto it = imageCache->find(imagePath);
    if (it != imageCache->end())
    {
        image = it->second.image;
        return true;
    }
    return false;
}

void Data::createThumbnails(const std::vector<std::string>& imagePaths,
    const int maxDim)
{
    for (const auto& imagePath : imagePaths)
    {
        createThumbnail(imagePath, maxDim);
    }
}

void Data::createThumbnail(const std::string& imagePath, const int maxDim)
{
    QImage image =
        loadImageNormal(nullptr, imagePath, QSize(maxDim, maxDim), false, 0);

    double scale = std::min(static_cast<double>(maxDim) / image.width(),
        static_cast<double>(maxDim) / image.height());

    QImage thumbnail = image.scaled(image.width() * scale, image.height() * scale,
        Qt::KeepAspectRatio);

    std::hash<std::string> hasher;
    size_t hashValue = hasher(imagePath);

    std::string extension = fs::path(imagePath).extension().string();

    std::string outputImage;
    if (maxDim == 128)
    {
        outputImage = options.at(THUMBNAIL_PATH_OPTION).value + "/normal/" +
            std::to_string(hashValue) + extension;
    } else if (maxDim == 256)
    {
        outputImage = options.at(THUMBNAIL_PATH_OPTION).value + "/large/" +
            std::to_string(hashValue) + extension;
    } else if (maxDim == 512)
    {
        outputImage = options.at(THUMBNAIL_PATH_OPTION).value + "/x-large/" +
            std::to_string(hashValue) + extension;
    }

    if (!fs::exists(fs::path(outputImage).parent_path()))
    {
        fs::create_directories(fs::path(outputImage).parent_path());
    }
    if (!thumbnail.save(QString::fromStdString(outputImage)))
    {
        std::cerr << "Error: Could not save thumbnail: " << outputImage
            << std::endl;
    }
}
void Data::createThumbnailsIfNotExists(
    const std::vector<std::string>& imagePaths, const int maxDim)
{
    for (const auto& imagePath : imagePaths){
        createThumbnailIfNotExists(imagePath, maxDim);
    }
}

void Data::createThumbnailIfNotExists(const std::string& imagePath,
    const int maxDim)
{
    if (!hasThumbnail(imagePath, maxDim))
    {
        // std::cout << "creating Thumbnail for: " << imagePath << std::endl;
        createThumbnail(imagePath, maxDim);
    } else
    {
        // std::cout << "Thumbnail already exists for: " << maxDim << " : " <<
        // imagePath << std::endl;
    }
}
bool Data::hasThumbnail(const std::string& imagePath, const int maxDim)
{
    std::hash<std::string> hasher;
    size_t hashValue = hasher(imagePath);
    std::string extension = fs::path(imagePath).extension().string();
    std::string outputImage;

    if (maxDim == 128)
    {
        outputImage = options.at(THUMBNAIL_PATH_OPTION).value + "/normal/" +
            std::to_string(hashValue) + extension;
    } else if (maxDim == 256)
    {
        outputImage = options.at(THUMBNAIL_PATH_OPTION).value + "/large/" +
            std::to_string(hashValue) + extension;
    } else if (maxDim == 512)
    {
        outputImage = options.at(THUMBNAIL_PATH_OPTION).value + "/x-large/" +
            std::to_string(hashValue) + extension;
    }

    // Check if the thumbnail file exists
    return fs::exists(outputImage);
}

void Data::createAllThumbnail(const std::string& imagePath, const int size)
{
    std::vector<std::string> imagePaths;

    if (size > 128)
    {
        createThumbnail(imagePath, 128);
    }
    if (size > 256)
    {
        createThumbnail(imagePath, 256);
    }
    if (size > 512)
    {
        createThumbnail(imagePath, 512);
    }
}

std::string Data::getThumbnailPath(const std::string& imagePath,
    const int size)
{
    std::hash<std::string> hasher;
    size_t hashValue = hasher(imagePath);
    std::string extension = fs::path(imagePath).extension().string();
    std::string outputImage;

    if (size == 128)
    {
        outputImage = options.at(THUMBNAIL_PATH_OPTION).value + "/normal/" +
            std::to_string(hashValue) + extension;
    } else if (size == 256)
    {
        outputImage = options.at(THUMBNAIL_PATH_OPTION).value + "/large/" +
            std::to_string(hashValue) + extension;
    } else if (size == 512)
    {
        outputImage = options.at(THUMBNAIL_PATH_OPTION).value + "/x-large/" +
            std::to_string(hashValue) + extension;
    }
    return outputImage;
}

bool Data::unloadFromCache(std::string imagePath)
{
    if (!imageCache)
    {
        std::cerr << "imageCache is not initialized" << std::endl;
        return false;
    }
    auto it = imageCache->find(imagePath);

    if (it != imageCache->end())
    {
        imageCache->erase(it);
        return true;
    }
    return false;
}

bool Data::unloadFromFutures(std::string imagePath)
{
    auto it = futures.find(QString::fromStdString(imagePath));
    if (it != futures.end())
    {
        futures.erase(it);
        return true;
    }
    return false;
}
void Data::exportImages(std::string exportPath, bool dateInName)
{
    Folders* firstFolder;

    firstFolder = findFirstFolderWithAllImages(imagesData, rootFolders);

    copyImages(firstFolder, exportPath, dateInName);
}

Folders* Data::findFirstFolderWithAllImages(
    const ImagesData& imagesData, const Folders& currentFolder) const
{
    for (const auto& folder : currentFolder.folders)
    {
        for (ImageData imageData : imagesData.imagesData)
        {
            for (Folders folderBis : imageData.folders.files)
            {
                if (folderBis.folderName == folder.folderName)
                {
                    return const_cast<Folders*>(&currentFolder);
                }
            }
        }
        return findFirstFolderWithAllImages(imagesData, folder);
    }
    return const_cast<Folders*>(&currentFolder);
}

void Data::copyTo(std::string filePath, std::string destinationPath,
    bool dateInName) const
{
    for (const auto& imageData : imagesData.imagesData)
    {
        for (const auto& file : imageData.folders.files)
        {
            if (file == filePath)
            {
                std::string destinationFile =
                    destinationPath + "/" + imageData.getImageName();
                if (dateInName)
                {
                    Exiv2::ExifData exifData = imageData.getMetaData().getExifData();
                    if (exifData["Exif.Image.DateTime"].count() != 0)
                    {
                        std::string date = exifData["Exif.Image.DateTime"].toString();
                        std::replace(date.begin(), date.end(), ':', '-');
                        std::replace(date.begin(), date.end(), ' ', '_');
                        destinationFile = destinationPath + "/" + date + "_" + file;
                    } else
                    {
                        destinationFile = destinationPath + "/" + "no_date" + "_" + file;
                    }
                }
                fs::copy(imageData.imagePath, destinationFile,
                    fs::copy_options::overwrite_existing);
            }
        }
    }
}
QImage Data::rotateQImage(QImage image, std::string imagePath)
{
    ImageData* imageData = imagesData.getImageData(imagePath);
    if (imageData != nullptr)
    {
        Exiv2::ExifData exifData = imageData->getMetaData()->getExifData();
        if (exifData.empty())
        {
        } else
        {
            if (exifData["Exif.Image.Orientation"].count() != 0)
            {
#ifdef _WIN32
                int orientation = exifData["Exif.Image.Orientation"].toLong();
#else
                int orientation = exifData["Exif.Image.Orientation"].toInt64();
#endif
                // int orientation = exifData["Exif.Image.Orientation"].toInt64();
                switch (orientation)
                {
                case 1:
                    // No transformation needed
                    break;
                case 2:
                    image = image.mirrored(true, false); // Horizontal mirror
                    break;
                case 3:
                    image = image.transformed(QTransform().rotate(180));
                    break;
                case 4:
                    image = image.mirrored(false, true); // Vertical mirror
                    break;
                case 5:
                    image = image.mirrored(true, false)
                        .transformed(QTransform().rotate(
                            90)); // Horizontal mirror + 90 degrees
                    break;
                case 6:
                    image = image.transformed(QTransform().rotate(90));
                    break;
                case 7:
                    image = image.mirrored(true, false)
                        .transformed(QTransform().rotate(
                            -90)); // Horizontal mirror + -90 degrees
                    break;
                case 8:
                    image = image.transformed(QTransform().rotate(-90));
                    break;
                default:
                    break;
                }
            }
        }
    }
    return image;
}

void Data::copyImages(Folders* currentFolders, std::string folderPath,
    bool dateInName)
{
    currentFolders->print();
    for (auto& folder : currentFolders->folders)
    {
        folderPath = folderPath + "/" + folder.folderName;
        std::cerr << "folderPath : " << folderPath << std::endl;
        if (!fs::exists(folderPath))
        {
            fs::create_directories(folderPath);
        }
        copyImages(&folder, folderPath, dateInName);
    }
    for (auto& file : currentFolders->folders)
    {
        copyTo(file.folderName, folderPath, dateInName);
    }
}

void Data::saveData(){
    std::cerr << "Saving data" << std::endl;

    std::string filePath = IMAGESDATA_SAVE_DATA_PATH;
    std::ofstream outFile(filePath, std::ios::binary);
    if (!outFile)
    {
        // Vérifiez si le répertoire parent existe
        if (!fs::exists(fs::path(filePath).parent_path()))
        {
            // Essayez de créer les répertoires nécessaires
            if (!fs::create_directories(fs::path(filePath).parent_path()))
            {
                std::cerr << "Couldn't create directories for save file." << std::endl;
                return;
            } else
            {
                std::cerr << "Directories created" << std::endl;
            }
        }

        // Essayez d'ouvrir le fichier à nouveau après avoir créé les répertoires
        outFile.open(filePath, std::ios::binary);
        if (!outFile)
        {
            std::cerr << "Couldn't open save file." << std::endl;
            return;
        }
    }

    // Serialize imagesData

    size_t imagesDataSize = imagesData.get()->size();
    outFile.write(reinterpret_cast<const char*>(&imagesDataSize),
        sizeof(imagesDataSize));
    for (const auto& imageData : *imagesData.get())
    {
        imageData.save(outFile);
    }

    // Serialize deletedImagesData
    size_t deletedImagesDataSize = deletedImagesData.get()->size();
    outFile.write(reinterpret_cast<const char*>(&deletedImagesDataSize),
        sizeof(deletedImagesDataSize));
    for (const auto& imageData : *deletedImagesData.get()){
        imageData.save(outFile);
    }

    // Serialize options
    size_t optionsSize = options.size();
    outFile.write(reinterpret_cast<const char*>(&optionsSize),
        sizeof(optionsSize));
    for (const auto& [key, option] : options){
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

void Data::loadData()
{
    std::string filePath = IMAGESDATA_SAVE_DATA_PATH;
    std::ifstream inFile(filePath, std::ios::binary);
    if (!inFile)
    {
        std::cerr << "Couldn't open load file." << std::endl;
        return;
    }

    // Deserialize imagesData
    size_t imagesDataSize;
    inFile.read(reinterpret_cast<char*>(&imagesDataSize), sizeof(imagesDataSize));
    for (size_t i = 0; i < imagesDataSize; ++i)
    {
        ImageData imageData;
        imageData.load(inFile);
        imagesData.addImage(imageData);
    }

    // Deserialize deletedImagesData
    size_t deletedImagesDataSize;
    inFile.read(reinterpret_cast<char*>(&deletedImagesDataSize),
        sizeof(deletedImagesDataSize));
    for (size_t i = 0; i < deletedImagesDataSize; ++i)
    {
        ImageData imageData;
        imageData.load(inFile);
        deletedImagesData.addImage(imageData);
    }

    // Deserialize options
    options.clear();
    size_t optionsSize;
    inFile.read(reinterpret_cast<char*>(&optionsSize), sizeof(optionsSize));
    for (size_t i = 0; i < optionsSize; ++i)
    {
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

void Data::cancelTasks()
{
    threadPool.shutdown();
    futures.clear();
}

void Data::addAction(std::function<void()> unDo, std::function<void()> reDo)
{
    Actions action;
    action.unDo = unDo;
    action.reDo = reDo;
    lastActions.emplace_back(action);
    if (lastActions.size() > 100)
    {
        lastActions.erase(lastActions.begin());
    }
}

void Data::addActionDone(Actions action)
{
    lastActionsDone.emplace_back(action);
    if (lastActionsDone.size() > 100)
    {
        lastActionsDone.erase(lastActionsDone.begin());
    }
}

void Data::reDoAction()
{
    if (!lastActionsDone.empty())
    {
        auto action = lastActionsDone.back();
        addAction(action.unDo, action.reDo);
        lastActionsDone.pop_back();
        action.reDo();
    }
}

void Data::unDoAction()
{
    if (!lastActions.empty())
    {
        auto action = lastActions.back();
        addActionDone(action);
        lastActions.pop_back();
        action.unDo();
    }
}
void Data::clearActions()
{
    lastActions.clear();
}

void Data::sortImagesData(){
    // Check that all the data are loaded before sorting using the existing thread pool
    std::vector<std::future<void>> futures;
    for (auto& imageData : *imagesData.get()) {
        futures.push_back(threadPool.enqueue([&imageData]() {
            imageData.loadData();
            }));
    }

    // Wait for all the data to be loaded
    for (auto& future : futures) {
        future.get();
    }

    // Sort the images data using multiple threads
    auto& data = *imagesData.get();

    size_t chunkSize = data.size() / numThreads;
    std::vector<std::future<void>> sortFutures;

    for (size_t i = 0; i < numThreads; ++i) {
        sortFutures.push_back(threadPool.enqueue([&, i]() {
            auto begin = data.begin() + i * chunkSize;
            auto end = (i == numThreads - 1) ? data.end() : begin + chunkSize;
            std::sort(begin, end, [](const ImageData& a, const ImageData& b) {
                return a.getMetaData().getExifData()["Exif.Image.DateTime"].toString() > b.getMetaData().getExifData()["Exif.Image.DateTime"].toString();
                });
            }));
    }

    for (auto& future : sortFutures) {
        future.get();
    }

    // Merge sorted chunks
    std::inplace_merge(data.begin(), data.begin() + chunkSize, data.end(), [](const ImageData& a, const ImageData& b) {
        return a.getMetaData().getExifData()["Exif.Image.DateTime"].toString() > b.getMetaData().getExifData()["Exif.Image.DateTime"].toString();
        });
}

