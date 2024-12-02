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

#include "../../display/loadImage/LoadImage.h"
#include "../../display/initialWindow/InitialWindow.h"



class ImageEditor : public QMainWindow {
    Q_OBJECT

public:
    ImageEditor(Data* dat, QWidget* parent = nullptr);

    ~ImageEditor() {}

    void reload();

    void nextImage(int nbr = 1);
    void previousImage(int nbr = 1);

    void rotateLeft();
    void rotateRight();

    void rotateLeftJpg();
    void rotateRightJpg();

    void rotateLeftPng();
    void rotateRightPng();

    void mirrorUpDown();
    void mirrorLeftRight();

    void mirrorUpDownJpg();
    void mirrorLeftRightJpg();

    void mirrorUpDownPng();
    void mirrorLeftRightPng();

    void createPreview();
    void updatePreview();

    void createButtons();
    void updateButtons();

    void clear();

    ClickableLabel* createImagePreview(std::string imagePath, int imageNbr);


    ClickableLabel* createImageRotateRight();
    ClickableLabel* createImageRotateLeft();
    ClickableLabel* createImageMirrorUpDown();
    ClickableLabel* createImageMirrorLeftRight();
    ClickableLabel* createImageDelete();
    ClickableLabel* createImageSave();
    ClickableLabel* createImageExport();

    ClickableLabel* createImageEditExif();

    ClickableLabel* createImageBefore();
    ClickableLabel* createImageNext();

    ClickableLabel* createImageLabel();
    void reloadImageLabel();

    void saveImage();
    void exportImage();

    void deleteImage();
    void unDeleteImage();


    void populateMetadataFields();
    void validateMetadata();

    void startImageOpenTimer();
    void stopImageOpenTimer();

    void checkCache();
    void checkLoadedImage();

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    bool exifEditor = false;
    Data* data;

    QLineEdit* nameEdit;
    QDateTimeEdit* dateEdit;
    QLineEdit* geoEdit;
    QLineEdit* descriptionEdit;
    QPushButton* validateButton;


    ClickableLabel* imageLabel;

    QHBoxLayout* mainLayout;
    QHBoxLayout* previewButtonLayout;
    QHBoxLayout* actionButtonLayout;
    QHBoxLayout* buttonLayout;
    QVBoxLayout* infoLayout;
    QVBoxLayout* editionLayout;

    QSize previewSize = data->sizes.imagesEditorSizes->previewSize;
    QSize actionSize = data->sizes.imagesEditorSizes->actionSize;
    QSize mainImageSize = data->sizes.imagesEditorSizes->mainImageSize;

    std::vector<ClickableLabel*> previewButtons;

    ClickableLabel* buttonImageBefore;
    ClickableLabel* buttonImageNext;

    ClickableLabel* imageRotateRight;
    ClickableLabel* imageRotateLeft;

    ClickableLabel* imageMirrorUpDown;
    ClickableLabel* imageMirrorLeftRight;

    ClickableLabel* imageDelete;
    ClickableLabel* imageSave;
    ClickableLabel* imageExport;

    ClickableLabel* imageEditExif;

    QTimer* imageOpenTimer = new QTimer(this);
    std::vector<QTimer*> imagePreviewOpenTimers = std::vector<QTimer*>();
signals:
    void switchToImageBooth();
};



