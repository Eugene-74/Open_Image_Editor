#include "imageEditor.h"


// #include "../../functions/clickableLabel/ClickableLabel.h"

ClickableLabel::ClickableLabel(const QString& imagePath, QWidget* parent, QSize size)
    : QLabel(parent) {
    // Load image using OpenCV with alpha channel
    cv::Mat image = cv::imread(imagePath.toStdString(), cv::IMREAD_UNCHANGED);

    size = (size - QSize(10, 10));

    if (!image.empty()) {

        // Convert BGR to RGB (if needed) and keep alpha channel
        if (image.channels() == 4) {
            // Create QImage from OpenCV Mat with alpha channel
            QImage qImage(image.data, image.cols, image.rows, image.step[0], QImage::Format_ARGB32);
            // Set the pixmap with scaling
            this->setPixmap(QPixmap::fromImage(qImage).scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
        else {
            // Handle images without an alpha channel (optional)
            cv::cvtColor(image, image, cv::COLOR_BGR2RGB);

            cv::Mat resizedImage;

            // cv::resize(image, resizedImage, cv::Size(size.width(), size.height()), 0, 0, cv::INTER_CUBIC); // Utilisez INTER_LINEAR pour un redimensionnement rapide
            // QImage qImage(resizedImage.data, resizedImage.cols, resizedImage.rows, resizedImage.step[0], QImage::Format_RGB888);
            QImage qImage(image.data, image.cols, image.rows, image.step[0], QImage::Format_RGB888);
            this->setPixmap(QPixmap::fromImage(qImage).scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }

    }
    else {
        // Handle the case where the image is not valid (optional)
        this->setText("Erreur : Image non valide !");
    }

    this->setAlignment(Qt::AlignCenter);

    // Enable mouse tracking
    setMouseTracking(true);


    // opacityEffect->setOpacity(1.0);  // Default opacity
    setStyleSheet(R"(
        QLabel {
            border: 2px solid transparent;
            border-radius: 15px;
        }
        QLabel:hover {
            border: 2px solid transparent;
            border-radius: 15px;
            background-color: #b3b3b3; 
        }
    )");
}
// Gérer l'entrée de la souris
void ClickableLabel::enterEvent(QEvent* event) {
    QLabel::enterEvent(event);
}

// Gérer la sortie de la souris
void ClickableLabel::leaveEvent(QEvent* event) {
    QLabel::leaveEvent(event);
}

void ClickableLabel::mousePressEvent(QMouseEvent* event) {

    if (event->button() == Qt::LeftButton) {
        // emit clicked();
        // Change style to indicate click
        setStyleSheet(R"(
        QLabel {
            border: 2px solid transparent;
            border-radius: 15px;
        }
        QLabel:hover {
            border: 2px solid #969393;
            border-radius: 15px;
            background-color: #9c9c9c; 
        }
    )");
    }
    QLabel::mousePressEvent(event);  // Call the base class implementation

}

void ClickableLabel::mouseReleaseEvent(QMouseEvent* event) {
    emit clicked();  // Émettre le signal quand on clique

    if (event->button() == Qt::LeftButton) {
        // Reset style on mouse release
        setStyleSheet(R"(
        QLabel {
            border: 2px solid transparent;
            border-radius: 15px;
        }
        QLabel:hover {
            border: 2px solid transparent;
            border-radius: 15px;
            background-color: #b3b3b3; 
        }
    )");
    }
    QLabel::mouseReleaseEvent(event);  // Call the base class implementation
}



ImageEditor::ImageEditor(Data& i, QWidget* parent) : QMainWindow(parent), // Initialize the base class
data(i) {
    QScreen* screen = QGuiApplication::primaryScreen();
    QRect screenR = screen->availableGeometry();
    // TO delete after 
    screenR.setSize(QSize(1920, 1080));
    pixelRatio = screen->devicePixelRatio();
    screenGeometry = screenR.size();

    int actionButtonSize;
    if (screenGeometry.width() < screenGeometry.height()) {
        actionButtonSize = (screenGeometry.width() * 1 / 24) / pixelRatio;
    }
    else {
        actionButtonSize = (screenGeometry.height() * 1 / 24) / pixelRatio;

    }

    previewSize = (screenGeometry * 1 / 12) / pixelRatio;



    // Créer un widget central
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // Créer un layout vertical pour toute la fenêtre
    mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(5);  // Espacement entre les widgets
    mainLayout->setContentsMargins(5, 5, 5, 5); // Marges autour des bords (gauche, haut, droite, bas)



    QSize actionSizeSet(actionButtonSize, actionButtonSize);
    actionSize = actionSizeSet;


    actionButtonLayout = new QHBoxLayout();
    actionButtonLayout->setAlignment(Qt::AlignCenter);
    mainLayout->addLayout(actionButtonLayout);

    buttonLayout = new QHBoxLayout();
    buttonLayout->setAlignment(Qt::AlignCenter);
    mainLayout->addLayout(buttonLayout);

    previewButtonLayout = new QHBoxLayout();
    previewButtonLayout->setAlignment(Qt::AlignCenter);
    mainLayout->addLayout(previewButtonLayout);

    createButtons();
    // updateButtons();
    updatePreview();


    setWindowTitle("Changer l'image");

    if (data.imagesData.get().size() > 0) {
        setImage(*data.imagesData.getImageData(data.imagesData.getImageNumber()));
    }



}

// Définit l'image principale de la fenetre ImageEditor
void ImageEditor::setImage(ImageData& imageData) {

    if (data.imagesData.get().size() <= 0) {
        showInformationMessage(this, "aucune image n'a pour l'instant été charger ! ");
        return;
    }
    std::string imagePath = imageData.getImagePath();


    cv::Mat image = cv::imread(imagePath, cv::IMREAD_UNCHANGED);
    if (!image.empty()) {


        Exiv2::ExifData exifData = imageData.getMetaData()->getExifData();
        if (exifData.empty()) {
            std::cerr << "No EXIF data found in image!" << std::endl;
        }
        else {

            if (exifData["Exif.Image.Orientation"].count() != 0) {
                int orientation = exifData["Exif.Image.Orientation"].toInt64();

                // Rotate the image based on the EXIF orientation
                switch (orientation) {
                case 3:
                    cv::rotate(image, image, cv::ROTATE_180);
                    break;
                case 6:
                    cv::rotate(image, image, cv::ROTATE_90_CLOCKWISE);
                    break;
                case 8:
                    cv::rotate(image, image, cv::ROTATE_90_COUNTERCLOCKWISE);
                    break;
                }
            }
        }
        // Convert the OpenCV image (BGR format) to QImage (RGB format)
        if (image.channels() == 4) {
            // Create QImage from OpenCV Mat with alpha channel
            QImage qImage(image.data, image.cols, image.rows, image.step[0], QImage::Format_ARGB32);
            // Set the pixmap with scaling
            imageLabel->setPixmap(QPixmap::fromImage(qImage).scaled(imageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
        else {
            // Handle images without an alpha channel (optional)
            cv::cvtColor(image, image, cv::COLOR_BGR2RGB);
            QImage qImage(image.data, image.cols, image.rows, image.step[0], QImage::Format_RGB888);
            imageLabel->setPixmap(QPixmap::fromImage(qImage).scaled(imageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
    }
    else {
        imageLabel->setText("Erreur");
    }
    reloadMainImage();

}


void ImageEditor::nextImage() {

    ImagesData& imagesData = data.imagesData;

    imagesData.setImageNumber(imagesData.getImageNumber() + 1);

    setImage(*imagesData.getImageData(imagesData.getImageNumber()));
    reload();
}

void ImageEditor::previousImage() {

    ImagesData& imagesData = data.imagesData;

    imagesData.setImageNumber(imagesData.getImageNumber() - 1);


    setImage(*imagesData.getImageData(imagesData.getImageNumber()));
    reload();
}

void ImageEditor::rotateLeft() {

    ImagesData& imagesData = data.imagesData;

    ImageData* imageData = imagesData.getCurrentImageData();
    int orientation = imageData->getImageOrientation();

    if (orientation == 1) {
        orientation = 8;
    }
    else if (orientation == 3) {
        orientation = 6;
    }
    else if (orientation == 8) {
        orientation = 3;
    }
    else {
        orientation = 1;
    }
    imageData->turnImage(orientation);

    imageData->saveMetaData();
    reload();


}
void ImageEditor::rotateRight() {

    ImagesData& imagesData = data.imagesData;

    ImageData* imageData = imagesData.getCurrentImageData();
    // imageData->turnImage(3);
    int orientation = imageData->getImageOrientation();

    if (orientation == 1) {
        orientation = 6;
    }
    else if (orientation == 3) {
        orientation = 8;
    }
    else if (orientation == 6) {
        orientation = 3;
    }
    else {
        orientation = 1;
    }
    imageData->turnImage(orientation);

    imageData->saveMetaData();
    reload();
}

void ImageEditor::reload() {
    std::cerr << "reload" << std::endl;


    ImagesData& imagesData = data.imagesData;

    updateButtons();
    updatePreview();
    std::cerr << "buton created" << std::endl;

    if (imagesData.get().size() <= 0) {
        showInformationMessage(this, "no image data loaded");
        std::cerr << "no image data loaded" << std::endl;

        return;
    }

    setImage(*imagesData.getCurrentImageData());
    std::cerr << "realod done" << std::endl;

}

void ImageEditor::reloadMainImage() {

    imageLabel->update();

}



void ImageEditor::updatePreview() {
    // TODO now TODONOW modifier les icons des boutons et leurs action si besoin

    // Effacer les boutons existants
    QLayoutItem* item;
    while ((item = previewButtonLayout->takeAt(0)) != nullptr) {
        delete item->widget(); // Supprimer le widget associé
        delete item; // Supprimer l'élément de layout
    }

    ImagesData& imagesData = data.imagesData;

    if (imagesData.get().size() <= 0) {
        return;
    }


    std::vector<QString> imagePaths;

    int currentImageNumber = imagesData.getImageNumber();
    int totalImages = imagesData.get().size();

    int under = 0;
    for (int i = 4; i >= 1; --i) {

        if (currentImageNumber - i >= 0) {
            imagePaths.push_back(QString::fromStdString(imagesData.getImageData(currentImageNumber - i)->getImagePath()));
            under += 1;
        }
    }

    imagePaths.push_back(QString::fromStdString(imagesData.getCurrentImageData()->getImagePath()));

    for (int i = 1; i <= 4; ++i) {

        if (currentImageNumber + i <= totalImages - 1) {
            imagePaths.push_back(QString::fromStdString(imagesData.getImageData(currentImageNumber + i)->getImagePath()));
        }
    }



    // Créer et ajouter les nouveaux boutons
    for (int i = 0; i < imagePaths.size(); ++i) {

        ClickableLabel* previewButton = new ClickableLabel(imagePaths[i], this, previewSize);

        previewButton->setFixedSize(previewSize); // Définir la taille fixe du bouton
        int imageNbr = imagesData.getImageNumber() + i - under;
        connect(previewButton, &ClickableLabel::clicked, [this, imageNbr]() {
            data.imagesData.setImageNumber(imageNbr);
            setImage(*data.imagesData.getImageData(data.imagesData.getImageNumber()));
            // TODO reload not working ::: Segmentation fault (core dumped) ::: so preview doesn't update
            // reload();
            });

        previewButtonLayout->addWidget(previewButton); // Ajouter le bouton au layout
    }

    previewButtonLayout->setAlignment(Qt::AlignCenter);
}


void ImageEditor::createButtons() {
    // TODO faire marcher la const ressources
    imageRotateRight = new ClickableLabel("../src/ressources/rotateRight.png", this, actionSize);
    imageRotateRight->setFixedSize(actionSize); // Définir la taille fixe du bouton (largeur, hauteur)

    imageRotateLeft = new ClickableLabel("../src/ressources/rotateLeft.png", this, actionSize);
    imageRotateLeft->setFixedSize(actionSize); // Définir la taille fixe du bouton (largeur, hauteur)

    if (isTurnable(data.imagesData.getCurrentImageData()->getImagePath())) {
        imageRotateLeft->setDisabled(true);
    }
    if (!imageRotateLeft->isEnabled())
        imageRotateLeft->setEnabled(true);
    if (isTurnable(data.imagesData.getCurrentImageData()->getImagePath())) {
        imageRotateRight->setDisabled(true);
    }
    if (!imageRotateRight->isEnabled())
        imageRotateRight->setEnabled(true);

    imageDelete = new ClickableLabel("../src/ressources/delete.png", this, actionSize);
    imageDelete->setFixedSize(actionSize); // Définir la taille fixe du bouton (largeur, hauteur)

    imageSave = new ClickableLabel("../src/ressources/save.png", this, actionSize);
    imageSave->setFixedSize(actionSize); // Définir la taille fixe du bouton (largeur, hauteur)


    actionButtonLayout->addWidget(imageRotateRight);
    actionButtonLayout->addWidget(imageRotateLeft);
    actionButtonLayout->addWidget(imageDelete);
    actionButtonLayout->addWidget(imageSave);


    // Créer les boutons avec des tailles spécifiques
    buttonImageBefore = new ClickableLabel("../src/ressources/before.png", this, actionSize);
    buttonImageBefore->setFixedSize(actionSize); // Définir la taille fixe du bouton (largeur, hauteur)

    buttonImageNext = new ClickableLabel("../src/ressources/next.png", this, actionSize);
    buttonImageNext->setFixedSize(actionSize);// Définir la taille fixe du bouton (largeur, hauteur)

    if (data.imagesData.getImageNumber() == 0) {
        buttonImageBefore->setDisabled(true);
    }
    if (data.imagesData.getImageNumber() == data.imagesData.get().size() - 1) {
        buttonImageNext->setDisabled(true);
    }

    imageLabel = new QLabel(this);
    imageLabel->setFixedSize((screenGeometry.width() * 4 / 6) / pixelRatio, (screenGeometry.height() * 4 / 6) / pixelRatio); // Ajuster la taille de l'image
    imageLabel->setAlignment(Qt::AlignCenter); // Centrer l'image dans le QLabel


    buttonLayout->addWidget(buttonImageBefore);
    buttonLayout->addWidget(imageLabel, 0, Qt::AlignCenter);
    buttonLayout->addWidget(buttonImageNext);
    buttonLayout->setAlignment(Qt::AlignCenter);


    // Connecter les images cliquables à des actions
    connect(buttonImageBefore, &ClickableLabel::clicked, [this]() { this->previousImage(); });
    connect(buttonImageNext, &ClickableLabel::clicked, [this]() { this->nextImage(); });

    connect(imageRotateLeft, &ClickableLabel::clicked, [this]() { this->rotateLeft(); });
    connect(imageRotateRight, &ClickableLabel::clicked, [this]() { this->rotateRight(); });

    connect(imageDelete, &ClickableLabel::clicked, [this]() { this->data.preDeleteImage(data.imagesData.getImageNumber()); });

    connect(imageSave, &ClickableLabel::clicked, [this]() { this->
        data.removeDeletedImages();
    if (data.imagesData.get().size() <= 0) {
        clearWindow();
    }
    data.imagesData.saveImagesData(IMAGESDATA_SAVE_DAT_PATH);
    data.imagesData.setImageNumber(0);
    reload();
        });
}


void ImageEditor::updateButtons() {
    if (imageRotateRight) {
        // TODO faire marcher la const ressources
        ClickableLabel* imageRotateRightNew = new ClickableLabel("../src/ressources/rotateRight.png", this, actionSize);
        imageRotateRightNew->setFixedSize(actionSize);

        connect(imageRotateRightNew, &ClickableLabel::clicked, [this]() { this->rotateRight(); });


        if (data.imagesData.get().size() > 0) {
            if (!isTurnable(data.imagesData.getCurrentImageData()->getImagePath())) {
                imageRotateRightNew->setDisabled(true);
            }
            else {
                if (!imageRotateRightNew->isEnabled())
                    imageRotateRightNew->setEnabled(true);
            }
        }
        actionButtonLayout->replaceWidget(imageRotateRight, imageRotateRightNew);

        imageRotateRight->hide();
        imageRotateRight->deleteLater();

        imageRotateRight = imageRotateRightNew;
    }
    if (imageRotateLeft) {
        ClickableLabel* imageRotateLeftNew = new ClickableLabel("../src/ressources/rotateLeft.png", this, actionSize);
        imageRotateLeftNew->setFixedSize(actionSize);
        connect(imageRotateLeftNew, &ClickableLabel::clicked, [this]() { this->rotateLeft(); });

        actionButtonLayout->replaceWidget(imageRotateLeft, imageRotateLeftNew);


        if (data.imagesData.get().size() > 0) {
            if (!isTurnable(data.imagesData.getCurrentImageData()->getImagePath())) {
                imageRotateLeftNew->setDisabled(true);
            }
            else {
                if (!imageRotateLeftNew->isEnabled())
                    imageRotateLeftNew->setEnabled(true);
            }
        }

        imageRotateLeft->hide();
        imageRotateLeft->deleteLater();

        imageRotateLeft = imageRotateLeftNew;
    }
    if (imageDelete) {
        ClickableLabel* imageDeleteNew = new ClickableLabel("../src/ressources/delete.png", this, actionSize);
        imageDeleteNew->setFixedSize(actionSize);
        connect(imageDeleteNew, &ClickableLabel::clicked, [this]() { this->data.preDeleteImage(data.imagesData.getImageNumber()); });

        actionButtonLayout->replaceWidget(imageDelete, imageDeleteNew);

        imageDelete->hide();
        imageDelete->deleteLater();

        imageDelete = imageDeleteNew;
    }
    if (imageSave) {
        ClickableLabel* imageSaveNew = new ClickableLabel("../src/ressources/save.png", this, actionSize);
        imageSaveNew->setFixedSize(actionSize);
        connect(imageSaveNew, &ClickableLabel::clicked, [this]() { this->
            data.removeDeletedImages();
        if (data.imagesData.get().size() <= 0) {
            clearWindow();
        }
        data.imagesData.saveImagesData(IMAGESDATA_SAVE_DAT_PATH);
        data.imagesData.setImageNumber(0);
        reload();
            });

        actionButtonLayout->replaceWidget(imageSave, imageSaveNew);

        imageSave->hide();
        imageSave->deleteLater();

        imageSave = imageSaveNew;
    }
    if (buttonImageBefore) {
        if (data.imagesData.getImageNumber() == 0) {
            buttonImageBefore->setDisabled(true);
        }

    }
    if (buttonImageNext) {
        if (data.imagesData.getImageNumber() == data.imagesData.get().size() - 1) {
            buttonImageNext->setDisabled(true);
        }

    }
    if (data.imagesData.getImageNumber() != 0) {
        if (!buttonImageBefore->isEnabled())
            buttonImageBefore->setEnabled(true);
    }

    if (data.imagesData.getImageNumber() != data.imagesData.get().size() - 1) {
        if (!buttonImageNext->isEnabled())
            buttonImageNext->setEnabled(true);
    }



}

// Delete all widget from the windows
void ImageEditor::clearWindow() {

    std::cerr << imageRotateRight << std::endl;
    std::cerr << imageRotateLeft << std::endl;
    std::cerr << imageDelete << std::endl;
    std::cerr << imageSave << std::endl;

    QTimer::singleShot(100, this, [this]() {
        if (actionButtonLayout) {
            QLayoutItem* item;
            while ((item = actionButtonLayout->takeAt(0)) != nullptr) {
                if (item->widget()) {
                    std::cerr << item->widget() << std::endl;

                    item->widget()->disconnect();
                    item->widget()->hide();
                    item->widget()->deleteLater();

                }
                delete item;
            }

            delete actionButtonLayout;
            actionButtonLayout = nullptr;
        }

        if (buttonLayout) {
            QLayoutItem* item;
            while ((item = buttonLayout->takeAt(0)) != nullptr) {
                if (item->widget()) {
                    item->widget()->disconnect();
                    item->widget()->hide();
                    item->widget()->deleteLater();
                }
                delete item;
            }

            delete buttonLayout;
            buttonLayout = nullptr;
        }


        if (mainLayout) {
            QLayoutItem* item;
            while ((item = mainLayout->takeAt(0)) != nullptr) {
                if (item->widget()) {
                    item->widget()->disconnect();
                    item->widget()->hide();
                    item->widget()->deleteLater();
                }
                delete item;
            }

            delete mainLayout;
            mainLayout = nullptr;
        }
        });




}