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
    // std::map<QString, cv::Mat> imageCache;



    // Constructor
    ImageEditor(Data dat, QWidget* parent = nullptr);

    ~ImageEditor() {}

    void reload();
    // void reloadMainImage();

    // void setImage(ImageData* imageData);

    void nextImage(int nbr = 1);
    void previousImage(int nbr = 1);
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

    ClickableLabel* createImageLabel();
    void restartImageLabel();

    void saveImage();
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

private:
    bool exifEditor = false;
    Data data;

    QLineEdit* nameEdit;
    QDateTimeEdit* dateEdit;
    QLineEdit* geoEdit;
    QLineEdit* descriptionEdit;
    QPushButton* validateButton;


    ClickableLabel* imageLabel;

    // QHBoxLayout* imageLabelLayout;
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
    QSize mainImageSize;
    qreal pixelRatio;

    std::vector<ClickableLabel*> previewButtons;

    ClickableLabel* buttonImageBefore;
    ClickableLabel* buttonImageNext;

    ClickableLabel* imageRotateRight;
    ClickableLabel* imageRotateLeft;
    ClickableLabel* imageDelete;
    ClickableLabel* imageSave;
    ClickableLabel* imageEditExif;

    QTimer* imageOpenTimer = new QTimer(this);
    std::vector<QTimer*> imagePreviewOpenTimers = std::vector<QTimer*>();

};



