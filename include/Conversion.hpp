#pragma once
#include <QDialog>
#include <QProgressDialog>

#include "ImageData.hpp"

// Forward declarations
class QComboBox;

class ConversionDialog : public QDialog {
    Q_OBJECT

   public:
    ConversionDialog(QWidget* parent = nullptr);

    QString getSelectedFormat() const;

    QComboBox* comboBox;

   private:
};

void launchConversionDialog(ImageData* imageData);
QString launchConversionDialog();
// void convertion(const QString& inputImagePath, const QString& selectedFormat, QProgressDialog* progressDialog = nullptr);
// bool convertImageWithMetadata(const std::string& inputPath, const std::string& outputPath, QProgressDialog* progressDialog = nullptr);

QImage readHeicAndHeif(const std::string& filePath);
bool writeHeicAndHeif(const QImage& image, const std::string& imagePath);

QImage readRaw(const std::string& filePath);
