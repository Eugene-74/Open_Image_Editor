#pragma once

#include <QMainWindow>
#include <QTimer>
#include <iostream>

#include "Data.hpp"
#include "MapWidget.hpp"

// Forward declaration
class ClickableLabel;
class MainImage;
class QDateTime;
class QDateTimeEdit;
class QKeyEvent;
class QLineEdit;
class QObject;
class QPushButton;
class QSize;
class QVBoxLayout;
class QWidget;
class QFrame;
class QHBoxLayout;
class QLayout;

class ImageEditor : public QMainWindow {
    Q_OBJECT

   public:
    ImageEditor(std::shared_ptr<Data> dat, QWidget* parent = nullptr);
    ~ImageEditor() {}

    void clear();
    void reload();

   protected:
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    bool eventFilter(QObject* obj, QEvent* event) override;

   private:
    void hide();
    void unHide();

    void nextImage(int nbr = 1);
    void previousImage(int nbr = 1);

    void rotateLeft(bool action = true);
    void rotateRight(bool action = true);

    void mirrorUpDown(bool action = true);
    void mirrorLeftRight(bool action = true);

    void createPreview();
    void updatePreview();

    void createButtons();
    void updateButtons();

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

    void addActionWithDelay(std::function<void()> unDo, std::function<void()> reDo, int nbrInTotal);

    ClickableLabel* createImagePreview(std::string imagePath, int nbrInCurrent);

    ClickableLabel* createImageRotateRight();
    ClickableLabel* createImageRotateLeft();
    ClickableLabel* createImageMirrorUpDown();
    ClickableLabel* createImageMirrorLeftRight();
    ClickableLabel* createImageDelete();
    ClickableLabel* createImageSave();
    ClickableLabel* createImageExport();
    ClickableLabel* createImageConversion();

    ClickableLabel* createImageEditMap();
    ClickableLabel* createImageDetection();

    ClickableLabel* createImageGimp();

    ClickableLabel* createImageBefore();
    ClickableLabel* createImageNext();

    MainImage* createImageLabel();

    void openBigImageLabel();
    void closeBigImageLabel(MainImage* bigImageLabel);

    MapWidget* createMapWidget();

    bool mapEditor = false;
    bool personsEditor = false;

    std::shared_ptr<Data> data;

    QDateTimeEdit* dateEdit;
    MapWidget* editGeo;
    QLabel* nameEdit;

    QPushButton* validateButton;

    MainImage* imageLabel;
    MainImage* bigImageLabel;

    QFrame* fixedFrame;
    QLayout* imageLabelLayout;
    QHBoxLayout* mainLayout;
    QHBoxLayout* nameAndDateLayout;
    QHBoxLayout* previewButtonLayout;
    QHBoxLayout* actionButtonLayout;
    QHBoxLayout* imageLayout;
    QVBoxLayout* buttonLayout;
    QVBoxLayout* infoLayout;
    QVBoxLayout* editionLayout;

    QSize* previewSize = &data->getSizesPtr()->imageEditorSizes->previewSize;
    QSize* actionSize = &data->getSizesPtr()->imageEditorSizes->actionSize;
    QSize* mainImageSize = &data->getSizesPtr()->imageEditorSizes->mainImageSize;

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

    ClickableLabel* imageEditMap;
    ClickableLabel* imagePersons;
    ClickableLabel* imageGimp;

    QTimer* imageOpenTimer = new QTimer(this);

    bool bigImage = false;

   signals:
    void switchToImageBooth();
    void switchToMainWindow();
};
