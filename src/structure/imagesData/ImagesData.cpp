#include "ImagesData.h"

ImagesData& ImagesData::operator = (const ImagesData& other) {
    if (this != &other) {
        imagesData = other.imagesData; // Utiliser l'opérateur d'affectation de std::vector
        imageNumber = other.imageNumber;

    }
    return *this;
}


void ImagesData::setImageNumber(int nbr) {
    while (nbr < 0) {
        nbr += 1;
    }
    while (nbr >= imagesData.size()) {
        nbr -= 1;
    }

    imageNumber = nbr;
}
int ImagesData::getImageNumber() {
    return imageNumber;
}
void ImagesData::saveImagesData(std::string savePath) {
    // Sauvegarder l'objet dans un fichier binaire
    std::ofstream outFile(savePath, std::ios::binary);
    save(outFile);
    outFile.close();

    std::cerr << "ImagesData saved in : " << IMAGESDATA_SAVE_DAT_PATH << std::endl;


}

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
        // return nullptr;

        throw std::out_of_range("getImageData :: Index hors limites" + std::to_string(id));
    }

    return &imagesData.at(id);
    // renvoie un pointeur ce qui permet une modification automatique dans ImagesData
}

ImageData* ImagesData::getImageData(std::string imagePath) {

    auto it = std::find_if(imagesData.begin(), imagesData.end(),
        [&imagePath](const ImageData& imgD) {
            return imgD.imagePath == imagePath;
        });

    if (it != imagesData.end()) {
        return &(*it);
    }
    else {
        throw std::out_of_range("getImageData :: Image path not found: " + imagePath);
    }
}




ImageData* ImagesData::getCurrentImageData() {
    if (imagesData.size() <= 0 || imageNumber >= imagesData.size()) {
        return nullptr;
        throw std::out_of_range("getCurrentImageData :: Index hors limites");

    }

    return &imagesData.at(imageNumber);
}
std::vector<ImageData> ImagesData::get() {
    return imagesData;
}

void ImagesData::save(std::ofstream& out) const {
    // Sauvegarder le nombre d'images
    size_t imageCount = imagesData.size();
    out.write(reinterpret_cast<const char*>(&imageCount), sizeof(imageCount));

    // Sauvegarder chaque image
    for (const auto& image : imagesData) {
        std::cerr << image.get() << std::endl;
        image.save(out);


    }


    // Sauvegarder imageNumber
    out.write(reinterpret_cast<const char*>(&imageNumber), sizeof(imageNumber));

}

void ImagesData::load(std::ifstream& in) {
    // Effacer les données existantes
    imagesData.clear();

    // Lire le nombre d'images
    size_t imageCount;
    in.read(reinterpret_cast<char*>(&imageCount), sizeof(imageCount));

    // Lire chaque image
    imagesData.resize(imageCount);
    for (size_t i = 0; i < imageCount; ++i) {
        imagesData[i].load(in);
    }

    // Charger imageNumber
    in.read(reinterpret_cast<char*>(&imageNumber), sizeof(imageNumber));
}

ImagesData* loadImagesData(std::string savePath) {
    ImagesData* loadedImagesData = new ImagesData();

    // Ouvrir le fichier en mode binaire
    std::ifstream inFile(savePath, std::ios::binary);

    // Vérifier si le fichier a été ouvert correctement
    if (!inFile) {
        std::cerr << "Erreur : le fichier" << savePath << "n'existe pas ou ne peut pas être ouvert." << std::endl;
        // Vous pouvez gérer cette situation selon vos besoins
        return loadedImagesData; // Retourne un objet vide ou lance une exception selon votre logique
    }

    // Charger les données à partir du fichier
    loadedImagesData->load(inFile);

    inFile.close(); // Fermer le fichier

    return loadedImagesData; // Retourner l'objet chargé
}

