#include "imageEditor.h"

ClickableLabel::ClickableLabel(const QString& imagePath, QWidget* parent, QSize size)
    : QLabel(parent) {
    // Load image using OpenCV with alpha channel
    cv::Mat image = cv::imread(imagePath.toStdString(), cv::IMREAD_UNCHANGED);

    size = (size - QSize(5, 5));


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

            // cv::Mat resizedImage;

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

    updateStyleSheet();

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

        // background_color = "#9c9c9c";

        updateStyleSheet();

    }
    QLabel::mousePressEvent(event);  // Call the base class implementation

}

void ClickableLabel::mouseReleaseEvent(QMouseEvent* event) {
    emit clicked();  // Émettre le signal quand on clique

    // background_color = "transparent";


    if (event->button() == Qt::LeftButton) {

        updateStyleSheet();
    }
    QLabel::mouseReleaseEvent(event);  // Call the base class implementation
}

void ClickableLabel::updateStyleSheet() {
    QString styleSheet = QString(R"(
        QLabel {
            border: %1px solid transparent;
            border-radius: %2px;
            background-color: %3; 
        }
        QLabel:hover {
            border: %1px solid transparent;
            border-radius: %2px;
            background-color: %4; 
        }
        QLabel:disabled {
            background-color: rgba(200, 200, 200, 1);

        }
    )").arg(border).arg(border_radius).arg(background_color).arg(hover_background_color);
    this->setStyleSheet(styleSheet);

    std::cerr << "styleSheet : " << background_color.toStdString() << std::endl;
}





ImageEditor::ImageEditor(Data& i, QWidget* parent) : QMainWindow(parent), // Initialize the base class
data(i) {
    const QList<QScreen*> screens = QGuiApplication::screens();

    QScreen* screen = QGuiApplication::primaryScreen();
    screen = screens[0];


    QRect screenR = screen->availableGeometry();



    // pixelRatio = screen->devicePixelRatio();
    // pixelRatio = QString(qgetenv("QT_SCALE_FACTOR").constData()).toFloat();
    pixelRatio = screen->devicePixelRatio();


    std::cerr << "ratio : " << pixelRatio << std::endl;

    screenGeometry = screenR.size() * pixelRatio;

    std::cerr << "scree size : " << screenGeometry.width() << " , " << screenGeometry.height() << std::endl;

    int actionButtonSize;


    if (screenGeometry.width() < screenGeometry.height()) {
        // actionButtonSize = (screenGeometry.width() * 1 / 24) * pixelRatio;
        actionButtonSize = 32;

    }
    else {
        // actionButtonSize = (screenGeometry.height() * 1 / 24) * pixelRatio;
        actionButtonSize = 32;


    }

    previewSize = (screenGeometry * 1 / 12);



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
    createPreview();


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

    ImagesData& imagesData = data.imagesData;

    updateButtons();
    updatePreview();

    if (imagesData.get().size() <= 0) {
        showInformationMessage(this, "no image data loaded");
        std::cerr << "no image data loaded" << std::endl;

        return;
    }

    setImage(*imagesData.getCurrentImageData());
}

void ImageEditor::reloadMainImage() {

    imageLabel->update();

}



void ImageEditor::createPreview() {


    ImagesData& imagesData = data.imagesData;

    if (imagesData.get().size() <= 0) {
        return;
    }


    std::vector<std::string> imagePaths;

    int currentImageNumber = imagesData.getImageNumber();
    int totalImages = imagesData.get().size();

    int under = 0;
    for (int i = PREVIEW_NBR + 2; i >= 1; --i) {

        if (currentImageNumber - i >= 0) {
            imagePaths.push_back(imagesData.getImageData(currentImageNumber - i)->getImagePath());
            under += 1;
        }
    }

    imagePaths.push_back(imagesData.getCurrentImageData()->getImagePath());

    for (int i = 1; i <= PREVIEW_NBR + 2; ++i) {

        if (currentImageNumber + i <= totalImages - 1) {
            imagePaths.push_back(imagesData.getImageData(currentImageNumber + i)->getImagePath());
        }
    }



    // Créer et ajouter les nouveaux boutons
    for (int i = 0; i < PREVIEW_NBR * 2 + 1; ++i) {
        if (i < imagePaths.size()) {


            int imageNbr = imagesData.getImageNumber() + i - under;
            ClickableLabel* previewButton = createImagePreview(imagePaths[i], imageNbr);

            previewButtonLayout->addWidget(previewButton);

            previewButtons.push_back(previewButton);
        }
        // ClickableLabel* previewButton = createImagePreview(imagePaths[0], 0);
        // previewButtonLayout->addWidget(previewButton);
        // previewButton->hide();
        // previewButtons.push_back(previewButton);


    }

    previewButtonLayout->setAlignment(Qt::AlignCenter);
}


void ImageEditor::updatePreview() {
    ImagesData& imagesData = data.imagesData;

    if (imagesData.get().size() <= 0) {
        return;
    }

    std::vector<std::string> imagePaths;

    int currentImageNumber = imagesData.getImageNumber();
    int totalImages = imagesData.get().size();

    int under = 0;
    for (int i = PREVIEW_NBR; i > 0; --i) {
        std::cerr << "for 1 : " << -i << std::endl;

        if (currentImageNumber - i >= 0) {
            imagePaths.push_back(imagesData.getImageData(currentImageNumber - i)->getImagePath());
            under += 1;
        }
    }

    imagePaths.push_back(imagesData.getCurrentImageData()->getImagePath());

    for (int i = 1; i <= PREVIEW_NBR; ++i) {
        std::cerr << "for 2 : " << +i << std::endl;


        if (currentImageNumber + i <= totalImages - 1) {
            imagePaths.push_back(imagesData.getImageData(currentImageNumber + i)->getImagePath());
        }
    }



    for (int i = 0; i < PREVIEW_NBR * 2 + 1; ++i) {
        int imageNbr = imagesData.getImageNumber() + i - under;
        if (i < imagePaths.size()) {

            ClickableLabel* previewButtonNew = createImagePreview(imagePaths[i], imageNbr);

            previewButtonLayout->replaceWidget(previewButtons[i], previewButtonNew);

            previewButtons[i]->hide();
            previewButtons[i]->deleteLater();
            previewButtons[i] = previewButtonNew;
        }
        else {
            previewButtons[i]->hide();
        }


    }

}

void ImageEditor::createButtons() {
    imageRotateRight = createImageRotateRight();
    imageRotateLeft = createImageRotateLeft();
    imageDelete = createImageDelete();
    imageSave = createImageSave();

    actionButtonLayout->addWidget(imageRotateRight);
    actionButtonLayout->addWidget(imageRotateLeft);
    actionButtonLayout->addWidget(imageDelete);
    actionButtonLayout->addWidget(imageSave);

    imageLabel = new QLabel(this);

    imageLabel->setFixedSize((screenGeometry.width() * 4 / 6), (screenGeometry.height() * 4 / 6));
    imageLabel->setAlignment(Qt::AlignCenter);

    buttonImageBefore = createImageBefore();
    buttonImageNext = createImageNext();

    buttonLayout->addWidget(buttonImageBefore);
    buttonLayout->addWidget(imageLabel, 0, Qt::AlignCenter);
    buttonLayout->addWidget(buttonImageNext);
    buttonLayout->setAlignment(Qt::AlignCenter);
}


void ImageEditor::updateButtons() {
    if (imageRotateRight) {

        ClickableLabel* imageRotateRightNew = createImageRotateRight();

        actionButtonLayout->replaceWidget(imageRotateRight, imageRotateRightNew);

        imageRotateRight->hide();
        imageRotateRight->deleteLater();

        imageRotateRight = imageRotateRightNew;
    }
    if (imageRotateLeft) {
        ClickableLabel* imageRotateLeftNew = createImageRotateLeft();

        actionButtonLayout->replaceWidget(imageRotateLeft, imageRotateLeftNew);

        imageRotateLeft->hide();
        imageRotateLeft->deleteLater();

        imageRotateLeft = imageRotateLeftNew;
    }
    if (imageDelete) {
        ClickableLabel* imageDeleteNew = createImageDelete();

        actionButtonLayout->replaceWidget(imageDelete, imageDeleteNew);

        imageDelete->hide();
        imageDelete->deleteLater();


        imageDelete = imageDeleteNew;
    }
    if (imageSave) {
        ClickableLabel* imageSaveNew = createImageSave();

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

/**
 * @brief Clears the window by removing and deleting all widgets and layouts.
 *
 * This function performs the following actions:
 * - Logs the current state of image-related actions (rotate right, rotate left, delete, save).
 * - Schedules a single-shot timer to execute the clearing process after 100 milliseconds.
 * - Iterates through the `actionButtonLayout`, `buttonLayout`, and `mainLayout` to:
 *   - Disconnect and hide each widget.
 *   - Delete each widget and layout item.
 *   - Delete the layout itself and set the corresponding pointer to nullptr.
 */
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


/**
 * @brief Creates a clickable label for pre deleting an image.
 *
 * This function creates a ClickableLabel object that represents a pre delete button for an image.
 * If the image is marked as pre deleted, the background color of the label is set to a specific color
 * and the stylesheet is updated. The label is also connected to a click event that triggers the
 * pre-delete action for the image and updates the buttons.
 *
 * @return A pointer to the created ClickableLabel object.
 */
ClickableLabel* ImageEditor::createImageDelete() {

    if (data.imagesData.get().size() <= 0) {
        return nullptr;
    }

    ClickableLabel* imageDelete = new ClickableLabel("../src/ressources/delete.png", this, actionSize);
    if (data.isDeleted(data.imagesData.getImageNumber())) {

        imageDelete->background_color = "#700c13";
        imageDelete->updateStyleSheet();
        connect(imageDelete, &ClickableLabel::clicked, [this]() { this->data.unPreDeleteImage(data.imagesData.getImageNumber());
        updateButtons();
            });
    }
    else {
        connect(imageDelete, &ClickableLabel::clicked, [this]() { this->data.preDeleteImage(data.imagesData.getImageNumber());
        updateButtons();
            });

    }
    imageDelete->setFixedSize(actionSize);

    return imageDelete;
}


ClickableLabel* ImageEditor::createImageSave() {

    if (data.imagesData.get().size() <= 0) {
        return nullptr;
    }

    ClickableLabel* imageSave = new ClickableLabel("../src/ressources/save.png", this, actionSize);

    connect(imageSave, &ClickableLabel::clicked, [this]() { this->
        data.removeDeletedImages();
    if (data.imagesData.get().size() <= 0) {
        clearWindow();
    }
    data.imagesData.saveImagesData(IMAGESDATA_SAVE_DAT_PATH);
    data.imagesData.setImageNumber(0);
    reload();
        });

    imageSave->setFixedSize(actionSize);

    return imageSave;
}

ClickableLabel* ImageEditor::createImageRotateRight() {

    if (data.imagesData.get().size() <= 0) {
        return nullptr;
    }

    imageRotateRight = new ClickableLabel("../src/ressources/rotateRight.png", this, actionSize);
    imageRotateRight->setFixedSize(actionSize);


    if (isTurnable(data.imagesData.getCurrentImageData()->getImagePath())) {
        imageRotateRight->setDisabled(true);
    }
    if (!imageRotateRight->isEnabled())
        imageRotateRight->setEnabled(true);


    connect(imageRotateRight, &ClickableLabel::clicked, [this]() { this->rotateRight(); });


    imageRotateRight->setFixedSize(actionSize);

    return imageRotateRight;
}


ClickableLabel* ImageEditor::createImageRotateLeft() {

    if (data.imagesData.get().size() <= 0) {
        return nullptr;
    }

    imageRotateLeft = new ClickableLabel("../src/ressources/rotateLeft.png", this, actionSize);
    imageRotateLeft->setFixedSize(actionSize);

    if (isTurnable(data.imagesData.getCurrentImageData()->getImagePath())) {
        imageRotateLeft->setDisabled(true);
    }
    if (!imageRotateLeft->isEnabled())
        imageRotateLeft->setEnabled(true);


    connect(imageRotateLeft, &ClickableLabel::clicked, [this]() { this->rotateLeft(); });

    imageRotateLeft->setFixedSize(actionSize);

    return imageRotateLeft;
}



ClickableLabel* ImageEditor::createImageBefore() {

    if (data.imagesData.get().size() <= 0) {
        return nullptr;
    }

    buttonImageBefore = new ClickableLabel("../src/ressources/before.png", this, actionSize);
    buttonImageBefore->setFixedSize(actionSize);

    if (data.imagesData.getImageNumber() == 0) {
        buttonImageBefore->setDisabled(true);
    }

    connect(buttonImageBefore, &ClickableLabel::clicked, [this]() { this->previousImage(); });
    buttonImageBefore->setFixedSize(actionSize);

    return buttonImageBefore;
}


ClickableLabel* ImageEditor::createImageNext() {

    if (data.imagesData.get().size() <= 0) {
        return nullptr;
    }

    buttonImageNext = new ClickableLabel("../src/ressources/next.png", this, actionSize);
    buttonImageNext->setFixedSize(actionSize);

    if (data.imagesData.getImageNumber() == data.imagesData.get().size() - 1) {
        buttonImageNext->setDisabled(true);
    }

    connect(buttonImageNext, &ClickableLabel::clicked, [this]() { this->nextImage(); });

    buttonImageNext->setFixedSize(actionSize);

    return buttonImageNext;
}


ClickableLabel* ImageEditor::createImagePreview(std::string imagePath, int imageNbr) {

    if (data.imagesData.get().size() <= 0) {
        return nullptr;
    }

    ClickableLabel* previewButton = new ClickableLabel(QString::fromStdString(imagePath), this, previewSize);

    previewButton->setFixedSize(previewSize); // Définir la taille fixe du bouton
    // int imageNbr = imagesData.getImageNumber() + i - under;
    connect(previewButton, &ClickableLabel::clicked, [this, imageNbr]() {
        data.imagesData.setImageNumber(imageNbr);
        setImage(*data.imagesData.getImageData(data.imagesData.getImageNumber()));
        });


    return previewButton;
}
