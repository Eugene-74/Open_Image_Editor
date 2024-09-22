#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;

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
    void addFolder(std::string folder) {
        folders.push_back(folder);
    }
    std::vector<std::string>  getFolders(){
        return folders;
    }

};

class ImageData
{
public:
    std::string imagePath;
    long date;
    int id;
    Folders folders;

    ImageData(std::string a, long d, int b, const Folders& c) : imagePath(a), date(d), id(b), folders(c) {}

    void print() const {
        std::cout << "Image : " << imagePath
            << " date : " << date
            << " numéro : " << id
            << " fichiers : " << folders.getFolderList();

        std::cout << std::endl;
    }
    std::string get() const {
        return "Image : " + imagePath +
            " date : " + std::to_string(date) +
            " numéro : " + std::to_string(id) +
            " fichiers : " + folders.getFolderList();
    }
    void addFolder(std::string toAddFolder) {
        folders.addFolder(toAddFolder);
    }
    void addFolders(std::vector<std::string>  toAddFolders) {
        for (size_t i = 0; i < toAddFolders.size(); ++i) {
            folders.addFolder(toAddFolders[i]);
        }
    }
    std::vector<std::string>  getFolders() {
        return folders.getFolders();
    }
    std::string getImageName() {
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
};


class ImagesData
{
public:

    std::vector<ImageData> imagesData;

    ImagesData(std::vector<ImageData>  a) : imagesData(a) {}

    void print() const {
        std::cout << "Images : ";

        for (const ImageData& valeur : imagesData) {
            std::cout << valeur.get() << " ";
        }

        std::cout << std::endl;
    }
    void addImage(ImageData& imageD){
        // chercher le bon element
        // Utiliser std::find_if avec une lambda pour comparer les noms d'image
        auto it = std::find_if(imagesData.begin(), imagesData.end(),
            [&imageD](ImageData& imgD) {
                return imgD.getImageName() == imageD.getImageName();
            });

        // Si l'élément est trouvé, le supprimer
        if (it != imagesData.end()) {
            ImageData lastImageD = *it;
            imagesData.erase(it);
            imageD.addFolders(lastImageD.getFolders());
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
};

bool estImage(const std::string& cheminFichier);
void listerContenu(const std::string& chemin);

int main() {
    // std::vector<std::string> fichiers = { "dossier1", "dossier2", "dossier3" };
    // Folders folders(fichiers);

    // ImageData image("/home/eugene/Documents/image.png", 128489237832, 0, folders);
    // image.print();

    std::string path = "/home/eugene/Documents";
    listerContenu(path);

    return 0;
}

void listerContenu(const std::string& chemin) {
    ImagesData imagesData({});
    // TODO definir image data plus tot 
    for (const auto& entry : fs::directory_iterator(chemin)) {
        if (fs::is_regular_file(entry.status())) {
            if (estImage(entry.path())) {
                std::cout << "Image: " << entry.path().filename() << std::endl;

                std::vector<std::string> fichiers = { entry.path().parent_path().filename() };
                Folders folders(fichiers);
                ImageData image(entry.path(), 128489237832, 0, folders);
                image.print();
                imagesData.addImage(image);
            }
            // else{
            //     std::cout << "Fichier: " << entry.path().filename() << std::endl;
            // }
        }
        else if (fs::is_directory(entry.status())) {
            listerContenu(entry.path());
            // std::cout << "Dossier: " << entry.path().filename() << std::endl;
        }
    }
    imagesData.print();
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