#include "MetaData.hpp"

#include <QDateTime>
#include <QDebug>
#include <codecvt>
#include <ctime>
#include <locale>

#include "GPS_Conversion.hpp"

/**
 * @brief Assignment operator for the MetaData class
 * @param other The other MetaData object to copy from
 * @return A reference to the current object
 */
MetaData& MetaData::operator=(const MetaData& other) {
    if (this != &other) {
        exifMetaData = other.exifMetaData;
        xmpMetaData = other.xmpMetaData;
        iptcMetaData = other.iptcMetaData;
    }
    return *this;
}

/**
 * @brief Equality operator for the MetaData class
 * @param other The other MetaData object to compare with
 * @return True if the objects are equal, false otherwise
 * @details NOT implemented yet. This function should compare the exifMetaData, xmpMetaData, and iptcMetaData members.
 */
bool MetaData::operator==(const MetaData& other) const {
    // return std::equal(this->exifMetaData.begin(), this->exifMetaData.end(), other.exifMetaData.begin(), other.exifMetaData.end()) &&
    //        std::equal(this->xmpMetaData.begin(), this->xmpMetaData.end(), other.xmpMetaData.begin(), other.xmpMetaData.end()) &&
    //        std::equal(this->iptcMetaData.begin(), this->iptcMetaData.end(), other.iptcMetaData.begin(), other.iptcMetaData.end());
    return true;
}

/**
 * @brief Saves the metadata to the specified image file
 * @param imagePath The path to the image file
 */
void MetaData::saveMetaData(const std::string& imagePath) {
    saveExifData(imagePath, exifMetaData);
    saveXmpData(imagePath, xmpMetaData);
    saveIptcData(imagePath, iptcMetaData);
}

/**
 * @brief Get the image orientation from the metadata
 * @return The image orientation (1-8)
 */
int MetaData::getImageOrientation() {
    for (auto& entry : exifMetaData) {
        if (entry.key() == "Exif.Image.Orientation") {
#ifdef _WIN32
            return entry.toLong();
#else
            return entry.toInt64();
#endif
        }
    }
    return 1;
}

/**
 * @brief Get the timestamp from the metadata
 * @return The timestamp in seconds since the Unix epoch
 */
long MetaData::getTimestamp() {
    for (auto& entry : exifMetaData) {
        if (entry.key() == "Exif.Image.DateTime") {
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

/**
 * @brief Set the timestamp in the metadata
 * @param timestamp The timestamp in seconds since the Unix epoch
 */
void MetaData::setTimestamp(long timestamp) {
    QDateTime dateTime = QDateTime::fromSecsSinceEpoch(timestamp);
    QString dateTimeStr = dateTime.toString("yyyy:MM:dd HH:mm:ss");
    modifyExifValue("Exif.Image.DateTime", dateTimeStr.toStdString());
}

/**
 * @brief Get the timestamp string from the metadata
 * @return The timestamp string in the format "yyyy-MM-dd HH:mm:ss"
 */
QString MetaData::getTimestampString() {
    for (auto& entry : exifMetaData) {
        if (entry.key() == "Exif.Image.DateTime") {
            QString dateTimeStr = QString::fromStdString(entry.toString());
            QDateTime dateTime = QDateTime::fromString(dateTimeStr, "yyyy:MM:dd HH:mm:ss");
            return dateTime.toString("yyyy-MM-dd HH:mm:ss");
        }
    }
    return "";
}

/**
 * @brief Set the Exif data
 * @param key The Exif key to set
 * @param newValue The new value to set for the Exif key
 * @return True if the value was modified successfully, false otherwise
 */
bool MetaData::modifyExifValue(const std::string& key, const std::string& newValue) {
    Exiv2::ExifKey exifKey(key);

    auto pos = exifMetaData.findKey(exifKey);
    if (pos != exifMetaData.end()) {
        try {
            pos->setValue(newValue);
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

/**
 * @brief Modify or create a value in Exiv2::XmpData
 * @param key The XMP key to modify or create
 * @param newValue The new value to set for the XMP key
 * @return True if the value was modified successfully, false otherwise
 */
bool MetaData::modifyXmpValue(const std::string& key, const std::string& newValue) {
    Exiv2::XmpKey xmpKey(key.c_str());

    auto pos = xmpMetaData.findKey(xmpKey);
    if (pos != xmpMetaData.end()) {
        pos->setValue(newValue);

        return true;
    } else {
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

/**
 * @brief Modify or create a value in Exiv2::IptcData
 * @param key The IPTC key to modify or create
 * @param newValue The new value to set for the IPTC key
 * @return True if the value was modified successfully, false otherwise
 */
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

/**
 * @brief Set or create Exif data for the image
 * @param imagePath The path to the image file
 * @details This function sets or creates Exif data for the image, including the date and time, make, model, and orientation.
 */
void MetaData::setOrCreateExifData(std::string imagePath) {
    time_t now = time(nullptr);
    std::tm* timeinfo = std::localtime(&now);

    char dateTime[20];
    strftime(dateTime, sizeof(dateTime), "%Y:%m:%d %H:%M:%S", timeinfo);

    modifyExifValue("Exif.Image.DateTime", dateTime);
    modifyExifValue("Exif.Image.Make", "made by photo editor");
    modifyExifValue("Exif.Image.Model", "my model");
    modifyExifValue("Exif.Image.DateTime", dateTime);

    saveMetaData(imagePath);
}

/**
 * @brief Load the metadata from the specified image file
 * @param imagePath The path to the image file
 * @details This function loads the metadata from the image file and copies location and timestamp data from EXIF to XMP.
 */
void MetaData::loadData(const std::string& imagePath) {
    try {
        std::unique_ptr<Exiv2::Image> image;
        if (!getDataLoadedConst()) {
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

            setDataLoaded(true);
        }
    } catch (const Exiv2::Error& e) {
        qWarning() << "Erreur lors de la lecture des métadonnées EXIF, Xmp ou Iptc : " << e.what();
    } catch (const std::exception& e) {
        qWarning() << "Erreur : " << e.what();
    }
}

/**
 * @brief Save the EXIF data to the specified image file
 * @param imagePath The path to the image file
 * @param exifData The EXIF data to save
 * @return True if the data was saved successfully, false otherwise
 */
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

/**
 * @brief Save the XMP data to the specified image file
 * @param imagePath The path to the image file
 * @param exifData The XMP data to save
 * @return True if the data was saved successfully, false otherwise
 */
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

/**
 * @brief Save the IPTC data to the specified image file
 * @param imagePath The path to the image file
 * @param exifData The IPTC data to save
 * @return True if the data was saved successfully, false otherwise
 */
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

/**
 * @brief Display the EXIF data
 * @param data The EXIF data to display
 */
void displayExifData(const Exiv2::ExifData& data) {
    if (data.empty()) {
        qWarning() << "Aucune métadonnée EXIF disponible.";
    }
    qInfo() << "métadonnée : ";

    for (const auto& item : data) {
        qInfo() << item.key() << " : " << item.value().toString();
    }
}

/**
 * @brief Display the XMP data
 * @param data The XMP data to display
 */
void displayXmpData(const Exiv2::XmpData& data) {
    if (data.empty()) {
        qWarning() << "Aucune métadonnée Xmp disponible.";
    }

    qInfo() << "métadonnée : ";

    for (const auto& item : data) {
        qInfo() << item.key() << " : " << item.value().toString();
    }
}

/**
 * @brief Display the IPTC data
 * @param data The IPTC data to display
 */
void displayIptcData(const Exiv2::IptcData& data) {
    if (data.empty()) {
        qWarning() << "Aucune métadonnée Iptc disponible.";
    }

    qInfo() << "métadonnée : ";

    for (const auto& item : data) {
        qInfo() << item.key() << " : " << item.value().toString();
    }
}

/**
 * @brief Display the metadata
 * @param metaData The metadata to display
 * @details This function displays the EXIF, XMP, and IPTC metadata.
 */
void displayData(const MetaData metaData) {
    displayExifData(metaData.getExifMetaDataConst());
    displayXmpData(metaData.getXmpMetaDataConst());
    displayIptcData(metaData.getIptcMetaDataConst());
}

/**
 * @brief Display the metadata
 * @details This function displays the EXIF, XMP, and IPTC metadata.
 */
void MetaData::displayMetaData() {
    displayData(*this);
}

/**
 * @brief Set the EXIF data
 * @param data The EXIF data to set
 */
void MetaData::setExifData(const Exiv2::ExifData data) {
    this->exifMetaData = data;
}

/**
 * @brief Set the XMP data
 * @param data The XMP data to set
 */
void MetaData::setXmpData(const Exiv2::XmpData data) {
    this->xmpMetaData = data;
}
/**
 * @brief Set the IPTC data
 * @param data The IPTC data to set
 */
void MetaData::setIptcData(const Exiv2::IptcData data) {
    this->iptcMetaData = data;
}

/**
 * @brief Get the EXIF data
 * @return The EXIF data
 */
Exiv2::ExifData MetaData::getExifData() {
    return this->exifMetaData;
}

/**
 * @brief Get the XMP data
 * @return The XMP data
 */
Exiv2::XmpData MetaData::getXmpData() {
    return this->xmpMetaData;
}

/**
 * @brief Get the IPTC data
 * @return The IPTC data
 */
Exiv2::IptcData MetaData::getIptcData() {
    return this->iptcMetaData;
}

/**
 * @brief Clear the metadata
 * @details This function clears the EXIF, XMP, and IPTC metadata.
 */
void MetaData::clear() {
    exifMetaData.clear();
    xmpMetaData.clear();
    iptcMetaData.clear();
}

/**
 * @brief Set the latitude in the metadata
 * @param latitude The latitude in degrees
 */
void MetaData::setLatitude(double latitude) {
    std::string gpsCoordinate = convertDecimalToGpsCoordinate(latitude);
    modifyExifValue("Exif.GPSInfo.GPSLatitude", gpsCoordinate);
}

/**
 * @brief Get the latitude from the metadata
 * @return The latitude in degrees
 */
double MetaData::getLatitude() const {
    for (auto& entry : exifMetaData) {
        if (entry.key() == "Exif.GPSInfo.GPSLatitude") {
            return entry.toFloat();
        }
    }
    return -1;
}

/**
 * @brief Set the longitude in the metadata
 * @param longitude The longitude in degrees
 */
void MetaData::setLongitude(double longitude) {
    std::string gpsCoordinate = convertDecimalToGpsCoordinate(longitude);
    modifyExifValue("Exif.GPSInfo.GPSLongitude", gpsCoordinate);
}

/**
 * @brief Get the longitude from the metadata
 * @return The longitude in degrees
 */
double MetaData::getLongitude() const {
    for (auto& entry : exifMetaData) {
        if (entry.key() == "Exif.GPSInfo.GPSLongitude") {
            return entry.toFloat();
        }
    }
    return -1;
}

/**
 * @brief Get the data loaded status
 * @return True if the data is loaded, false otherwise
 */
bool MetaData::getDataLoadedConst() const {
    return this->dataLoaded;
}

/**
 * @brief Set the data loaded status
 * @param dataLoaded The data loaded status to set
 */
void MetaData::setDataLoaded(bool dataLoaded) {
    this->dataLoaded = dataLoaded;
}

/**
 * @brief Get the EXIF metadata
 * @return A pointer to the EXIF metadata
 */
Exiv2::ExifData MetaData::getExifMetaDataConst() const {
    return exifMetaData;
}

/**
 * @brief Get the XMP metadata
 * @return A pointer to the XMP metadata
 */
Exiv2::XmpData MetaData::getXmpMetaDataConst() const {
    return xmpMetaData;
}

/**
 * @brief Get the IPTC metadata
 * @return A pointer to the IPTC metadata
 */
Exiv2::IptcData MetaData::getIptcMetaDataConst() const {
    return iptcMetaData;
}

/**
 * @brief Set the orientation in the metadata
 * @param orientation The orientation to set (1-8)
 */
void MetaData::setOrientation(int orientation) {
    if (orientation < 1 || orientation > 8) {
        qWarning() << "Orientation invalide : " << orientation;
        orientation = 1;
    }
    modifyExifValue("Exif.Image.Orientation", std::to_string(orientation));
}

/**
 * @brief Get the orientation from the metadata
 * @return The orientation (1-8)
 */
int MetaData::getOrientation() const {
    for (auto& entry : exifMetaData) {
        if (entry.key() == "Exif.Image.Orientation") {
#ifdef _WIN32
            return entry.toLong();
#else
            return entry.toInt64();
#endif
        }
    }
    return 1;
}