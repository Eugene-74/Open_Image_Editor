#include "Main.h"



namespace fs = std::filesystem;


int main(int argc, char* argv[]) {
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QApplication app(argc, argv);


    InitialWindow window;

    window.showMaximized();

    return app.exec();
}

// Charges dans un imagesData toutes les données des images dans un dossier et ses sous dossier
void startLoadingImagesFromFolder(Data* data, const std::string imagePaths, ImagesData* imagesData) {
    int nbrImage = 0;

    addFilesToTree(&data->rootFolders, imagePaths);

    countImagesFromFolder(imagePaths, nbrImage);

    data->rootFolders.print();

    std::cerr << "nombre d'image à charger : " << nbrImage << std::endl;

    loadImagesFromFolder(imagePaths, imagePaths, imagesData, nbrImage);

    loadImagesMetaData(imagesData);



}

// Conte toutes les images dans un dossier et ses sous dossier
void countImagesFromFolder(const std::string path, int& nbrImage) {

    int i = 0;
    for (const auto& entry : fs::directory_iterator(path)) {
        if (fs::is_regular_file(entry.status())) {

            if (isImage(entry.path())) {
                nbrImage += 1;

            }
        }
        else if (fs::is_directory(entry.status())) {

            countImagesFromFolder(entry.path(), nbrImage);

            i += 1;
        }
    }
}

// Charges concrètement dans un imagesData toutes les données des images dans un dossier et ses sous dossier
void loadImagesFromFolder(const std::string initialPath, const std::string path, ImagesData* imagesData, int& nbrImage) {
    for (const auto& entry : fs::directory_iterator(path)) {
        if (fs::is_regular_file(entry.status())) {
            if (isImage(entry.path())) {

                fs::path relativePath = fs::relative(entry.path(), fs::path(initialPath).parent_path());

                Folders folders;
                ImageData* imageData = imagesData->getImageData(entry.path());
                if (imageData != nullptr){
                    folders = imageData->folders;
                }
                else{
                    folders = Folders(relativePath.parent_path().string());

                }
                folders.files.push_back(relativePath.parent_path().filename().string());

                ImageData imageD(entry.path(), folders);
                imagesData->addImage(imageD);

                nbrImage -= 1;
                std::cerr << "Image restante : " << nbrImage << std::endl;
            }
        }
        else if (fs::is_directory(entry.status())) {
            loadImagesFromFolder(initialPath, entry.path(), imagesData, nbrImage);
        }
    }
}



void loadImagesMetaData(ImagesData* imagesData) {
    for (int i = 0; i < imagesData->get()->size(); ++i) {
        imagesData->getImageData(i)->loadData();
    }
}

