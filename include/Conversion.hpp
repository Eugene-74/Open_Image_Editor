#pragma once

#include <heif.h>

#include <QApplication>
#include <QComboBox>
#include <QDialog>
#include <QFileInfo>
#include <QImage>
#include <QPushButton>
#include <QStringList>
#include <QVBoxLayout>
#include <exiv2/exiv2.hpp>
#include <filesystem>
#include <iostream>
#include <string>

#include "Verification.hpp"

class ConversionDialog : public QDialog {
    Q_OBJECT

   public:
    ConversionDialog(QWidget* parent = nullptr)
        : QDialog(parent) {
        setWindowTitle("Choose Output File Type");

        QVBoxLayout* layout = new QVBoxLayout(this);

        // Liste des formats de sortie possibles
        QStringList formats = {".png", ".jpeg", ".jpg", ".bmp", ".gif", ".tiff", ".webp"};

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

QImage readHeicAndHeif(const std::string& filename);
QImage readRaw(const std::string& filename);
