#include "Folders.hpp"

#include <QDebug>
#include <filesystem>

#include "Verification.hpp"
namespace fs = std::filesystem;

/**
 * @brief Assignment operator for Folders class
 * @param other The other Folders object to copy from
 * @return A reference to the current object
 */
Folders& Folders::operator=(const Folders& other) {
    if (this != &other) {
        name = other.name;
        files = other.files;
        folders = other.folders;
    }
    return *this;
}

/**
 * @brief Equality operator for Folders class
 * @param other The other Folders object to compare with
 * @return True if the objects are equal, false otherwise
 * @details This function compares the name, files, and folders of the two objects
 */
bool Folders::operator==(const Folders& other) const {
    return this->name == other.name &&
           this->files == other.files &&
           this->folders == other.folders;
}

/**
 * @brief Save the folder to a file
 * @param out The output file stream
 */
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

/**
 * @brief Load the folder from a file
 * @param in The input file stream
 */
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

/**
 * @brief Add a folder to the current folder if it doesn't already exist
 * @param name the name of the folder to add
 * @return the index of the added folder
 */
int Folders::addFolder(std::string name) {
    if (std::find_if(folders.begin(), folders.end(), [&name](const Folders& folder) { return folder.name == name; }) != folders.end()) {
        return std::distance(folders.begin(), std::find_if(folders.begin(), folders.end(), [&name](const Folders& folder) { return folder.name == name; }));
    }
    Folders child = Folders(name);
    child.parent = this;
    folders.push_back(child);
    return folders.size() - 1;
}

void Folders::addFile(std::string name) {
    files.push_back(name);
}

void Folders::print() const {
    qInfo() << "Folder : {" << name << ",";

    for (const auto& folder : folders) {
        qInfo() << "[" << folder.name << "]";
    }
    qInfo() << "}";
    for (const auto& folder : folders) {
        folder.print();
    }
}

/**
 * @brief Give you the name of the Folder
 * @return Folder name
 */
std::string Folders::getName() const {
    return name;
}

/**
 * @brief Give you the folders of the current folder
 * @return A pointer to the vector of folders
 */
std::vector<Folders>* Folders::getFolders() {
    return &folders;
}

/**
 * @brief Give you the folders of the current folder (const version)
 * @return A vector of folders
 */
std::vector<Folders> Folders::getFoldersConst() const {
    return folders;
}

/**
 * @brief Give you the files of the current folder
 * @return A vector of files
 */
std::vector<std::string> Folders::getFiles() {
    return files;
}

/**
 * @brief Give you the files of the current folder (ptr version)
 * @return A pointer to the vector of files
 */
std::vector<std::string>* Folders::getFilesPtr() {
    return &files;
}

/**
 * @brief Give you the files of the current folder (const version)
 * @return A vector of files
 */
std::vector<std::string> Folders::getFilesConst() const {
    return files;
}

/**
 * @brief Give you the folder at the index
 * @param index The index of the folder to get
 * @return A pointer to the folder at the index
 * @details This function checks if the index is valid and returns nullptr if it is not
 * @details This function returns a pointer to the folder, not a copy of it
 */
Folders* Folders::getFolder(int index) {
    if (index < 0 || index >= folders.size()) {
        qCritical() << "Index out of range in getFolder()";
        return nullptr;
    }
    return &folders.at(index);
}

/**
 * @brief Give you the file at the index
 * @param index The index of the file to get
 * @return A pointer to the file at the index
 * @details This function checks if the index is valid and returns nullptr if it is not
 * @details This function returns a pointer to the file string, not a copy of it
 */
std::string* Folders::getFile(int index) {
    if (index < 0 || index >= files.size()) {
        qCritical() << "Index out of range in getFile()";
        return nullptr;
    }
    return &files.at(index);
}

/**
 * @brief Give you the parent of the current folder
 * @return A pointer to the parent folder
 */
Folders* Folders::getParent() {
    return parent;
}

/**
 * @brief Set the parent of the current folder
 * @param parent The parent folder to set
 */
void Folders::setParent(Folders* parent) {
    this->parent = parent;
}

/**
 * @brief Clear the current folder
 * @details This function clears the name, files, and folders of the current folder
 */
void Folders::clear() {
    name.clear();
    files.clear();
    folders.clear();
}

/**
 * @brief Clear the current folder and all its subfolders recursively
 * @details This function clears the name, files, and folders of the current folder and all its subfolders
 */
void Folders::clearRecursively() {
    for (auto& folder : folders) {
        folder.clearRecursively();
    }
    clear();
}

/**
 * @brief Check if a folder with the given name exists in the current folder
 * @param currentFolder The current folder to check in
 * @param folderName The name of the folder to check for
 * @return True if the folder exists, false otherwise
 */
bool getIfExist(Folders* currentFolder, const std::string& folderName) {
    auto folder = std::find_if(currentFolder->getFolders()->begin(), currentFolder->getFolders()->end(), [&folderName](const Folders& folder) { return folder.getName() == folderName; });

    return folder != currentFolder->getFolders()->end();
}

/**
 * @brief Check if a path contains media files (images or videos)
 * @param path The path to check
 * @return True if the path contains media files, false otherwise
 */
bool containMedia(const std::string& path) {
    bool valide = false;
    for (const auto& entry : fs::recursive_directory_iterator(path)) {
        if (fs::is_regular_file(entry.status()) && (isImage(entry.path().filename().string()) || isVideo(entry.path().filename().string()))) {
            return true;
        }
    }
    return false;
}
