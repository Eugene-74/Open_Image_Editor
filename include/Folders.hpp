#pragma once

#include <QProgressDialog>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

// #include "ImageData.hpp"g
#include "Vector.hpp"
#include "Verification.hpp"

class Folders {
    //    private:
   public:
    Folders* parent = nullptr;
    std::string name;
    std::vector<Folders> folders;

    std::vector<std::string> files;

    //    public:
    Folders() = default;

    Folders(std::string f)
        : name(f), parent(nullptr) {}

    Folders(std::string f, Folders* p)
        : name(f), parent(p) {}

    // Constructeur de copie
    Folders(const Folders& other)
        : name(other.name), files(other.files), folders(other.folders) {}

    Folders& operator=(const Folders& other);

    void save(std::ofstream& out) const;

    void load(std::ifstream& in);

    void addFolder(std::string name);
    void addFile(std::string name);

    void print() const;

    std::string getName() const;
    std::vector<Folders>* getFolders();
    std::vector<Folders> getFoldersConst() const;

    std::vector<std::string>* getFiles();
    std::vector<std::string> getFilesConst() const;

    Folders* getFolder(int index);
    std::string* getFile(int index);
    Folders* getParent();
    void setParent(Folders* parent);
};

bool createIfNotExist(Folders*& currentFolder, const std::string& path);
bool getIfExist(Folders* currentFolder, const std::string& path);

void addFiles(Folders* root, const std::string& path);

bool containImage(const std::string& path);