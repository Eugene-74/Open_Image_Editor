#include "InitialWindow.hpp"

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    qDebug() << "Downloaded: " << size * nmemb << " bytes";
    return size * nmemb;
}

size_t WriteCallbackBis(void* contents, size_t size, size_t nmemb, void* userp) {
    std::ofstream* out = static_cast<std::ofstream*>(userp);
    size_t totalSize = size * nmemb;
    out->write(static_cast<char*>(contents), totalSize);
    return totalSize;
}

std::string getLatestGitHubTag() {
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

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");  // Nécessaire pour l'API GitHub

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
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

int progressCallback(void* ptr, curl_off_t totalToDownload, curl_off_t nowDownloaded, curl_off_t totalToUpload, curl_off_t nowUploaded) {
    QProgressDialog* progressDialog = static_cast<QProgressDialog*>(ptr);
    if (totalToDownload > 0) {
        double progress = (nowDownloaded / (double)totalToDownload) * 100.0;
        progressDialog->setValue((int)progress);
        QApplication::processEvents();
    }
    return 0;
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
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallbackBis);
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

bool lookForUpdate() {
    for (const auto& entry : fs::directory_iterator(SAVE_PATH)) {
        if (entry.path().extension() == ".exe") {
            fs::remove(entry.path());
        }
    }

    std::string latestTag = getLatestGitHubTag();

    int latestMajor = 0, latestMinor = 0, latestPatch = 0;
    if (!latestTag.empty()) {
        std::sscanf(latestTag.c_str(), "v%d.%d.%d", &latestMajor, &latestMinor, &latestPatch);
    }

    int currentMajor = 0, currentMinor = 0, currentPatch = 0;
    std::sscanf(APP_VERSION, "%d.%d.%d", &currentMajor, &currentMinor, &currentPatch);

    if (currentMajor < latestMajor || currentMinor < latestMinor || currentPatch < latestPatch) {
        if (showQuestionMessage(nullptr, "A new version of the application is available\nDo you want to open the download page?")) {
            // TODO download and run
            std::string downloadUrl = "https://github.com/" + std::string(REPO_OWNER) + "/" + std::string(REPO_NAME) + "/releases/download/v" + std::to_string(latestMajor) + "." + std::to_string(latestMinor) + "." + std::to_string(latestPatch) + "/" + std::string(INSTALLER_APP_NAME) + "-" + std::to_string(latestMajor) + "." + std::to_string(latestMinor) + "." + std::to_string(latestPatch) + ".exe";
            std::string outputPath = SAVE_PATH + "/" + std::string(INSTALLER_APP_NAME) + "-" + std::to_string(latestMajor) + "." + std::to_string(latestMinor) + "." + std::to_string(latestPatch) + ".exe";
            if (!fs::exists(SAVE_PATH)) {
                fs::create_directories(SAVE_PATH);
            }
            QProgressDialog progressDialog;
            progressDialog.setLabelText("Downloading...");
            progressDialog.setRange(0, 100);
            progressDialog.setCancelButton(nullptr);
            progressDialog.show();
            downloadFile(downloadUrl, outputPath, &progressDialog);

            std::string command = "\"" + outputPath + "\"";

            [command, outputPath]() {
                QProcess::startDetached(QString::fromStdString(command));
            }();

            return true;
        }
    }

    qDebug() << "Latest GitHub Tag Version: " << latestMajor << "." << latestMinor << "." << latestPatch;
    qDebug() << "Current App Version: " << currentMajor << "." << currentMinor << "." << currentPatch;
    return false;
}

void startLog() {
    QString logPath = QString::fromUtf8(APPDATA_PATH.toUtf8()) + "/." + QString::fromUtf8(APP_NAME) + "/logs";
    QDir logDir(logPath);
    if (!logDir.exists()) {
        if (!logDir.mkpath(".")) {
            qCritical() << "Could not create log directory:" << logDir.absolutePath();
            return;
        }
    }
    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm* now_tm = std::localtime(&now);
    QString logFileName = QString(APPDATA_PATH.toUtf8() + "/." + QString::fromUtf8(APP_NAME) + "/logs/%1-%2-%3.log")
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

InitialWindow::InitialWindow() {
    // qDebug() << "InitialWindow started at:" << getCurrentFormattedDate();

    startLog();

    QTimer::singleShot(100, this, [this]() {
        if (lookForUpdate()) {
            QApplication::quit();

            return;
        }
        qDebug() << "Application started at:" << getCurrentFormattedDate();

        QTranslator translator;
        QString locale = QLocale::system().name();
        QString language = locale.section('_', 0, 0);  // Extraire uniquement la partie de la langue
        if (translator.load(":/translations/open_image_editor_" + language + ".qm")) {
            // app.installTranslator(&translator);
        } else {
            qDebug() << "Translation file not found for language:" << language;
        }

        // startDlib();

        data = new Data();

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
        QHBoxLayout* linkLayout = new QHBoxLayout();

        linkLayout->addWidget(createImageDiscord());
        linkLayout->addWidget(createImageGithub());
        linkLayout->addWidget(createImageOption());
        linkLayout->setAlignment(Qt::AlignRight);
        windowLayout->addLayout(linkLayout);

        createMainWindow(data);
        // qDebug() << "InitialWindow finished at:" << getCurrentFormattedDate();
    });
}

void InitialWindow::closeEvent(QCloseEvent* event) {
    if (!data->saved && data->deletedImagesData.get()->size() > 0) {
        if (showQuestionMessage(this, "Do you want to save before quit ?")) {
            if (imageEditor != nullptr) {
                data->removeDeletedImages();
            }
        }
    }
    data->saveData();
    event->accept();
}

void InitialWindow::createImageEditor(Data* data) {
    imageEditor = new ImageEditor(data, this);

    layout->addWidget(imageEditor);

    imageEditor->setFocus();
    imageEditor->setFocusPolicy(Qt::StrongFocus);

    connect(imageEditor, &ImageEditor::switchToImageBooth, this, &InitialWindow::showImageBooth);
    connect(imageEditor, &ImageEditor::switchToMainWindow, this, &InitialWindow::showMainWindow);
}

void InitialWindow::createImageBooth(Data* data) {
    imageBooth = new ImageBooth(data, this);

    layout->addWidget(imageBooth);

    imageBooth->setFocus();
    imageBooth->setFocusPolicy(Qt::StrongFocus);

    connect(imageBooth, &ImageBooth::switchToImageEditor, this, &InitialWindow::showImageEditor);
    connect(imageBooth, &ImageBooth::switchToMainWindow, this, &InitialWindow::showMainWindow);
}

void InitialWindow::createMainWindow(Data* data) {
    mainWindow = new MainWindow(data, this);

    layout->addWidget(mainWindow);

    mainWindow->setFocus();
    mainWindow->setFocusPolicy(Qt::StrongFocus);

    connect(mainWindow, &MainWindow::switchToImageBooth, this, &InitialWindow::showImageBooth);
    connect(mainWindow, &MainWindow::switchToImageEditor, this, &InitialWindow::showImageEditor);
}
void InitialWindow::clearImageEditor() {
    layout->removeWidget(imageEditor);
    imageEditor->clear();
    imageEditor = nullptr;
    data->clearActions();
}

void InitialWindow::clearImageBooth() {
    layout->removeWidget(imageBooth);
    imageBooth->clear();
    imageBooth = nullptr;
    data->clearActions();
}

void InitialWindow::clearMainWindow() {
    layout->removeWidget(mainWindow);
    mainWindow->clear();
    mainWindow = nullptr;
    data->clearActions();
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
    qDebug() << 11;

    ClickableLabel* imageDiscord = new ClickableLabel(data, ICON_PATH_DISCORD, TOOL_TIP_DISCORD, this, QSize(data->sizes.linkButtons.width() / 2, data->sizes.linkButtons.height()), false, 0, true);
    qDebug() << 12;

    connect(imageDiscord, &ClickableLabel::clicked, [this]() {
        QDesktopServices::openUrl(QUrl("https://discord.gg/Q2HhZucmxU"));
    });
    qDebug() << 13;

    return imageDiscord;
}

ClickableLabel* InitialWindow::createImageGithub() {
    ClickableLabel* imageGithub = new ClickableLabel(data, ICON_PATH_GITHUB, TOOL_TIP_GITHUB, this, QSize(data->sizes.linkButtons.width() / 2, data->sizes.linkButtons.height()), false, 0, true);

    connect(imageGithub, &ClickableLabel::clicked, [this]() {
        QDesktopServices::openUrl(QUrl("https://github.com/Eugene-74/Easy_Image_Editor"));
    });

    return imageGithub;
}
ClickableLabel* InitialWindow::createImageOption() {
    ClickableLabel* imageOption = new ClickableLabel(data, ICON_PATH_OPTION, TOOL_TIP_PARAMETER, this, QSize(50, 50), false, 0, true);

    connect(imageOption, &ClickableLabel::clicked, [this]() {
        openOption();
    });

    return imageOption;
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
            darkMode = (value == 0);  // 0 means dark mode, 1 means light mode
        }
        RegCloseKey(hKey);
    }
    return darkMode;
}