#include <iostream>
#include "../imagesData/imagesData.h"
#include "../imageData/imageData.h"



void ImagesData::print() const {
    std::cerr << "ImagesData : \n";
    for (const ImageData& valeur : imagesData) {
        std::cerr << valeur.get();
    }
    std::cerr << std::endl;
}

void ImagesData::addImage(ImageData& imageD) {
    auto it = std::find_if(imagesData.begin(), imagesData.end(),
        [&imageD](const ImageData& imgD) {
            return imgD == imageD;
        });

    if (it != imagesData.end()) {
        ImageData lastImageD = *it;
        imagesData.erase(it);
        imageD.addFolders(lastImageD.getFolders());

        imagesData.push_back(imageD);
    }
    else {
        imagesData.push_back(imageD);
    }
}

void ImagesData::removeImage(const ImageData& image) {
    auto it = std::find(imagesData.begin(), imagesData.end(), image);

    if (it != imagesData.end()) {
        imagesData.erase(it);
    }
}

ImageData* ImagesData::getImageData(int id) {
    if (id < 0 || id >= imagesData.size()) {
        throw std::out_of_range("Index hors limites");
    }
    return &imagesData.at(id);
    // renvoie un pointeur ce qui permet une modification automatique dans ImagesData
}
std::vector<ImageData>  ImagesData::get() {
    return imagesData;
}

ImagesData loadImagesData() {
    ImagesData loadedImagesData({});

    // Ouvrir le fichier en mode binaire
    std::ifstream inFile(SAVE_DAT_PATH, std::ios::binary);

    // Vérifier si le fichier a été ouvert correctement
    if (!inFile) {
        std::cerr << "Erreur : le fichier imagesData.dat n'existe pas ou ne peut pas être ouvert." << std::endl;
        // Vous pouvez gérer cette situation selon vos besoins
        return loadedImagesData; // Retourne un objet vide ou lance une exception selon votre logique
    }

    // Charger les données à partir du fichier
    loadedImagesData.load(inFile);

    inFile.close(); // Fermer le fichier

    return loadedImagesData; // Retourner l'objet chargé
}