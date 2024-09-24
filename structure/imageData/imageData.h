#ifndef IMAGEDATA_H
#define IMAGEDATA_H

#include <iostream>
#include <vector>

#include "../folders/folders.h"
#include "../metaData/metaData.h"

// #include <folders.h>


class ImageData
{
public:
    std::string imagePath;
    Folders folders;
    MetaData metaData;

    ImageData(std::string a, const Folders& c) : imagePath(a), folders(c) {}

    void print() const;

    std::string get() const;

    MetaData* getMetaData();

    std::vector<std::string> getFolders();

    void addFolder(const std::string& toAddFolder);

    void addFolders(const std::vector<std::string>& toAddFolders);




    std::string getImageName() const;

    bool operator==(const ImageData& other) const;

    std::string getImagePath();



    void setMetaData(const Exiv2::ExifData& toAddMetaData);

    void loadMetaData();

    void saveMetaData();

    int getImageWidth();

    int getImageHeight();

    int getImageOrientation();

    Date getImageDate();

    void turnImage(int rotation);


};


#endif
