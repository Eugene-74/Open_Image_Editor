#include <iostream>
#include "folders.h"

#include "../functions/vector/vector.h"

void Folders::print() const {
    for (const std::string& valeur : folders) {
        std::cout << valeur << " ";
    }
    std::cout << std::endl;
}

std::string Folders::getFolderList() const {
    std::string folderList;
    for (const auto& folder : folders) {
        if (!folderList.empty()) {
            folderList += ", "; // Ajouter une virgule entre les dossiers
        }
        folderList += folder; // Ajouter le dossier
    }
    return folderList; // Retourner la chaîne complète
}

void Folders::addFolder(const std::string& folder) {
    addUnique(folders, folder);

}

std::vector<std::string>  Folders::getFolders(){
    return folders;
}
