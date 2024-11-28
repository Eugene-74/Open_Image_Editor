#pragma once


#include <iostream>
#include <vector>
#include <exiv2/exiv2.hpp>
#include <filesystem>

#include "structure/folders/Folders.h"
#include "structure/metaData/MetaData.h"
#include "structure/imageData/ImageData.h"
#include "structure/imagesData/ImagesData.h"
#include "structure/data/Data.h"



#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <algorithm>
#include <exiv2/exiv2.hpp>

#include "structure/folders/Folders.h"

#include "functions/vector/Vector.h"
#include "functions/verification/Verification.h"

#include "display/imageEditor/ImageEditor.h"
#include "display/initialWindow/InitialWindow.h"


#include <QApplication>
#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include <QPixmap>
#include <QVBoxLayout>
#include <QWidget>
#include <QDebug>

#include <QFileDialog>
#include <chrono>







// void countImagesFromFolder(const std::string path, int& nbrImage);

void countImagesFromFolder(const std::string path, int& nbrImage);
void loadImagesFromFolder(const std::string initialPath, const std::string path, ImagesData* imagesData, int& nbrImage);
void startLoadingImagesFromFolder(Data* data, const std::string imagePaths, ImagesData* imagesData);
void loadImagesMetaData(ImagesData* imagesData);
void nextImage(int nbr);
