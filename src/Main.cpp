#include "Main.hpp"

#include <QApplication>
#include <QCursor>
#include <QDebug>
#include <QGuiApplication>
#include <QIcon>
#include <QScreen>

#include "InitialWindow.hpp"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    app.setWindowIcon(QIcon(":/icons/icon.ico"));

    InitialWindow window;
    window.showMaximized();
    QScreen* screen = QGuiApplication::screenAt(QCursor::pos());

    QRect screenGeometry = screen->geometry();
    window.setGeometry(0, 0, screenGeometry.width(), screenGeometry.height());
    window.setMinimumSize(screenGeometry.width() / 3, screenGeometry.height() / 3);

    return app.exec();
}
