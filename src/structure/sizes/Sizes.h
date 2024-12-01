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
    int pixelRatio;
    QSize screenGeometry;



    class ImagesEditorSizes;
    class ImagesBoothSizes;

    ImagesEditorSizes* imagesEditorSizes;
    ImagesBoothSizes* imagesBoothSizes;

    Sizes();
};



class Sizes::ImagesEditorSizes{
public:
    ImagesEditorSizes(Sizes* parent);

    QSize previewSize;
    QSize mainImageSize;
    QSize actionSize;

    int mainLayoutSpacing = 5;
    std::vector<int> mainLayoutMargins = { 5,5,5,5 };


private:
    Sizes* parentSizes;

};

class Sizes::ImagesBoothSizes{
public:
    ImagesBoothSizes(Sizes* parent);

private:
    Sizes* parentSizes;
};


