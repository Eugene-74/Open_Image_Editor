#include "FileSelector.hpp"

#include <QFileDialog>

/**
 * @brief Open a file dialog to select files
 * @return A list of selected file names
 */
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

/**
 * @brief Open a directory dialog to select directories
 * @return A list of selected directory names
 */
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