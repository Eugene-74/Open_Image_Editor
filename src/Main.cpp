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
    QString language = locale.section('_', 0, 0);  // Extraire uniquement la partie de la langue
    std::cerr << "System locale:" << locale.toStdString() << std::endl;
    if (translator.load(":/translations/open_image_editor_" + language + ".qm")) {
        std::cerr << "Translation file loaded for language:" << language.toStdString() << std::endl;
        app.installTranslator(&translator);
    } else {
        std::cerr << "Translation file not found for language:" << language.toStdString() << std::endl;
        qDebug() << "Translation file not found for language:" << language;
    }

    // Vérifier si le fichier de traduction existe
    // QString translationFile = ":/translations/open_image_editor_" + language + ".qm";
    // if (QFile::exists(translationFile)) {
    //     std::cerr << "Translation file exists:" << translationFile.toStdString() << std::endl;
    // } else {
    //     std::cerr << "Translation file does not exist:" << translationFile.toStdString() << std::endl;
    // }

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
