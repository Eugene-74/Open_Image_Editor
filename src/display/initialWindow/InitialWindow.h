#pragma once
#include <QMainWindow>
#include <QApplication>
#include <QScreen>
#include <QVBoxLayout>
#include <QDesktopServices>

#include "../../structure/imagesData/ImagesData.h"
#include "../../structure/data/Data.h"

#include "../../display/imageEditor/ImageEditor.h"
#include "../../display/imageBooth/ImageBooth.h"


class ImageEditor;
class ImageBooth;

class InitialWindow : public QMainWindow {
    Q_OBJECT

public:
    Data* data;


    InitialWindow();
    qreal pixelRatio;
    QSize screenGeometry;

    void createImageEditor(Data* data);
    void clearImageEditor();

    void createImageBooth(Data* data);
    void clearImageBooth();

private slots:
    void showImageEditor();
    void showImageBooth();

private:
    ImageEditor* imageEditor = nullptr;
    ImageBooth* imageBooth = nullptr;
    QVBoxLayout* layout;
    QWidget* centralWidget;
    ClickableLabel* createImageDiscord();
    ClickableLabel* createImageGithub();
    ClickableLabel* createImageOption();

    void openOption();
protected:
    void closeEvent(QCloseEvent* event) override;
};