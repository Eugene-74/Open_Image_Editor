#include "loadImage.h"


// #include "loadImage.h"
#include "../../functions/fileSelector/fileSelector.h"
#include <QStringList>
#include <string>
#include <vector>

// Fonction pour ajouter des fichiers sélectionnés à la liste des dossiers
void addSelectedFilesToFolders() {
    fileSelector fileSelector;
    QStringList selectedFiles = fileSelector.openFileDialog();

    std::vector<std::string> folders;
    for (const QString& fileName : selectedFiles) {
        folders.push_back(fileName.toStdString());
    }

    // Ajoutez ici le code pour traiter les dossiers (par exemple, les ajouter à une liste existante)
}