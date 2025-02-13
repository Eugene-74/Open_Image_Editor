#pragma once

#include <QApplication>
#include <QCalendarWidget>
#include <QDateTime>
#include <QDateTimeEdit>
#include <QFileDialog>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMainWindow>
#include <QPixmap>
#include <QPushButton>
#include <QScreen>
#include <QSize>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>
#include <iostream>

#include "Box.hpp"
#include "ClickableLabel.hpp"
#include "Const.hpp"
#include "Data.hpp"
#include "ImagesData.hpp"
#include "InitialWindow.hpp"
#include "LoadImage.hpp"
#include "MainImage.hpp"

class MainImage;

class ImageEditor : public QMainWindow {
    Q_OBJECT

   public:
    ImageEditor(Data* dat, QWidget* parent = nullptr);
    ~ImageEditor() {}

    void clear();
    void reload();

   protected:
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

   private:
    void hide();
    void unHide();

    void nextImage(int nbr = 1);
    void previousImage(int nbr = 1);

    void rotateLeft();
    void rotateRight();

    void mirrorUpDown();
    void mirrorLeftRight();

    void createPreview();
    void updatePreview();

    void createButtons();
    void updateButtons();

    // void createBigImage();

    void reloadImageLabel();

    void saveImage();
    void exportImage();

    void deleteImage();
    void unDeleteImage();

    void populateMetadataFields();
    void validateMetadata();

    void startImageOpen();
    void stopImageOpen();

    void checkCache();
    void checkLoadedImage();

    ClickableLabel* createImagePreview(std::string imagePath, int imageNbr);

    ClickableLabel* createImageRotateRight();
    ClickableLabel* createImageRotateLeft();
    ClickableLabel* createImageMirrorUpDown();
    ClickableLabel* createImageMirrorLeftRight();
    ClickableLabel* createImageDelete();
    ClickableLabel* createImageSave();
    ClickableLabel* createImageExport();
    ClickableLabel* createImageConversion();

    ClickableLabel* createImageEditExif();
    ClickableLabel* createImagePersons();

    ClickableLabel* createImageBefore();
    ClickableLabel* createImageNext();

    MainImage* createImageLabel();

    void openBigImageLabel();
    void closeBigImageLabel(MainImage* bigImageLabel, bool oldExifEditor);

    bool exifEditor = false;
    bool personsEditor = false;

    Data* data;

    QLineEdit* nameEdit;
    QDateTimeEdit* dateEdit;
    QLineEdit* geoEdit;
    QLineEdit* descriptionEdit;
    QPushButton* validateButton;

    MainImage* imageLabel;
    MainImage* bigImageLabel;

    QFrame* fixedFrame;
    QLayout* imageLabelLayout;
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

    ClickableLabel* imageConversion;

    ClickableLabel* imageEditExif;
    ClickableLabel* imagePersons;

    QTimer* imageOpenTimer = new QTimer(this);

    bool bigImage = false;
   signals:
    void switchToImageBooth();
    void switchToMainWindow();
};
