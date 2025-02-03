#include "Main.h"



namespace fs = std::filesystem;


int main(int argc, char* argv[]) {

    auto start = std::chrono::high_resolution_clock::now();
    QApplication app(argc, argv);

    // fichier log avec Qt debug
    QDir logDir("logs");
    if (!logDir.exists()) {
        logDir.mkpath(".");
    }
    static QFile logFile("logs/application.log");
    logFile.open(QIODevice::WriteOnly | QIODevice::Append);
    static QTextStream logStream(&logFile);
    qInstallMessageHandler([](QtMsgType type, const QMessageLogContext& context, const QString& msg) {
        logStream << msg << Qt::endl;
        logStream.flush();
        });

    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm* now_tm = std::localtime(&now);
    qDebug() << "Application started at:" << getCurrentFormattedDate();


    // ouvrir le fichier de traduction
    QTranslator translator;
    QString locale = QLocale::system().name();
    qDebug() << "System locale:" << locale;
    if (translator.load(":/translations/Open_Image_Editor_" + locale)) {
        app.installTranslator(&translator);
    } else {
        qDebug() << "Translation file not found for locale:" << locale;
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


    // Appliquer la palette de couleurs claires
    app.setPalette(lightPalette);

    InitialWindow window;

    window.showMaximized();

    return app.exec();
}


