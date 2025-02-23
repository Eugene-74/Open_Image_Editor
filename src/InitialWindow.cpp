#include "InitialWindow.hpp"

InitialWindow::InitialWindow() {
    startLog();

    resizeTimer = new QTimer(this);
    resizeTimer->setInterval(TIME_BEFORE_REZISE);
    resizeTimer->setSingleShot(true);
    connect(resizeTimer, &QTimer::timeout, this, [this]() {
        if (imageEditor) {
            data->sizes.update();
            clearImageEditor();
            createImageEditor(data);
        }
        if (imageBooth) {
            data->sizes.update();
            clearImageBooth();
            createImageBooth(data);
        }
        if (mainWindow) {
            data->sizes.update();
            clearMainWindow();
            createMainWindow(data);
        }
        emit resize();
    });

    QTimer::singleShot(1, this, [this]() {
        qDebug() << "Application started at:" << getCurrentFormattedDate();
        data = new Data();

        QTranslator translator;
        QString locale = QLocale::system().name();
        QString language = locale.section('_', 0, 0);
        if (translator.load(":/translations/open_image_editor_" + language + ".qm")) {
            // app.installTranslator(&translator);
        } else {
            qDebug() << "Translation file not found for language:" << language;
        }

        ImagesData imagesData(std::vector<ImageData>{});
        ImagesData deletedImagesData(std::vector<ImageData>{});
        data->imageCache = new std::map<std::string, QImageAndPath>();

        data->imagesData = imagesData;
        data->deletedImagesData = deletedImagesData;
        data->darkMode = isDarkMode();

        try {
            data->loadData();
        } catch (const std::exception& e) {
            qDebug() << "Error loading data: " << e.what();
            showErrorMessage(nullptr, "Error loading data: data corrupted");
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

        linkButton = &data->sizes.linkButton;
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
    });
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
    if (totalToDownload > 0) {
        double progress = (nowDownloaded / (double)totalToDownload) * 100.0;
        progressDialog->setValue((int)progress);
        QApplication::processEvents();
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
        std::string url = "https://api.github.com/repos/" + repoOwner + "/" + repoName + "/tags";
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

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
            qDebug() << "Error : Could not check for update (low connexion) : " << curl_easy_strerror(res);
        } else if (res != CURLE_OK) {
            qDebug() << "curl_easy_perform() failed: " << curl_easy_strerror(res);
        }
        curl_easy_cleanup(curl);
    }

    // Parse JSON response
    Json::CharReaderBuilder readerBuilder;
    Json::Value root;
    std::string errs;

    std::istringstream s(readBuffer);
    if (Json::parseFromStream(readerBuilder, s, &root, &errs)) {
        if (!root.empty() && root.isArray() && root.size() > 0) {
            return root[0]["name"].asString();
        }
    } else {
        qDebug() << "Failed to parse JSON: " << errs;
    }

    return "";
}

bool downloadFile(const std::string& url, const std::string& outputPath, QProgressDialog* progressDialog) {
    CURL* curl;
    CURLcode res;
    std::ofstream outFile(outputPath, std::ios::binary);

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallbackAndSave);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &outFile);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progressCallback);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(curl, CURLOPT_XFERINFODATA, progressDialog);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            qDebug() << "curl_easy_perform() failed: " << curl_easy_strerror(res);
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

    int latestMajor = 0, latestMinor = 0, latestPatch = 0;
    if (!latestTag.empty()) {
        std::sscanf(latestTag.c_str(), "v%d.%d.%d", &latestMajor, &latestMinor, &latestPatch);
    }

    int currentMajor = 0, currentMinor = 0, currentPatch = 0;
    std::sscanf(APP_VERSION, "%d.%d.%d", &currentMajor, &currentMinor, &currentPatch);
    if (currentMajor < latestMajor || currentMinor < latestMinor || currentPatch < latestPatch) {
        showQuestionMessage(nullptr, "A new version of the application is available\nDo you want to open the download page?", [latestMajor, latestMinor, latestPatch](bool result) {
                                if (result) {
                                    std::string downloadUrl = "https://github.com/" + std::string(REPO_OWNER) + "/" + std::string(REPO_NAME) + "/releases/download/v" + std::to_string(latestMajor) + "." + std::to_string(latestMinor) + "." + std::to_string(latestPatch) + "/" + std::string(INSTALLER_APP_NAME) + "-" + std::to_string(latestMajor) + "." + std::to_string(latestMinor) + "." + std::to_string(latestPatch) + ".exe";
                                    std::string outputPath = SAVE_PATH + "/" + std::string(INSTALLER_APP_NAME) + "-" + std::to_string(latestMajor) + "." + std::to_string(latestMinor) + "." + std::to_string(latestPatch) + ".exe";

                                    QProgressDialog progressDialog;
                                    progressDialog.setWindowModality(Qt::ApplicationModal);
                                    progressDialog.setLabelText("Downloading...");
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

    qDebug() << "Latest GitHub Tag Version: " << latestMajor << "." << latestMinor << "." << latestPatch;
    qDebug() << "Current App Version: " << currentMajor << "." << currentMinor << "." << currentPatch;

    return false;
}

void startLog() {
    QString logPath = QString::fromUtf8(APPDATA_PATH.toUtf8()) + "/" + QString::fromUtf8(APP_NAME) + "/logs";
    QDir logDir(logPath);
    if (!logDir.exists()) {
        if (!logDir.mkpath(".")) {
            qCritical() << "Could not create log directory:" << logDir.absolutePath();
            return;
        }
    }
    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm* now_tm = std::localtime(&now);
    QString logFileName = QString(APPDATA_PATH.toUtf8() + "/" + QString::fromUtf8(APP_NAME) + "/logs/%1-%2-%3.log")
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
        logStream << msg << Qt::endl;
        logStream.flush();
        std::cerr << msg.toStdString() << std::endl;
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
    data->saveData();
    event->accept();
}

void InitialWindow::createImageEditor(Data* data) {
    imageEditor = new ImageEditor(data, this);

    layout->addWidget(imageEditor);

    imageEditor->setFocus();
    imageEditor->setFocusPolicy(Qt::StrongFocus);

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
    imageBooth = new ImageBooth(data, this);

    layout->addWidget(imageBooth);

    imageBooth->setFocus();
    imageBooth->setFocusPolicy(Qt::StrongFocus);

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

    mainWindow->setFocus();
    mainWindow->setFocusPolicy(Qt::StrongFocus);

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
    layout->removeWidget(imageEditor);
    imageEditor->clear();
    imageEditor->deleteLater();
    imageEditor = nullptr;
}

void InitialWindow::clearImageBooth() {
    layout->removeWidget(imageBooth);
    imageBooth->clear();
    imageBooth->deleteLater();
    imageBooth = nullptr;
}

void InitialWindow::clearMainWindow() {
    layout->removeWidget(mainWindow);
    mainWindow->clear();
    mainWindow->deleteLater();
    mainWindow = nullptr;
}

void InitialWindow::showImageEditor() {
    qDebug() << "showImageEditor";
    if (imageBooth != nullptr) {
        clearImageBooth();
    }
    if (mainWindow != nullptr) {
        clearMainWindow();
    }
    createImageEditor(data);
}

void InitialWindow::showImageBooth() {
    qDebug() << "showImageBooth";
    if (imageEditor != nullptr) {
        clearImageEditor();
    }
    if (mainWindow != nullptr) {
        clearMainWindow();
    }

    createImageBooth(data);
}

void InitialWindow::showMainWindow() {
    qDebug() << "showMainWindow";
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
    std::map<std::string, std::string> options = showOptionsDialog(this, "Options", DEFAULT_OPTIONS);

    for (const auto& [key, value] : options) {
        qDebug() << key << " : " << value;
    }
}

bool isDarkMode() {
    bool darkMode = false;
    HKEY hKey;
    LONG result = RegOpenKeyEx(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", 0, KEY_READ, &hKey);
    if (result == ERROR_SUCCESS) {
        DWORD value = 0;
        DWORD valueSize = sizeof(value);
        result = RegQueryValueEx(hKey, L"AppsUseLightTheme", nullptr, nullptr, reinterpret_cast<LPBYTE>(&value), &valueSize);
        if (result == ERROR_SUCCESS) {
            darkMode = (value == 0);
        }
        RegCloseKey(hKey);
    }
    return darkMode;
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