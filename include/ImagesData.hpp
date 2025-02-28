#pragma once

#include <iostream>
#include <unordered_map>
#include <vector>

#include "Const.hpp"
#include "ImageData.hpp"

class ImagesData {
    // TODO netoyer
   public:
    std::vector<ImageData> imagesData;
    std::vector<ImageData*> currentImagesData;

    std::unordered_map<std::string, ImageData*> imageMap;

    int imageNumber = 0;

    // private:
    ImagesData()
        : imageNumber(0) {}

    // Constructeur de copie
    ImagesData(const ImagesData& other)
        : imagesData(other.imagesData), imageNumber(other.imageNumber), currentImagesData(other.currentImagesData), imageMap(other.imageMap) {
    }

    ImagesData(const std::vector<ImageData> a)
        : imagesData(a) {}

    ImagesData& operator=(const ImagesData& other);

    //  Définit le numéro de l'image actuel dans imagesData
    void setImageNumber(int nbr);

    //  Renvoie le numéro de l'image actuel dans imagesData
    int getImageNumber();

    // Affiche imagesData
    void print() const;

    // Permet d'ajouter une ImageData à imagesData
    void addImage(ImageData& imageD);

    // Permet d'enlever une ImageData à imagesData
    void removeImage(const ImageData& image);

    // Permet de recuperer la MetaData de l'ImageData à l'id
    ImageData* getImageData(int id);
    ImageData* getImageDataInCurrent(int id);

    ImageData* getImageData(std::string imagePath);

    // Permet de recuperer la MetaData de l'ImageData actuellement utilisé
    ImageData* getCurrentImageData();

    // Permet de recuperer imagesData
    std::vector<ImageData>* get();
    std::vector<ImageData*>* getCurrent();

    int getImageNumberInTotal();
    int getImageNumberInTotal(int imageNbrInCurrent);
    int getImageNumberInCurrent(int imageNbrInTotal);

    std::vector<ImageData> getConst() const;

    int getImageDataIdInCurrent(std::string imagePath);
    int getImageDataId(std::string imagePath);

    std::unordered_map<std::string, ImageData*>* getImageMap();
    void setImageMapValue(std::string imagePath, ImageData* imageData);
};
