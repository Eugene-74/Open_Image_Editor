#include "main.h"



namespace fs = std::filesystem;

// impossible de faire l'orientation effective des PNG


int main(int argc, char* argv[]) {
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    std::string path = "/home/eugene/Documents/photo trié";
    Data data;
    ImagesData imagesData(std::vector<ImageData>{});
    ImagesData deletedImagesData(std::vector<ImageData>{});
    using ImagesData = std::vector<ImageData*>;

    data.imagesData = imagesData;
    data.deletedImagesData = deletedImagesData;

    // TODO mettre const dans un autre fichier et ajouter bouton save + pop up quand on ferme
    // charger de save.dat
    data.imagesData = loadImagesData(IMAGESDATA_SAVE_DAT_PATH);

    startLoadingImagesFromFolder(path, data.imagesData);

    data.imagesData.setImageNumber(0);

    QApplication app(argc, argv);

    ImageEditor window(data);

    window.showMaximized();

    return app.exec();
}

// Charges dans un imagesData toutes les données des images dans un dossier et ses sous dossier
void startLoadingImagesFromFolder(const std::string imagePaths, ImagesData& imagesData) {
    int nbrImage = 0;
    countImagesFromFolder(imagePaths, nbrImage);
    std::cerr << "nombre d'image à charger : " << nbrImage << std::endl;

    loadImagesFromFolder(imagePaths, imagePaths, imagesData, nbrImage);

    loadImagesMetaData(imagesData);

}

// Conte toutes les images dans un dossier et ses sous dossier
void countImagesFromFolder(const std::string path, int& nbrImage) {

    for (const auto& entry : fs::directory_iterator(path)) {
        if (fs::is_regular_file(entry.status())) {

            if (isImage(entry.path())) {
                nbrImage += 1;

            }
        }
        else if (fs::is_directory(entry.status())) {
            countImagesFromFolder(entry.path(), nbrImage);
        }
    }
    // return nbrImage;
}

// Charges concrètement dans un imagesData toutes les données des images dans un dossier et ses sous dossier
void loadImagesFromFolder(const std::string initialPath, const std::string path, ImagesData& imagesData, int& nbrImage) {
    for (const auto& entry : fs::directory_iterator(path)) {
        if (fs::is_regular_file(entry.status())) {
            if (isImage(entry.path())) {

                // On ne garde que la partie après "Documents"
                fs::path relativePath = fs::relative(entry.path(), fs::path(initialPath).parent_path());

                std::vector<std::string> fichiers;

                fichiers = { relativePath };

                Folders folders(fichiers);

                ImageData imageD(entry.path(), folders);
                imagesData.addImage(imageD);
                nbrImage -= 1;
                std::cerr << "Viens de charger : " << entry.path() << std::endl;
                std::cerr << "Image restante : " << nbrImage << std::endl;


            }
        }
        else if (fs::is_directory(entry.status())) {
            loadImagesFromFolder(initialPath, entry.path(), imagesData, nbrImage);
        }
    }
    // return imagesData;
}

// Revoie True si l'extension du fichier correspond à l'extension d'une image
bool isImage(const std::string& path) {
    // Liste des extensions d'images courantes
    std::vector<std::string> extensionsImages = { ".jpg", ".jpeg", ".png", ".gif", ".bmp", ".tiff", ".webp",".webp" };

    // Récupération de l'extension du fichier
    std::string extension = fs::path(path).extension().string();

    // Convertir l'extension en minuscule pour éviter les problèmes de casse
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

    // Vérifier si l'extension est dans la liste des extensions d'images
    return std::find(extensionsImages.begin(), extensionsImages.end(), extension) != extensionsImages.end();
}

bool isTurnable(const std::string& path) {
    // Liste des extensions d'images courantes
    std::vector<std::string> extensionsImages = { ".jpg", ".jpeg" };

    // Récupération de l'extension du fichier
    std::string extension = fs::path(path).extension().string();

    // Convertir l'extension en minuscule pour éviter les problèmes de casse
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

    // Vérifier si l'extension est dans la liste des extensions d'images
    return std::find(extensionsImages.begin(), extensionsImages.end(), extension) != extensionsImages.end();
}

// Charger les meta donnée contenue dans les images
void loadImagesMetaData(ImagesData& imagesData) {
    for (int i = 0; i < imagesData.get().size(); ++i) {
        imagesData.getImageData(i)->loadData();
    }
}

