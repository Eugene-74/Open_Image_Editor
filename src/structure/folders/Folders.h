#pragma once

#include <iostream>
#include <vector>
#include <fstream>
#include "../../functions/vector/Vector.h"
class Folders
{
public:
    std::vector<std::string> folders;

    Folders() = default;

    // Constructeur de copie
    Folders(const Folders& other) : folders(other.folders) {}

    // Constructeur qui accepte un std::vector<std::string>
    Folders(const std::vector<std::string>& f) : folders(f) {}
    Folders& operator=(const Folders& other);

    void print()const;
    std::string getFolderList() const;
    void addFolder(const std::string& folder);
    std::vector<std::string>  getFolders();

    void save(std::ofstream& out) const;

    void load(std::ifstream& in);

};
