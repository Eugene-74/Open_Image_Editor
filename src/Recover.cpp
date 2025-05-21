#include "Recover.hpp"

#include <QCoreApplication>
#include <QDebug>
#include <QDesktopServices>
#include <QFile>
#include <QProcess>
#include <QTimer>
#include <QUrl>
#include <filesystem>

#include "Const.hpp"

void recover() {
    if (QFile(QString::fromUtf8(IMAGESDATA_SAVE_DATA_PATH)).exists()) {
        deleteSaveFile();
    } else if (QFile(QString::fromUtf8(SAVE_PATH)).exists()) {
        deleteSaveFolder();
    } else {
        reDownloadApp();
    }
}

void deleteSaveFile() {
    qFatal() << "Deleting save file and restarting application...";
    QTimer::singleShot(1000, []() {
        QProcess::startDetached("rm", QStringList() << "-rf" << QString::fromUtf8(IMAGESDATA_SAVE_DATA_PATH));
        QProcess::startDetached(QCoreApplication::applicationFilePath());
    });
}

void deleteSaveFolder() {
    qFatal() << "Deleting save folder and restarting application...";
    QTimer::singleShot(1000, []() {
        QProcess::startDetached("rm", QStringList() << "-rf" << QString::fromUtf8(SAVE_PATH));
        QProcess::startDetached(QCoreApplication::applicationFilePath());
    });
}

void reDownloadApp() {
    qFatal() << "Could not recover download the app";
    QDesktopServices::openUrl(QUrl("https://github.com/Eugene-74/Open_Image_Editor/releases/latest"));
}