#pragma once

#include <iostream>
#include <unordered_map>
#include <vector>

#include "ImageData.hpp"

class ImagesData {
   private:
    int imageNumber = 0;
    std::unordered_map<std::string, ImageData*> imageMap;
    std::unordered_map<int, ImageData*> imageMapInt;

    std::vector<ImageData*> currentImagesData;
    std::vector<ImageData*> imagesData;

    std::map<std::string, bool> filters = {
        {"image", true},
        {"video", true},
        {"person", true},
        {"bicycle", true},
        {"car", true},
        {"motorbike", true},
        {"aeroplane", true},
        {"bus", true},
        {"train", true},
        {"truck", true},
        {"boat", true},
        {"traffic light", true},
        {"fire hydrant", true},
        {"stop sign", true},
        {"parking meter", true},
        {"bench", true},
        {"bird", true},
        {"cat", true},
        {"dog", true},
        {"horse", true},
        {"sheep", true},
        {"cow", true},
        {"elephant", true},
        {"bear", true},
        {"zebra", true},
        {"giraffe", true},
        {"backpack", true},
        {"umbrella", true},
        {"handbag", true},
        {"tie", true},
        {"suitcase", true},
        {"frisbee", true},
        {"skis", true},
        {"snowboard", true},
        {"sports ball", true},
        {"kite", true},
        {"baseball bat", true},
        {"baseball glove", true},
        {"skateboard", true},
        {"surfboard", true},
        {"tennis racket", true},
        {"bottle", true},
        {"wine glass", true},
        {"cup", true},
        {"fork", true},
        {"knife", true},
        {"spoon", true},
        {"bowl", true},
        {"banana", true},
        {"apple", true},
        {"sandwich", true},
        {"orange", true},
        {"broccoli", true},
        {"carrot", true},
        {"hot dog", true},
        {"pizza", true},
        {"donut", true},
        {"cake", true},
        {"chair", true},
        {"sofa", true},
        {"pottedplant", true},
        {"bed", true},
        {"diningtable", true},
        {"toilet", true},
        {"tvmonitor", true},
        {"laptop", true},
        {"mouse", true},
        {"remote", true},
        {"keyboard", true},
        {"cell phone", true},
        {"microwave", true},
        {"oven", true},
        {"toaster", true},
        {"sink", true},
        {"refrigerator", true},
        {"book", true},
        {"clock", true},
        {"vase", true},
        {"scissors", true},
        {"teddy bear", true},
        {"hair drier", true},
        {"toothbrush", true},
    };

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

    void setImageMapIntValue(int index, ImageData* imageData);
};
