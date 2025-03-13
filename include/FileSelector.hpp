#pragma once

// class QStringList;
#include <QStringList>

class fileSelector {
public:
    QStringList openFileDialog();
    QStringList openDirectoryDialog();
};

