#pragma once

#include <QProgressDialog>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

#include "Vector.hpp"
#include "Verification.hpp"

class Folders {
   private:
    Folders* parent = nullptr;
    std::string name;
    std::vector<std::string> files;

   public:
    std::vector<Folders> folders;

   public:
    Folders() = default;

    Folders(std::string name)
        : name(name), parent(nullptr) {}

    Folders(std::string name, Folders* parent)
        : name(name), parent(parent) {}

    // Constructeur de copie
    Folders(const Folders& other)
        : name(other.name), files(other.files), folders(other.folders) {}

    Folders& operator=(const Folders& other);
    bool operator==(const Folders& other) const;

    void save(std::ofstream& out) const;
    void load(std::ifstream& in);

    void addFolder(std::string name);
    void addFile(std::string name);

    void print() const;

    std::string getName() const;
    std::vector<Folders>* getFolders();
    std::vector<Folders> getFoldersConst() const;

    std::vector<std::string> getFiles();
    std::vector<std::string>* getFilesPtr();
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