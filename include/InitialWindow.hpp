#pragma once
#include <curl/curl.h>
#include <json/json.h>

#include <QApplication>
#include <QDebug>
#include <QDesktopServices>
#include <QMainWindow>
#include <QScreen>
#include <QVBoxLayout>

#include "Box.hpp"
#include "Const.hpp"
#include "Data.hpp"
#include "Date.hpp"
#include "FaceRecognition.hpp"
#include "ImageBooth.hpp"
#include "ImageEditor.hpp"
#include "ImagesData.hpp"
#include "MainWindow.hpp"
#include "Version.hpp"

class ImageEditor;
class ImageBooth;
class MainWindow;

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

    void createMainWindow(Data* data);
    void clearMainWindow();

   private slots:
    void showImageEditor();
    void showImageBooth();
    void showMainWindow();

   private:
    ImageEditor* imageEditor = nullptr;
    ImageBooth* imageBooth = nullptr;
    MainWindow* mainWindow = nullptr;

    QVBoxLayout* layout;
    QWidget* centralWidget;
    ClickableLabel* createImageDiscord();
    ClickableLabel* createImageGithub();
    ClickableLabel* createImageOption();

    void openOption();

   protected:
    void closeEvent(QCloseEvent* event) override;
};

bool isDarkMode();
