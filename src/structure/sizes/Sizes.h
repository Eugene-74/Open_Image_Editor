#pragma once

#include <QSize>
#include <QApplication>
#include <QRect>
#include <QScreen>



class Sizes {
public:
    // Membres de Sizes
    QScreen* screen;
    QRect screenR;
    QSize screenGeometry;
    QSize linkButtons;

    int pixelRatio;

    class ImagesEditorSizes;
    class ImagesBoothSizes;

    ImagesEditorSizes* imagesEditorSizes;
    ImagesBoothSizes* imagesBoothSizes;

    Sizes();
};



class Sizes::ImagesEditorSizes {
public:
    ImagesEditorSizes(Sizes* parent);

    QSize previewSize;
    QSize mainImageSize;
    QSize actionSize;

    int mainLayoutSpacing = 5;
    std::vector<int> mainLayoutMargins = { 5,5,5,5 };

    QSize bigImage;



private:
    Sizes* parentSizes;

};

class Sizes::ImagesBoothSizes {
public:
    ImagesBoothSizes(Sizes* parent);
    QSize imageSize;
    QSize realImageSize;
    QSize scrollAreaSize;

    int linesLayoutSpacing = 3;
    std::vector<int> linesLayoutMargins = { 5,5,5,5 };

    int widthImageNumber;
    int heightImageNumber;

private:
    Sizes* parentSizes;

};


