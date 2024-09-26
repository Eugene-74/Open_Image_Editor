#ifndef METADATA_H
#define METADATA_H

#include <iostream>
#include <vector>
#include <fstream>
#include <exiv2/exiv2.hpp>

#include "../../structure/date/date.h"


class MetaData
{
public:
    Exiv2::ExifData metaData;

    // Constructeur par défaut
    MetaData() = default;

    // Constructeur de copie
    MetaData(const MetaData& other) : metaData(other.metaData) {}

    // Opérateur d'affectation
    MetaData& operator=(const MetaData& other) {
        if (this != &other) {
            metaData = other.metaData; // Utiliser l'opérateur d'affectation de std::vector
        }
        return *this;
    }

    Exiv2::ExifData get();

    void set(const Exiv2::ExifData& toAddMetaData);

    void loadMetaData(const std::string& imagePath);

    void saveMetaData(const std::string& imageName);

    int getImageWidth();

    int getImageHeight();

    int getImageOrientation();

    Date getImageDate();

    bool modifyExifValue(const std::string& key, const std::string& newValue);

    // Sauvegarder en binaire
    void save(std::ofstream& out) const {
        // Sauvegarder le nombre d'entrées EXIF
        size_t count = metaData.count();
        out.write(reinterpret_cast<const char*>(&count), sizeof(count));

        // Pour chaque entrée dans les données EXIF
        for (const auto& exifEntry : metaData) {
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

    // Charger à partir d'un fichier binaire
    void load(std::ifstream& in) {
        // Effacer toutes les données EXIF existantes
        metaData.clear();

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
            Exiv2::Value::AutoPtr exifValue = Exiv2::Value::create(Exiv2::asciiString);
            exifValue->read(value);
            metaData.add(exifKey, exifValue.get());
        }
    }
};


Exiv2::ExifData loadExifData(const std::string& imagePath);

bool saveExifData(const std::string& imagePath, const Exiv2::ExifData& exifData);

void displayExifData(const Exiv2::ExifData& exifData);

#endif
