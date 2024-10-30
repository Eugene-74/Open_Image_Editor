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
#include <QSize>

#include <QKeyEvent>
#include <QListWidget>
#include <QLineEdit>
#include <QDateTime>
#include <QDateTimeEdit>
#include <QCalendarWidget>



// #include "../../Main.h"
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
    ImageEditor(Data& a, QWidget* parent = nullptr);

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
    ClickableLabel* createImageEditExif();

    ClickableLabel* createImageBefore();
    ClickableLabel* createImageNext();
    void saveImage();
    void deleteImage();
    void unDeleteImage();


    void populateMetadataFields();
    void validateMetadata();



protected:
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;

private:
    bool exifEditor = false;

    QDateTimeEdit* dateEdit;
    QLineEdit* geoEdit;
    QLineEdit* descriptionEdit;
    QPushButton* validateButton;


    QLabel* imageLabel;
    QHBoxLayout* mainLayout;
    QHBoxLayout* previewButtonLayout;
    QHBoxLayout* actionButtonLayout;
    QHBoxLayout* buttonLayout;
    QVBoxLayout* infoLayout;
    QVBoxLayout* editionLayout;
    // QHBoxLayout* infoAndButtonLayout;

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
    ClickableLabel* imageEditExif;

};



