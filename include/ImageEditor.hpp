#pragma once

#include <QMainWindow>
#include <QTimer>
#include <iostream>

#include "Data.hpp"

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

    void rotateLeft();
    void rotateRight();

    void mirrorUpDown();
    void mirrorLeftRight();

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

    void showMapDialog();

    bool exifEditor = false;
    bool personsEditor = false;

    std::shared_ptr<Data> data;

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

    QSize* previewSize = &data->sizes->imageEditorSizes->previewSize;
    QSize* actionSize = &data->sizes->imageEditorSizes->actionSize;
    QSize* mainImageSize = &data->sizes->imageEditorSizes->mainImageSize;

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

#include <QDialog>
#include <QGeoCoordinate>
#include <QQmlComponent>
#include <QQuickItem>
#include <QQuickWidget>
#include <QVBoxLayout>
#include <QVariant>

class MapDialog : public QDialog {
    Q_OBJECT
   public:
    MapDialog(QWidget* parent = nullptr) : QDialog(parent) {
        setWindowTitle("Carte");
        resize(600, 400);

        QVBoxLayout* layout = new QVBoxLayout(this);

        quickWidget = new QQuickWidget(this);
        quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
        quickWidget->setSource(QUrl(QStringLiteral("qrc:/MapView.qml")));

        layout->addWidget(quickWidget);

        // Enregistrement du type QGeoCoordinate
        qRegisterMetaType<QGeoCoordinate>("QGeoCoordinate");
    }

    void setMapCenter(double latitude, double longitude) {
        if (!quickWidget)
            return;

        QQuickItem* rootItem = quickWidget->rootObject();
        if (!rootItem)
            return;

        QQuickItem* mapItem = rootItem->findChild<QQuickItem*>("mapView");
        if (!mapItem)
            return;

        QGeoCoordinate newCenter(latitude, longitude);
        mapItem->setProperty("center", QVariant::fromValue(newCenter));
    }

    void addMapPoint(double latitude, double longitude) {
        if (!quickWidget)
            return;

        QQuickItem* rootItem = quickWidget->rootObject();
        if (!rootItem)
            return;

        QQuickItem* mapItem = rootItem->findChild<QQuickItem*>("mapView");
        if (!mapItem)
            return;

        QVariantList point;
        point << latitude << longitude;

        QMetaObject::invokeMethod(mapItem, "addPoint", Q_ARG(QVariant, QVariant::fromValue(point)));
    }

   private:
    QQuickWidget* quickWidget;
};
