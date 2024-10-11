#pragma once

#include <iostream>
#include <vector>

#include "../imageData/imageData.h"




class ImagesData
{
public:
    std::vector<ImageData> imagesData;



    int imageNumber;

    ImagesData() : imageNumber(0) {}

    // Constructeur de copie
    ImagesData(const ImagesData& other)
        : imagesData(other.imagesData), imageNumber(other.imageNumber) {}

    ImagesData(const std::vector<ImageData> a) : imagesData(a) {}

    ImagesData& operator=(const ImagesData& other);

    void setImageNumber(int nbr);
    int getImageNumber();


    void print() const;

    void addImage(ImageData& imageD);
    void removeImage(const ImageData& image);



    ImageData* getImageData(int id);
    ImageData* getCurrentImageData();


    std::vector<ImageData>  get();


    void saveImagesData(std::string savePath);

    // Sauvegarder l'objet entier en binaire
    void save(std::ofstream& out) const;

    // Charger l'objet à partir d'un fichier binaire
    void load(std::ifstream& in);

};
ImagesData loadImagesData(std::string savePath);
