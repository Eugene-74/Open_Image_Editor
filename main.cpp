#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <algorithm>

#include <exiv2/exiv2.hpp>

namespace fs = std::filesystem;


template <typename T>
void addUnique(std::vector<T>& vec, const T& element);


class Date
{
public:
    int day;
    int month;
    int year;

    Date(int d, int m, int y) : day(d), month(m), year(y) {
        if (day < 0 or day > 31) {
            std::cout << "jour invalide" << std::endl;
            return;
        }
        else if (month < 0 or month > 12) {
            std::cout << "mois invalide" << std::endl;
            return;
        }
        else if (year < 0 or year > 9999) {
            std::cout << "année invalide" << std::endl;
            return;
        }
    }

    void print() const {
        std::cout << "Jour : " << day << "Mois : " << month << "Année : " << year << std::endl;
    }
    std::string getStringJJ_MM_AAAA() const {
        // std::cout << "Jour : " << day << "Mois : " << month << "Année : " << year << std::endl;
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

std::map<std::string, std::string> getImageMetadata(const std::string& imagePath);

class MetaData
{
public:
    std::map<std::string, std::string> metaData;


    std::map<std::string, std::string> get(){
        return metaData;
    }

    void set(std::map<std::string, std::string> toAddMetaData){
        metaData = toAddMetaData;
    }
    void load(std::string imagePath){
        std::map<std::string, std::string> toAddMetaData = getImageMetadata(imagePath);
        if (!toAddMetaData.empty()){
            metaData = toAddMetaData;
        }
    }

    void setData(std::string key, std::string value){
        metaData[key] = value;
    }


};


class ImageData
{
public:
    std::string imagePath;
    long date;
    Folders folders;
    MetaData metaData;

    ImageData(std::string a, long d, const Folders& c) : imagePath(a), date(d), folders(c) {}

    void print() const {
        std::cout << "Image : " << imagePath
            << " date : " << date
            << " fichiers : " << folders.getFolderList();

        std::cout << std::endl;
    }
    std::string get() const {
        return "Image : " + imagePath +
            " date : " + std::to_string(date) +
            " fichiers : " + folders.getFolderList() + "\n";
    }
    void addFolder(std::string toAddFolder) {
        folders.addFolder(toAddFolder);
    }
    void addFolders(const std::vector<std::string>  toAddFolders)  {
        for (size_t i = 0; i < toAddFolders.size(); ++i) {
            folders.addFolder(toAddFolders[i]);
        }
    }
    std::vector<std::string>  getFolders() {
        return folders.getFolders();
    }
    std::string getImageName() const {
        std::filesystem::path filePath(imagePath);
        std::string imageName = filePath.filename().string();
        return imageName;
    }
    // Définition de l'opérateur ==
    bool operator==(const ImageData& other) const {
        std::filesystem::path filePath(imagePath);
        std::string imageName = filePath.filename().string();

        std::filesystem::path filePathOther(other.imagePath);
        std::string imageNameOther = filePathOther.filename().string();

        return imageName == imageNameOther;
    }
    std::string getImagePath(){
        return imagePath;
    }
    std::map<std::string, std::string> getMetaData(){
        return metaData.get();
    }
    void setMetaData(std::map<std::string, std::string> toAddMetaData){
        metaData.set(toAddMetaData);
    }

    void loadMetaData(){
        metaData.load(imagePath);

    }

};


class ImagesData
{
public:

    std::vector<ImageData> imagesData;

    ImagesData(std::vector<ImageData>  a) : imagesData(a) {}

    void print() const {
        std::cout << "ImagesData : \n";

        for (const ImageData& valeur : imagesData) {
            std::cout << valeur.get();
        }

        std::cout << std::endl;
    }
    void addImage(ImageData& imageD){

        // Utiliser std::find_if avec une lambda pour comparer les noms d'image

        auto it = std::find_if(imagesData.begin(), imagesData.end(),
            [&imageD](const ImageData& imgD) {
                return imgD == imageD;
            });

        // Si l'élément est trouvé, le supprimer
        if (it != imagesData.end()) {
            ImageData lastImageD = *it;
            imagesData.erase(it);
            imageD.addFolders(lastImageD.getFolders());

            imagesData.push_back(imageD);
        }
        else {
            // si il n'existe pas encore
            imagesData.push_back(imageD);
        }
    }

    void removeImage(const ImageData& image){
        imagesData.push_back(image);
        // chercher le bon element
        auto it = std::find(imagesData.begin(), imagesData.end(), image);

        // Si l'élément est trouvé, le supprimer
        if (it != imagesData.end()) {
            imagesData.erase(it);
        }
    }
    ImageData getImageData(int id){
        return imagesData[id];
    }
};



bool estImage(const std::string& cheminFichier);
void listerContenu(const std::string initialPath, const std::string chemin, ImagesData& imagesData);


int main() {
    std::string path = "/home/eugene/Documents";
    ImagesData imagesData({});
    listerContenu(path, path, imagesData);
    imagesData.print();
    imagesData.getImageData(0).loadMetaData();
    // getImageMetadata(imagesData.getImageData(0).getImagePath());
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

                std::string text = entry.path();

                std::transform(text.begin(), text.end(), text.begin(),
                    [](unsigned char c) { return std::tolower(c); });

                ImageData imageD(text, 128489237832, folders);
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

// TODO l'activer
// Fonction qui renvoie les métadonnées EXIF sous forme de map
std::map<std::string, std::string> getImageMetadata(const std::string& imagePath) {
    std::map<std::string, std::string> metadataMap;

    try {
        // Ouvrir l'image avec Exiv2
        Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(imagePath);
        image->readMetadata();

        // Récupérer les métadonnées EXIF
        Exiv2::ExifData& exifData = image->exifData();
        if (exifData.empty()) {
            std::cerr << "No EXIF data found in the file: " << imagePath << std::endl;
        }
        else {
            for (Exiv2::ExifData::const_iterator md = exifData.begin(); md != exifData.end(); ++md) {
                metadataMap[md->key()] = md->value().toString();
            }
        }
    }
    catch (Exiv2::Error& e) {
        std::cerr << "Error reading metadata: " << e.what() << std::endl;
    }

    return metadataMap;
}