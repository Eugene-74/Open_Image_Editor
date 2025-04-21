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

/**
 * @brief Constructor for the InitialWindow class
 * @details This constructor initializes the initialWindow of the application.
 */
InitialWindow::InitialWindow() {
    startLog();

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

/**
 * @brief Check for files to download
 * @details This function checks if the files to download are present in the application directory.
 * @details If not, it downloads them from the specified URL and saves them in the application directory.
 * @details It uses a progress dialog to show the download progress.
 */
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

/**
 * @brief Callback function for writing data received from the server
 * @param contents Pointer to the data received from the server
 * @param size Size of each data element
 * @param nmemb Number of data elements
 * @param userp Pointer to the user-defined data (in this case, a string)
 * @return The size of the data written
 */
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

/**
 * @brief Callback function for writing data received from the server and saving it to a file
 * @param contents Pointer to the data received from the server
 * @param size Size of each data element
 * @param nmemb Number of data elements
 * @param userp Pointer to the user-defined data (in this case, an ofstream object)
 * @return The size of the data written
 */
size_t WriteCallbackAndSave(void* contents, size_t size, size_t nmemb, void* userp) {
    std::ofstream* out = static_cast<std::ofstream*>(userp);
    static_cast<std::ofstream*>(userp);
    size_t totalSize = size * nmemb;
    out->write(static_cast<char*>(contents), totalSize);
    return totalSize;
}

/**
 * @brief Callback function for progress updates during file download (shows the progress)
 * @param ptr Pointer to the user-defined data (in this case, a QProgressDialog object)
 * @param totalToDownload Total size of the data to be downloaded
 * @param nowDownloaded Size of the data downloaded so far
 * @param totalToUpload Total size of the data to be uploaded (not used in this case)
 * @param nowUploaded Size of the data uploaded so far (not used in this case)
 * @return 0 to continue, non-zero to abort the operation
 * @details This function is used to update the progress dialog during the download process.
 */
int progressCallback(void* ptr, curl_off_t totalToDownload, curl_off_t nowDownloaded, curl_off_t totalToUpload, curl_off_t nowUploaded) {
    QProgressDialog* progressDialog = static_cast<QProgressDialog*>(ptr);
    QApplication::processEvents();
    if (totalToDownload > 0) {
        double progress = (nowDownloaded / (double)totalToDownload) * 100.0;
        progressDialog->setValue((int)progress);
    }
    return 0;
}

/**
 * @brief Callback function for progress updates during file download (alternative version (dosn't shows the progress))
 * @param ptr Pointer to the user-defined data (in this case, a QProgressDialog object)
 * @param totalToDownload Total size of the data to be downloaded
 * @param nowDownloaded Size of the data downloaded so far
 * @param totalToUpload Total size of the data to be uploaded (not used in this case)
 * @param nowUploaded Size of the data uploaded so far (not used in this case)
 * @return 0 to continue, non-zero to abort the operation
 * @details This function is used to update the progress dialog during the download process.
 */
int progressCallbackBis(void* ptr, curl_off_t totalToDownload, curl_off_t nowDownloaded, curl_off_t totalToUpload, curl_off_t nowUploaded) {
    QProgressDialog* progressDialog = static_cast<QProgressDialog*>(ptr);
    QApplication::processEvents();
    return 0;
}

/**
 * @brief Get the latest GitHub tag from the repository (without pre-release)
 * @param progressDialog Pointer to the progress dialog for showing download progress
 * @return The latest GitHub tag as a string
 * @details This function uses the GitHub API to get the latest release tag from the specified repository.
 */
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
        curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_3);

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

/**
 * @brief Download a file from the specified URL and save it to the specified output path
 * @param url The URL of the file to download
 * @param outputPath The path where the downloaded file will be saved
 * @param progressDialog Pointer to the progress dialog for showing download progress
 * @return true if the download was successful, false otherwise
 */
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

/**
 * @brief Check for updates and prompt the user to download the latest version if available
 * @param progressDialog Pointer to the progress dialog for showing download progress
 * @return true if it's updating, false otherwise
 * @details This function checks for the latest version of the application on GitHub and compares it with the current version.
 */
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

/**
 * @brief Start the log file and remove old log files
 * @details This function creates a log file in the specified directory and sets up a message handler to log messages to the file.
 */
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
    // Remove log files older than 1 month if there are more than 10
    QFileInfoList logFiles = logDir.entryInfoList(QDir::Files, QDir::Time);
    const int maxLogFiles = 10;
    const QDateTime oneMonthAgo = QDateTime::currentDateTime().addMonths(-1);

    if (logFiles.size() > maxLogFiles) {
        for (const QFileInfo& fileInfo : logFiles) {
            if (fileInfo.lastModified() < oneMonthAgo) {
                QFile::remove(fileInfo.filePath());
            }
        }
    }
}

/**
 * @brief Handle the close event of the window
 * @param event Pointer to the close event
 * @details This function is called when the window is closed. It checks if there are unsaved changes and prompts the user to save before quitting.
 */
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
 * @brief Create the imageEditor widget
 * @param data Pointer to the Data object
 * @details This function creates the image editor widget and connects its signals to the appropriate slots.
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

/**
 * @brief Create the imageBooth widget
 * @param data Pointer to the Data object
 * @details This function creates the image booth widget and connects its signals to the appropriate slots.
 * @details It also sets the number of images per line based on the configuration options.
 */
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

/**
 * @brief Create the mainWindow widget
 * @param data Pointer to the Data object
 * @details This function creates the main window widget and connects its signals to the appropriate slots.
 */
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

/**
 * @brief Clear the imageEditor widget
 * @details This function removes the image editor widget from the layout and deletes it.
 * @details It also stops all threads associated with the data object.
 */
void InitialWindow::clearImageEditor() {
    data->stopAllThreads();
    layout->removeWidget(imageEditor);
    imageEditor->clear();
    imageEditor->deleteLater();
    imageEditor = nullptr;
}

/**
 * @brief Clear the imageBooth widget
 * @details This function removes the image booth widget from the layout and deletes it.
 * @details It also stops all threads associated with the data object.
 */
void InitialWindow::clearImageBooth() {
    data->stopAllThreads();
    layout->removeWidget(imageBooth);
    imageBooth->deleteLater();
    imageBooth = nullptr;
}

/**
 * @brief Clear the mainWindow widget
 * @details This function removes the main window widget from the layout and deletes it.
 * @details It also stops all threads associated with the data object.
 */
void InitialWindow::clearMainWindow() {
    data->stopAllThreads();
    layout->removeWidget(mainWindow);
    mainWindow->deleteLater();
    mainWindow = nullptr;
}

/**
 * @brief Show the imageEditor widget
 * @details This function clears the existing windows and creates a new imageEditor widget.
 */
void InitialWindow::showImageEditor() {
    qInfo() << "showImageEditor";
    clearWindows();
    createImageEditor(data);
}

/**
 * @brief Show the imageBooth widget
 * @details This function clears the existing windows and creates a new imageBooth widget.
 */
void InitialWindow::showImageBooth() {
    qInfo() << "showImageBooth";
    clearWindows();
    createImageBooth(data);
}

/**
 * @brief Show the mainWindow widget
 * @details This function clears the existing windows and creates a new mainWindow widget.
 */
void InitialWindow::showMainWindow() {
    qInfo() << "showMainWindow";
    clearWindows();
    createMainWindow(data);
}

/**
 * @brief Create the Discord image label
 * @return ClickableLabel* Pointer to the created ClickableLabel object
 * @details This function creates a clickable label to open the Discord server link.
 * @details It also sets up a signal to handle the resize event and update the label accordingly.
 */
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

/**
 * @brief Create the GitHub image label
 * @return ClickableLabel* Pointer to the created ClickableLabel object
 * @details This function creates a clickable label to open the GitHub repository link.
 * @details It also sets up a signal to handle the resize event and update the label accordingly.
 */
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

/**
 * @brief Open the options dialog and update the options
 * @details This function opens the options dialog and allows the user to modify the application options.
 */
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

/**
 * @brief Check if the system is in dark mode
 * @return true if the system is in dark mode, false otherwise
 */
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

/**
 * @brief Handle the resize event of the window
 * @param event Pointer to the resize event
 * @details This function is called when the window is resized. It starts a timer to delay the resize event handling.
 */
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

/**
 * @brief Clear all windows (imageEditor, imageBooth, mainWindow)
 */
void InitialWindow::clearWindows() {
    if (imageEditor != nullptr) {
        clearImageEditor();
    }
    if (imageBooth != nullptr) {
        clearImageBooth();
    }
    if (mainWindow != nullptr) {
        clearMainWindow();
    }
}