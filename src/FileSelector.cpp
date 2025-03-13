#include "FileSelector.hpp"

#include <QFileDialog>

QStringList fileSelector::openFileDialog() {
    QStringList fileNames;

    QFileDialog fileDialog(nullptr, "Sélectionner des fichiers");
    fileDialog.setFileMode(QFileDialog::ExistingFiles);

    if (fileDialog.exec()) {
        QStringList selectedFiles = fileDialog.selectedFiles();
        fileNames.append(selectedFiles);
    }
    return fileNames;
}

QStringList fileSelector::openDirectoryDialog() {
    QStringList fileNames;

    QFileDialog dirDialog(nullptr, "Sélectionner des dossiers");
    dirDialog.setFileMode(QFileDialog::Directory);
    dirDialog.setOption(QFileDialog::DontUseNativeDialog, false);
    dirDialog.setOption(QFileDialog::ShowDirsOnly, true);

    if (dirDialog.exec()) {
        QStringList selectedDirs = dirDialog.selectedFiles();
        fileNames.append(selectedDirs);
    }

    return fileNames;
}