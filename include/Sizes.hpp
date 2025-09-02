#pragma once

#include <QRect>
#include <QScreen>
#include <QSize>

class Sizes {
   public:
    Sizes();

    QScreen* screen;
    QRect screenR;
    QSize screenGeometry;

    QSize linkButton;

    int pixelRatio;
    int fontSize;

    class ImageEditorSizes;
    class ImagesBoothSizes;
    class MainWindowSizes;

    std::unique_ptr<ImageEditorSizes> imageEditorSizes;
    std::unique_ptr<ImagesBoothSizes> imagesBoothSizes;
    std::unique_ptr<MainWindowSizes> mainWindowSizes;
    void update();
};

class Sizes::ImageEditorSizes {
   public:
    ImageEditorSizes(Sizes* parent)
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