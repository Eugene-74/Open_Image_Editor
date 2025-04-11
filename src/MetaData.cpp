#include "MetaData.hpp"

#include <QDateTime>
#include <QDebug>
#include <codecvt>
#include <ctime>
#include <locale>

MetaData& MetaData::operator=(const MetaData& other) {
    if (this != &other) {
        exifMetaData = other.exifMetaData;
        xmpMetaData = other.xmpMetaData;
        iptcMetaData = other.iptcMetaData;
    }
    return *this;
}

bool MetaData::operator==(const MetaData& other) const {
    // return std::equal(this->exifMetaData.begin(), this->exifMetaData.end(), other.exifMetaData.begin(), other.exifMetaData.end()) &&
    //        std::equal(this->xmpMetaData.begin(), this->xmpMetaData.end(), other.xmpMetaData.begin(), other.xmpMetaData.end()) &&
    //        std::equal(this->iptcMetaData.begin(), this->iptcMetaData.end(), other.iptcMetaData.begin(), other.iptcMetaData.end());
    return true;
}

void MetaData::saveMetaData(const std::string& imageName) {
    saveExifData(imageName, exifMetaData);
    saveXmpData(imageName, xmpMetaData);
    saveIptcData(imageName, iptcMetaData);
}

// Fonction pour récupérer l'orientation de l'image (rotation)
int MetaData::getImageOrientation() {
    for (auto& entry : xmpMetaData) {
        if (entry.key() == "Xmp.Exif.Image.Orientation") {
#ifdef _WIN32
            return entry.toLong();
#else
            return entry.toInt64();
#endif
        }
    }
    return 1;
}

long MetaData::getTimestamp() {
    for (auto& entry : xmpMetaData) {
        if (entry.key() == "Xpm.Exif.Image.DateTime") {
            QString dateTimeStr = QString::fromStdString(entry.toString());
            QDateTime dateTime = QDateTime::fromString(dateTimeStr, "yyyy:MM:dd HH:mm:ss");
            QDateTime epoch(QDate(1970, 1, 1), QTime(0, 0, 0));
            qint64 secs = epoch.secsTo(dateTime);
            if (secs < 0) {
                qWarning() << "Erreur : dateTime est antérieur à l'époque Unix.";
                return 0;
            }
            return secs;
        }
    }
    return 0;
}

// Fonction pour modifier une valeur dans Exiv2::ExifData ou la créer si elle n'existe pas
bool MetaData::modifyExifValue(const std::string& key, const std::string& newValue) {
    Exiv2::ExifKey exifKey(key);

    // Chercher la clé dans les métadonnées
    auto pos = exifMetaData.findKey(exifKey);
    if (pos != exifMetaData.end()) {
        try {
            pos->setValue(newValue);  // Assigner la nouvelle valeur
            return true;
        } catch (const Exiv2::Error& e) {
            qWarning() << "Invalid value for key " << key.c_str() << ": " << e.what();
            return false;
        }
        return true;
    }
        try {
            Exiv2::Exifdatum newDatum(exifKey);
            newDatum.setValue(newValue);
            exifMetaData.add(newDatum);

            return true;
        } catch (const Exiv2::Error& e) {
            qWarning() << "Erreur lors de l'ajout de la clé : " << e.what();
            return false;
        }
}

// Fonction pour modifier une valeur dans Exiv2::ExifData ou la créer si elle n'existe pas
bool MetaData::modifyXmpValue(const std::string& key, const std::string& newValue) {
    // qDebug() << "test modify";
    Exiv2::XmpKey xmpKey(key.c_str());
    // qDebug() << "created modify";

    // qDebug() << "try to find";

    auto pos = xmpMetaData.findKey(xmpKey);
    if (pos != xmpMetaData.end()) {
        qDebug() << "found";

        pos->setValue(newValue);

        return true;
    } else {
        qDebug() << "not found";

        try {
            qInfo() << "Creating the key : " << key;
            Exiv2::Xmpdatum newDatum(xmpKey);
            newDatum.setValue(newValue);

            xmpMetaData.add(newDatum);

            return true;
        } catch (const Exiv2::Error& e) {
            qWarning() << "Erreur lors de l'ajout de la clé : " << e.what();
            return false;
        }
    }
}
// Fonction pour modifier une valeur dans Exiv2::ExifData ou la créer si elle n'existe pas
bool MetaData::modifyIptcValue(const std::string& key, const std::string& newValue) {
    Exiv2::IptcKey exifKey(key);

    auto pos = iptcMetaData.findKey(exifKey);
    if (pos != iptcMetaData.end()) {
        pos->setValue(newValue);

        return true;
    } else {
        try {
            Exiv2::Iptcdatum newDatum(exifKey);
            newDatum.setValue(newValue);

            iptcMetaData.add(newDatum);

            return true;
        } catch (const Exiv2::Error& e) {
            qWarning() << "Erreur lors de l'ajout de la clé : " << e.what();
            return false;
        }
    }
}

// Fonction pour mettre à jour ou créer les métadonnées EXIF si elles n'existent pas
void MetaData::setOrCreateExifData(std::string imagePath) {
    time_t now = time(0);
    // struct tm* timeinfo = localtime(&now);
    // std::tm* timeinfo;
    // localtime_s(timeinfo, &now);
    std::tm* timeinfo = std::localtime(&now);

    char dateTime[20];
    strftime(dateTime, sizeof(dateTime), "%Y:%m:%d %H:%M:%S", timeinfo);

    modifyXmpValue("Xmp.Exif.Image.DateTime", dateTime);
    modifyXmpValue("Xmp.Exif.Image.Make", "made by photo editor");
    modifyXmpValue("Xmp.Exif.Image.Model", "my model");
    modifyXmpValue("Xmp.Exif.Image.DateTime", dateTime);

    saveMetaData(imagePath);
}

// Fonction pour charger les métadonnées EXIF d'une image
void MetaData::loadData(const std::string& imagePath) {
    try {
        std::unique_ptr<Exiv2::Image> image;
        if (!dataLoaded) {
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

            // Copy location data
            auto gpsLatitude = xmpMetaData.findKey(Exiv2::XmpKey("Xmp.exif.GPSLatitude"));
            if (gpsLatitude != xmpMetaData.end()) {
                exifMetaData["Exif.GPSInfo.GPSLatitude"] = gpsLatitude->value();
            }

            auto gpsLongitude = xmpMetaData.findKey(Exiv2::XmpKey("Xmp.exif.GPSLongitude"));
            if (gpsLongitude != xmpMetaData.end()) {
                exifMetaData["Exif.GPSInfo.GPSLongitude"] = gpsLongitude->value();
            }

            // Copy timestamp data
            auto dateTime = xmpMetaData.findKey(Exiv2::XmpKey("Xmp.Exif.Image.DateTime"));
            if (dateTime != xmpMetaData.end()) {
                exifMetaData["Exif.Image.DateTime"] = dateTime->value();
            }

            // Copy orientation data
            auto orientation = xmpMetaData.findKey(Exiv2::XmpKey("Xmp.Exif.Image.Orientation"));
            if (orientation != xmpMetaData.end()) {
                exifMetaData["Exif.Image.Orientation"] = orientation->value();
            }

            dataLoaded = true;

            auto pos = xmpMetaData.findKey(Exiv2::XmpKey("Xmp.Exif.Image.DateTime"));
            if (pos == xmpMetaData.end()) {
                qWarning() << "Erreur : 'Xmp.Exif.Image.DateTime' n'existe pas dans les métadonnées.";
            }
        }
    } catch (const Exiv2::Error& e) {
        qWarning() << "Erreur lors de la lecture des métadonnées EXIF, Xmp ou Iptc : " << e.what();
    } catch (const std::exception& e) {
        qWarning() << "Erreur : " << e.what();
    }
}

// Fonction pour sauvegarder les métadonnées EXIF dans une image
bool saveExifData(const std::string& imagePath, const Exiv2::ExifData& exifData) {
    try {
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
    } catch (const Exiv2::Error& e) {
        qWarning() << "Erreur lors de la sauvegarde des métadonnées EXIF : " << e.what();
        return false;
    }
}
bool saveXmpData(const std::string& imagePath, const Exiv2::XmpData& exifData) {
    try {
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
    } catch (const Exiv2::Error& e) {
        qWarning() << "Erreur lors de la sauvegarde des métadonnées EXIF : " << e.what();
        return false;
    }
}
bool saveIptcData(const std::string& imagePath, const Exiv2::IptcData& exifData) {
    try {
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
    } catch (const Exiv2::Error& e) {
        qWarning() << "Erreur lors de la sauvegarde des métadonnées EXIF : " << e.what();
        return false;
    }
}

// Fonction pour afficher les métadonnées EXIF
void displayExifData(const Exiv2::ExifData& data) {
    if (data.empty()) {
        qWarning() << "Aucune métadonnée EXIF disponible.";
    }
    qInfo() << "métadonnée : ";

    for (const auto& item : data) {
        qInfo() << item.key() << " : " << item.value().toString();
    }
}

// Fonction pour afficher les métadonnées EXIF
void displayXmpData(const Exiv2::XmpData& data) {
    if (data.empty()) {
        qWarning() << "Aucune métadonnée Xmp disponible.";
    }

    qInfo() << "métadonnée : ";

    for (const auto& item : data) {
        qInfo() << item.key() << " : " << item.value().toString();
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
        qWarning() << "Aucune métadonnée Iptc disponible.";
    }

    qInfo() << "métadonnée : ";

    for (const auto& item : data) {
        qInfo() << item.key() << " : " << item.value().toString();
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
    return exifMetaData;
}
Exiv2::XmpData MetaData::getXmpData() {
    return xmpMetaData;
}
Exiv2::IptcData MetaData::getIptcData() {
    return iptcMetaData;
}

void MetaData::clear() {
    exifMetaData.clear();
    xmpMetaData.clear();
    iptcMetaData.clear();
}