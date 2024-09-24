#include "main.h"


#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <algorithm>
#include <exiv2/exiv2.hpp>

#include "structure/date/date.h"
#include "structure/folders/folders.h"


#include "functions/vector/vector.h"


namespace fs = std::filesystem;


int main() {
    std::string path = "/home/eugene/Documents";
    ImagesData imagesData({});
    using ImagesData = std::vector<ImageData*>;

    listerContenu(path, path, imagesData);
    imagesData.print();
    loadImagesMetaData(imagesData);

    imagesData.getImageData(1)->getMetaData()->modifyExifValue("Exif.Image.Orientation", std::to_string(9));
    displayExifData(imagesData.getImageData(1)->getMetaData()->get());



    return 0;
}



void listerContenu(const std::string initialPath, const std::string path, ImagesData& imagesData) {



    for (const auto& entry : fs::directory_iterator(path)) {
        if (fs::is_regular_file(entry.status())) {
            if (estImage(entry.path())) {

                // On ne garde que la partie après "Documents"
                fs::path relativePath = fs::relative(entry.path(), fs::path(initialPath).parent_path());

                std::vector<std::string> fichiers;

                fichiers = { relativePath };

                Folders folders(fichiers);

                // std::string text = entry.path();

                // std::transform(text.begin(), text.end(), text.begin(),
                //     [](unsigned char c) { return std::tolower(c); });

                ImageData imageD(entry.path(), folders);
                imagesData.addImage(imageD);
            }
        }
        else if (fs::is_directory(entry.status())) {
            listerContenu(initialPath, entry.path(), imagesData);
        }
    }
    // return imagesData;
}

bool estImage(const std::string& cheminFichier) {
    // Liste des extensions d'images courantes
    std::vector<std::string> extensionsImages = { ".jpg", ".jpeg", ".png", ".gif", ".bmp", ".tiff", ".webp" };

    // Récupération de l'extension du fichier
    std::string extension = fs::path(cheminFichier).extension().string();

    // Convertir l'extension en minuscule pour éviter les problèmes de casse
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

    // Vérifier si l'extension est dans la liste des extensions d'images
    return std::find(extensionsImages.begin(), extensionsImages.end(), extension) != extensionsImages.end();
}

// 







Date timestampToDate(time_t timestamp) {
    // Convertir le timestamp en une structure tm
    std::tm* timeStruct = std::localtime(&timestamp);

    // Date date;
    Date date = Date(timeStruct->tm_sec, timeStruct->tm_min, timeStruct->tm_hour, timeStruct->tm_mday, timeStruct->tm_mon + 1, timeStruct->tm_year + 1900);
    // date.seconds = timeStruct->tm_sec;
    // date.minutes = timeStruct->tm_min;
    // date.hours = timeStruct->tm_hour;
    // date.day = timeStruct->tm_mday;
    // date.month = timeStruct->tm_mon + 1; // tm_mon commence à 0 pour janvier
    // date.year = timeStruct->tm_year + 1900; // tm_year est le nombre d'années depuis 1900

    return date;
}

void loadImagesMetaData(ImagesData& imagesData) {
    // using ImagesData = std::vector<ImageData*>;
    // for (int i : ) {
    for (int i = 0; i < imagesData.get().size(); ++i) {
        // if (imageData) { 
        imagesData.getImageData(i)->loadMetaData();
        // std::cerr << " | Valeur : " << imagesData.getImageData(i)->getImageOrientation() << std::endl;
        // displayExifData(imagesData.getImageData(i)->getMetaData().get());


        // imageData.loadMetaData();
        // }
    }
}

