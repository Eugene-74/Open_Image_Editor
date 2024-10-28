#include "fileSelector.h"
#include <QFileDialog>

QStringList fileSelector::openFileDialog() {
    // Ouvrir le dialogue de sélection de fichiers
    QStringList fileNames = QFileDialog::getOpenFileNames(
        nullptr,
        "Sélectionner des images",
        "",
        "Images (*.png *.xpm *.jpg *.jpeg *.bmp *.gif)"
    );

    return fileNames;
}