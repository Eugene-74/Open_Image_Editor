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
    // QSize* linkButton = &data->sizes.linkButton;
    QSize* linkButton;

    QTimer* resizeTimer;
    ImageEditor* imageEditor = nullptr;
    ImageBooth* imageBooth = nullptr;
    MainWindow* mainWindow = nullptr;

    QVBoxLayout* layout;
    QHBoxLayout* linkLayout;
    QWidget* centralWidget;

    ClickableLabel* imageDiscord;
    ClickableLabel* imageGithub;
    ClickableLabel* imageOption;

    ClickableLabel* createImageDiscord();
    ClickableLabel* createImageGithub();
    ClickableLabel* createImageOption();

    void openOption();

   protected:
    void closeEvent(QCloseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
   signals:
    void resize();
};

bool isDarkMode();

bool checkForUpdate(QProgressDialog* progressDialog);
void startLog();
bool downloadFile(const std::string& url, const std::string& outputPath, QProgressDialog* progressDialog);
std::string getLatestGitHubTag(QProgressDialog* progressDialog);
int progressCallbackBis(void* ptr, curl_off_t totalToDownload, curl_off_t nowDownloaded, curl_off_t totalToUpload, curl_off_t nowUploaded);
int progressCallback(void* ptr, curl_off_t totalToDownload, curl_off_t nowDownloaded, curl_off_t totalToUpload, curl_off_t nowUploaded);
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
