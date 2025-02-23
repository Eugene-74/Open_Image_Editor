#include "Main.hpp"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    // TODO ne marche pas apres une release
    if (isDarkMode()) {
        app.setWindowIcon(QIcon(":/255-255-255-255/icons/icon.ico"));
    } else {
        app.setWindowIcon(QIcon(":/0-0-0-255/icons/icon.ico"));
    }

    InitialWindow window;
    qDebug() << "InitialWindow window;";
    window.showMaximized();
    qDebug() << "window.showMaximized();";
    QScreen* screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    window.setGeometry(0, 0, screenGeometry.width(), screenGeometry.height());
    window.setMinimumSize(screenGeometry.width() / 3, screenGeometry.height() / 3);

    return app.exec();
}
