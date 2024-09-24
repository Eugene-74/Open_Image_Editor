#ifndef FOLDERS_H
#define FOLDERS_H

#include <iostream>
#include <vector>
class Folders
{
public:
    std::vector<std::string> folders;

    Folders(std::vector<std::string> a) : folders(a) {
    }

    void print()const;
    std::string getFolderList() const;
    void addFolder(const std::string& folder);
    std::vector<std::string>  getFolders();
};


#endif
