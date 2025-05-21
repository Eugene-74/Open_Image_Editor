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
#include "Download.hpp"
#include "ImageBooth.hpp"
#include "ImageEditor.hpp"
#include "MainWindow.hpp"
#include "Network.hpp"
#include "Text.hpp"

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
        emit reload();
    });

    connect(this, &InitialWindow::reload, this, [this]() {
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
    });

    QTimer::singleShot(1, this, [this]() {
        qInfo() << "Application started";
        data = std::make_shared<Data>();

        ImagesData imagesData(std::vector<ImageData*>{});
        ImagesData deletedImagesData(std::vector<ImageData*>{});

        *data->getImagesDataPtr() = imagesData;

        data->deletedImagesData = deletedImagesData;
        data->setDarkMode(isDarkMode());

        try {
            data->loadData();
        } catch (const std::exception& e) {
            qWarning() << "Error loading data: " << e.what();
            showErrorMessage(nullptr, "Error loading data: data corrupted");
        }

        data->checkThumbnailAndDetectObjects();

        if (data->getCurrentFolders() == nullptr) {
            data->setCurrentFolders(data->findFirstFolderWithAllImages());
        }

        const QList<QScreen*> screens = QGuiApplication::screens();
        QScreen* screen = QGuiApplication::primaryScreen();
        screen = screens[0];

        QRect screenR = screen->availableGeometry();

        pixelRatio = screen->devicePixelRatio();

        screenGeometry = screenR.size() / pixelRatio;

        setWindowTitle("OpenImageEditor : Initial Window");

        QVBoxLayout* windowLayout = new QVBoxLayout();

        centralWidget = new QWidget(this);
        centralWidget->setLayout(windowLayout);
        setCentralWidget(centralWidget);

        layout = new QVBoxLayout;
        windowLayout->addLayout(layout);

        linkButton = &data->sizes->linkButton;

        imageLanguage = createImageLanguage();
        imageLanguage->setAlignment(Qt::AlignLeft);

        imageDiscord = createImageDiscord();
        imageGithub = createImageGithub();
        imageOption = createImageOption();

        QLabel* centerText = new QLabel(Text::welcome(), this);
        centerText->setAlignment(Qt::AlignCenter);
        connect(this, &InitialWindow::reload, centerText, [this, centerText]() {
            centerText->setText(Text::welcome());
            QFont font = centerText->font();
            font.setPointSize(data.get()->sizes->fontSize);
            centerText->setFont(font);
        });
        QFont font = centerText->font();
        font.setPointSize(data.get()->sizes->fontSize);
        centerText->setFont(font);
        data->setCenterTextLabel(centerText);

        QHBoxLayout* bottomLayout = new QHBoxLayout();
        linkLayout = new QHBoxLayout();
        languageLayout = new QHBoxLayout();
        bottomLayout->addLayout(languageLayout);
        bottomLayout->addLayout(linkLayout);

        languageLayout->addWidget(imageLanguage);
        languageLayout->setAlignment(Qt::AlignLeft);

        linkLayout->addWidget(imageDiscord);
        linkLayout->addWidget(imageGithub);
        linkLayout->addWidget(imageOption);
        linkLayout->setAlignment(Qt::AlignRight);

        windowLayout->addWidget(centerText);
        windowLayout->addLayout(bottomLayout);

        createMainWindow(data);
        QProgressDialog progressDialog("Checking for updates...", nullptr, 0, 0, this);
        progressDialog.setWindowModality(Qt::ApplicationModal);
        progressDialog.setCancelButton(nullptr);
        progressDialog.show();
        progressDialog.move(0, 0);
        QApplication::processEvents();
        if (hasConnection()) {
            checkForUpdate(&progressDialog);
        } else {
            data->setCenterText("No internet connection, could not check for updates");
        }
        progressDialog.close();
    });
}

/**
 * @brief Check for updates and prompt the user to download the latest version if available
 * @param progressDialog Pointer to the progress dialog for showing download progress
 * @return true if it's updating, false otherwise
 * @details This function checks for the latest version of the application on GitHub and compares it with the current version.
 */
bool checkForUpdate(QProgressDialog* progressDialog) {
    if (!hasConnection()) {
        return false;
    }
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
    if (!data->getSaved() && data->deletedImagesData.get()->size() > 0) {
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
void InitialWindow::createImageEditor(std::shared_ptr<Data> data) {
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
void InitialWindow::createImageBooth(std::shared_ptr<Data> data) {
    qInfo() << "createImageBooth";
    data->sizes->imagesBoothSizes->imagesPerLine = std::stoi(data->getOptionsConst().at("Sizes::imageBooth::ImagesPerLine").getValueConst());
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
void InitialWindow::createMainWindow(std::shared_ptr<Data> data) {
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
    // data->stopAllThreads();
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
    // data->stopAllThreads();
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
    // data->stopAllThreads();
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
 * @brief Create language image label
 * @return ClickableLabel* Pointer to the created ClickableLabel object
 */
ClickableLabel* InitialWindow::createImageLanguage() {
    QString language = QString::fromStdString(data->getOptionsConst().at("Language").getValueConst());
    Text::translationManager.setLanguage(language.toStdString());

    if (language == "en") {
        language = Const::IconPath::Language::EN;
    } else if (language == "fr") {
        language = Const::IconPath::Language::FR;
    } else if (language == "es") {
        language = Const::IconPath::Language::ES;
    } else {
        *(*data.get()->getOptionsPtr())["Language"].getValuePtr() = "en";
        language = Const::IconPath::Language::EN;
    }

    ClickableLabel* newImageLanguage = new ClickableLabel(data, language, Text::Tooltip::language(), this, linkButton, false, 0, true);
    newImageLanguage->setInitialBackground(Const::Color::TRANSPARENT1, Const::Color::LIGHT_GRAY);

    connect(newImageLanguage, &ClickableLabel::clicked, [this]() {
        openOption();
    });

    connect(this, &InitialWindow::reload, newImageLanguage, [this]() {
        ClickableLabel* newImageLanguage = createImageLanguage();
        languageLayout->replaceWidget(imageLanguage, newImageLanguage);
        imageLanguage->deleteLater();
        imageLanguage = newImageLanguage;
    });

    return newImageLanguage;
}

/**
 * @brief Create the Discord image label
 * @return ClickableLabel* Pointer to the created ClickableLabel object
 * @details This function creates a clickable label to open the Discord server link.
 * @details It also sets up a signal to handle the reload event and update the label accordingly.
 */
ClickableLabel* InitialWindow::createImageDiscord() {
    ClickableLabel* newImageDiscord = new ClickableLabel(data, Const::IconPath::DISCORD, Text::Tooltip::discord(), this, linkButton, false, 0, true);
    newImageDiscord->setInitialBackground(Const::Color::TRANSPARENT1, Const::Color::LIGHT_GRAY);

    connect(newImageDiscord, &ClickableLabel::clicked, [this]() {
        QDesktopServices::openUrl(QUrl("https://discord.gg/Q2HhZucmxU"));
    });

    connect(this, &InitialWindow::reload, newImageDiscord, [this]() {
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
 * @details It also sets up a signal to handle the reload event and update the label accordingly.
 */
ClickableLabel* InitialWindow::createImageGithub() {
    ClickableLabel* newImageGithub = new ClickableLabel(data, Const::IconPath::GITHUB, Text::Tooltip::github(), this, linkButton, false, 0, true);
    newImageGithub->setInitialBackground(Const::Color::TRANSPARENT1, Const::Color::LIGHT_GRAY);

    connect(newImageGithub, &ClickableLabel::clicked, [this]() {
        QDesktopServices::openUrl(QUrl("https://github.com/Eugene-74/Open_Image_Editor"));
    });

    connect(this, &InitialWindow::reload, newImageGithub, [this]() {
        ClickableLabel* newImageGithub = createImageGithub();
        linkLayout->replaceWidget(imageGithub, newImageGithub);
        imageGithub->deleteLater();
        imageGithub = newImageGithub;
    });

    return newImageGithub;
}
ClickableLabel* InitialWindow::createImageOption() {
    ClickableLabel* newImageOption = new ClickableLabel(data, Const::IconPath::OPTION, Text::Tooltip::option(), this, linkButton, false, 0, true);
    newImageOption->setInitialBackground(Const::Color::TRANSPARENT1, Const::Color::LIGHT_GRAY);

    connect(newImageOption, &ClickableLabel::clicked, [this]() {
        openOption();
    });

    connect(this, &InitialWindow::reload, newImageOption, [this]() {
        ClickableLabel* newnewImageOption = createImageOption();
        linkLayout->replaceWidget(imageOption, newnewImageOption);
        imageOption->deleteLater();
        imageOption = newnewImageOption;
    });

    return newImageOption;
}

/**
 * @brief Open the options dialog and update the options
 * @details This function opens the options dialog and allows the user to modify the application options.
 */
void InitialWindow::openOption() {
    if (data->getOptionsConst().size() == 0) {
        *data->getOptionsPtr() = DEFAULT_OPTIONS;
    }

    std::map<std::string, std::string> options = showOptionsDialog(this, "Options", data->getOptionsConst());

    for (const auto& [key, value] : options) {
        *(*data->getOptionsPtr())[key].getValuePtr() = value;
    }

    Text::translationManager.setLanguage(data->getOptionsConst().at("Language").getValueConst());

    emit reload();
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