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
        if (isExifPath(inputPath) && isExifPath(outputPath)) {
            Exiv2::Image::AutoPtr exivOutputImage = Exiv2::ImageFactory::open(outputPath);
            exivOutputImage->readMetadata();

            exivOutputImage->setExifData(exivImage->exifData());
            exivOutputImage->setIptcData(exivImage->iptcData());
            exivOutputImage->setXmpData(exivImage->xmpData());

            exivOutputImage->writeMetadata();
        }

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

    for (int row = 0; row < qImage.height(); ++row) {
        memcpy(qImage.scanLine(row), data + (row * stride), qImage.width() * 3);
    }

    heif_image_release(img);
    heif_image_handle_release(handle);
    heif_context_free(ctx);

    return qImage;
}

bool writeHeicAndHeif(const QImage& image, const std::string& imagePath) {
    struct heif_context* ctx = heif_context_alloc();
    struct heif_encoder* encoder;
    struct heif_image* img;

    heif_context_get_encoder_for_format(ctx, heif_compression_HEVC, &encoder);

    heif_image_create(image.width(), image.height(), heif_colorspace_RGB, heif_chroma_interleaved_RGB, &img);

    uint8_t* data = heif_image_get_plane(img, heif_channel_interleaved, nullptr);
    for (int y = 0; y < image.height(); ++y) {
        memcpy(data + y * image.bytesPerLine(), image.scanLine(y), image.bytesPerLine());
    }

    struct heif_error err = heif_context_encode_image(ctx, img, encoder, nullptr, nullptr);
    if (err.code != heif_error_Ok) {
        qCritical() << "Error: Unable to encode HEIC/HEIF image: " << imagePath.c_str();
        heif_image_release(img);
        heif_encoder_release(encoder);
        heif_context_free(ctx);
        return false;
    }

    err = heif_context_write_to_file(ctx, imagePath.c_str());
    if (err.code != heif_error_Ok) {
        qCritical() << "Error: Unable to write HEIC/HEIF file: " << imagePath.c_str();
        heif_image_release(img);
        heif_encoder_release(encoder);
        heif_context_free(ctx);
        return false;
    }

    heif_image_release(img);
    heif_encoder_release(encoder);
    heif_context_free(ctx);

    return true;
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
    }
    qInfo() << "Le format sélectionné est identique au format actuel. Aucune conversion nécessaire.";
}

QImage readRaw(const std::string& filename) {
    return QImage();
}