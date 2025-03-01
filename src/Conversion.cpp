#include "Conversion.hpp"

namespace fs = std::filesystem;

bool convertImageWithMetadata(const std::string& inputPath, const std::string& outputPath) {
    try {
        QImage image;
        if (isHeicOrHeif(inputPath)) {
            image = readHeicAndHeif(inputPath);
        } else if (isRaw(inputPath)) {
            image = readRaw(inputPath);
        } else {
            image.load(QString::fromStdString(inputPath));
        }

        if (image.isNull()) {
            qWarning() << "Could not open or find the image : " << QString::fromStdString(inputPath);
            return false;
        }

        Exiv2::Image::AutoPtr exivImage = Exiv2::ImageFactory::open(inputPath);
        exivImage->readMetadata();

        if (!image.save(QString::fromStdString(outputPath))) {
            qWarning() << "Could not write the image : " << QString::fromStdString(outputPath);
            return false;
        }

        Exiv2::Image::AutoPtr exivOutputImage = Exiv2::ImageFactory::open(outputPath);
        exivOutputImage->readMetadata();

        exivOutputImage->setExifData(exivImage->exifData());
        exivOutputImage->setIptcData(exivImage->iptcData());
        exivOutputImage->setXmpData(exivImage->xmpData());

        exivOutputImage->writeMetadata();

        qInfo() << "Image converted successfully with metadata";
        return true;
    } catch (const Exiv2::Error& e) {
        qCritical() << "Error: " << e.what();
        return false;
    }
}

QImage readHeicAndHeif(const std::string& filename) {
    struct heif_context* ctx = heif_context_alloc();
    struct heif_error err = heif_context_read_from_file(ctx, filename.c_str(), nullptr);

    if (err.code != heif_error_Ok) {
        qCritical() << "Error: Unable to read HEIC/HEIF file: " << filename;
        heif_context_free(ctx);
        return QImage();
    }

    struct heif_image_handle* handle;
    err = heif_context_get_primary_image_handle(ctx, &handle);
    if (err.code != heif_error_Ok) {
        qCritical() << "Error: Unable to get image handle: " << filename;
        heif_context_free(ctx);
        return QImage();
    }

    struct heif_image* img;
    err = heif_decode_image(handle, &img, heif_colorspace_RGB, heif_chroma_interleaved_RGB, nullptr);
    if (err.code != heif_error_Ok) {
        qCritical() << "Erreur : Impossible de décoder l'image : " << filename;
        heif_image_handle_release(handle);
        heif_context_free(ctx);
        return QImage();
    }

    int stride;
    const uint8_t* data = heif_image_get_plane_readonly(img, heif_channel_interleaved, &stride);

    int width = heif_image_get_width(img, heif_channel_interleaved);
    int height = heif_image_get_height(img, heif_channel_interleaved);
    QImage qImage(data, width, height, stride, QImage::Format_RGB888);

    for (int y = 0; y < qImage.height(); ++y) {
        memcpy(qImage.scanLine(y), data + y * stride, qImage.width() * 3);
    }

    heif_image_release(img);
    heif_image_handle_release(handle);
    heif_context_free(ctx);

    return qImage;
}

void launchConversionDialogAndConvert(const QString& inputImagePath) {
    ConversionDialog dialog;
    if (dialog.exec() == QDialog::Accepted) {
        QString selectedFormat = dialog.getSelectedFormat().mid(1);  // Retirer le point (.) du format

        convertion(inputImagePath, selectedFormat);
    }
}

QString launchConversionDialog() {
    ConversionDialog dialog;
    if (dialog.exec() == QDialog::Accepted) {
        QString selectedFormat = dialog.getSelectedFormat().mid(1);
        return selectedFormat;
    }
    return nullptr;
}

void convertion(const QString& inputImagePath, const QString& selectedFormat) {
    QFileInfo fileInfo(inputImagePath);
    QString currentExtension = fileInfo.suffix().toLower();
    if (selectedFormat != currentExtension) {
        QString outputImagePath = fileInfo.path() + "/" + fileInfo.completeBaseName() + "." + selectedFormat;

        if (!convertImageWithMetadata(inputImagePath.toStdString(), outputImagePath.toStdString())) {
            qWarning() << "Erreur : Impossible de convertir l'image avec les métadonnées : " << inputImagePath << " -> " << outputImagePath;
            return;
        }

        qInfo() << "Image convertie avec succès : " << outputImagePath.toStdString();
        return;
    } else {
        qInfo() << "Le format sélectionné est identique au format actuel. Aucune conversion nécessaire.";
    }
}

// TODO non fonctionel
QImage readRaw(const std::string& filename) {
    return QImage();
}