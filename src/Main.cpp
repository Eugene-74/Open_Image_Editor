
#include <QApplication>
#include <QCursor>
#include <QDebug>
#include <QFile>
#include <QGuiApplication>
#include <QIcon>
#include <QMessageBox>
#include <QScreen>

#include "InitialWindow.hpp"
#include "Recover.hpp"
#include "Text.hpp"

/**
 * @brief Main function of the application
 * @param argc ...
 * @param argv ...
 * @return ...
 * @details This function initializes the application, sets the window icon, and shows the initial window.
 */
int main(int argc, char* argv[]) {
    try {
        QApplication app(argc, argv);

        app.setWindowIcon(QIcon(":/icons/icon.ico"));

        InitialWindow window;
        window.showMaximized();
        QScreen* screen = QGuiApplication::primaryScreen();

        QRect screenGeometry = screen->geometry();
        window.setGeometry(0, 0, screenGeometry.width(), screenGeometry.height());
        window.setMinimumSize(screenGeometry.width() / 3, screenGeometry.height() / 3);

        return app.exec();
    } catch (const std::exception& e) {
        qFatal() << "Exception caught:" << e.what();
        recover();

    } catch (...) {
        qFatal() << "Unknown exception caught!";
    }
    return -1;
}