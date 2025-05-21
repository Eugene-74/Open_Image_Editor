#pragma once

#include <iostream>
#include <unordered_map>
#include <vector>

#include "ImageData.hpp"

class ImagesData {
   public:
    ImagesData()
        : imageNumber(0) {}

    // Constructeur de copie
    ImagesData(const ImagesData& other)
        : imagesData(other.imagesData), imageNumber(other.imageNumber), currentImagesData(other.currentImagesData), imageMap(other.imageMap) {
    }

    ImagesData(const std::vector<ImageData*> ImageDataList)
        : imagesData(ImageDataList) {}

    ImagesData& operator=(const ImagesData& other);

    void clear();

    void setImageNumber(int nbr);
    int getImageNumber() const;

    std::map<std::string, bool> getFilters();
    bool setFilters(std::map<std::string, bool> filters);
    bool setFiltersValue(std::string filterName, bool filterValue);

    // Permet d'ajouter une ImageData à imagesData
    void addImage(ImageData* imageData);

    // Permet d'enlever une ImageData à imagesData
    void removeImage(const ImageData& image);

    // Permet de recuperer la MetaData de l'ImageData à l'id
    ImageData* getImageData(int id);
    ImageData* getImageDataInCurrent(int id);

    ImageData* getImageData(std::string imagePath);

    // Permet de recuperer la MetaData de l'ImageData actuellement utilisé
    ImageData* getCurrentImageData();

    // Permet de recuperer imagesData
    std::vector<ImageData*>* get();
    std::vector<ImageData*> getConst() const;

    std::vector<ImageData*>* getCurrent();

    int getImageNumberInTotal();
    int getImageNumberInTotal(int imageNbrInCurrent);
    int getImageNumberInCurrent(int imageNbrInTotal);

    int getImageDataIdInCurrent(std::string imagePath);
    int getImageDataId(std::string imagePath);

    std::unordered_map<std::string, ImageData*>* getImageMap();
    void setImageMapValue(std::string imagePath, ImageData* imageData);

   private:
    int imageNumber = 0;
    std::unordered_map<std::string, ImageData*> imageMap;

    std::vector<ImageData*> currentImagesData;
    std::vector<ImageData*> imagesData;

    std::map<std::string, bool> filters = {
        {"image", true},
        {"video", true},
        {"person", false},
        {"bicycle", false},
        {"car", false},
        {"motorbike", false},
        {"aeroplane", false},
        {"bus", false},
        {"train", false},
        {"truck", false},
        {"boat", false},
        {"traffic light", false},
        {"fire hydrant", false},
        {"stop sign", false},
        {"parking meter", false},
        {"bench", false},
        {"bird", false},
        {"cat", false},
        {"dog", false},
        {"horse", false},
        {"sheep", false},
        {"cow", false},
        {"elephant", false},
        {"bear", false},
        {"zebra", false},
        {"giraffe", false},
        {"backpack", false},
        {"umbrella", false},
        {"handbag", false},
        {"tie", false},
        {"suitcase", false},
        {"frisbee", false},
        {"skis", false},
        {"snowboard", false},
        {"sports ball", false},
        {"kite", false},
        {"baseball bat", false},
        {"baseball glove", false},
        {"skateboard", false},
        {"surfboard", false},
        {"tennis racket", false},
        {"bottle", false},
        {"wine glass", false},
        {"cup", false},
        {"fork", false},
        {"knife", false},
        {"spoon", false},
        {"bowl", false},
        {"banana", false},
        {"apple", false},
        {"sandwich", false},
        {"orange", false},
        {"broccoli", false},
        {"carrot", false},
        {"hot dog", false},
        {"pizza", false},
        {"donut", false},
        {"cake", false},
        {"chair", false},
        {"sofa", false},
        {"pottedplant", false},
        {"bed", false},
        {"diningtable", false},
        {"toilet", false},
        {"tvmonitor", false},
        {"laptop", false},
        {"mouse", false},
        {"remote", false},
        {"keyboard", false},
        {"cell phone", false},
        {"microwave", false},
        {"oven", false},
        {"toaster", false},
        {"sink", false},
        {"refrigerator", false},
        {"book", false},
        {"clock", false},
        {"vase", false},
        {"scissors", false},
        {"teddy bear", false},
        {"hair drier", false},
        {"toothbrush", false},
    };
};
