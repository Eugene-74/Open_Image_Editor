#include "LoadImage.h"


// Fonction pour ajouter des fichiers sélectionnés à la liste des dossiers
ImagesData addSelectedFilesToFolders(Data* data, QWidget* parent) {
    // Data data;
    ImagesData imagesData = ImagesData(std::vector<ImageData>{});;
    fileSelector fileSelector;

    QStringList selectedFiles;
    // if (showQuestionMessage(parent, "Do you want to add images ? (yes for files, no for folders)")) {
    //     // selectedFiles = fileSelector.openFileDialog();
    //     showErrorMessage(parent, "Not implemented yet");

    // }
    // else {
    selectedFiles = fileSelector.openDirectoryDialog();

    // std::vector<std::string> folders;
    for (const QString& fileName : selectedFiles) {
        startLoadingImagesFromFolder(data, fileName.toStdString(), &imagesData);
    }
    // }


    return imagesData;


}

std::string getDirectoryFromUser(QWidget* parent) {
    QFileDialog dialog(parent);
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setOption(QFileDialog::ShowDirsOnly, true);

    if (dialog.exec()) {
        QStringList selectedDirectories = dialog.selectedFiles();
        if (!selectedDirectories.isEmpty()) {
            return selectedDirectories.first().toStdString();
        }
    }
    return "";
}