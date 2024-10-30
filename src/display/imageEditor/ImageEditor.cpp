#include "ImageEditor.h"





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

    screenGeometry = screenR.size() / pixelRatio;

    std::cerr << "scree size : " << screenGeometry.width() << " , " << screenGeometry.height() << std::endl;

    int actionButtonSize;


    if (screenGeometry.width() < screenGeometry.height()) {
        actionButtonSize = (screenGeometry.width() * 1 / 48) * pixelRatio;
        // actionButtonSize = 32;

    }
    else {
        actionButtonSize = (screenGeometry.height() * 1 / 48) * pixelRatio;
        // actionButtonSize = 32;


    }

    previewSize = (screenGeometry * 1 / 12);



    // Créer un widget central
    QWidget* centralWidget = new QWidget(parent);
    setCentralWidget(centralWidget);

    // Créer un layout vertical pour toute la fenêtre
    mainLayout = new QHBoxLayout(centralWidget);

    // Ajouter le layout principal au widget parent
    // parent->setLayout(mainLayout);

    mainLayout->setSpacing(5);  // Espacement entre les widgets
    mainLayout->setContentsMargins(5, 5, 5, 5); // Marges autour des bords (gauche, haut, droite, bas)




    QSize actionSizeSet(actionButtonSize, actionButtonSize);
    actionSize = actionSizeSet;


    editionLayout = new QVBoxLayout();
    editionLayout->setAlignment(Qt::AlignCenter);

    actionButtonLayout = new QHBoxLayout();
    actionButtonLayout->setAlignment(Qt::AlignCenter);

    buttonLayout = new QHBoxLayout();
    buttonLayout->setAlignment(Qt::AlignCenter);

    previewButtonLayout = new QHBoxLayout();
    previewButtonLayout->setAlignment(Qt::AlignCenter);

    infoLayout = new QVBoxLayout();
    infoLayout->setAlignment(Qt::AlignCenter);


    mainLayout->addLayout(editionLayout);
    editionLayout->addLayout(actionButtonLayout);
    editionLayout->addLayout(buttonLayout);
    editionLayout->addLayout(previewButtonLayout);
    mainLayout->addLayout(infoLayout);


    createButtons();
    createPreview();

    // dateEdit = new QLineEdit(this);
    dateEdit = new QDateTimeEdit(this);
    dateEdit->setDisplayFormat("dd/MM/yyyy, HH:mm");
    dateEdit->setCalendarPopup(true);

    geoEdit = new QLineEdit(this);
    descriptionEdit = new QLineEdit(this);
    validateButton = new QPushButton("Valider", this);
    connect(validateButton, &QPushButton::clicked, this, &ImageEditor::validateMetadata);

    // QVBoxLayout* layout = new QVBoxLayout();
    infoLayout->addWidget(new QLabel("Date:", this));
    infoLayout->addWidget(dateEdit);
    infoLayout->addWidget(new QLabel("Géolocalisation:", this));
    infoLayout->addWidget(geoEdit);
    infoLayout->addWidget(new QLabel("Description:", this));
    infoLayout->addWidget(descriptionEdit);
    infoLayout->addWidget(validateButton);
    if (exifEditor) {

        populateMetadataFields();
    }
    else {
        for (int i = 0; i < infoLayout->count(); ++i) {
            QWidget* widget = infoLayout->itemAt(i)->widget();
            if (widget) {
                widget->hide();
            }
        }
    }

    // infoLayout->addItem(layout);


    // setWindowTitle("Changer l'image");
    // setWindowTitle("EasyImageEditor : Image Editor");


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
    if (!isTurnable(imageData->getImagePath())) {
        return;
    }
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
    if (!isTurnable(imageData->getImagePath())) {
        return;
    }

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
    if (exifEditor) {
        populateMetadataFields();
    }
    else {
        for (int i = 0; i < infoLayout->count(); ++i) {
            QWidget* widget = infoLayout->itemAt(i)->widget();
            if (widget) {
                widget->hide();
            }
        }
    }

    if (imagesData.get().size() <= 0) {
        // TODO reactivate
        showInformationMessage(this, "no image data loaded");
        addSelectedFilesToFolders(this);

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
    for (int i = PREVIEW_NBR; i > 0; --i) {

        if (currentImageNumber - i >= 0) {
            imagePaths.push_back(imagesData.getImageData(currentImageNumber - i)->getImagePath());
            under += 1;
        }
    }

    imagePaths.push_back(imagesData.getCurrentImageData()->getImagePath());

    for (int i = 1; i <= PREVIEW_NBR; ++i) {


        if (currentImageNumber + i <= totalImages - 1) {
            imagePaths.push_back(imagesData.getImageData(currentImageNumber + i)->getImagePath());
        }
    }



    // Créer et ajouter les nouveaux boutons
    for (int i = 0; i < PREVIEW_NBR * 2 + 1; ++i) {
        if (i < imagePaths.size()) {
            int imageNbr = imagesData.getImageNumber() + i - under;
            if (imageNbr == imagesData.getImageNumber()) {
                ClickableLabel* previewButton = createImagePreview(imagePaths[i], imageNbr);
                previewButton->background_color = "#b3b3b3";
                previewButton->updateStyleSheet();

                previewButtonLayout->addWidget(previewButton);

                previewButtons.push_back(previewButton);
            }
            else {


                ClickableLabel* previewButton = createImagePreview(imagePaths[i], imageNbr);

                previewButtonLayout->addWidget(previewButton);

                previewButtons.push_back(previewButton);
            }
        }
        // create all the button but hide 
        ClickableLabel* previewButton = createImagePreview(imagePaths[0], 0);
        previewButtonLayout->addWidget(previewButton);
        previewButton->hide();
        previewButtons.push_back(previewButton);


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

        if (currentImageNumber - i >= 0) {
            imagePaths.push_back(imagesData.getImageData(currentImageNumber - i)->getImagePath());
            under += 1;
        }
    }

    imagePaths.push_back(imagesData.getCurrentImageData()->getImagePath());

    for (int i = 1; i <= PREVIEW_NBR; ++i) {


        if (currentImageNumber + i <= totalImages - 1) {
            imagePaths.push_back(imagesData.getImageData(currentImageNumber + i)->getImagePath());
        }
    }




    for (int i = 0; i < PREVIEW_NBR * 2 + 1; ++i) {
        if (i < imagePaths.size()) {
            int imageNbr = imagesData.getImageNumber() + i - under;
            if (imageNbr == imagesData.getImageNumber()) {
                ClickableLabel* previewButtonNew = createImagePreview(imagePaths[i], imageNbr);
                // TODO choose the color of the current image
                previewButtonNew->background_color = "#b3b3b3";
                previewButtonNew->updateStyleSheet();

                previewButtonLayout->replaceWidget(previewButtons[i], previewButtonNew);

                previewButtons[i]->hide();
                previewButtons[i]->deleteLater();

                previewButtons[i] = previewButtonNew;
            }
            else {

                ClickableLabel* previewButtonNew = createImagePreview(imagePaths[i], imageNbr);

                previewButtonLayout->replaceWidget(previewButtons[i], previewButtonNew);

                previewButtons[i]->hide();
                previewButtons[i]->deleteLater();

                previewButtons[i] = previewButtonNew;
            }
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
    imageEditExif = createImageEditExif();


    actionButtonLayout->addWidget(imageRotateRight);
    actionButtonLayout->addWidget(imageRotateLeft);
    actionButtonLayout->addWidget(imageDelete);
    actionButtonLayout->addWidget(imageSave);
    actionButtonLayout->addWidget(imageEditExif);


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
    if (data.imagesData.get().size() <= 0) {
        return;
    }

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
    if (exifEditor) {
        if (imageEditExif) {
            ClickableLabel* imageEditExifNew = createImageEditExif();

            actionButtonLayout->replaceWidget(imageEditExif, imageEditExifNew);

            imageEditExif->hide();
            imageEditExif->deleteLater();

            imageEditExif = imageEditExifNew;
        }
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
void ImageEditor::clear() {

    std::cerr << imageRotateRight << std::endl;
    std::cerr << imageRotateLeft << std::endl;
    std::cerr << imageDelete << std::endl;
    std::cerr << imageSave << std::endl;

    QTimer::singleShot(100, this, [this]() {
        if (actionButtonLayout) {
            QLayoutItem* item;
            while ((item = actionButtonLayout->takeAt(0)) != nullptr) {
                if (item->widget()) {
                    item->widget()->disconnect();
                    item->widget()->hide();
                    item->widget()->deleteLater();

                }
                delete item;
            }

            delete actionButtonLayout;
            actionButtonLayout = nullptr;
        }

        if (previewButtonLayout) {
            QLayoutItem* item;
            while ((item = previewButtonLayout->takeAt(0)) != nullptr) {
                if (item->widget()) {
                    item->widget()->disconnect();
                    item->widget()->hide();
                    item->widget()->deleteLater();

                }
                delete item;
            }

            delete previewButtonLayout;
            previewButtonLayout = nullptr;
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

    ClickableLabel* imageDelete = new ClickableLabel(":/delete.png", this, actionSize);

    if (data.isDeleted(data.imagesData.getImageNumber())) {

        imageDelete->background_color = "#700c13";
        imageDelete->updateStyleSheet();
        // connect(imageDelete, &ClickableLabel::clicked, [this]() { this->unDeleteImage();

        //     });
    }
    // else {
    connect(imageDelete, &ClickableLabel::clicked, [this]() { this->deleteImage();
        });

    // }

    return imageDelete;
}


ClickableLabel* ImageEditor::createImageSave() {

    if (data.imagesData.get().size() <= 0) {
        return nullptr;
    }

    ClickableLabel* imageSaveNew = new ClickableLabel(":/save.png", this, actionSize);

    connect(imageSaveNew, &ClickableLabel::clicked, [this]() { this->
        saveImage();
        });


    return imageSaveNew;
}

ClickableLabel* ImageEditor::createImageRotateRight() {

    if (data.imagesData.get().size() <= 0) {
        return nullptr;
    }

    ClickableLabel* imageRotateRightNew = new ClickableLabel(":/rotateRight.png", this, actionSize);

    if (!isTurnable(data.imagesData.getCurrentImageData()->getImagePath())) {
        imageRotateRightNew->setDisabled(true);
    }
    else {
        if (!imageRotateRightNew->isEnabled())
            imageRotateRightNew->setEnabled(true);
    }

    connect(imageRotateRightNew, &ClickableLabel::clicked, [this]() { this->rotateRight(); });


    return imageRotateRightNew;
}


ClickableLabel* ImageEditor::createImageRotateLeft() {

    if (data.imagesData.get().size() <= 0) {
        return nullptr;
    }

    ClickableLabel* imageRotateLeftNew = new ClickableLabel(":/rotateLeft.png", this, actionSize);

    if (!isTurnable(data.imagesData.getCurrentImageData()->getImagePath())) {
        imageRotateLeftNew->setDisabled(true);
    }
    else {
        if (!imageRotateLeftNew->isEnabled())
            imageRotateLeftNew->setEnabled(true);
    }

    connect(imageRotateLeftNew, &ClickableLabel::clicked, [this]() { this->rotateLeft(); });

    return imageRotateLeftNew;
}


ClickableLabel* ImageEditor::createImageEditExif() {
    if (data.imagesData.get().size() <= 0) {
        return nullptr;
    }

    ClickableLabel* imageEditExifNew = new ClickableLabel(":/editExif.png", this, actionSize);

    connect(imageEditExifNew, &ClickableLabel::clicked, [this]() {
        // exifEditor = !exifEditor;
        if (exifEditor) {
            exifEditor = false;
        }
        else {
            exifEditor = true;
            for (int i = 0; i < infoLayout->count(); ++i) {
                QWidget* widget = infoLayout->itemAt(i)->widget();
                if (widget) {
                    widget->setHidden(false);
                }
            }
        }
        reload();
        });

    return imageEditExifNew;
}


ClickableLabel* ImageEditor::createImageBefore() {

    if (data.imagesData.get().size() <= 0) {
        return nullptr;
    }




    ClickableLabel* buttonImageBeforeNew = new ClickableLabel(":/before.png", this, actionSize);
    // ClickableLabel* buttonImageBeforeNew = new ClickableLabel(":/ressources/before.png", this, actionSize);

    buttonImageBeforeNew->setFixedSize(actionSize);

    if (data.imagesData.getImageNumber() == 0) {
        buttonImageBeforeNew->setDisabled(true);
    }

    connect(buttonImageBeforeNew, &ClickableLabel::clicked, [this]() { this->previousImage(); });
    buttonImageBeforeNew->setFixedSize(actionSize);

    return buttonImageBeforeNew;
}


ClickableLabel* ImageEditor::createImageNext() {

    if (data.imagesData.get().size() <= 0) {
        return nullptr;
    }

    ClickableLabel* buttonImageNextNew = new ClickableLabel(":/next.png", this, actionSize);

    if (data.imagesData.getImageNumber() == data.imagesData.get().size() - 1) {
        buttonImageNextNew->setDisabled(true);
    }

    connect(buttonImageNextNew, &ClickableLabel::clicked, [this]() { this->nextImage(); });

    buttonImageNextNew->setFixedSize(actionSize);

    return buttonImageNextNew;
}


ClickableLabel* ImageEditor::createImagePreview(std::string imagePath, int imageNbr) {

    if (data.imagesData.get().size() <= 0) {
        return nullptr;
    }

    ClickableLabel* previewButton = new ClickableLabel(QString::fromStdString(imagePath), this, previewSize, false);


    connect(previewButton, &ClickableLabel::clicked, [this, imageNbr]() {
        data.imagesData.setImageNumber(imageNbr);
        setImage(*data.imagesData.getImageData(data.imagesData.getImageNumber()));
        reload();
        });


    return previewButton;
}


void ImageEditor::keyPressEvent(QKeyEvent* event) {
    // std::cerr << event->key() << std::endl;
    switch (event->key()) {
    case Qt::Key_Left:
        if (event->modifiers() & Qt::ControlModifier) {
            imageRotateLeft->background_color = CLICK_BACKGROUND_COLOR;
            imageRotateLeft->updateStyleSheet();
        }
        else {
            buttonImageBefore->background_color = CLICK_BACKGROUND_COLOR;
            buttonImageBefore->updateStyleSheet();
        }
        break;
    case Qt::Key_Right:
        if (event->modifiers() & Qt::ControlModifier) {
            imageRotateRight->background_color = CLICK_BACKGROUND_COLOR;
            imageRotateRight->updateStyleSheet();
        }
        else {
            buttonImageNext->background_color = CLICK_BACKGROUND_COLOR;
            buttonImageNext->updateStyleSheet();
        }
        break;

    case Qt::Key_S:
        if (event->modifiers() & Qt::ControlModifier) {
            imageSave->background_color = CLICK_BACKGROUND_COLOR;
            imageSave->updateStyleSheet();
        }
        break;
    case Qt::Key_Delete:
        imageDelete->background_color = CLICK_BACKGROUND_COLOR;
        imageDelete->updateStyleSheet();
        imageDelete->background_color = CLICK_BACKGROUND_COLOR;
        imageDelete->updateStyleSheet();
        break;
    case Qt::Key_Backspace:
        imageDelete->background_color = CLICK_BACKGROUND_COLOR;
        imageDelete->updateStyleSheet();
        break;
    case Qt::Key_D:
        if (event->modifiers() & Qt::ControlModifier) {
            imageDelete->background_color = CLICK_BACKGROUND_COLOR;
            imageDelete->updateStyleSheet();
        }
        break;

    default:
        QWidget::keyPressEvent(event); // Call base class implementation for other keys
    }
}

void ImageEditor::keyReleaseEvent(QKeyEvent* event) {
    switch (event->key()) {

    case Qt::Key_Left:
        if (event->modifiers() & Qt::ControlModifier) {
            imageRotateLeft->background_color = BACKGROUND_COLOR;
            imageRotateLeft->updateStyleSheet();
            rotateLeft();
        }
        else {
            buttonImageBefore->background_color = BACKGROUND_COLOR;
            buttonImageBefore->updateStyleSheet();
            previousImage();
        }
        break;
    case Qt::Key_Right:
        if (event->modifiers() & Qt::ControlModifier) {
            imageRotateRight->background_color = BACKGROUND_COLOR;
            imageRotateRight->updateStyleSheet();
            rotateRight();

        }
        else {
            buttonImageNext->background_color = BACKGROUND_COLOR;
            buttonImageNext->updateStyleSheet();
            nextImage();
        }
        break;

    case Qt::Key_S:
        if (event->modifiers() & Qt::ControlModifier) {
            imageSave->background_color = BACKGROUND_COLOR;
            imageSave->updateStyleSheet();
            saveImage();
        }
        break;
    case Qt::Key_Delete:
        imageDelete->background_color = BACKGROUND_COLOR;
        imageDelete->updateStyleSheet();
        deleteImage();
        break;
    case Qt::Key_Backspace:
        imageDelete->background_color = BACKGROUND_COLOR;
        imageDelete->updateStyleSheet();
        deleteImage();
        break;
    case Qt::Key_D:
        if (event->modifiers() & Qt::ControlModifier) {
            imageDelete->background_color = BACKGROUND_COLOR;
            imageDelete->updateStyleSheet();
            deleteImage();
        }
        break;


    default:
        QWidget::keyReleaseEvent(event); // Call base class implementation for other keys
    }
}


void ImageEditor::saveImage() {
    data.removeDeletedImages();
    if (data.imagesData.get().size() <= 0) {
        clear();
    }
    data.imagesData.saveImagesData(IMAGESDATA_SAVE_DAT_PATH);

    data.imagesData.setImageNumber(0);
    reload();
}


void ImageEditor::deleteImage() {
    if (data.isDeleted(data.imagesData.getImageNumber())) {

        data.unPreDeleteImage(data.imagesData.getImageNumber());
        updateButtons();
    }
    else {
        data.preDeleteImage(data.imagesData.getImageNumber());
        updateButtons();

    }
}


void ImageEditor::populateMetadataFields() {
    ImagesData& imagesData = data.imagesData;
    ImageData* imageData = imagesData.getCurrentImageData();
    Exiv2::ExifData exifData = imageData->getMetaData()->getExifData();

    dateEdit->setDateTime(QDateTime::currentDateTime());
    geoEdit->clear();
    descriptionEdit->clear();


    if (exifData["Exif.Image.DateTime"].count() != 0) {
        QString dateTimeStr = QString::fromStdString(exifData["Exif.Image.DateTime"].toString());
        QDateTime dateTime = QDateTime::fromString(dateTimeStr, "yyyy:MM:dd HH:mm:ss");
        dateEdit->setDateTime(dateTime);
    }
    if (exifData["Exif.GPSInfo.GPSLatitude"].count() != 0 && exifData["Exif.GPSInfo.GPSLongitude"].count() != 0) {
        geoEdit->setText(QString::fromStdString(
            exifData["Exif.GPSInfo.GPSLatitude"].toString() + ", " +
            exifData["Exif.GPSInfo.GPSLongitude"].toString()));
    }
    if (exifData["Exif.Image.ImageDescription"].count() != 0) {
        descriptionEdit->setText(QString::fromStdString(exifData["Exif.Image.ImageDescription"].toString()));
    }
}



void ImageEditor::validateMetadata() {
    ImagesData& imagesData = data.imagesData;
    ImageData* imageData = imagesData.getCurrentImageData();
    MetaData* metaData = imageData->getMetaData();

    // metaData->modifyExifValue("Exif.Image.DateTime", dateEdit->text().toStdString());
    QString dateTimeStr = dateEdit->dateTime().toString("yyyy:MM:dd HH:mm:ss");
    metaData->modifyExifValue("Exif.Image.DateTime", dateTimeStr.toStdString());
    // Suppose that geoEdit contains latitude and longitude separated by a comma
    QStringList geoData = geoEdit->text().split(",");
    if (geoData.size() == 2) {
        metaData->modifyExifValue("Exif.GPSInfo.GPSLatitude", geoData[0].trimmed().toStdString());
        metaData->modifyExifValue("Exif.GPSInfo.GPSLongitude", geoData[1].trimmed().toStdString());
    }
    metaData->modifyExifValue("Exif.Image.ImageDescription", descriptionEdit->text().toStdString());

    imageData->saveMetaData();
}