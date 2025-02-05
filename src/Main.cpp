#include "Main.h"

namespace fs = std::filesystem;

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    std::cerr << "Downloaded: " << size * nmemb << " bytes" << std::endl;
    return size * nmemb;
}

std::string getLatestGitHubTag(const std::string& repoOwner, const std::string& repoName) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    curl = curl_easy_init();
    if (curl) {
        std::string url = "https://api.github.com/repos/" + repoOwner + "/" + repoName + "/tags";
        std::cerr << "URL: " << url << std::endl;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");  // Nécessaire pour l'API GitHub

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
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
        std::cerr << "Failed to parse JSON: " << errs << std::endl;
    }

    return "";
}

bool downloadFile(const std::string& url, const std::string& outputPath) {
    CURL* curl;
    CURLcode res;
    std::ofstream ofs(outputPath, std::ios::binary);

    if (!ofs.is_open()) {
        std::cerr << "Erreur : Impossible d'ouvrir le fichier en écriture : " << outputPath << std::endl;
        return false;
    }

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ofs);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK) {
            std::cerr << "Erreur lors du téléchargement : " << curl_easy_strerror(res) << std::endl;
            return false;
        }
    } else {
        std::cerr << "Erreur : Impossible d'initialiser CURL." << std::endl;
        return false;
    }

    return true;
}

int main(int argc, char* argv[]) {
    auto start = std::chrono::high_resolution_clock::now();
    QApplication app(argc, argv);

    // Exemple d'utilisation de la fonction
    std::string latestTag = getLatestGitHubTag("Eugene-74", "Open_Image_Editor");

    int latestMajor = 0, latestMinor = 0, latestPatch = 0;
    if (!latestTag.empty()) {
        std::sscanf(latestTag.c_str(), "v%d.%d.%d", &latestMajor, &latestMinor, &latestPatch);
    }

    // Split APP_VERSION into major, minor, and patch integers
    int currentMajor = 0, currentMinor = 0, currentPatch = 0;
    std::sscanf(APP_VERSION, "%d.%d.%d", &currentMajor, &currentMinor, &currentPatch);

    if (currentMajor < latestMajor || currentMinor < latestMinor || currentPatch < latestPatch) {
        if (showQuestionMessage(nullptr, "A new version of the application is available\nDo you want to open the download page?")) {
            // TODO download and run
            std::string downloadUrl = "https://github.com/Eugene-74/Open_Image_Editor/releases/download/v1.0.0/Image_Editor_Installer-1.0.0.exe";
            std::string outputPath = "C:/Users/eugen/Documents/MesDocuments/text.exe";
            // TODO corriger tout ce que je telecharge fait 0 Ko
            downloadFile(downloadUrl, outputPath);
            return 0;
        }
    }

    qDebug() << "Latest GitHub Tag Version: " << latestMajor << "." << latestMinor << "." << latestPatch;
    qDebug() << "Current App Version: " << currentMajor << "." << currentMinor << "." << currentPatch;

    // CREATING LOGS
    QDir logDir("logs");
    if (!logDir.exists()) {
        logDir.mkpath(".");
    }
    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm* now_tm = std::localtime(&now);
    QString logFileName = QString("logs/%1-%2-%3.log")
                              .arg(now_tm->tm_mday, 2, 10, QChar('0'))
                              .arg(now_tm->tm_mon + 1, 2, 10, QChar('0'))
                              .arg(now_tm->tm_year + 1900);
    static QFile logFile(logFileName);
    logFile.open(QIODevice::WriteOnly | QIODevice::Append);
    static QTextStream logStream(&logFile);
    qInstallMessageHandler([](QtMsgType type, const QMessageLogContext& context, const QString& msg) {
        logStream << msg << Qt::endl;
        logStream.flush();
        std::cerr << msg.toStdString() << std::endl;
    });

    qDebug() << "Application started at:" << getCurrentFormattedDate();

    // ouvrir le fichier de traduction
    QTranslator translator;
    QString locale = QLocale::system().name();
    QString language = locale.section('_', 0, 0);  // Extraire uniquement la partie de la langue
    std::cerr << "System locale:" << locale.toStdString() << std::endl;
    if (translator.load(":/translations/open_image_editor_" + language + ".qm")) {
        std::cerr << "Translation file loaded for language:" << language.toStdString() << std::endl;
        app.installTranslator(&translator);
    } else {
        std::cerr << "Translation file not found for language:" << language.toStdString() << std::endl;
        qDebug() << "Translation file not found for language:" << language;
    }

    // Définir le style sur Fusion
    app.setStyle(QStyleFactory::create("Fusion"));
    QPalette lightPalette;
    lightPalette.setColor(QPalette::Window, QColor(255, 255, 255));
    lightPalette.setColor(QPalette::WindowText, QColor(0, 0, 0));
    lightPalette.setColor(QPalette::Base, QColor(240, 240, 240));
    lightPalette.setColor(QPalette::AlternateBase, QColor(255, 255, 255));
    lightPalette.setColor(QPalette::ToolTipBase, QColor(255, 255, 255));
    lightPalette.setColor(QPalette::ToolTipText, QColor(0, 0, 0));
    lightPalette.setColor(QPalette::Text, QColor(0, 0, 0));
    lightPalette.setColor(QPalette::Button, QColor(255, 255, 255));
    lightPalette.setColor(QPalette::ButtonText, QColor(0, 0, 0));
    lightPalette.setColor(QPalette::BrightText, QColor(255, 0, 0));
    lightPalette.setColor(QPalette::Link, QColor(0, 0, 255));
    lightPalette.setColor(QPalette::Highlight, QColor(0, 120, 215));
    lightPalette.setColor(QPalette::HighlightedText, QColor(255, 255, 255));
    app.setPalette(lightPalette);

    InitialWindow window;

    window.showMaximized();

    return app.exec();
}
