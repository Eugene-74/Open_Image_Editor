#include "Main.h"



namespace fs = std::filesystem;


int main(int argc, char* argv[]) {

    auto start = std::chrono::high_resolution_clock::now();
    QApplication app(argc, argv);

    // QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

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



