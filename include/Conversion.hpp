#pragma once
#include <QComboBox>
#include <QDialog>
#include <QProgressDialog>
#include <memory>

#include "ImageData.hpp"

class ConversionDialog : public QDialog {
    Q_OBJECT

   public:
    ConversionDialog(QWidget* parent = nullptr);

    QString getSelectedFormat() const;

   private:
    std::unique_ptr<QComboBox> comboBox;
};

void launchConversionDialog(ImageData* imageData);
QString launchConversionDialog();

QImage readHeicAndHeif(const std::string& filePath);
bool writeHeicAndHeif(const QImage& image, const std::string& imagePath);

QImage readRaw(const std::string& filePath);
