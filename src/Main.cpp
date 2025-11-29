
#include <QApplication>
#include <QCursor>
#include <QDebug>
#include <QFile>
#include <QGuiApplication>
#include <QIcon>
#include <QMessageBox>
#include <QQmlApplicationEngine>
#include <QScreen>

#include "Database.hpp"
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

        // initialize DB (placed in APP_FILES alongside options, thumbnails, etc.)
        if (!initDatabase()) {
            qWarning() << "Database initialization failed.";
        }

        InitialWindow window;
        window.setWindowState(Qt::WindowActive);
        window.showMaximized();
        window.raise();
        window.activateWindow();
        window.setFocus();
        // #ifdef Q_OS_WIN
        // #include <windows.h>
        //         HWND hwnd = reinterpret_cast<HWND>(window.winId());
        //         SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        // #endif
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