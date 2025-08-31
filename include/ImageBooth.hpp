#pragma once

#include <QMainWindow>
#include <QTimer>
#include <vector>

#include "Data.hpp"
#include "MapWidget.hpp"

// Forward declaration
class QScrollArea;
class QVBoxLayout;
class ClickableLabel;
class QLabel;
class QSpacerItem;
class QHBoxLayout;

class ImageBooth : public QMainWindow {
    Q_OBJECT

   public:
    ImageBooth(std::shared_ptr<Data> dat, QWidget* parent = nullptr);
    ~ImageBooth() {}
    void reload();

   protected:
    void keyReleaseEvent(QKeyEvent* event) override;
    void enterEvent(QEnterEvent* event) override;

   private:
    QTimer* updateTimer = new QTimer(this);
    int getCurrentFoldersSize();
    // QTimer* updateTimer;
    int imageShiftSelected = -1;
    bool imageShiftSelectedSelect;
    std::shared_ptr<Data> data;

    int lastLineNbr = 0;
    int imageQuality = Const::Thumbnail::POOR_QUALITY;
    int maxVisibleLines = data->getSizesPtr()->imagesBoothSizes->heightImageNumber + 2;
    std::vector<QLabel*> imageWidgets;

    QScrollArea* scrollArea;
    QWidget* scrollWidget;

    QSpacerItem* spacer;

    QVBoxLayout* mainLayout;
    QHBoxLayout* centralLayout;

    bool mapEditor = false;

    QVBoxLayout* linesLayout;

    QSize* imageSize = &data->getSizesPtr()->imagesBoothSizes->imageSize;
    QSize* actionSize = &data->getSizesPtr()->imageEditorSizes->actionSize;
    QSize* realImageSize = &data->getSizesPtr()->imagesBoothSizes->realImageSize;

    QHBoxLayout* scrollLayout;

    QHBoxLayout* actionButtonLayout;

    ClickableLabel* imageRotateRight;
    ClickableLabel* imageRotateLeft;
    ClickableLabel* imageMirrorUpDown;
    ClickableLabel* imageMirrorLeftRight;
    ClickableLabel* imageDelete;
    ClickableLabel* imageSave;
    ClickableLabel* imageExport;
    ClickableLabel* imageConversion;
    ClickableLabel* imageEditMap;

    ClickableLabel* editFilters;

    MapWidget* mapWidget = nullptr;

    std::vector<QHBoxLayout*>
        lineLayouts;

    void onScroll(int value);

    void updateImages();

    ClickableLabel* getClickableLabelIfExist(int imageNbr);
    ClickableLabel* getClickableLabelIfExist(int imageNbr, QHBoxLayout*& lineLayout);

    void gotToImage(int nbr, bool force = false);
    void addNbrToSelectedImages(int nbr);
    void removeNbrToSelectedImages(int nbr);

    bool isImageVisible(int imageIndex);

    void createButtons();

    ClickableLabel* createImageRotateRight();
    ClickableLabel* createImageRotateLeft();
    ClickableLabel* createImageMirrorUpDown();
    ClickableLabel* createImageMirrorLeftRight();
    ClickableLabel* createImageDelete();
    ClickableLabel* createImageSave();
    ClickableLabel* createImageExport();
    ClickableLabel* createImageConversion();
    ClickableLabel* createImageEditMap();
    ClickableLabel* createEditFilters();

    void openFiltersPopup();

    void openFolder(int index);

    void createFirstImages();
    ClickableLabel* createImage(std::string imagePath, int nbrInCurrent);
    void updateVisibleImages(bool force = false);

    void countImagesInFolder(Folders* currentFolder, int& totalImages);

    void addActionWithDelay(std::function<void()> unDo, std::function<void()> reDo, int nbrInTotal);
    void exportImage();

    MapWidget* createMapWidget();
    void updateMapWidget();

   signals:
    void switchToImageEditor();
    void switchToImageBooth();
    void switchToMainWindow();
};

std::tuple<double, double, int> calculateMapCenterAndZoom(const std::vector<ImageData*>& images);
