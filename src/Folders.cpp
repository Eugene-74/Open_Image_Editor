#include "Folders.hpp"

namespace fs = std::filesystem;

Folders& Folders::operator=(const Folders& other) {
    if (this != &other) {
        name = other.name;
        files = other.files;
        folders = other.folders;
    }
    return *this;
}

void Folders::save(std::ofstream& out) const {
    size_t folderNameSize = name.size();
    out.write(reinterpret_cast<const char*>(&folderNameSize), sizeof(folderNameSize));
    out.write(name.c_str(), folderNameSize);

    size_t filesCount = files.size();
    out.write(reinterpret_cast<const char*>(&filesCount), sizeof(filesCount));
    for (const auto& file : files) {
        size_t fileSize = file.size();
        out.write(reinterpret_cast<const char*>(&fileSize), sizeof(fileSize));
        out.write(file.c_str(), fileSize);
    }

    size_t foldersCount = folders.size();
    out.write(reinterpret_cast<const char*>(&foldersCount), sizeof(foldersCount));
    for (const auto& folder : folders) {
        folder.save(out);
    }
}

void Folders::load(std::ifstream& in) {
    size_t folderNameSize;
    in.read(reinterpret_cast<char*>(&folderNameSize), sizeof(folderNameSize));
    name.resize(folderNameSize);
    in.read(&name[0], folderNameSize);

    size_t filesCount;
    in.read(reinterpret_cast<char*>(&filesCount), sizeof(filesCount));
    files.resize(filesCount);
    for (auto& file : files) {
        size_t fileSize;
        in.read(reinterpret_cast<char*>(&fileSize), sizeof(fileSize));
        file.resize(fileSize);
        in.read(&file[0], fileSize);
    }

    size_t foldersCount;
    in.read(reinterpret_cast<char*>(&foldersCount), sizeof(foldersCount));
    folders.resize(foldersCount);
    for (auto& folder : folders) {
        folder.load(in);
        folder.parent = this;
    }
}

void Folders::addFolder(std::string name) {
    if (std::find_if(folders.begin(), folders.end(), [&name](const Folders& folder) { return folder.name == name; }) != folders.end()) {
        return;
    }
    Folders child = Folders(name);
    child.parent = this;
    folders.push_back(child);
}

void Folders::addFile(std::string name) {
    files.push_back(name);
}

void Folders::print() const {
    qDebug() << "Folder : {" << name << ",";

    for (const auto& folder : folders) {
        qDebug() << "[" << folder.name << "]";
    }
    qDebug() << "}";
    for (const auto& folder : folders) {
        folder.print();
    }
}

std::string* Folders::getName() {
    return &name;
}

std::vector<Folders>* Folders::getFolders() {
    return &folders;
}

std::vector<std::string>* Folders::getFiles() {
    return &files;
}

Folders* Folders::getFolder(int index) {
    return &folders.at(index);
}

std::string* Folders::getFile(int index) {
    return &files.at(index);
}

Folders* Folders::getParent() {
    return parent;
}

// Verifie si un dossier existe dans un Folders
bool getIfExist(Folders* currentFolder, const std::string& path) {
    auto it = std::find_if(currentFolder->folders.begin(), currentFolder->folders.end(), [&path](const Folders& folder) { return folder.name == path; });

    if (it != currentFolder->folders.end()) {
        return true;
    }
    return false;
}

// Verifie si un dossier contient une image dans un de ses sous dossier
bool containImage(const std::string& path) {
    bool valide = false;
    for (const auto& entry : fs::recursive_directory_iterator(path)) {
        if (fs::is_regular_file(entry.status()) && isImage(entry.path().filename().string())) {
            return true;
        }
    }
    return false;
}
