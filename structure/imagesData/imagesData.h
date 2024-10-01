#ifndef IMAGESDATA_H
#define IMAGESDATA_H

#include <iostream>
#include <vector>

#include "../imageData/imageData.h"


const std::string SAVE_DAT_PATH = "/home/eugene/save.bat";


class ImagesData
{
public:
    std::vector<ImageData> imagesData;
    int imageNumber;

    // ImagesData() : imageNumber(0) {}

    // Constructeur de copie
    ImagesData(const ImagesData& other)
        : imagesData(other.imagesData), imageNumber(other.imageNumber) {}

    ImagesData(const std::vector<ImageData> a) : imagesData(a) {}

    ImagesData& operator=(const ImagesData& other) {
        if (this != &other) {
            imagesData = other.imagesData; // Utiliser l'opérateur d'affectation de std::vector
            imageNumber = other.imageNumber;

        }
        return *this;
    }

    void setImageNumber(int nbr){

        if (nbr < 0){
            nbr += imagesData.size();
        }
        else if (nbr >= imagesData.size()){
            nbr -= imagesData.size();

        }

        imageNumber = nbr;
    }
    int getImageNumber(){
        return imageNumber;
    }


    void print() const;

    void addImage(ImageData& imageD);

    void removeImage(const ImageData& image);

    ImageData* getImageData(int id);

    std::vector<ImageData>  get();


    void   saveImagesData() {
        // Sauvegarder l'objet dans un fichier binaire
        std::ofstream outFile(SAVE_DAT_PATH, std::ios::binary);
        save(outFile);
        outFile.close();

        std::cerr << "ImagesData saved in : " << SAVE_DAT_PATH << std::endl;


    }






    // Sauvegarder l'objet entier en binaire
    void save(std::ofstream& out) const {
        // Sauvegarder le nombre d'images
        size_t imageCount = imagesData.size();
        out.write(reinterpret_cast<const char*>(&imageCount), sizeof(imageCount));

        // Sauvegarder chaque image
        for (const auto& image : imagesData) {
            image.save(out);
            std::cerr << "save an image." << std::endl;

        }

        // Sauvegarder imageNumber
        out.write(reinterpret_cast<const char*>(&imageNumber), sizeof(imageNumber));

    }

    // Charger l'objet à partir d'un fichier binaire
    void load(std::ifstream& in) {
        // Effacer les données existantes
        imagesData.clear();

        // Lire le nombre d'images
        size_t imageCount;
        in.read(reinterpret_cast<char*>(&imageCount), sizeof(imageCount));

        // Lire chaque image
        imagesData.resize(imageCount);
        for (size_t i = 0; i < imageCount; ++i) {
            imagesData[i].load(in);
        }

        // Charger imageNumber
        in.read(reinterpret_cast<char*>(&imageNumber), sizeof(imageNumber));
    }
};
ImagesData loadImagesData();



#endif
