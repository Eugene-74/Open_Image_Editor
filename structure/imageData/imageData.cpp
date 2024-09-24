#include <iostream>
#include "imageData.h"

// #include "../../functions/vector/vector.h"
#include <filesystem>


void ImageData::print() const {
    std::cout << "Image : " << imagePath
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



void ImageData::setMetaData(const Exiv2::ExifData& toAddMetaData) {
    metaData.set(toAddMetaData);
    // Sauvegarde après modification des métadonnées
    saveMetaData();
}

void ImageData::loadMetaData() {
    metaData.load(imagePath);
}

void ImageData::saveMetaData() {
    metaData.save(imagePath);
}

int ImageData::getImageWidth(){
    return metaData.getImageWidth();
}
int ImageData::getImageHeight(){
    return metaData.getImageHeight();
}
int ImageData::getImageOrientation(){
    return metaData.getImageOrientation();
}
Date ImageData::getImageDate(){
    return metaData.getImageDate();
}
void ImageData::turnImage(int rotation){
    // 1 : normal, 3 : 90° left, 6 : 180°, 9: 90 right
    metaData.modifyExifValue("Exif.Image.Orientation", std::to_string(rotation));
}


