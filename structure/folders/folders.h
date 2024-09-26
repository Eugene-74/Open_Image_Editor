#ifndef FOLDERS_H
#define FOLDERS_H

#include <iostream>
#include <vector>
#include <fstream>
class Folders
{
public:
    std::vector<std::string> folders;



    Folders() = default;

    // Constructeur de copie
    Folders(const Folders& other) : folders(other.folders) {}

    // Constructeur qui accepte un std::vector<std::string>
    Folders(const std::vector<std::string>& f) : folders(f) {}
    Folders& operator=(const Folders& other) {
        if (this != &other) {
            folders = other.folders; // Utiliser l'opérateur d'affectation de std::vector
        }
        return *this;
    }

    void print()const;
    std::string getFolderList() const;
    void addFolder(const std::string& folder);
    std::vector<std::string>  getFolders();

    void save(std::ofstream& out) const {
        size_t folderCount = folders.size();
        out.write(reinterpret_cast<const char*>(&folderCount), sizeof(folderCount));
        for (const auto& folder : folders) {
            size_t length = folder.size();
            out.write(reinterpret_cast<const char*>(&length), sizeof(length));
            out.write(folder.c_str(), length);
        }
    }

    void load(std::ifstream& in) {
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
};


#endif
