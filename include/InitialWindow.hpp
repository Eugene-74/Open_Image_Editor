#pragma once

#include <curl/curl.h>

#include <QMainWindow>
#include <QProgressBar>

#include "AsyncProgressBar.hpp"

// Forward declarations
class Data;
class QProgressDialog;
class QVBoxLayout;
class ClickableLabel;
class ImageBooth;
class ImageEditor;
class MainWindow;
class QTimer;
class QHBoxLayout;
class QVBoxLayout;
class QSize;

class InitialWindow : public QMainWindow {
    Q_OBJECT

   public:
    std::shared_ptr<Data> data;

    InitialWindow();
    qreal pixelRatio;
    QSize screenGeometry;

    void createImageEditor(std::shared_ptr<Data> data);
    void clearImageEditor();

    void createImageBooth(std::shared_ptr<Data> data);
    void clearImageBooth();

    void createMainWindow(std::shared_ptr<Data> data);
    void clearMainWindow();

   private slots:
    void showImageEditor();
    void showImageBooth();
    void showMainWindow();
    void clearWindows();

   private:
    QSize linkButton;

    QTimer* resizeTimer;
    ImageEditor* imageEditor = nullptr;
    ImageBooth* imageBooth = nullptr;
    MainWindow* mainWindow = nullptr;

    QVBoxLayout* layout;

    QHBoxLayout* languageLayout;
    QHBoxLayout* linkLayout;
    QHBoxLayout* topLayout;
    QWidget* centralWidget;

    ClickableLabel* imageHome;
    ClickableLabel* imageBack;

    ClickableLabel* imageLanguage;

    AsyncProgressBar* detectionProgressBar;

    ClickableLabel* imageWifi;
    ClickableLabel* imageDiscord;
    ClickableLabel* imageGithub;
    ClickableLabel* imageOption;

    ClickableLabel* createImageHome();
    ClickableLabel* createImageBack();

    ClickableLabel* createImageLanguage();

    AsyncProgressBar* createDetectionProgressBar();

    ClickableLabel* createImageWifi();
    ClickableLabel* createImageDiscord();
    ClickableLabel* createImageGithub();
    ClickableLabel* createImageOption();

    void openOption();

   protected:
    void closeEvent(QCloseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
   signals:
    void reload();
};

bool isDarkMode();

bool checkForUpdate();
void startLog();
