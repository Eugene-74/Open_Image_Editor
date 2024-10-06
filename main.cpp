#include "main.h"


#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <algorithm>
#include <exiv2/exiv2.hpp>

#include "structure/folders/folders.h"

#include "functions/vector/vector.h"
#include "functions/thumbnail/thumbnail.h"
#include "display/imageEditor/imageEditor.h"

#include <QApplication>
#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include <QPixmap>
#include <QVBoxLayout>
#include <QWidget>
#include <QFileDialog>
#include <chrono>


namespace fs = std::filesystem;

// impossible de faire l'orientation effective des PNG




int main(int argc, char* argv[]) {
    std::string path = "/home/eugene/Documents/photo trié";
    ImagesData imagesData(std::vector<ImageData>{});
    imagesData = loadImagesData();
    using ImagesData = std::vector<ImageData*>;


    startLoadingImagesFromFolder(path, imagesData);
    loadImagesMetaData(imagesData);


    // imagesData.getImageData(2)->setOrCreateExifData();

    // std::cerr << imagesData.getImageData(2)->getImageOrientation() << std::endl;
    // std::cerr << imagesData.getImageData(2)->getImageName() << std::endl;



    // imagesData.getImageData(4)->getMetaData()->modifyExifValue("Exif.Image.Orientation", std::to_string(3));
    // imagesData.getImageData(4)->getMetaData()->modifyExifValue("Exif.Thumbnail.Orientation", std::to_string(3));



    loadImagesMetaData(imagesData);
    // int nbr = 1;


    // imagesData.getImageData(nbr)->getMetaData()->displayMetaData();

    // imagesData.getImageData(nbr)->turnImage(8);

    // imagesData.getImageData(nbr)->getMetaData()->displayMetaData();


    // // imagesData.getImageData(2)->getMetaData()->get()["Exif.Image.Orientation"] = 8;

    // imagesData.getImageData(nbr)->saveMetaData();



    // imagesData.sa
    // imagesData.saveImagesData();

    // TODO create a function
    // Save metadata in the images itself
    for (int i = 0;i < imagesData.get().size();i++){
        imagesData.getImageData(i)->saveMetaData();
        // std::cerr << imagesData.getImageData(i)->getImageName() << std::endl;
    }


    QApplication app(argc, argv);

    ImageEditor window(imagesData);

    window.showMaximized();

    // window.show();

    imagesData.setImageNumber(0);

    std::string cheminImage = imagesData.getImageData(0)->imagePath;


    std::vector<std::string> imagePaths;
    imagePaths.push_back(cheminImage);

    // auto start = std::chrono::high_resolution_clock::now();
    createThumbnails(imagePaths, THUMBNAIL_PATH);
    // auto end = std::chrono::high_resolution_clock::now();
    // std::chrono::duration<double, std::milli> duration = end - start;
    // std::cout << "Temps d'exécution: " << duration.count() << " ms" << std::endl;

    window.setImage(*imagesData.getImageData(0));
    return app.exec();
}

void startLoadingImagesFromFolder(const std::string imagePaths, ImagesData& imagesData){
    int nbrImage = 0;
    countImagesFromFolder(imagePaths, nbrImage);
    std::cerr << "nombre d'image à charger : " << nbrImage << std::endl;

    loadImagesFromFolder(imagePaths, imagePaths, imagesData, nbrImage);

    loadImagesMetaData(imagesData);
    imagesData.print();
    std::cerr << imagesData.getImageData(3)->getImageOrientation() << std::endl;

}

void countImagesFromFolder(const std::string path, int& nbrImage){
    // int nbrImage;
    // std::cerr << "path : " << path << std::endl;

    for (const auto& entry : fs::directory_iterator(path)) {
        if (fs::is_regular_file(entry.status())) {
            // std::cerr << "nombre d'image à charger :: " << nbrImage << std::endl;

            if (estImage(entry.path())) {
                nbrImage += 1;
                // std::cerr << "nombre d'image à charger ::: " << nbrImage << std::endl;

            }
        }
        else if (fs::is_directory(entry.status())) {
            countImagesFromFolder(entry.path(), nbrImage);
        }
    }
    // return nbrImage;
}

void loadImagesFromFolder(const std::string initialPath, const std::string path, ImagesData& imagesData, int& nbrImage) {



    for (const auto& entry : fs::directory_iterator(path)) {
        if (fs::is_regular_file(entry.status())) {
            if (estImage(entry.path())) {

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

// 









void loadImagesMetaData(ImagesData& imagesData) {
    for (int i = 0; i < imagesData.get().size(); ++i) {
        imagesData.getImageData(i)->loadData();
    }
}

