#pragma once

#include <QApplication>
#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include <QPixmap>
#include <QVBoxLayout>
#include <QWidget>
#include <QFileDialog>
#include <QScreen>
#include <QTimer>
#include <iostream>
#include <QResizeEvent>
#include <QSize>
#include <QResource>

#include <opencv2/opencv.hpp>

#include "../../Main.h"
#include "../../structure/imagesData/ImagesData.h"
#include "../../display/loadImage/LoadImage.h"
#include "../../structure/data/Data.h"
#include "../../display/box/Box.h"
#include "../../display/clickableLabel/ClickableLabel.h"

#include "../../Const.h"



class ImageEditor : public QMainWindow {
    Q_OBJECT

public:

    // Constructor
    ImageEditor(Data& a, QWidget* parent = nullptr);   // Initialize the reference member

    ~ImageEditor() {}

    void reload();
    void reloadMainImage();

    void setImage(ImageData& imageData);

    void nextImage();
    void previousImage();
    void rotateLeft();
    void rotateRight();

    void createPreview();
    void updatePreview();

    void createButtons();
    void updateButtons();

    void clear();

    ClickableLabel* createImagePreview(std::string imagePath, int imageNbr);


    ClickableLabel* createImageRotateRight();
    ClickableLabel* createImageRotateLeft();
    ClickableLabel* createImageDelete();
    ClickableLabel* createImageSave();

    ClickableLabel* createImageBefore();
    ClickableLabel* createImageNext();




protected:
    // void resizeEvent(ImagesData& imagesData) override;


private:
    QLabel* imageLabel;
    QVBoxLayout* mainLayout;
    QHBoxLayout* previewButtonLayout;
    QHBoxLayout* actionButtonLayout;
    QHBoxLayout* buttonLayout;
    QSize previewSize;
    QSize actionSize;
    QSize screenGeometry;
    Data data;
    qreal pixelRatio;

    std::vector<ClickableLabel*> previewButtons;

    ClickableLabel* buttonImageBefore;
    ClickableLabel* buttonImageNext;

    ClickableLabel* imageRotateRight;
    ClickableLabel* imageRotateLeft;
    ClickableLabel* imageDelete;
    ClickableLabel* imageSave;





};



