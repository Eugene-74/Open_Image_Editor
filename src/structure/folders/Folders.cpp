#include "Folders.h"

Folders& Folders::operator=(const Folders& other) {
    if (this != &other) {
        folders = other.folders; // Utiliser l'opérateur d'affectation de std::vector
    }
    return *this;
}

void Folders::print() const {
    for (const std::string& valeur : folders) {
        std::cerr << valeur << " ";
    }
    std::cerr << std::endl;
}

void Folders::load(std::ifstream& in) {
    folders.clear();
    size_t folderCount;
    in.read(reinterpret_cast<char*>(&folderCount), sizeof(folderCount));
    for (size_t i = 0; i < folderCount; ++i) {
        size_t length;
        in.read(reinterpret_cast<char*>(&length), sizeof(length));
        std::string folder(length, '\0');
        in.read(&folder[0], length);
        folders.push_back(folder);
    }
}

void Folders::save(std::ofstream& out) const {
    size_t folderCount = folders.size();
    out.write(reinterpret_cast<const char*>(&folderCount), sizeof(folderCount));
    for (const auto& folder : folders) {
        size_t length = folder.size();
        out.write(reinterpret_cast<const char*>(&length), sizeof(length));
        out.write(folder.c_str(), length);
    }
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

std::vector<std::string>  Folders::getFolders() {
    return folders;
}
