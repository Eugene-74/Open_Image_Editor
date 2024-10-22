#ifndef MAIN_H
#define MAIN_H

#include <iostream>
#include <vector>
#include <exiv2/exiv2.hpp>
#include <filesystem>

#include "structure/folders/folders.h"
#include "structure/metaData/metaData.h"
#include "structure/imageData/imageData.h"
#include "structure/imagesData/imagesData.h"



#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <algorithm>
#include <exiv2/exiv2.hpp>

#include "structure/folders/folders.h"

#include "functions/vector/vector.h"
#include "functions/thumbnail/thumbnail.h"
#include "display/imageEditor/imageEditor.h"

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


const std::string THUMBNAIL_PATH = "/home/eugene/.cache/thumbnails";

bool isImage(const std::string& cheminFichier);
bool isTurnable(const std::string& cheminFichier);




void countImagesFromFolder(const std::string path, int& nbrImage);
void loadImagesFromFolder(const std::string initialPath, const std::string path, ImagesData& imagesData, int& nbrImage);
void startLoadingImagesFromFolder(const std::string imagePaths, ImagesData& imagesData);
void loadImagesMetaData(ImagesData& imagesData);
void nextImage(int nbr);



#endif
