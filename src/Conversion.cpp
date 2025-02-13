#include "Conversion.hpp"

namespace fs = std::filesystem;

bool convertImageWithMetadata(const std::string& inputPath, const std::string& outputPath) {
    try {
        QImage image;
        // Charger l'image avec Qt
        if (isHeicOrHeif(inputPath)) {
            image = readHeicAndHeif(inputPath);
        } else if (isRaw(inputPath)) {
            image = readRaw(inputPath);
        } else {
            image.load(QString::fromStdString(inputPath));
        }

        if (image.isNull()) {
            qDebug() << "Could not open or find the image : " << QString::fromStdString(inputPath);
            return false;
        }

        // Charger les métadonnées avec Exiv2
        Exiv2::Image::AutoPtr exivImage = Exiv2::ImageFactory::open(inputPath);
        exivImage->readMetadata();

        // Sauvegarder l'image dans le nouveau format avec Qt
        if (!image.save(QString::fromStdString(outputPath))) {
            qDebug() << "Could not write the image : " << QString::fromStdString(outputPath);
            return false;
        }

        // Charger l'image convertie pour y ajouter les métadonnées
        Exiv2::Image::AutoPtr exivOutputImage = Exiv2::ImageFactory::open(outputPath);
        exivOutputImage->readMetadata();

        // Copier les métadonnées de l'image d'origine vers l'image convertie
        exivOutputImage->setExifData(exivImage->exifData());
        exivOutputImage->setIptcData(exivImage->iptcData());
        exivOutputImage->setXmpData(exivImage->xmpData());

        // Sauvegarder les métadonnées dans l'image convertie
        exivOutputImage->writeMetadata();

        std::cout << "Image converted successfully with metadata" << std::endl;
        return true;
    } catch (const Exiv2::Error& e) {
        qDebug() << "Error: " << e.what();
        return false;
    }
}

QImage readHeicAndHeif(const std::string& filename) {
    struct heif_context* ctx = heif_context_alloc();
    struct heif_error err = heif_context_read_from_file(ctx, filename.c_str(), nullptr);

    if (err.code != heif_error_Ok) {
        qDebug() << "Error: Unable to read HEIC/HEIF file: " << filename;
        qDebug() << "HEIF Error Code:" << err.code << ", Message:" << err.message;
        heif_context_free(ctx);
        return QImage();
    }

    struct heif_image_handle* handle;
    err = heif_context_get_primary_image_handle(ctx, &handle);
    if (err.code != heif_error_Ok) {
        qDebug() << "Error: Unable to get image handle: " << filename;
        qDebug() << "HEIF Error Code:" << err.code << ", Message:" << err.message;
        heif_context_free(ctx);
        return QImage();
    }

    struct heif_image* img;
    err = heif_decode_image(handle, &img, heif_colorspace_RGB, heif_chroma_interleaved_RGB, nullptr);
    if (err.code != heif_error_Ok) {
        qDebug() << "Erreur : Impossible de décoder l'image : " << filename;
        heif_image_handle_release(handle);
        heif_context_free(ctx);
        return QImage();
    }

    int stride;
    const uint8_t* data = heif_image_get_plane_readonly(img, heif_channel_interleaved, &stride);

    // Créer un QImage vide avec les mêmes dimensions
    int width = heif_image_get_width(img, heif_channel_interleaved);
    int height = heif_image_get_height(img, heif_channel_interleaved);
    QImage qImage(data, width, height, stride, QImage::Format_RGB888);

    // Copier les données de l'image HEIC dans le QImage
    for (int y = 0; y < qImage.height(); ++y) {
        memcpy(qImage.scanLine(y), data + y * stride, qImage.width() * 3);
    }

    heif_image_release(img);
    heif_image_handle_release(handle);
    heif_context_free(ctx);

    return qImage;
}

void launchConversionDialogAndConvert(const QString& inputImagePath) {
    // Obtenir l'extension actuelle de l'image
    // QFileInfo fileInfo(inputImagePath);
    // QString currentExtension = fileInfo.suffix().toLower();

    // Créer et afficher la boîte de dialogue
    ConversionDialog dialog;
    if (dialog.exec() == QDialog::Accepted) {
        // Récupérer le format sélectionné
        QString selectedFormat = dialog.getSelectedFormat().mid(1);  // Retirer le point (.) du format

        convertion(inputImagePath, selectedFormat);
        // Vérifier si le format sélectionné est différent du format actuel
        // if (selectedFormat != currentExtension) {
        //     // Construire le nouveau chemin de l'image avec le nouveau format
        //     QString outputImagePath = fileInfo.path() + "/" + fileInfo.completeBaseName() + "." + selectedFormat;

        //     // Convertir l'image avec les métadonnées
        //     if (!convertImageWithMetadata(inputImagePath.toStdString(), outputImagePath.toStdString())) {
        //         qDebug() << "Erreur : Impossible de convertir l'image avec les métadonnées : " << inputImagePath << " -> " << outputImagePath;
        //         return;
        //     }

        //     std::cout << "Image convertie avec succès : " << outputImagePath.toStdString() << std::endl;
        //     return;
        // } else {
        //     std::cout << "Le format sélectionné est identique au format actuel. Aucune conversion nécessaire." << std::endl;
        // }
    }
}

QString launchConversionDialog() {
    ConversionDialog dialog;
    if (dialog.exec() == QDialog::Accepted) {
        // Récupérer le format sélectionné
        QString selectedFormat = dialog.getSelectedFormat().mid(1);
        return selectedFormat;
    }
    return nullptr;
}

void convertion(const QString& inputImagePath, const QString& selectedFormat) {
    QFileInfo fileInfo(inputImagePath);
    QString currentExtension = fileInfo.suffix().toLower();
    if (selectedFormat != currentExtension) {
        // Construire le nouveau chemin de l'image avec le nouveau format
        QString outputImagePath = fileInfo.path() + "/" + fileInfo.completeBaseName() + "." + selectedFormat;

        // Convertir l'image avec les métadonnées
        if (!convertImageWithMetadata(inputImagePath.toStdString(), outputImagePath.toStdString())) {
            qDebug() << "Erreur : Impossible de convertir l'image avec les métadonnées : " << inputImagePath << " -> " << outputImagePath;
            return;
        }

        std::cout << "Image convertie avec succès : " << outputImagePath.toStdString() << std::endl;
        return;
    } else {
        std::cout << "Le format sélectionné est identique au format actuel. Aucune conversion nécessaire." << std::endl;
    }
}

// TODO non fonctionel
QImage readRaw(const std::string& filename) {
    try {
        // LibRaw rawProcessor;
        // if (rawProcessor.open_file(filename.c_str()) != LIBRAW_SUCCESS) {
        //     throw std::runtime_error("Erreur : Impossible de lire le fichier RAW");
        // }

        // if (rawProcessor.unpack() != LIBRAW_SUCCESS) {
        //     throw std::runtime_error("Erreur : Impossible de décompresser le fichier RAW");
        // }

        // if (rawProcessor.dcraw_process() != LIBRAW_SUCCESS) {
        //     throw std::runtime_error("Erreur : Impossible de traiter le fichier RAW");
        // }

        // libraw_processed_image_t* image = rawProcessor.dcraw_make_mem_image();
        // if (!image) {
        //     throw std::runtime_error("Erreur : Impossible de créer une image à partir du fichier RAW");
        // }

        // QImage qImage;
        // if (image->type == LIBRAW_IMAGE_BITMAP) {
        //     qImage = QImage(image->data, image->width, image->height, QImage::Format_RGB888).copy();
        // } else {
        //     throw std::runtime_error("Erreur : Format d'image non supporté");
        // }

        // libraw_dcraw_clear_mem(image);

        return QImage();
    } catch (const std::exception& e) {
        qDebug() << "Exception: " << e.what();
        return QImage();
    }
}