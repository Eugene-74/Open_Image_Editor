#include "Recover.hpp"

#include <QCoreApplication>
#include <QDebug>
#include <QProcess>
#include <QTimer>
#include <filesystem>

#include "Const.hpp"

void deleteSave() {
    qFatal() << "Deleting save folder and resterting application...";
    QTimer::singleShot(1000, []() {
        QProcess::startDetached("rm", QStringList() << "-rf" << QString::fromUtf8(SAVE_PATH));
        QProcess::startDetached(QCoreApplication::applicationFilePath());
    });
}
