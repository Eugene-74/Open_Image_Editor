#include "Conversion.h"



bool convertImageWithMetadata(const std::string& inputPath, const std::string& outputPath) {
    try {
        QImage image;
        // Charger l'image avec Qt
        if (HEICOrHEIF(inputPath)) {
            image = readHEICAndHEIF(inputPath);
        } else if (isRAW(inputPath)) {
            image = readRAW(inputPath);
        } else{
            image.load(QString::fromStdString(inputPath));
        }


        if (image.isNull()) {
            std::cerr << "Could not open or find the image" << std::endl;
            return false;
        }

        // Charger les métadonnées avec Exiv2
        Exiv2::Image::AutoPtr exivImage = Exiv2::ImageFactory::open(inputPath);
        exivImage->readMetadata();

        // Sauvegarder l'image dans le nouveau format avec Qt
        if (!image.save(QString::fromStdString(outputPath))) {
            std::cerr << "Could not write the image" << std::endl;
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
        std::cerr << "Error: " << e.what() << std::endl;
        return false;
    }
}

QImage readHEICAndHEIF(const std::string& filename) {
    struct heif_context* ctx = heif_context_alloc();
    struct heif_error err = heif_context_read_from_file(ctx, filename.c_str(), nullptr);
    if (err.code != heif_error_Ok) {
        std::cerr << "Erreur : Impossible de lire le fichier HEIC/HEIF " << filename << std::endl;
        heif_context_free(ctx);
        return QImage();
    }

    struct heif_image_handle* handle;
    err = heif_context_get_primary_image_handle(ctx, &handle);
    if (err.code != heif_error_Ok) {
        std::cerr << "Erreur : Impossible d'obtenir le handle de l'image" << std::endl;
        heif_context_free(ctx);
        return QImage();
    }

    struct heif_image* img;
    err = heif_decode_image(handle, &img, heif_colorspace_RGB, heif_chroma_interleaved_RGB, nullptr);
    if (err.code != heif_error_Ok) {
        std::cerr << "Erreur : Impossible de décoder l'image" << std::endl;
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

QString launchConversionDialog() {
    // Créer une instance de QApplication si elle n'existe pas déjà
    int argc = 0;
    char* argv[] = { nullptr };
    QApplication app(argc, argv);

    // Créer et afficher la boîte de dialogue
    ConversionDialog dialog;
    if (dialog.exec() == QDialog::Accepted) {
        // Récupérer le format sélectionné
        return dialog.getSelectedFormat();
    }

    // Retourner une chaîne vide si l'utilisateur annule la boîte de dialogue
    return QString();
}


// TODO non fonctionel
QImage readRAW(const std::string& filename) {
    std::cerr << " begin :: : " << filename << std::endl;

    try {
        LibRaw rawProcessor;
        if (rawProcessor.open_file(filename.c_str()) != LIBRAW_SUCCESS) {
            std::cerr << "Erreur : Impossible de lire le fichier RAW " << filename << std::endl;
            throw std::runtime_error("Erreur : Impossible de lire le fichier RAW");
        }

        if (rawProcessor.unpack() != LIBRAW_SUCCESS) {
            std::cerr << "Erreur : Impossible de décompresser le fichier RAW " << filename << std::endl;
            throw std::runtime_error("Erreur : Impossible de décompresser le fichier RAW");
        }

        if (rawProcessor.dcraw_process() != LIBRAW_SUCCESS) {
            std::cerr << "Erreur : Impossible de traiter le fichier RAW " << filename << std::endl;
            throw std::runtime_error("Erreur : Impossible de traiter le fichier RAW");
        }

        libraw_processed_image_t* image = rawProcessor.dcraw_make_mem_image();
        if (!image) {
            std::cerr << "Erreur : Impossible de créer une image à partir du fichier RAW " << filename << std::endl;
            throw std::runtime_error("Erreur : Impossible de créer une image à partir du fichier RAW");
        }

        QImage qImage;
        if (image->type == LIBRAW_IMAGE_BITMAP) {
            qImage = QImage(image->data, image->width, image->height, QImage::Format_RGB888).copy();
        } else {
            std::cerr << "Erreur : Format d'image non supporté " << filename << std::endl;
            throw std::runtime_error("Erreur : Format d'image non supporté");
        }

        libraw_dcraw_clear_mem(image);

        std::cerr << " end :: : " << filename << std::endl;

        return qImage;
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return QImage();
    }
}