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
