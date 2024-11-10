#include "LoadImage.h"


// Fonction pour ajouter des fichiers sélectionnés à la liste des dossiers
ImagesData* addSelectedFilesToFolders(QWidget* parent) {
    // Data data;
    ImagesData* imagesData = new ImagesData();
    fileSelector fileSelector;

    QStringList selectedFiles;
    if (showQuestionMessage(parent, "Do you want to add images ? (yes for files, no for folders)")) {
        // selectedFiles = fileSelector.openFileDialog();
        showErrorMessage(parent, "Not implemented yet");

    }
    else {
        selectedFiles = fileSelector.openDirectoryDialog();

        // std::vector<std::string> folders;
        for (const QString& fileName : selectedFiles) {
            startLoadingImagesFromFolder(fileName.toStdString(), imagesData);
        }
    }


    return imagesData;


}