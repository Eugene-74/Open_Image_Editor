#include "data.h"

void Data::preDeleteImage(int imageNbr){
    ImageData* imageData;
    imageData = imagesData.getImageData(imageNbr);

    deletedImagesData.addImage(*imageData);
    std::cerr << "image deleted" << imageNbr << std::endl;
    // removeImage(*imageData);
    deletedImagesData.print();

}
void Data::revocerDeletedImage(int imageNbr){
    ImageData* imageData;
    imageData = deletedImagesData.getImageData(imageNbr);

    imagesData.addImage(*imageData);
    deletedImagesData.removeImage(*imageData);

}