#include "Main.hpp"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    InitialWindow window;

    window.showMaximized();
    QScreen* screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    window.setGeometry(0, 0, screenGeometry.width(), screenGeometry.height());

    return app.exec();
}
