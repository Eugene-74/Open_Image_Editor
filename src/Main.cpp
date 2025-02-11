#include "Main.hpp"

#include <QStyleHints>

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    auto start = std::chrono::high_resolution_clock::now();
    QApplication app(argc, argv);

    // if (app.styleHints()->colorScheme() == Qt::ColorScheme::Dark) {
    //     qDebug() << "Dark mode";
    // }

    InitialWindow window;

    window.showMaximized();

    return app.exec();
}
