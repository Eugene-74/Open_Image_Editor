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
        QStringList formats;
        for (const auto& format : IMAGE_CONVERTION) {
            formats.append(QString::fromStdString(format));
        }

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

    QComboBox* comboBox;

   private:
};

void launchConversionDialogAndConvert(const QString& inputImagePath);
QString launchConversionDialog();
void convertion(const QString& inputImagePath, const QString& selectedFormat);
bool convertImageWithMetadata(const std::string& inputPath, const std::string& outputPath);

QImage readHeicAndHeif(const std::string& filename);
bool writeHeicAndHeif(const QImage& image, const std::string& imagePath);

QImage readRaw(const std::string& filename);
