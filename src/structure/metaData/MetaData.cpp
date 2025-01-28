#include "MetaData.h"


MetaData& MetaData::operator=(const MetaData& other)
{
    if (this != &other)
    {
        exifMetaData = other.exifMetaData; // Utiliser l'opérateur d'affectation de std::vector
        xmpMetaData = other.xmpMetaData;   // Utiliser l'opérateur d'affectation de std::vector
        iptcMetaData = other.iptcMetaData; // Utiliser l'opérateur d'affectation de std::vector
    }
    return *this;
}

void MetaData::saveMetaData(const std::string& imageName)
{

    saveExifData(imageName, exifMetaData);
    saveXmpData(imageName, xmpMetaData);
    saveIptcData(imageName, iptcMetaData);
    // displayExifData(exifMetaData);
}

// Fonction pour récupérer la largeur de l'image
int MetaData::getImageWidth()
{
    for (auto& entry : exifMetaData)
    {
        if (entry.key() == "Exif.Image.ImageWidth")
        {
#ifdef _WIN32
            return entry.toLong();
#else
            return entry.toInt64();
#endif
            // return entry.toInt64();
        }
    }
    return -1; // Retourne -1 si la largeur n'est pas trouvée
}

// Fonction pour récupérer la hauteur de l'image
int MetaData::getImageHeight()
{
    for (auto& entry : exifMetaData)
    {
        if (entry.key() == "Exif.Image.ImageLength")
        {
#ifdef _WIN32
            return entry.toLong();
#else
            return entry.toInt64();
#endif
            // return entry.toInt64();
        }
    }
    return -1; // Retourne -1 si la hauteur n'est pas trouvée
}
// Fonction pour récupérer l'orientation de l'image (rotation)
int MetaData::getImageOrientation()
{
    for (auto& entry : exifMetaData)
    {
        if (entry.key() == "Exif.Image.Orientation")
        {
#ifdef _WIN32
            return entry.toLong();
#else
            return entry.toInt64();
#endif
            // return entry.toInt64();
        }
    }
    return 1; // Retourne -1 si l'orientation n'est pas trouvée
}

// Fonction pour modifier une valeur dans Exiv2::ExifData ou la créer si elle n'existe pas
bool MetaData::modifyExifValue(const std::string& key, const std::string& newValue)
{

    Exiv2::ExifKey exifKey(key);

    // Chercher la clé dans les métadonnées
    auto pos = exifMetaData.findKey(exifKey);
    if (pos != exifMetaData.end())
    {
        // Si la clé existe, modifier la valeur existante
        pos->setValue(newValue); // Utiliser setValue pour assigner la nouvelle valeur
        // displayExifData(metaData);

        return true; // Retourne vrai si l'opération est réussie
    } else
    {
        // Si la clé n'existe pas, la créer et y ajouter la nouvelle valeur
        try
        {
            // Créer un nouvel Exifdatum avec la clé
            Exiv2::Exifdatum newDatum(exifKey);
            newDatum.setValue(newValue); // Assigner la nouvelle valeur

            // Ajouter le nouvel Exifdatum dans les métadonnées
            exifMetaData.add(newDatum);

            return true;
        } catch (const Exiv2::Error& e)
        {
            std::cerr << "Erreur lors de l'ajout de la clé : " << e.what() << std::endl;
            return false;
        }
    }
}

// Fonction pour modifier une valeur dans Exiv2::ExifData ou la créer si elle n'existe pas
bool MetaData::modifyXmpValue(const std::string& key, const std::string& newValue)
{
    Exiv2::XmpKey exifKey(key);

    // Chercher la clé dans les métadonnées
    auto pos = xmpMetaData.findKey(exifKey);
    if (pos != xmpMetaData.end())
    {
        // Si la clé existe, modifier la valeur existante
        pos->setValue(newValue); // Utiliser setValue pour assigner la nouvelle valeur
        // displayExifData(metaData);

        return true; // Retourne vrai si l'opération est réussie
    } else
    {
        // Si la clé n'existe pas, la créer et y ajouter la nouvelle valeur
        try
        {
            // Créer un nouvel Exifdatum avec la clé
            Exiv2::Xmpdatum newDatum(exifKey);
            newDatum.setValue(newValue); // Assigner la nouvelle valeur

            // Ajouter le nouvel Exifdatum dans les métadonnées
            xmpMetaData.add(newDatum);

            return true;
        } catch (const Exiv2::Error& e)
        {
            std::cerr << "Erreur lors de l'ajout de la clé : " << e.what() << std::endl;
            return false;
        }
    }
}
// Fonction pour modifier une valeur dans Exiv2::ExifData ou la créer si elle n'existe pas
bool MetaData::modifyIptcValue(const std::string& key, const std::string& newValue)
{
    Exiv2::IptcKey exifKey(key);

    // Chercher la clé dans les métadonnées
    auto pos = iptcMetaData.findKey(exifKey);
    if (pos != iptcMetaData.end())
    {
        // Si la clé existe, modifier la valeur existante
        pos->setValue(newValue); // Utiliser setValue pour assigner la nouvelle valeur
        // displayExifData(metaData);

        return true; // Retourne vrai si l'opération est réussie
    } else
    {
        // Si la clé n'existe pas, la créer et y ajouter la nouvelle valeur
        try
        {
            // Créer un nouvel Exifdatum avec la clé
            Exiv2::Iptcdatum newDatum(exifKey);
            newDatum.setValue(newValue); // Assigner la nouvelle valeur

            // Ajouter le nouvel Exifdatum dans les métadonnées
            iptcMetaData.add(newDatum);

            return true;
        } catch (const Exiv2::Error& e)
        {
            std::cerr << "Erreur lors de l'ajout de la clé : " << e.what() << std::endl;
            return false;
        }
    }
}

// Fonction pour mettre à jour ou créer les métadonnées EXIF si elles n'existent pas
void MetaData::setOrCreateExifData(std::string& imagePath)
{

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
void MetaData::loadData(const std::string& imagePath){
    try{
        std::unique_ptr<Exiv2::Image> image;
        if (!dataLoaded){

#ifdef _WIN32
            std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
            std::wstring wImagePath = converter.from_bytes(imagePath);
            image = Exiv2::ImageFactory::open(wImagePath);
#else
            image = Exiv2::ImageFactory::open(imagePath);
#endif

            image->readMetadata();

            exifMetaData = image->exifData();
            xmpMetaData = image->xmpData();
            iptcMetaData = image->iptcData();

            dataLoaded = true;
        }
    } catch (const Exiv2::Error& e){
        std::cerr << "Erreur lors de la lecture des métadonnées EXIF, Xmp ou Iptc : " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Erreur : " << e.what() << std::endl;
    }
}

// Fonction pour sauvegarder les métadonnées EXIF dans une image
bool saveExifData(const std::string& imagePath, const Exiv2::ExifData& exifData){
    try
    {
#ifdef _WIN32
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        std::wstring wImagePath = converter.from_bytes(imagePath);
        std::unique_ptr<Exiv2::Image> image = Exiv2::ImageFactory::open(wImagePath);
#else
        std::unique_ptr<Exiv2::Image> image = Exiv2::ImageFactory::open(imagePath);
#endif

        image->readMetadata();
        image->setExifData(exifData);

        image->writeMetadata();


        return true;
    } catch (const Exiv2::Error& e)
    {
        std::cerr << "Erreur lors de la sauvegarde des métadonnées EXIF : " << e.what() << std::endl;
        return false;
    }
}
bool saveXmpData(const std::string& imagePath, const Exiv2::XmpData& exifData)
{
    try
    {
#ifdef _WIN32
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        std::wstring wImagePath = converter.from_bytes(imagePath);
        std::unique_ptr<Exiv2::Image> image = Exiv2::ImageFactory::open(wImagePath);
#else
        std::unique_ptr<Exiv2::Image> image = Exiv2::ImageFactory::open(imagePath);
#endif
        image->readMetadata();

        image->setXmpData(exifData);

        image->writeMetadata();
        return true;
    } catch (const Exiv2::Error& e)
    {
        std::cerr << "Erreur lors de la sauvegarde des métadonnées EXIF : " << e.what() << std::endl;
        return false;
    }
}
bool saveIptcData(const std::string& imagePath, const Exiv2::IptcData& exifData)
{
    try
    {
#ifdef _WIN32
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        std::wstring wImagePath = converter.from_bytes(imagePath);
        std::unique_ptr<Exiv2::Image> image = Exiv2::ImageFactory::open(wImagePath);
#else
        std::unique_ptr<Exiv2::Image> image = Exiv2::ImageFactory::open(imagePath);
#endif
        image->readMetadata();

        image->setIptcData(exifData);

        image->writeMetadata();
        return true;
    } catch (const Exiv2::Error& e)
    {
        std::cerr << "Erreur lors de la sauvegarde des métadonnées EXIF : " << e.what() << std::endl;
        return false;
    }
}

// Fonction pour afficher les métadonnées EXIF
void displayExifData(const Exiv2::ExifData& data)
{
    if (data.empty())
    {
        std::cerr << "Aucune métadonnée EXIF disponible." << std::endl;
    }
    std::cerr << "métadonnée : " << std::endl;

    for (const auto& item : data)
    {
        if (item.key().substr(0, 10) == "Exif.Image")
        {
            std::cerr << item.key() << " : " << item.value() << std::endl;
        }
    }
}

// Fonction pour afficher les métadonnées EXIF
void displayXmpData(const Exiv2::XmpData& data)
{
    if (data.empty())
    {
        std::cerr << "Aucune métadonnée Xmp disponible." << std::endl;
    }

    for (const auto& item : data)
    {
        // if (item.key().substr(0, 10) == "Exif.Image") {
        std::cerr << item.key() << " : " << item.value() << std::endl;
        // }
    }
}

void displayData(const MetaData metaData)
{
    displayExifData(metaData.exifMetaData);
    displayXmpData(metaData.xmpMetaData);
    displayIptcData(metaData.iptcMetaData);
}

void MetaData::displayMetaData()
{
    displayData(*this);
}

// Fonction pour afficher les métadonnées EXIF
void displayIptcData(const Exiv2::IptcData& data)
{
    if (data.empty())
    {
        std::cerr << "Aucune métadonnée Iptc disponible." << std::endl;
    }

    for (const auto& item : data)
    {
        // if (item.key().substr(0, 10) == "Exif.Image") {
        std::cerr << item.key() << " : " << item.value() << std::endl;
        // }
    }
}


void MetaData::setExifData(const Exiv2::ExifData data)
{
    exifMetaData = data;
}
void MetaData::setXmpData(const Exiv2::XmpData data)
{
    xmpMetaData = data;
}
void MetaData::setIptcData(const Exiv2::IptcData data)
{
    iptcMetaData = data;
}

Exiv2::ExifData MetaData::getExifData()
{
    return exifMetaData;
}
Exiv2::XmpData MetaData::getXmpData()
{
    return xmpMetaData;
}
Exiv2::IptcData MetaData::getIptcData()
{
    return iptcMetaData;
}
