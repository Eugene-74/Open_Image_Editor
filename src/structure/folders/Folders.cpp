#include "Folders.h"

namespace fs = std::filesystem;

Folders& Folders::operator=(const Folders& other)
{
    if (this != &other)
    {
        name = other.name; // Utiliser l'opérateur d'affectation de std::vector
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

void Folders::addFolder(std::string name)
{

    Folders child = Folders(name);
    folders.push_back(child);
    child.parent = this;
}

void Folders::addFile(std::string name)
{
    files.push_back(name);
}

void Folders::print() const{
    std::cerr << "Folder : {" << name << ",";

    for (const auto& folder : folders){
        std::cerr << "[" << folder.name << "]";
    }
    std::cerr << "}" << std::endl;
    for (const auto& folder : folders)
    {
        folder.print();
    }
}

void addFilesToTree(Folders* root, const std::string& path)
{
    fs::path fsPath(path);

    Folders* currentFolder = root;

    for (const auto& part : fsPath)
    {
        if (part == "/")
            continue;

        std::string folderName = part.string();
        if (!createIfNotExist(currentFolder, folderName))
        {
            currentFolder->addFolder(folderName);
            currentFolder = &currentFolder->folders.back();
        }
    }
    addFiles(currentFolder, path);
}

// ajouter tout les sous fichier contenant des images
void addFiles(Folders* root, const std::string& path){
    fs::path fsPath(path);
    for (const auto& entry : fs::directory_iterator(fsPath)){
        if (fs::is_directory(entry.status())){
            if (entry.path().filename().string().front() != '.'){
                createIfNotExist(root, entry.path().filename().string());

                if (containImage(entry.path().string())){
                    root->addFolder(entry.path().filename().string());
                }

                // Find the newly created folder and update the root pointer
                Folders* newRoot = nullptr;
                for (auto& folder : root->folders) {
                    if (folder.name == entry.path().filename().string()) {
                        newRoot = &folder;
                        break;
                    }
                }

                if (newRoot) {
                    addFiles(newRoot, entry.path().string());
                }
            }
        }
    }
}

// passer au sous dossier si il existe
bool createIfNotExist(Folders*& currentFolder, const std::string& folderName)
{
    if (getIfExist(currentFolder, folderName))
    {
        return true;
    } else
    {

        return false;
    }
}

// Verifie si un dossier existe dans un Folders
bool getIfExist(Folders* currentFolder, const std::string& path)
{
    auto it = std::find_if(currentFolder->folders.begin(), currentFolder->folders.end(), [&path](const Folders& folder)
        { return folder.name == path; });

    if (it != currentFolder->folders.end())
    {
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
