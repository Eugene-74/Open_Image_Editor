#pragma once

#include <iostream>
#include <vector>
#include <fstream>
#include "../../functions/vector/Vector.h"
#include <filesystem>
#include "../../functions/verification/Verification.h"

class Folders
{
public:
    Folders* parent;
    std::string name;
    std::vector<Folders> folders;
    std::vector<std::string> files;

    Folders() = default;

    Folders(std::string f) : name(f), parent(nullptr) {}


    Folders(std::string f, Folders* p) : name(f), parent(p) {}

    // Constructeur de copie
    Folders(const Folders& other) : name(other.name), files(other.files) {}


    Folders& operator=(const Folders& other);


    void save(std::ofstream& out) const;

    void load(std::ifstream& in);

    void addFolder(std::string name);
    void addFile(std::string name);

    void print() const;



};


void addSubfolders(Folders& rootFolder, const std::string& path);

void addFilesToTree(Folders* currentFolder, const std::string& path);

bool createIfNotExist(Folders*& currentFolder, const std::string& path);
bool getIfExist(Folders* currentFolder, const std::string& path);

void addFiles(Folders* root, const std::string& path);

bool containImage(const std::string& path);