#include "Main.hpp"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    if (isDarkMode()) {
        app.setWindowIcon(QIcon(":/255-255-255-255/icons/icon.ico"));
    } else {
        app.setWindowIcon(QIcon(":/0-0-0-255/icons/icon.ico"));
    }

    InitialWindow window;

    window.showMaximized();
    QScreen* screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    window.setGeometry(0, 0, screenGeometry.width(), screenGeometry.height());

    return app.exec();
}
