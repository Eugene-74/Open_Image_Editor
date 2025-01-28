#pragma once
#include <string>
#include <iostream>
#include <QImage>
#include <exiv2/exiv2.hpp>
#include <QApplication>
#include <QDialog>
#include <QComboBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QStringList>
#include <QFileInfo>

#include <heif.h>
#include <libraw/libraw.h>

#include "../../functions/verification/Verification.h"

class ConversionDialog : public QDialog {
    Q_OBJECT

public:
    ConversionDialog(QWidget* parent = nullptr) : QDialog(parent) {
        setWindowTitle("Choose Output File Type");

        QVBoxLayout* layout = new QVBoxLayout(this);

        // Liste des formats de sortie possibles
        QStringList formats = { ".png", ".jpeg", ".jpg", ".bmp", ".gif", ".tiff", ".webp" };

        // ComboBox pour choisir le format de sortie
        comboBox = new QComboBox(this);
        comboBox->addItems(formats);
        layout->addWidget(comboBox);

        // Bouton pour valider le choix
        QPushButton* okButton = new QPushButton("OK", this);
        layout->addWidget(okButton);

        connect(okButton, &QPushButton::clicked, this, &ConversionDialog::accept);
    }

    QString getSelectedFormat() const {
        return comboBox->currentText();
    }

private:
    QComboBox* comboBox;
};



void launchConversionDialog(const QString& inputImagePath);


QImage readHEICAndHEIF(const std::string& filename);
QImage readRAW(const std::string& filename);


