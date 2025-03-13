#pragma once
#include <QDialog>

// Forward declarations
class QComboBox;

class ConversionDialog : public QDialog {
    Q_OBJECT

   public:
    ConversionDialog(QWidget* parent = nullptr);
    // : QDialog(parent);

    QString getSelectedFormat() const;

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
