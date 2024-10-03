#ifndef IMAGEDATA_H
#define IMAGEDATA_H

#include <iostream>
#include <vector>

#include "../folders/folders.h"
#include "../metaData/metaData.h"
// #include "../imagesData/imagesData.h"


// #include <folders.h>


class ImageData
{
public:
    std::string imagePath;
    Folders folders;
    MetaData metaData;

    ImageData() : imagePath(""), folders(Folders()), metaData(MetaData()) {}

    ImageData(const ImageData& other)
        : imagePath(other.imagePath), folders(other.folders), metaData(other.metaData) {}

    ImageData(std::string a, const Folders& c) : imagePath(a), folders(c) {}

    // Opérateur d'affectation
    ImageData& operator=(const ImageData& other) {
        if (this != &other) {
            imagePath = other.imagePath;
            folders = other.folders;
            metaData = other.metaData;
        }
        return *this;
    }

    void print() const;

    std::string get() const;

    MetaData* getMetaData();

    std::vector<std::string> getFolders();

    void addFolder(const std::string& toAddFolder);

    void addFolders(const std::vector<std::string>& toAddFolders);




    std::string getImageName() const;

    bool operator==(const ImageData& other) const;

    std::string getImagePath();



    void setExifMetaData(const Exiv2::ExifData& toAddMetaData);

    void loadData();

    void saveMetaData();

    int getImageWidth();

    int getImageHeight();

    int getImageOrientation();

    Date getImageDate();

    void turnImage(int rotation);


    void save(std::ofstream& out) const {
        size_t pathLength = imagePath.size();
        out.write(reinterpret_cast<const char*>(&pathLength), sizeof(pathLength));
        out.write(imagePath.c_str(), pathLength);
        folders.save(out);
        metaData.save(out);
        // metaData.exifMetaData;
        // displayExifData(metaData.exifMetaData);
        // std::cerr <<  << std::endl;

    }

    void load(std::ifstream& in) {
        size_t pathLength;
        in.read(reinterpret_cast<char*>(&pathLength), sizeof(pathLength));
        imagePath.resize(pathLength);
        in.read(&imagePath[0], pathLength);
        folders.load(in);
        metaData.load(in);
    }

    void setOrCreateExifData();

};


#endif
