#include "Main.hpp"

#include <QStyleHints>

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    auto start = std::chrono::high_resolution_clock::now();
    QApplication app(argc, argv);

    Data* data = new Data();

    InitialWindow window(data);

    window.showMaximized();

    return app.exec();
}
