#pragma once

#include <curl/curl.h>

#include <QMainWindow>

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
    void clearWindows();

   private:
    // QSize* linkButton = &data->sizes->linkButton;
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
