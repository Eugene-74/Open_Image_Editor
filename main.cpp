#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <algorithm>
#include <exiv2/exiv2.hpp>

#include <structure.h>

namespace fs = std::filesystem;


template <typename T>
void addUnique(std::vector<T>& vec, const T& element);


class Date {
public:
    int seconds;
    int minutes;
    int hours;
    int day;
    int month;
    int year;

    Date(int s, int m, int h, int d, int m2, int y)
        : seconds(s), minutes(m), hours(h), day(d), month(m2), year(y) {
        if (day < 1 || day > 31) {
            std::cout << "Jour invalide" << std::endl;
            return;
        }
        if (month < 1 || month > 12) {
            std::cout << "Mois invalide" << std::endl;
            return;
        }
        if (year < 0 || year > 9999) {
            std::cout << "Année invalide" << std::endl;
            return;
        }
    }

    void print() const {
        std::cout << "Jour : " << day << " Mois : " << month << " Année : " << year << std::endl;
    }

    std::string getStringJJ_MM_AAAA() const {
        return std::to_string(day) + "/" + std::to_string(month) + "/" + std::to_string(year);
    }
};

class Folders
{
public:
    std::vector<std::string> folders;

    Folders(std::vector<std::string> a) : folders(a) {
    }

    void print() const {
        for (const std::string& valeur : folders) {
            std::cout << valeur << " ";
        }
        std::cout << std::endl;
    }
    std::string getFolderList() const {
        std::string folderList;
        for (const auto& folder : folders) {
            if (!folderList.empty()) {
                folderList += ", "; // Ajouter une virgule entre les dossiers
            }
            folderList += folder; // Ajouter le dossier
        }
        return folderList; // Retourner la chaîne complète
    }
    void addFolder(const std::string& folder) {
        addUnique(folders, folder);

    }
    std::vector<std::string>  getFolders(){
        return folders;
    }

};

bool saveExifData(const std::string& imagePath, const Exiv2::ExifData& exifData);
Exiv2::ExifData loadExifData(const std::string& imagePath);
void displayExifData(const Exiv2::ExifData& exifData);
Date timestampToDate(time_t timestamp);

class MetaData
{
public:
    Exiv2::ExifData metaData;

    Exiv2::ExifData get(){
        return metaData;
    }

    void set(const Exiv2::ExifData& toAddMetaData){
        metaData = toAddMetaData;
    }

    void load(const std::string& imagePath){
        Exiv2::ExifData toAddMetaData = loadExifData(imagePath);
        // if (!toAddMetaData){
        metaData = toAddMetaData;

        displayExifData(metaData);

        std::cout
            << "\norientation: " << getImageOrientation()
            << "\nx : " << getImageWidth()
            << "\ny : " << getImageHeight()


            << std::endl;
        // modifyExifValue("Exif.Image.Orientation", std::to_string(6));
        // displayExifData(metaData);
        // saveExifData(imagePath, metaData);


    }

    void save(const std::string& imageName){

        saveExifData(imageName, metaData);
        std::cout << "Métadonnées sauvegardées pour l'image : " << imageName << std::endl;
    }

    // Fonction pour récupérer la largeur de l'image
    int getImageWidth() {
        for (auto& entry : metaData) {
            if (entry.key() == "Exif.Image.ImageWidth") {
                return entry.toLong();
            }
        }
        return -1;  // Retourne -1 si la largeur n'est pas trouvée
    }

    // Fonction pour récupérer la hauteur de l'image
    int getImageHeight() {
        for (auto& entry : metaData) {
            if (entry.key() == "Exif.Image.ImageLength") {
                return entry.toLong();
            }
        }
        return -1;  // Retourne -1 si la hauteur n'est pas trouvée
    }
    // Fonction pour récupérer l'orientation de l'image (rotation)
    int getImageOrientation() {
        for (auto& entry : metaData) {
            if (entry.key() == "Exif.Image.Orientation") {
                return entry.toLong();
            }
        }
        return -1;  // Retourne -1 si l'orientation n'est pas trouvée
    }


    Date getImageDate() {
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
    bool modifyExifValue(const std::string& key, const std::string& newValue) {
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
};



class ImageData
{
public:
    std::string imagePath;
    Folders folders;
    MetaData metaData;

    ImageData(std::string a, const Folders& c) : imagePath(a), folders(c) {}

    void print() const {
        std::cout << "Image : " << imagePath
            << " fichiers : " << folders.getFolderList() << std::endl;
    }

    std::string get() const {
        return "Image : " + imagePath + " fichiers : " + folders.getFolderList() + "\n";
    }
    MetaData* getMetaData() {
        // Débogage pour afficher la taille des métadonnées
        return &metaData;
    }

    std::vector<std::string> getFolders() {
        return folders.getFolders();
    }

    void addFolder(const std::string& toAddFolder) {
        folders.addFolder(toAddFolder);
    }

    void addFolders(const std::vector<std::string>& toAddFolders) {
        for (const auto& folder : toAddFolders) {
            folders.addFolder(folder);
        }
    }



    std::string getImageName() const {
        std::filesystem::path filePath(imagePath);
        return filePath.filename().string();
    }

    bool operator==(const ImageData& other) const {
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

    std::string getImagePath() {
        return imagePath;
    }



    void setMetaData(const Exiv2::ExifData& toAddMetaData) {
        metaData.set(toAddMetaData);
        // Sauvegarde après modification des métadonnées
        saveMetaData();
    }

    void loadMetaData() {
        metaData.load(imagePath);
    }

    void saveMetaData() {
        metaData.save(imagePath);
    }

    int getImageWidth(){
        return metaData.getImageWidth();
    }
    int getImageHeight(){
        return metaData.getImageHeight();
    }
    int getImageOrientation(){
        return metaData.getImageOrientation();
    }
    Date getImageDate(){
        return metaData.getImageDate();
    }
    void turnImage(int rotation){
        // 1 : normal, 3 : 90° left, 6 : 180°, 9: 90 right
        metaData.modifyExifValue("Exif.Image.Orientation", std::to_string(rotation));
    }


};

class ImagesData
{
public:
    std::vector<ImageData> imagesData;

    ImagesData(const std::vector<ImageData> a) : imagesData(a) {}

    void print() const {
        std::cout << "ImagesData : \n";
        for (const ImageData& valeur : imagesData) {
            std::cout << valeur.get();
        }
        std::cout << std::endl;
    }

    void addImage(ImageData& imageD) {
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

    void removeImage(const ImageData& image) {
        auto it = std::find(imagesData.begin(), imagesData.end(), image);

        if (it != imagesData.end()) {
            imagesData.erase(it);
        }
    }

    ImageData* getImageData(int id) {
        if (id < 0 || id >= imagesData.size()) {
            throw std::out_of_range("Index hors limites");
        }
        return &imagesData.at(id);
        // renvoie un pointeur ce qui permet une modification automatique dans ImagesData
    }
    std::vector<ImageData>  get() {
        return imagesData;
    }
};



bool estImage(const std::string& cheminFichier);
void listerContenu(const std::string initialPath, const std::string chemin, ImagesData& imagesData);
void loadImagesMetaData(ImagesData& imagesData);

int main() {
    std::string path = "/home/eugene/Documents";
    ImagesData imagesData({});
    using ImagesData = std::vector<ImageData*>;

    listerContenu(path, path, imagesData);
    imagesData.print();
    loadImagesMetaData(imagesData);

    // TODO modifyExifValue not working
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

template <typename T>
void addUnique(std::vector<T>& vec, const T& element) {
    // Cherche l'élément dans le vecteur
    auto it = std::find(vec.begin(), vec.end(), element);

    // Si l'élément est trouvé, le retire
    if (it != vec.end()) {
        return;
    }
    // Ajoute l'élément à la fin du vecteur
    vec.push_back(element);
}





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
        // std::cout << " | Valeur : " << imagesData.getImageData(i)->getImageOrientation() << std::endl;
        // displayExifData(imagesData.getImageData(i)->getMetaData().get());


        // imageData.loadMetaData();
        // }
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
        std::cout << "Aucune métadonnée EXIF disponible." << std::endl;
        return;
    }

    // Parcourir toutes les entrées EXIF
    for (const auto& exifItem : exifData) {
        if (exifItem.key().substr(0, 10) == "Exif.Image") {
            std::cout << exifItem.key() << " : " << exifItem.value() << std::endl;
        }
    }
}