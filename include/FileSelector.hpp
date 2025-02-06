#pragma once


#include <QStringList>
#include <QFileDialog>


class fileSelector {
public:
    QStringList openFileDialog();
    QStringList openDirectoryDialog();
};

