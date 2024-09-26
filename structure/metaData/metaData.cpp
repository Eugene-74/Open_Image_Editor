#include <iostream>
#include "metaData.h"



Exiv2::ExifData MetaData::get(){
    return metaData;
}

void MetaData::set(const Exiv2::ExifData& toAddMetaData){
    metaData = toAddMetaData;
}

void MetaData::loadMetaData(const std::string& imagePath){
    Exiv2::ExifData toAddMetaData = loadExifData(imagePath);
    // if (!toAddMetaData){
    metaData = toAddMetaData;

}

void MetaData::saveMetaData(const std::string& imageName){

    saveExifData(imageName, metaData);
    std::cerr << "Métadonnées sauvegardées pour l'image : " << imageName << std::endl;
}

// Fonction pour récupérer la largeur de l'image
int MetaData::getImageWidth() {
    for (auto& entry : metaData) {
        if (entry.key() == "Exif.Image.ImageWidth") {
            return entry.toLong();
        }
    }
    return -1;  // Retourne -1 si la largeur n'est pas trouvée
}

// Fonction pour récupérer la hauteur de l'image
int MetaData::getImageHeight() {
    for (auto& entry : metaData) {
        if (entry.key() == "Exif.Image.ImageLength") {
            return entry.toLong();
        }
    }
    return -1;  // Retourne -1 si la hauteur n'est pas trouvée
}
// Fonction pour récupérer l'orientation de l'image (rotation)
int MetaData::getImageOrientation() {
    for (auto& entry : metaData) {
        if (entry.key() == "Exif.Image.Orientation") {
            return entry.toLong();
        }
    }
    return -1;  // Retourne -1 si l'orientation n'est pas trouvée
}


Date MetaData::getImageDate() {
    for (auto& entry : metaData) {
        if (entry.key() == "Exif.Image.Orientation") {
            // Extraire les éléments de date et d'heure
            std::string dateStr = entry.toString();  // Format: "YYYY:MM:DD HH:MM:SS"
            int year, month, day, hours, minutes, seconds;
            char colon;  // Pour ignorer les caractères ":"
            std::stringstream ss(dateStr);
            ss >> year >> colon >> month >> colon >> day >> hours >> colon >> minutes >> colon >> seconds;

            return Date(seconds, minutes, hours, day, month, year);
        }
    }
    return Date(0, 0, 0, 0, 0, 0);  // Retourne -1 si l'orientation n'est pas trouvée
}

// Fonction pour modifier une valeur dans Exiv2::ExifData ou la créer si elle n'existe pas
bool MetaData::modifyExifValue(const std::string& key, const std::string& newValue) {
    std::cerr << "modification de clékey en cours : " << std::endl;

    Exiv2::ExifKey exifKey(key);

    // Chercher la clé dans les métadonnées
    auto pos = metaData.findKey(exifKey);
    if (pos != metaData.end()) {
        // Si la clé existe, modifier la valeur existante
        pos->setValue(newValue);  // Utiliser setValue pour assigner la nouvelle valeur
        // displayExifData(metaData);

        return true;  // Retourne vrai si l'opération est réussie
    }
    else {
        // Si la clé n'existe pas, la créer et y ajouter la nouvelle valeur
        try {
            // Créer un nouvel Exifdatum avec la clé
            Exiv2::Exifdatum newDatum(exifKey);
            newDatum.setValue(newValue);  // Assigner la nouvelle valeur

            // Ajouter le nouvel Exifdatum dans les métadonnées
            metaData.add(newDatum);

            return true;
        }
        catch (const Exiv2::Error& e) {
            std::cerr << "Erreur lors de l'ajout de la clé : " << e.what() << std::endl;
            return false;
        }
    }

}

// Fonction pour charger les métadonnées EXIF d'une image
Exiv2::ExifData loadExifData(const std::string& imagePath) {
    try {
        // Charger l'image
        Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(imagePath);
        image->readMetadata();

        // Retourner les métadonnées EXIF
        return image->exifData();
    }
    catch (Exiv2::Error& e) {
        std::cerr << "Erreur lors de la lecture des métadonnées EXIF : " << e.what() << std::endl;
        return Exiv2::ExifData(); // Retourne des métadonnées vides en cas d'erreur
    }
}

// Fonction pour sauvegarder les métadonnées EXIF dans une image
bool saveExifData(const std::string& imagePath, const Exiv2::ExifData& exifData) {
    try {
        // Charger l'image
        Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(imagePath);
        image->readMetadata();

        // Ajouter ou remplacer les métadonnées EXIF
        image->setExifData(exifData);

        // Sauvegarder les métadonnées dans l'image
        image->writeMetadata();
        return true;
    }
    catch (Exiv2::Error& e) {
        std::cerr << "Erreur lors de la sauvegarde des métadonnées EXIF : " << e.what() << std::endl;
        return false;
    }
}


// Fonction pour afficher les métadonnées EXIF
void displayExifData(const Exiv2::ExifData& exifData) {
    if (exifData.empty()) {
        std::cerr << "Aucune métadonnée EXIF disponible." << std::endl;
    }

    for (const auto& exifItem : exifData) {
        if (exifItem.key().substr(0, 10) == "Exif.Image") {
            std::cerr << exifItem.key() << " : " << exifItem.value() << std::endl;
        }
    }
}