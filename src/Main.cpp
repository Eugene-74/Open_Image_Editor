#include "Main.h"

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    auto start = std::chrono::high_resolution_clock::now();
    QApplication app(argc, argv);

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

    // Créer une palette de couleurs claires
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
