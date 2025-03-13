#pragma once

#include <QRect>
#include <QScreen>
#include <QSize>

class Sizes {
   public:
    Sizes();

    // Sizes(Data* parent);
    // Data* data;
    QScreen* screen;
    QRect screenR;
    QSize screenGeometry;

    QSize linkButton;

    int pixelRatio;

    class ImagesEditorSizes;
    class ImagesBoothSizes;
    class MainWindowSizes;

    ImagesEditorSizes* imagesEditorSizes;
    ImagesBoothSizes* imagesBoothSizes;
    MainWindowSizes* mainWindowSizes;
    void update();
};

class Sizes::ImagesEditorSizes {
   public:
    ImagesEditorSizes(Sizes* parent)
        : parentSizes(parent) {
          };
    QSize previewSize;
    QSize mainImageSize;
    QSize actionSize;

    int mainLayoutSpacing = 5;
    std::vector<int> mainLayoutMargins = {5, 5, 5, 5};

    QSize bigImage;

    void update();

   private:
    Sizes* parentSizes;
};

class Sizes::ImagesBoothSizes {
   public:
    ImagesBoothSizes(Sizes* parent)
        : parentSizes(parent) {
          };
    //   Cast to int
    // int imagesPerLine = std::stoi(parentSizes->data->options["Sizes::imageBooth::ImagesPerLine"].value);
    int imagesPerLine = 10;

    QSize imageSize;
    QSize realImageSize;
    QSize scrollAreaSize;

    int linesLayoutSpacing = 3;
    std::vector<int> linesLayoutMargins = {5, 5, 5, 5};

    int widthImageNumber;
    int heightImageNumber;

    void setimagesPerLine(int imagesPerLine);
    void changeimagesPerLine(int imagesPerLine);

    void update();

   private:
    Sizes* parentSizes;
};

class Sizes::MainWindowSizes {
   public:
    MainWindowSizes(Sizes* parent)
        : parentSizes(parent) {
          };
    QSize actionSize;

    void update();

   private:
    Sizes* parentSizes;
};