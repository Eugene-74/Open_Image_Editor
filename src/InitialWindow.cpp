#include "InitialWindow.hpp"

#include <curl/curl.h>
#include <json/json.h>

#include <QApplication>
#include <QCloseEvent>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QProcess>
#include <QProgressDialog>
#include <QThreadPool>
#include <QTimer>
#include <QTranslator>
#include <ctime>

#include "AppConfig.hpp"
#include "Box.hpp"
#include "ClickableLabel.hpp"
#include "Const.hpp"
#include "Data.hpp"
#include "ImageBooth.hpp"
#include "ImageEditor.hpp"
#include "MainWindow.hpp"

namespace fs = std::filesystem;

InitialWindow::InitialWindow() {
    startLog();

    // QThreadPool* threadPool = QThreadPool::globalInstance();
    // threadPool->setMaxThreadCount(std::max(threadPool->maxThreadCount() - 1, 1));

    resizeTimer = new QTimer(this);
    resizeTimer->setInterval(TIME_BEFORE_REZISE);
    resizeTimer->setSingleShot(true);
    connect(resizeTimer, &QTimer::timeout, this, [this]() {
        if (imageEditor) {
            data->sizes->update();
            clearImageEditor();
            createImageEditor(data);
        }
        if (imageBooth) {
            data->sizes->update();
            clearImageBooth();
            createImageBooth(data);
        }
        if (mainWindow) {
            data->sizes->update();
            clearMainWindow();
            createMainWindow(data);
        }
        emit resize();
    });

    QTimer::singleShot(1, this, [this]() {
        qInfo() << "Application started";
        data = new Data();

        QTranslator translator;
        QString locale = QLocale::system().name();
        QString language = locale.section('_', 0, 0);
        if (translator.load(":/translations/open_image_editor_" + language + ".qm")) {
            // app.installTranslator(&translator);
        } else {
            qWarning() << "Translation file not found for language:" << language;
        }

        ImagesData imagesData(std::vector<ImageData*>{});
        ImagesData deletedImagesData(std::vector<ImageData*>{});

        data->imagesData = imagesData;
        data->deletedImagesData = deletedImagesData;
        data->darkMode = isDarkMode();

        try {
            data->loadData();
        } catch (const std::exception& e) {
            qWarning() << "Error loading data: " << e.what();
            showErrorMessage(nullptr, "Error loading data: data corrupted");
        }
        if (!data->currentFolder) {
            data->currentFolder = data->findFirstFolderWithAllImages();
        }

        const QList<QScreen*> screens = QGuiApplication::screens();
        QScreen* screen = QGuiApplication::primaryScreen();
        screen = screens[0];

        QRect screenR = screen->availableGeometry();

        pixelRatio = screen->devicePixelRatio();

        screenGeometry = screenR.size() / pixelRatio;

        setWindowTitle("EasyImageEditor : Initial Window");

        QVBoxLayout* windowLayout = new QVBoxLayout();

        centralWidget = new QWidget(this);
        centralWidget->setLayout(windowLayout);
        setCentralWidget(centralWidget);

        layout = new QVBoxLayout;
        windowLayout->addLayout(layout);

        linkLayout = new QHBoxLayout();

        linkButton = &data->sizes->linkButton;
        imageDiscord = createImageDiscord();
        imageGithub = createImageGithub();
        imageOption = createImageOption();

        linkLayout->addWidget(imageDiscord);
        linkLayout->addWidget(imageGithub);
        linkLayout->addWidget(imageOption);
        linkLayout->setAlignment(Qt::AlignBottom | Qt::AlignRight);
        windowLayout->addLayout(linkLayout);

        createMainWindow(data);
        QProgressDialog progressDialog("Checking for updates...", nullptr, 0, 0, this);
        progressDialog.setWindowModality(Qt::ApplicationModal);
        progressDialog.setCancelButton(nullptr);
        progressDialog.show();
        progressDialog.move(0, 0);
        QApplication::processEvents();
        checkForUpdate(&progressDialog);
        progressDialog.close();

        for (std::string file : filesToDownload) {
            std::string filePath = APP_FILES.toStdString() + "/" + file;
            if (!fs::exists(filePath)) {
                QProgressDialog progressDialog("Downloading " + QString::fromStdString(file), nullptr, 0, 100, this);
                progressDialog.setWindowModality(Qt::ApplicationModal);
                progressDialog.setCancelButton(nullptr);
                progressDialog.setValue(0);
                progressDialog.show();

                if (!downloadFile("https://github.com/Eugene-74/Open_Image_Editor/raw/refs/heads/files/" + file, filePath, &progressDialog)) {
                    qDebug() << "Failed to download file:" << QString::fromStdString(file);
                }
            }
        }
    });
}
void checkForFilesToDownload() {
    QProgressDialog progressDialog("Downloading ", nullptr, 0, 100, nullptr);
    progressDialog.setWindowModality(Qt::ApplicationModal);
    progressDialog.setCancelButton(nullptr);
    progressDialog.setAutoClose(false);
    for (std::string file : filesToDownload) {
        std::string filePath = APP_FILES.toStdString() + "/" + file;
        if (!fs::exists(filePath)) {
            progressDialog.show();
            QApplication::processEvents();

            progressDialog.setObjectName("Downloading " + file);
            progressDialog.setValue(0);

            if (!downloadFile("https://github.com/Eugene-74/Open_Image_Editor/raw/refs/heads/files/" + file, filePath, &progressDialog)) {
                qDebug() << "Failed to download file:" << QString::fromStdString(file);
            }
        }
    }
    progressDialog.close();
}

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

size_t WriteCallbackAndSave(void* contents, size_t size, size_t nmemb, void* userp) {
    std::ofstream* out = static_cast<std::ofstream*>(userp);
    static_cast<std::ofstream*>(userp);
    size_t totalSize = size * nmemb;
    out->write(static_cast<char*>(contents), totalSize);
    return totalSize;
}
int progressCallback(void* ptr, curl_off_t totalToDownload, curl_off_t nowDownloaded, curl_off_t totalToUpload, curl_off_t nowUploaded) {
    QProgressDialog* progressDialog = static_cast<QProgressDialog*>(ptr);
    QApplication::processEvents();
    if (totalToDownload > 0) {
        double progress = (nowDownloaded / (double)totalToDownload) * 100.0;
        progressDialog->setValue((int)progress);
    }
    return 0;
}

int progressCallbackBis(void* ptr, curl_off_t totalToDownload, curl_off_t nowDownloaded, curl_off_t totalToUpload, curl_off_t nowUploaded) {
    QProgressDialog* progressDialog = static_cast<QProgressDialog*>(ptr);
    QApplication::processEvents();
    return 0;
}

std::string getLatestGitHubTag(QProgressDialog* progressDialog) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    const std::string& repoOwner = REPO_OWNER;
    const std::string& repoName = REPO_NAME;

    curl = curl_easy_init();
    if (curl) {
        std::string url = "https://api.github.com/repos/" + repoOwner + "/" + repoName + "/releases";
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
        curl_easy_setopt(curl, CURLOPT_SSLVERSION, (long)CURL_SSLVERSION_MAX_DEFAULT);

        // Set timeout time to avoir bug
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");

        curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progressCallbackBis);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(curl, CURLOPT_XFERINFODATA, progressDialog);

        res = curl_easy_perform(curl);
        if (res == CURLE_OPERATION_TIMEDOUT) {
            showWarningMessage(nullptr, "Could not check for update (low connexion)", "Checking for updates");
            qWarning() << "Error : Could not check for update (low connexion) : " << curl_easy_strerror(res);
            curl_easy_cleanup(curl);
            return "";
        }
        if (res != CURLE_OK) {
            qWarning() << "curl_easy_perform() failed: " << curl_easy_strerror(res);
            curl_easy_cleanup(curl);
            return "";
        }
        curl_easy_cleanup(curl);
    }

    // Parse JSON response
    Json::CharReaderBuilder readerBuilder;
    Json::Value root;
    std::string errs;

    std::istringstream s(readBuffer);
    if (Json::parseFromStream(readerBuilder, s, &root, &errs)) {
        if (!root.empty() && root.isArray()) {
            for (const auto& release : root) {
                if (!release["prerelease"].asBool()) {
                    return release["tag_name"].asString();
                }
            }
        }
    } else {
        qWarning() << "Failed to parse JSON: " << errs;
    }

    return "";
}

bool downloadFile(const std::string& url, const std::string& outputPath, QProgressDialog* progressDialog) {
    CURL* curl;
    CURLcode res;
    std::ofstream outFile(outputPath, std::ios::binary);

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
        curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_3);

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallbackAndSave);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &outFile);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progressCallback);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(curl, CURLOPT_XFERINFODATA, progressDialog);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            qWarning() << "curl_easy_perform() failed: " << curl_easy_strerror(res);
        }
        curl_easy_cleanup(curl);
    }

    outFile.close();

    return res == CURLE_OK;
}

bool checkForUpdate(QProgressDialog* progressDialog) {
    if (!fs::exists(SAVE_PATH)) {
        fs::create_directories(SAVE_PATH);
    }
    for (const auto& entry : fs::directory_iterator(SAVE_PATH)) {
        if (entry.path().extension() == ".exe") {
            fs::remove(entry.path());
        }
    }

    std::string latestTag = getLatestGitHubTag(progressDialog);

    int latestMajor = 0;
    int latestMinor = 0;
    int latestPatch = 0;
    if (!latestTag.empty()) {
        std::sscanf(latestTag.c_str(), "v%d.%d.%d", &latestMajor, &latestMinor, &latestPatch);
    }

    int currentMajor = 0;
    int currentMinor = 0;
    int currentPatch = 0;
    std::sscanf(APP_VERSION, "%d.%d.%d", &currentMajor, &currentMinor, &currentPatch);
    if (currentMajor < latestMajor || currentMinor < latestMinor || currentPatch < latestPatch) {
        showQuestionMessage(nullptr, "A new version of the application is available\nDo you want to open the download page?", [latestMajor, latestMinor, latestPatch](bool result) {
                                if (result) {
                                    std::string downloadUrl = "https://github.com/" + std::string(REPO_OWNER) + "/" + std::string(REPO_NAME) + "/releases/download/v" + std::to_string(latestMajor) + "." + std::to_string(latestMinor) + "." + std::to_string(latestPatch) + "/" + std::string(INSTALLER_APP_NAME) + "-" + std::to_string(latestMajor) + "." + std::to_string(latestMinor) + "." + std::to_string(latestPatch) + ".exe";
                                    std::string outputPath = SAVE_PATH + "/" + std::string(INSTALLER_APP_NAME) + "-" + std::to_string(latestMajor) + "." + std::to_string(latestMinor) + "." + std::to_string(latestPatch) + ".exe";

                                    QProgressDialog progressDialog;
                                    progressDialog.setLabelText("Downloading...");
                                    progressDialog.setWindowModality(Qt::ApplicationModal);
                                    progressDialog.setRange(0, 100);
                                    progressDialog.setValue(0);
                                    progressDialog.setCancelButton(nullptr);
                                    progressDialog.show();
                                    downloadFile(downloadUrl, outputPath, &progressDialog);

                                    std::string command = "\"" + outputPath + "\"";

                                    [command, outputPath]() {
                                        QProcess::startDetached(QString::fromStdString(command));
                                    }();
                                    
                                    QApplication::quit();
                                    
                                } }, "Download last version", 0, 0);
        return true;
    }

    qInfo() << "Latest GitHub Tag Version: " << latestMajor << "." << latestMinor << "." << latestPatch;
    qInfo() << "Current App Version: " << currentMajor << "." << currentMinor << "." << currentPatch;

    return false;
}

void startLog() {
    QString logPath = APP_FILES + "/logs";
    QDir logDir(logPath);
    if (!logDir.exists()) {
        if (!logDir.mkpath(".")) {
            qCritical() << "Could not create log directory:" << logDir.absolutePath();
            return;
        }
    }
    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    // std::tm* now_tm;
    // localtime_s(now_tm, &now);
    std::tm* now_tm = std::localtime(&now);

    QString logFileName = QString(APP_FILES + "/logs/%1-%2-%3.log")
                              .arg(now_tm->tm_mday, 2, 10, QChar('0'))
                              .arg(now_tm->tm_mon + 1, 2, 10, QChar('0'))
                              .arg(now_tm->tm_year + 1900);
    static QFile logFile(logFileName);
    if (!logFile.open(QIODevice::WriteOnly | QIODevice::Append)) {
        qCritical() << "Could not open log file for writing:" << logFile.errorString();
        return;
    }
    static QTextStream logStream(&logFile);
    qInstallMessageHandler([](QtMsgType type, const QMessageLogContext& context, const QString& msg) {
        QString formattedMsg = QString("[%1] %2").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"), msg);
        QString colorCode;
        switch (type) {
            case QtDebugMsg:
                colorCode = "\033[34m";  // Blue
                formattedMsg = "[DEBUG   ] " + formattedMsg;
                break;
            case QtInfoMsg:
                colorCode = "\033[32m";  // Green
                formattedMsg = "[INFO    ] " + formattedMsg;
                break;
            case QtWarningMsg:
                colorCode = "\033[33m";  // Yellow
                formattedMsg = "[WARNING ] " + formattedMsg;
                break;
            case QtCriticalMsg:
                colorCode = "\033[31m";  // Red
                formattedMsg = "[CRITICAL] " + formattedMsg;
                break;
            case QtFatalMsg:
                colorCode = "\033[41m";  // Red background
                formattedMsg = "[FATAL   ] " + formattedMsg;
                break;
        }
        logStream << formattedMsg << Qt::endl;
        logStream.flush();
        std::cerr << colorCode.toStdString() << formattedMsg.toStdString() << "\033[0m" << std::endl;  // Reset color
    });
}

void InitialWindow::closeEvent(QCloseEvent* event) {
    if (!data->saved && data->deletedImagesData.get()->size() > 0) {
        showQuestionMessage(this, "Do you want to save before quit ?",
                            [this, event](bool result) {
                                if (imageEditor != nullptr) {
                                    data->removeDeletedImages();
                                }
                            });
    }
    data->stopAllThreads();
    data->saveData();
    event->accept();
}

/**
 * @brief
 * @param data
 */
void InitialWindow::createImageEditor(Data* data) {
    imageEditor = new ImageEditor(this->data, this);

    layout->addWidget(imageEditor);

    connect(imageEditor, &ImageEditor::switchToImageBooth, this, [this]() {
        showImageBooth();
        this->data->addAction(
            [this]() {
                showImageEditor();
            },
            [this]() {
                showImageBooth();
            });
    });
    connect(imageEditor, &ImageEditor::switchToMainWindow, this, [this]() {
        showMainWindow();
        this->data->addAction(
            [this]() {
                showImageEditor();
            },
            [this]() {
                showMainWindow();
            });
    });
}

void InitialWindow::createImageBooth(Data* data) {
    qInfo() << "createImageBooth";
    data->sizes->imagesBoothSizes->imagesPerLine = std::stoi(data->options.at("Sizes::imageBooth::ImagesPerLine").value);
    data->sizes->update();

    imageBooth = new ImageBooth(data, this);

    layout->addWidget(imageBooth);

    connect(imageBooth, &ImageBooth::switchToImageEditor, this, [this]() {
        showImageEditor();
        this->data->addAction(
            [this]() {
                showImageBooth();
            },
            [this]() {
                showImageEditor();
            });
    });
    connect(imageBooth, &ImageBooth::switchToImageBooth, this, [this]() {
        showImageBooth();
        this->data->addAction(
            [this]() {
                showMainWindow();
            },
            [this]() {
                showImageBooth();
            });
    });
    connect(imageBooth, &ImageBooth::switchToMainWindow, this, [this]() {
        showMainWindow();
        this->data->addAction(
            [this]() {
                showImageBooth();
            },
            [this]() {
                showMainWindow();
            });
    });
}

void InitialWindow::createMainWindow(Data* data) {
    mainWindow = new MainWindow(data, this);

    layout->addWidget(mainWindow);

    connect(mainWindow, &MainWindow::switchToImageBooth, this, [this]() {
        showImageBooth();
        this->data->addAction(
            [this]() {
                showMainWindow();
            },
            [this]() {
                showImageBooth();
            });
    });
    connect(mainWindow, &MainWindow::switchToImageEditor, this, [this]() {
        showImageEditor();
        this->data->addAction(
            [this]() {
                showMainWindow();
            },
            [this]() {
                showImageEditor();
            });
    });
}
void InitialWindow::clearImageEditor() {
    data->stopAllThreads();
    layout->removeWidget(imageEditor);
    imageEditor->clear();
    imageEditor->deleteLater();
    imageEditor = nullptr;
}

void InitialWindow::clearImageBooth() {
    data->stopAllThreads();
    layout->removeWidget(imageBooth);
    imageBooth->deleteLater();
    imageBooth = nullptr;
}

void InitialWindow::clearMainWindow() {
    data->stopAllThreads();
    layout->removeWidget(mainWindow);
    mainWindow->deleteLater();
    mainWindow = nullptr;
}

void InitialWindow::showImageEditor() {
    qInfo() << "showImageEditor";
    if (imageBooth != nullptr) {
        clearImageBooth();
    }
    if (mainWindow != nullptr) {
        clearMainWindow();
    }
    createImageEditor(data);
}

void InitialWindow::showImageBooth() {
    qInfo() << "showImageBooth";
    if (imageBooth != nullptr) {
        clearImageBooth();
    }
    if (imageEditor != nullptr) {
        clearImageEditor();
    }
    if (mainWindow != nullptr) {
        clearMainWindow();
    }

    createImageBooth(data);
}

void InitialWindow::showMainWindow() {
    qInfo() << "showMainWindow";
    if (imageEditor != nullptr) {
        clearImageEditor();
    }
    if (imageBooth != nullptr) {
        clearImageBooth();
    }
    createMainWindow(data);
}

ClickableLabel* InitialWindow::createImageDiscord() {
    ClickableLabel* newImageDiscord = new ClickableLabel(data, ICON_PATH_DISCORD, TOOL_TIP_DISCORD, this, linkButton, false, 0, true);
    newImageDiscord->setInitialBackground("transparent", "#b3b3b3");

    connect(newImageDiscord, &ClickableLabel::clicked, [this]() {
        QDesktopServices::openUrl(QUrl("https://discord.gg/Q2HhZucmxU"));
    });

    connect(this, &InitialWindow::resize, newImageDiscord, [this]() {
        ClickableLabel* newImageDiscord = createImageDiscord();
        linkLayout->replaceWidget(imageDiscord, newImageDiscord);
        imageDiscord->deleteLater();
        imageDiscord = newImageDiscord;
    });

    return newImageDiscord;
}

ClickableLabel* InitialWindow::createImageGithub() {
    ClickableLabel* newImageGithub = new ClickableLabel(data, ICON_PATH_GITHUB, TOOL_TIP_GITHUB, this, linkButton, false, 0, true);
    newImageGithub->setInitialBackground("transparent", "#b3b3b3");

    connect(newImageGithub, &ClickableLabel::clicked, [this]() {
        QDesktopServices::openUrl(QUrl("https://github.com/Eugene-74/Open_Image_Editor"));
    });

    connect(this, &InitialWindow::resize, newImageGithub, [this]() {
        ClickableLabel* newImageGithub = createImageGithub();
        linkLayout->replaceWidget(imageGithub, newImageGithub);
        imageGithub->deleteLater();
        imageGithub = newImageGithub;
    });

    return newImageGithub;
}
ClickableLabel* InitialWindow::createImageOption() {
    ClickableLabel* newImageOption = new ClickableLabel(data, ICON_PATH_OPTION, TOOL_TIP_PARAMETER, this, linkButton, false, 0, true);
    newImageOption->setInitialBackground("transparent", "#b3b3b3");

    connect(newImageOption, &ClickableLabel::clicked, [this]() {
        openOption();
    });

    connect(this, &InitialWindow::resize, newImageOption, [this]() {
        ClickableLabel* newImageOption = createImageOption();
        linkLayout->replaceWidget(imageOption, newImageOption);
        imageOption->deleteLater();
        imageOption = newImageOption;
    });

    return newImageOption;
}

void InitialWindow::openOption() {
    if (data->options.size() == 0) {
        data->options = DEFAULT_OPTIONS;
    }

    std::map<std::string, std::string> options = showOptionsDialog(this, "Options", data->options);

    for (const auto& [key, value] : options) {
        data->options[key].value = value;
    }

    // Reload the window
    if (imageBooth) {
        showImageBooth();
    }
}

bool isDarkMode() {
    HKEY hKey;
    DWORD value = 0;
    DWORD valueSize = sizeof(value);
    LONG result = RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", 0, KEY_READ, &hKey);
    if (result == ERROR_SUCCESS) {
        result = RegQueryValueExW(hKey, L"AppsUseLightTheme", nullptr, nullptr, reinterpret_cast<LPBYTE>(&value), &valueSize);
        RegCloseKey(hKey);
    }
    return (result == ERROR_SUCCESS) && (value == 0);
}

void InitialWindow::resizeEvent(QResizeEvent* event) {
    QWidget* widget = QApplication::activeWindow();
    if (dynamic_cast<QMainWindow*>(widget)) {
        if (resizeTimer->isActive()) {
            resizeTimer->stop();
        }
        resizeTimer->start();
    }
    QMainWindow::resizeEvent(event);
}

