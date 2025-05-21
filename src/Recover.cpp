#include "Recover.hpp"

#include <QCoreApplication>
#include <QDebug>
#include <QProcess>
#include <QTimer>
#include <filesystem>

#include "Const.hpp"

void deleteSave() {
    qFatal() << "Deleting save file and resterting application...";
    QTimer::singleShot(1000, []() {
        // TODO mettre en const
        QProcess::startDetached("rm", QStringList() << "-rf" << QString::fromUtf8(SAVE_PATH + "save.OpenImageEditor"));
        QProcess::startDetached(QCoreApplication::applicationFilePath());
    });
}

