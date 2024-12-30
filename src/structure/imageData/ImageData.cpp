#include "ImageData.h"

ImageData& ImageData::operator=(const ImageData& other) {
    if (this != &other) {
        imagePath = other.imagePath;
        folders = other.folders;
        metaData = other.metaData;
    }
    return *this;
}

void ImageData::print() const {
    std::cerr << "Image : " << imagePath
        << " fichiers : ";
    for (const auto& file : folders.files) {
        std::cerr << " " << file;
    }
    std::cerr << " " << std::endl;
}

std::string ImageData::get() const {
    std::string name;
    name += "Image : " + imagePath + " fichiers : ";
    for (const auto& file : folders.files) {
        name += " ";
        name += file;
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
    std::filesystem::path filePath(imagePath);
    return filePath.filename().string();
}

bool ImageData::operator==(const ImageData& other) const {
    std::filesystem::path filePath(imagePath);
    std::string imageName = filePath.filename().string();
    std::transform(imageName.begin(), imageName.end(), imageName.begin(),
        [](unsigned char c) { return std::tolower(c); });

    std::filesystem::path filePathOther(other.imagePath);
    std::string imageNameOther = filePathOther.filename().string();
    std::transform(imageNameOther.begin(), imageNameOther.end(), imageNameOther.begin(),
        [](unsigned char c) { return std::tolower(c); });

    return imageName == imageNameOther;
}

std::string ImageData::getImagePath() {
    return imagePath;
}



std::string ImageData::getImageExtension() {

    return fs::path(imagePath).extension().string();

}

void ImageData::handleExiv2Error(const Exiv2::Error& e) {
    std::cerr << "Exiv2 error: " << e.what() << std::endl;
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
        metaData.loadData(imagePath);
    } catch (const Exiv2::Error& e) {
        handleExiv2Error(e);
    }
}

void ImageData::saveMetaData() {
    try {
        metaData.saveMetaData(imagePath);
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

    metaData.modifyExifValue("Exif.Image.Orientation", std::to_string(rotation));
    metaData.modifyExifValue("Exif.Thumbnail.Orientation", std::to_string(rotation));
    // metaData.modifyXmpValue("Xmp.tiff.Orientation", std::to_string(rotation));

}

void ImageData::setOrCreateExifData() {
    metaData.setOrCreateExifData(imagePath);

}


void ImageData::save(std::ofstream& out) const {

    size_t pathLength = imagePath.size();
    out.write(reinterpret_cast<const char*>(&pathLength), sizeof(pathLength));
    out.write(imagePath.c_str(), pathLength);
    folders.save(out);
    // TODO charger a chaque fois sinon trop long pour rien
    // metaData.save(out);

    size_t cropSizesSize = cropSizes.size();

    out.write(reinterpret_cast<const char*>(&cropSizesSize), sizeof(cropSizesSize));

    for (const auto& cropSize : cropSizes) {
        size_t innerSize = cropSize.size();
        out.write(reinterpret_cast<const char*>(&innerSize), sizeof(innerSize));
        out.write(reinterpret_cast<const char*>(cropSize.data()), innerSize * sizeof(QPoint));
    }
}

void ImageData::load(std::ifstream& in) {
    size_t pathLength;
    in.read(reinterpret_cast<char*>(&pathLength), sizeof(pathLength));
    imagePath.resize(pathLength);
    in.read(&imagePath[0], pathLength);
    folders.load(in);
    // metaData.load(in);

    size_t cropSizesSize;
    in.read(reinterpret_cast<char*>(&cropSizesSize), sizeof(cropSizesSize));
    cropSizes.resize(cropSizesSize);
    for (auto& cropSize : cropSizes) {
        size_t innerSize;
        in.read(reinterpret_cast<char*>(&innerSize), sizeof(innerSize));
        cropSize.resize(innerSize);
        in.read(reinterpret_cast<char*>(cropSize.data()), innerSize * sizeof(QPoint));
    }
}
