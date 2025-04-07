#pragma once

#include <QMainWindow>
#include <QTimer>
#include <vector>

#include "Data.hpp"

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
    ImageBooth(Data* dat, QWidget* parent = nullptr);
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
    Data* data;

    int lastLineNbr = 0;
    int imageQuality = 16;
    int maxVisibleLines = data->sizes->imagesBoothSizes->heightImageNumber + 2;
    std::vector<QLabel*> imageWidgets;

    QScrollArea* scrollArea;
    QWidget* scrollWidget;

    QSpacerItem* spacer;

    QVBoxLayout* mainLayout;
    QVBoxLayout* linesLayout;

    QSize* imageSize = &data->sizes->imagesBoothSizes->imageSize;
    QSize* actionSize = &data->sizes->imagesEditorSizes->actionSize;
    QSize* realImageSize = &data->sizes->imagesBoothSizes->realImageSize;

    QHBoxLayout* scrollLayout;

    QHBoxLayout* actionButtonLayout;

    ClickableLabel* imageRotateRight;
    ClickableLabel* imageRotateLeft;
    ClickableLabel* imageMirrorUpDown;
    ClickableLabel* imageMirrorLeftRight;
    ClickableLabel* imageDelete;
    ClickableLabel* imageSave;
    ClickableLabel* imageExport;
    ClickableLabel* imageEditExif;
    ClickableLabel* imageConversion;
    ClickableLabel* editFilters;
    // ClickableLabel* imageMore;
    // ClickableLabel* imageLess;

    std::vector<QHBoxLayout*>
        lineLayouts;

    void onScroll(int value);

    void updateImages();

    void preLoadImages();

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
    ClickableLabel* createImageEditExif();
    ClickableLabel* createImageConversion();
    ClickableLabel* createEditFilters();

    // ClickableLabel* createImageMore();
    // ClickableLabel* createImageLess();

    void openFiltersPopup();

    void openFolder(int index);

    void createFirstImages();
    ClickableLabel* createImage(std::string imagePath, int nbr);
    void updateVisibleImages(bool force = false);
    // void checkThumbnailAndCorrect();
   signals:
    void switchToImageEditor();
    void switchToImageBooth();
    void switchToMainWindow();
};
