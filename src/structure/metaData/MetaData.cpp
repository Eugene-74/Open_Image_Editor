#include "MetaData.h"
#include <fstream>


MetaData& MetaData::operator=(const MetaData& other) {
    if (this != &other) {
        exifMetaData = other.exifMetaData; // Utiliser l'opérateur d'affectation de std::vector
        xmpMetaData = other.xmpMetaData; // Utiliser l'opérateur d'affectation de std::vector
        iptcMetaData = other.iptcMetaData; // Utiliser l'opérateur d'affectation de std::vector

    }
    return *this;
}

void MetaData::saveMetaData(const std::string& imageName) {

    saveExifData(imageName, exifMetaData);
    saveXmpData(imageName, xmpMetaData);
    saveIptcData(imageName, iptcMetaData);
    displayExifData(exifMetaData);
    std::cerr << "Métadonnées sauvegardées pour l'image : " << imageName << std::endl;
}

// Fonction pour récupérer la largeur de l'image
int MetaData::getImageWidth() {
    for (auto& entry : exifMetaData) {
        if (entry.key() == "Exif.Image.ImageWidth") {
            return entry.toInt64();
        }
    }
    return -1;  // Retourne -1 si la largeur n'est pas trouvée
}

// Fonction pour récupérer la hauteur de l'image
int MetaData::getImageHeight() {
    for (auto& entry : exifMetaData) {
        if (entry.key() == "Exif.Image.ImageLength") {
            return entry.toInt64();
        }
    }
    return -1;  // Retourne -1 si la hauteur n'est pas trouvée
}
// Fonction pour récupérer l'orientation de l'image (rotation)
int MetaData::getImageOrientation() {
    for (auto& entry : exifMetaData) {
        if (entry.key() == "Exif.Image.Orientation") {
            return entry.toInt64();
        }
    }
    return 1;  // Retourne -1 si l'orientation n'est pas trouvée
}




// Fonction pour modifier une valeur dans Exiv2::ExifData ou la créer si elle n'existe pas
bool MetaData::modifyExifValue(const std::string& key, const std::string& newValue) {
    std::cerr << "modification de clékey en cours : " << std::endl;

    Exiv2::ExifKey exifKey(key);

    // Chercher la clé dans les métadonnées
    auto pos = exifMetaData.findKey(exifKey);
    if (pos != exifMetaData.end()) {
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
            exifMetaData.add(newDatum);

            return true;
        }
        catch (const Exiv2::Error& e) {
            std::cerr << "Erreur lors de l'ajout de la clé : " << e.what() << std::endl;
            return false;
        }
    }
}

// Fonction pour modifier une valeur dans Exiv2::ExifData ou la créer si elle n'existe pas
bool MetaData::modifyXmpValue(const std::string& key, const std::string& newValue) {
    std::cerr << "modification de clékey en cours : " << std::endl;

    Exiv2::XmpKey exifKey(key);

    // Chercher la clé dans les métadonnées
    auto pos = xmpMetaData.findKey(exifKey);
    if (pos != xmpMetaData.end()) {
        // Si la clé existe, modifier la valeur existante
        pos->setValue(newValue);  // Utiliser setValue pour assigner la nouvelle valeur
        // displayExifData(metaData);

        return true;  // Retourne vrai si l'opération est réussie
    }
    else {
        // Si la clé n'existe pas, la créer et y ajouter la nouvelle valeur
        try {
            // Créer un nouvel Exifdatum avec la clé
            Exiv2::Xmpdatum newDatum(exifKey);
            newDatum.setValue(newValue);  // Assigner la nouvelle valeur

            // Ajouter le nouvel Exifdatum dans les métadonnées
            xmpMetaData.add(newDatum);

            return true;
        }
        catch (const Exiv2::Error& e) {
            std::cerr << "Erreur lors de l'ajout de la clé : " << e.what() << std::endl;
            return false;
        }
    }
}
// Fonction pour modifier une valeur dans Exiv2::ExifData ou la créer si elle n'existe pas
bool MetaData::modifyIptcValue(const std::string& key, const std::string& newValue) {
    std::cerr << "modification de clékey en cours : " << std::endl;

    Exiv2::IptcKey exifKey(key);

    // Chercher la clé dans les métadonnées
    auto pos = iptcMetaData.findKey(exifKey);
    if (pos != iptcMetaData.end()) {
        // Si la clé existe, modifier la valeur existante
        pos->setValue(newValue);  // Utiliser setValue pour assigner la nouvelle valeur
        // displayExifData(metaData);

        return true;  // Retourne vrai si l'opération est réussie
    }
    else {
        // Si la clé n'existe pas, la créer et y ajouter la nouvelle valeur
        try {
            // Créer un nouvel Exifdatum avec la clé
            Exiv2::Iptcdatum newDatum(exifKey);
            newDatum.setValue(newValue);  // Assigner la nouvelle valeur

            // Ajouter le nouvel Exifdatum dans les métadonnées
            iptcMetaData.add(newDatum);

            return true;
        }
        catch (const Exiv2::Error& e) {
            std::cerr << "Erreur lors de l'ajout de la clé : " << e.what() << std::endl;
            return false;
        }
    }
}



// Fonction pour mettre à jour ou créer les métadonnées EXIF si elles n'existent pas
void MetaData::setOrCreateExifData(std::string& imagePath) {

    // Obtenir le timestamp actuel
    time_t now = time(0);
    struct tm* timeinfo = localtime(&now);
    char dateTime[20];
    strftime(dateTime, sizeof(dateTime), "%Y:%m:%d %H:%M:%S", timeinfo);

    modifyExifValue("Exif.Image.DateTime", dateTime);
    modifyExifValue("Exif.Image.Make", "made by photo editor");
    modifyExifValue("Exif.Image.Model", "my model");
    modifyXmpValue("Exif.Image.DateTime", dateTime);

    saveMetaData(imagePath);
}

// Fonction pour charger les métadonnées EXIF d'une image
void MetaData::loadData(const std::string& imagePath) {
    try {
        // Charger l'image
        std::unique_ptr<Exiv2::Image> image = Exiv2::ImageFactory::open(imagePath);
        image->readMetadata();
        exifMetaData = image->exifData();
        xmpMetaData = image->xmpData();
        iptcMetaData = image->iptcData();



    }
    catch (const Exiv2::Error& e) {
        std::cerr << "Erreur lors de la lecture des métadonnées EXIF, Xmp ou Iptc : " << e.what() << std::endl;
        // return Exiv2::ExifData(); // Retourne des métadonnées vides en cas d'erreur
    }
}

// Fonction pour sauvegarder les métadonnées EXIF dans une image
bool saveExifData(const std::string& imagePath, const Exiv2::ExifData& exifData) {
    try {
        // Charger l'image
        std::unique_ptr<Exiv2::Image> image = Exiv2::ImageFactory::open(imagePath);
        image->readMetadata();

        // Ajouter ou remplacer les métadonnées EXIF
        image->setExifData(exifData);

        // Sauvegarder les métadonnées dans l'image
        image->writeMetadata();
        return true;
    }
    catch (const Exiv2::Error& e) {
        std::cerr << "Erreur lors de la sauvegarde des métadonnées EXIF : " << e.what() << std::endl;
        return false;
    }
}
bool saveXmpData(const std::string& imagePath, const Exiv2::XmpData& exifData) {
    try {
        // Charger l'image
        std::unique_ptr<Exiv2::Image> image = Exiv2::ImageFactory::open(imagePath);
        image->readMetadata();

        // Ajouter ou remplacer les métadonnées EXIF
        image->setXmpData(exifData);

        // Sauvegarder les métadonnées dans l'image
        image->writeMetadata();
        return true;
    }
    catch (const Exiv2::Error& e) {
        std::cerr << "Erreur lors de la sauvegarde des métadonnées EXIF : " << e.what() << std::endl;
        return false;
    }
}
bool saveIptcData(const std::string& imagePath, const Exiv2::IptcData& exifData) {
    try {
        // Charger l'image
        std::unique_ptr<Exiv2::Image> image = Exiv2::ImageFactory::open(imagePath);
        image->readMetadata();

        // Ajouter ou remplacer les métadonnées EXIF
        image->setIptcData(exifData);

        // Sauvegarder les métadonnées dans l'image
        image->writeMetadata();
        return true;
    }
    catch (const Exiv2::Error& e) {
        std::cerr << "Erreur lors de la sauvegarde des métadonnées EXIF : " << e.what() << std::endl;
        return false;
    }
}


// Fonction pour afficher les métadonnées EXIF
void displayExifData(const Exiv2::ExifData& data) {
    if (data.empty()) {
        std::cerr << "Aucune métadonnée EXIF disponible." << std::endl;
    }
    std::cerr << "métadonnée : " << std::endl;

    for (const auto& item : data) {
        if (item.key().substr(0, 10) == "Exif.Image") {
            std::cerr << item.key() << " : " << item.value() << std::endl;
        }
    }
}

// Fonction pour afficher les métadonnées EXIF
void displayXmpData(const Exiv2::XmpData& data) {
    if (data.empty()) {
        std::cerr << "Aucune métadonnée Xmp disponible." << std::endl;
    }

    for (const auto& item : data) {
        // if (item.key().substr(0, 10) == "Exif.Image") {
        std::cerr << item.key() << " : " << item.value() << std::endl;
        // }
    }
}

void displayData(const MetaData metaData) {
    displayExifData(metaData.exifMetaData);
    displayXmpData(metaData.xmpMetaData);
    displayIptcData(metaData.iptcMetaData);
}

void MetaData::displayMetaData() {
    displayData(*this);

}

// Fonction pour afficher les métadonnées EXIF
void displayIptcData(const Exiv2::IptcData& data) {
    if (data.empty()) {
        std::cerr << "Aucune métadonnée Iptc disponible." << std::endl;
    }

    for (const auto& item : data) {
        // if (item.key().substr(0, 10) == "Exif.Image") {
        std::cerr << item.key() << " : " << item.value() << std::endl;
        // }
    }
}

void MetaData::load(std::ifstream& in) {
    // Effacer toutes les données EXIF existantes
    exifMetaData.clear();

    // Lire le nombre d'entrées EXIF
    size_t count;
    in.read(reinterpret_cast<char*>(&count), sizeof(count));

    // Lire chaque entrée EXIF
    for (size_t i = 0; i < count; ++i) {
        // Lire la clé de l'entrée
        size_t keyLength;
        in.read(reinterpret_cast<char*>(&keyLength), sizeof(keyLength));
        std::string key(keyLength, '\0');
        in.read(&key[0], keyLength);

        // Lire la valeur associée
        size_t valueLength;
        in.read(reinterpret_cast<char*>(&valueLength), sizeof(valueLength));
        std::string value(valueLength, '\0');
        in.read(&value[0], valueLength);

        // Insérer l'entrée dans metaData (la clé et la valeur)
        Exiv2::ExifKey exifKey(key);
        // Exiv2::Value::AutoPtr exifValue = Exiv2::Value::create(Exiv2::asciiString);
        std::unique_ptr<Exiv2::Value> exifValue = Exiv2::Value::create(Exiv2::asciiString);
        exifValue->read(value);
        exifMetaData.add(exifKey, exifValue.get());
    }
}

void MetaData::save(std::ofstream& out) const {
    // Sauvegarder le nombre d'entrées EXIF
    size_t count = exifMetaData.count();
    out.write(reinterpret_cast<const char*>(&count), sizeof(count));

    // Pour chaque entrée dans les données EXIF
    for (const auto& exifEntry : exifMetaData) {
        // Sauvegarder la clé de l'entrée
        std::string key = exifEntry.key();
        size_t keyLength = key.size();
        out.write(reinterpret_cast<const char*>(&keyLength), sizeof(keyLength));
        out.write(key.c_str(), keyLength);

        // Sauvegarder les valeurs associées
        std::string value = exifEntry.toString();
        size_t valueLength = value.size();
        out.write(reinterpret_cast<const char*>(&valueLength), sizeof(valueLength));
        out.write(value.c_str(), valueLength);
    }
}

void MetaData::setExifData(const Exiv2::ExifData data) {
    exifMetaData = data;
}
void MetaData::setXmpData(const Exiv2::XmpData data) {
    xmpMetaData = data;
}
void MetaData::setIptcData(const Exiv2::IptcData data) {
    iptcMetaData = data;
}

Exiv2::ExifData MetaData::getExifData() {
    return  exifMetaData;
}
Exiv2::XmpData MetaData::getXmpData() {
    return xmpMetaData;
}
Exiv2::IptcData MetaData::getIptcData() {
    return iptcMetaData;
}
