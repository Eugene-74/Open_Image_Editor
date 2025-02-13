#include "Main.hpp"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    InitialWindow window;

    window.showMaximized();

    return app.exec();
}
