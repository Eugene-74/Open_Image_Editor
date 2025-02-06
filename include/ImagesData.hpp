#pragma once

#include <iostream>
#include <vector>

#include "Const.hpp"
#include "ImageData.hpp"

class ImagesData {
   public:
    std::vector<ImageData> imagesData;
    int imageNumber = 0;

    ImagesData()
        : imageNumber(0) {}

    // Constructeur de copie
    ImagesData(const ImagesData& other)
        : imagesData(other.imagesData), imageNumber(other.imageNumber) {
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

    ImageData* getImageData(std::string imagePath);

    // Permet de recuperer la MetaData de l'ImageData actuellement utilisé
    ImageData* getCurrentImageData();

    // Permet de recuperer imagesData
    std::vector<ImageData>* get();

    int getImageIdByName(std::string imagePath);
};
