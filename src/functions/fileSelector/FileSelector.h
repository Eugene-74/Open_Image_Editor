#pragma once


#include <QStringList>

class fileSelector {
public:
    QStringList openFileDialog();
    QStringList openDirectoryDialog();
};

