#include "Folders.h"

namespace fs = std::filesystem;

Folders& Folders::operator=(const Folders& other)
{
    if (this != &other){
        name = other.name; // Utiliser l'opérateur d'affectation de std::vector
        files = other.files;
        folders = other.folders;
    }
    return *this;
}

void Folders::save(std::ofstream& out) const
{
    // Save folder name
    size_t folderNameSize = name.size();
    out.write(reinterpret_cast<const char*>(&folderNameSize), sizeof(folderNameSize));
    out.write(name.c_str(), folderNameSize);

    // Save files
    size_t filesCount = files.size();
    out.write(reinterpret_cast<const char*>(&filesCount), sizeof(filesCount));
    for (const auto& file : files)
    {
        size_t fileSize = file.size();
        out.write(reinterpret_cast<const char*>(&fileSize), sizeof(fileSize));
        out.write(file.c_str(), fileSize);
    }

    // Save subfolders
    size_t foldersCount = folders.size();
    out.write(reinterpret_cast<const char*>(&foldersCount), sizeof(foldersCount));
    for (const auto& folder : folders)
    {
        folder.save(out);
    }
}

void Folders::load(std::ifstream& in)
{
    // Load folder name
    size_t folderNameSize;
    in.read(reinterpret_cast<char*>(&folderNameSize), sizeof(folderNameSize));
    name.resize(folderNameSize);
    in.read(&name[0], folderNameSize);

    // Load files
    size_t filesCount;
    in.read(reinterpret_cast<char*>(&filesCount), sizeof(filesCount));
    files.resize(filesCount);
    for (auto& file : files)
    {
        size_t fileSize;
        in.read(reinterpret_cast<char*>(&fileSize), sizeof(fileSize));
        file.resize(fileSize);
        in.read(&file[0], fileSize);
    }

    // Load subfolders
    size_t foldersCount;
    in.read(reinterpret_cast<char*>(&foldersCount), sizeof(foldersCount));
    folders.resize(foldersCount);
    for (auto& folder : folders)
    {
        folder.load(in);
        folder.parent = this;
    }
}

void Folders::addFolder(std::string name){
    if (std::find_if(folders.begin(), folders.end(), [&name](const Folders& folder) { return folder.name == name; }) != folders.end()) {
        return;
    }
    Folders child = Folders(name);
    child.parent = this;
    folders.push_back(child);

}

void Folders::addFile(std::string name)
{
    files.push_back(name);
}

void Folders::print() const{
    qDebug() << "Folder : {" << name << ",";

    for (const auto& folder : folders){
        qDebug() << "[" << folder.name << "]";
    }
    qDebug() << "}";
    for (const auto& folder : folders)
    {
        folder.print();
    }
}

void addSubfolders(Folders& rootFolder, const std::string& path) {
    namespace fs = std::filesystem;

    for (const auto& entry : fs::directory_iterator(path)) {
        if (entry.is_directory()) {
            if (containImage(entry.path().string())){
                std::string folderName = entry.path().filename().string();
                rootFolder.addFolder(folderName);
                addSubfolders(rootFolder.folders.back(), entry.path().string());
            }
        }
    }
}

void addFilesToTree(Folders* currentFolder, const std::string& path)
{
    fs::path fsPath(path);

    for (const auto& part : fsPath){
        if (part == "/")
            continue;

        std::string folderName = part.string();
        if (!createIfNotExist(currentFolder, folderName)){
            currentFolder->addFolder(folderName);
            currentFolder = &currentFolder->folders.back();
        }
    }
    addSubfolders(*currentFolder, path);
}

// passer au sous dossier si il existe
bool createIfNotExist(Folders*& currentFolder, const std::string& folderName)
{
    if (getIfExist(currentFolder, folderName)){
        return true;
    } else{
        return false;
    }
}

// Verifie si un dossier existe dans un Folders
bool getIfExist(Folders* currentFolder, const std::string& path){
    auto it = std::find_if(currentFolder->folders.begin(), currentFolder->folders.end(), [&path](const Folders& folder)
        { return folder.name == path; });

    if (it != currentFolder->folders.end()){
        return true;
    }
    return false;
}

// Verifie si un dossier contient une image
bool containImage(const std::string& path)
{
    bool valide = false;
    for (const auto& entry : fs::recursive_directory_iterator(path))
    {
        if (fs::is_regular_file(entry.status()) && isImage(entry.path().filename().string()))
        {
            return true;
        }
    }
    return false;
}