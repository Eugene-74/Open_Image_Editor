#include "FileSelector.h"
#include <QFileDialog>

QStringList fileSelector::openFileDialog() {
    QStringList fileNames;

    // Ouvrir le dialogue de sélection de fichiers
    QFileDialog fileDialog(nullptr, "Sélectionner des fichiers");
    fileDialog.setFileMode(QFileDialog::ExistingFiles); // Permet de sélectionner des fichiers existants
    fileDialog.setOption(QFileDialog::DontUseNativeDialog, false); // Utiliser le dialogue non natif pour permettre la sélection multiple

    // Afficher le dialogue et récupérer les chemins des fichiers sélectionnés
    if (fileDialog.exec()) {
        QStringList selectedFiles = fileDialog.selectedFiles();
        fileNames.append(selectedFiles);
    }
    return fileNames;

}

QStringList fileSelector::openDirectoryDialog() {
    QStringList fileNames;

    // Ouvrir le dialogue de sélection de dossiers
    QFileDialog dirDialog(nullptr, "Sélectionner des dossiers");
    dirDialog.setFileMode(QFileDialog::Directory); // Permet de sélectionner des dossiers
    dirDialog.setOption(QFileDialog::DontUseNativeDialog, false); // Utiliser le dialogue non natif pour permettre la sélection multiple
    dirDialog.setOption(QFileDialog::ShowDirsOnly, true); // Afficher uniquement les dossiers

    // Afficher le dialogue et récupérer les chemins des dossiers sélectionnés
    if (dirDialog.exec()) {
        QStringList selectedDirs = dirDialog.selectedFiles();
        fileNames.append(selectedDirs);
    }


    return fileNames;
}