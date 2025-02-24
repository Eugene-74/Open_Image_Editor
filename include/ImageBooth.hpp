#pragma once

#include <QApplication>
#include <QLabel>
#include <QMainWindow>
#include <QScrollArea>
#include <QScrollBar>
#include <QTimer>
#include <QVBoxLayout>
#include <vector>

#include "Box.hpp"
#include "ClickableLabel.hpp"
#include "Data.hpp"
#include "InitialWindow.hpp"

// #include "../../Const.hpp"

class ImageBooth : public QMainWindow {
    Q_OBJECT

   public:
    ImageBooth(Data* dat, QWidget* parent = nullptr);
    ~ImageBooth() {}
    void reload();
    // void clear();

   protected:
    void keyReleaseEvent(QKeyEvent* event) override;
    void enterEvent(QEnterEvent* event) override;

   private:
    QTimer* updateTimer;
    int imageShiftSelected = -1;
    bool imageShiftSelectedSelect;

    Data* data;
    int imageNumber;

    int lastLineNbr = 0;

    int maxVisibleLines = data->sizes.imagesBoothSizes->heightImageNumber + 2;
    std::vector<QLabel*> imageWidgets;

    QScrollArea* scrollArea;

    QSpacerItem* spacer;

    QVBoxLayout* mainLayout;
    QVBoxLayout* linesLayout;

    QSize* imageSize = &data->sizes.imagesBoothSizes->imageSize;
    QSize* actionSize = &data->sizes.imagesEditorSizes->actionSize;
    QSize* realImageSize = &data->sizes.imagesBoothSizes->realImageSize;

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

    std::vector<QHBoxLayout*> lineLayouts;

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
    ClickableLabel* createImageEditExif();
    ClickableLabel* createImageConversion();

    void createFirstImages();
    ClickableLabel* createImage(std::string imagePath, int nbr);
    void updateVisibleImages(bool force = false);

   signals:
    void switchToImageEditor();
    void switchToMainWindow();
};
