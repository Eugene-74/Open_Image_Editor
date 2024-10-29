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
        << " fichiers : " << folders.getFolderList() << std::endl;
}

std::string ImageData::get() const {
    return "Image : " + imagePath + " fichiers : " + folders.getFolderList() + "\n";
}
MetaData* ImageData::getMetaData() {
    // Débogage pour afficher la taille des métadonnées
    return &metaData;
}

std::vector<std::string> ImageData::getFolders() {
    return folders.getFolders();
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



void ImageData::setExifMetaData(const Exiv2::ExifData& toAddMetaData) {
    metaData.setExifData(toAddMetaData);
    // Sauvegarde après modification des métadonnées
    saveMetaData();
}

void ImageData::loadData() {
    metaData.loadData(imagePath);
}

void ImageData::saveMetaData() {
    metaData.saveMetaData(imagePath);
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


    // 1 : normal, 3 : 90° left, 6 : 180°, 8: 90 right
    metaData.modifyExifValue("Exif.Image.Orientation", std::to_string(rotation));
    metaData.modifyExifValue("Exif.Thumbnail.Orientation", std::to_string(rotation));
    metaData.modifyXmpValue("Xmp.tiff.Orientation", std::to_string(rotation));


}

void ImageData::setOrCreateExifData() {
    metaData.setOrCreateExifData(imagePath);

}




void ImageData::save(std::ofstream& out) const {
    size_t pathLength = imagePath.size();
    out.write(reinterpret_cast<const char*>(&pathLength), sizeof(pathLength));
    out.write(imagePath.c_str(), pathLength);
    folders.save(out);
    metaData.save(out);
}

void ImageData::load(std::ifstream& in) {
    size_t pathLength;
    in.read(reinterpret_cast<char*>(&pathLength), sizeof(pathLength));
    imagePath.resize(pathLength);
    in.read(&imagePath[0], pathLength);
    folders.load(in);
    metaData.load(in);
}

