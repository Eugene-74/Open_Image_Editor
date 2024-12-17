#include "Main.h"



namespace fs = std::filesystem;


int main(int argc, char* argv[]) {
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QApplication app(argc, argv);

    InitialWindow window;

    window.showMaximized();

    return app.exec();
}



