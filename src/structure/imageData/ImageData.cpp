#include "ImageData.h"

ImageData& ImageData::operator=(const ImageData& other) {
    if (this != &other) {
        // imagePath = other.imagePath;
        folders = other.folders;
        metaData = other.metaData;
        cropSizes = other.cropSizes;
        orientation = other.orientation;
    }
    return *this;
}

void ImageData::print() const {
    qDebug() << "Image : " << folders.name
        << " fichiers : ";
    for (const auto& file : folders.folders) {
        qDebug() << " " << file.name;
    }
    qDebug() << " ";
}

std::string ImageData::get() const {
    std::string name;
    name += "Image : " + folders.name + " fichiers : ";
    for (const auto& file : folders.folders) {
        name += " ";
        name += file.name;
    }
    name += "\n";
    return  name;
}
MetaData* ImageData::getMetaData() {
    // Débogage pour afficher la taille des métadonnées
    return &metaData;
}
MetaData ImageData::getMetaData()const {
    // Débogage pour afficher la taille des métadonnées
    return metaData;
}

std::vector<std::string> ImageData::getFolders() {
    return folders.files;
}

void ImageData::addFolder(const std::string& toAddFolder) {
    folders.addFolder(toAddFolder);
}

void ImageData::addFolders(const std::vector<std::string>& toAddFolders) {
    for (const auto& folder : toAddFolders) {
        folders.addFolder(folder);
    }
}



std::string ImageData::getImageName() const {
    std::filesystem::path filePath(folders.name);
    return filePath.filename().string();
}

bool ImageData::operator==(const ImageData& other) const {
    std::filesystem::path filePath(folders.name);
    std::string imageName = filePath.filename().string();
    std::transform(imageName.begin(), imageName.end(), imageName.begin(),
        [](unsigned char c) { return std::tolower(c); });

    std::filesystem::path filePathOther(other.folders.name);
    std::string imageNameOther = filePathOther.filename().string();
    std::transform(imageNameOther.begin(), imageNameOther.end(), imageNameOther.begin(),
        [](unsigned char c) { return std::tolower(c); });

    return imageName == imageNameOther;
}

std::string ImageData::getImagePath() {
    return folders.name;
}



std::string ImageData::getImageExtension() {

    return fs::path(folders.name).extension().string();

}

void ImageData::handleExiv2Error(const Exiv2::Error& e) {
    qDebug() << "Exiv2 error: " << e.what();
}

void ImageData::setExifMetaData(const Exiv2::ExifData& toAddMetaData) {
    try {
        metaData.setExifData(toAddMetaData);
        saveMetaData();
    } catch (const Exiv2::Error& e) {
        handleExiv2Error(e);
    }
}

void ImageData::loadData() {
    try {
        if (!metaData.dataLoaded){
            metaData.loadData(folders.name);

            orientation = metaData.getImageOrientation();

            metaData.dataLoaded = true;
        }
    } catch (const Exiv2::Error& e) {
        qDebug() << "Error loading metadata for image: " << folders.name;
        handleExiv2Error(e);
    }
}

void ImageData::saveMetaData() {
    try {
        metaData.saveMetaData(folders.name);
    } catch (const Exiv2::Error& e) {
        handleExiv2Error(e);
    }
}

int ImageData::getImageWidth() {
    return metaData.getImageWidth();
}
int ImageData::getImageHeight() {
    return metaData.getImageHeight();
}
int ImageData::getImageOrientation() {
    return metaData.getImageOrientation();
}

void ImageData::turnImage(int rotation) {
    orientation = rotation;
    metaData.modifyExifValue("Exif.Image.Orientation", std::to_string(rotation));
    // metaData.modifyExifValue("Exif.Thumbnail.Orientation", std::to_string(rotation));
    // metaData.modifyXmpValue("Xmp.tiff.Orientation", std::to_string(rotation));

}

void ImageData::setOrCreateExifData() {
    metaData.setOrCreateExifData(folders.name);

}


void ImageData::save(std::ofstream& out) const {

    out.write(reinterpret_cast<const char*>(&orientation), sizeof(orientation));

    folders.save(out);

    size_t cropSizesSize = cropSizes.size();
    out.write(reinterpret_cast<const char*>(&cropSizesSize), sizeof(cropSizesSize));
    for (const auto& cropSize : cropSizes) {
        size_t innerSize = cropSize.size();
        out.write(reinterpret_cast<const char*>(&innerSize), sizeof(innerSize));
        out.write(reinterpret_cast<const char*>(cropSize.data()), innerSize * sizeof(QPoint));
    }

}

void ImageData::load(std::ifstream& in) {

    in.read(reinterpret_cast<char*>(&orientation), sizeof(orientation));
    // size_t pathLength;
    // in.read(reinterpret_cast<char*>(&pathLength), sizeof(pathLength));
    // folders.name.resize(pathLength);
    // in.read(&folders.name[0], pathLength);
    folders.load(in);

    size_t cropSizesSize;
    in.read(reinterpret_cast<char*>(&cropSizesSize), sizeof(cropSizesSize));
    if (cropSizesSize >= 1){
        qDebug() << "imagePath:" << folders.name;

        qDebug() << "cropSizesSize:" << cropSizesSize;

        cropSizes.resize(cropSizesSize);
        for (size_t i = 0; i < cropSizesSize; ++i) {
            size_t innerSize;
            in.read(reinterpret_cast<char*>(&innerSize), sizeof(innerSize));
            qDebug() << "innerSize for cropSize" << i << ":" << innerSize;

            cropSizes[i].resize(innerSize);
            in.read(reinterpret_cast<char*>(cropSizes[i].data()), innerSize * sizeof(QPoint));

            // Debug output for the points
            for (size_t j = 0; j < innerSize; ++j) {
                qDebug() << "Point" << j << ":" << cropSizes[i][j];
            }
        }
    }

}


std::vector<std::vector<QPoint>> ImageData::getCropSizes() const {
    return cropSizes;
}



void ImageData::setCropSizes(const std::vector<std::vector<QPoint>>& cropSizes) {
    this->cropSizes = cropSizes;
}