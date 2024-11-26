#include "ImageEditor.h"





ImageEditor::ImageEditor(Data* dat, QWidget* parent) : QMainWindow(parent), data(dat) {
    const QList<QScreen*> screens = QGuiApplication::screens();


    // initialisation pour les times de preload
    for (int i = 0; i < PRE_LOAD_RADIUS; i++)
        imagePreviewOpenTimers.push_back(new QTimer(this));
    // data = dat;


    // std::cerr << "imagesData \n\n\n\n" << std::endl;
    // data.imagesData->print();

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
    mainImageSize = (screenGeometry * 4 / 6);




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




    nameEdit = new QLineEdit(this);

    // dateEdit = new QLineEdit(this);
    dateEdit = new QDateTimeEdit(this);
    dateEdit->setDisplayFormat("dd/MM/yyyy, HH:mm");
    dateEdit->setCalendarPopup(true);

    QCalendarWidget* calendarWidget = dateEdit->calendarWidget();
    calendarWidget->setGridVisible(true); // Afficher une grille
    calendarWidget->setFirstDayOfWeek(Qt::Monday); // Définir le premier jour de la semaine
    calendarWidget->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader); // Masquer les en-têtes verticaux
    calendarWidget->setNavigationBarVisible(true); // Afficher la barre de navigation


    geoEdit = new QLineEdit(this);
    descriptionEdit = new QLineEdit(this);
    validateButton = new QPushButton("Valider", this);
    connect(validateButton, &QPushButton::clicked, this, &ImageEditor::validateMetadata);

    // QVBoxLayout* layout = new QVBoxLayout();
    infoLayout->addWidget(new QLabel("Name:", this));
    infoLayout->addWidget(nameEdit);
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

}

void ImageEditor::nextImage(int nbr) {
    data->imagesData.setImageNumber(data->imagesData.getImageNumber() + nbr);
    reload();
}

void ImageEditor::previousImage(int nbr) {
    data->imagesData.setImageNumber(data->imagesData.getImageNumber() - nbr);
    reload();
}

void ImageEditor::rotateLeftJpg() {

    std::cerr << "rotateLeftJpg" << std::endl;

    ImagesData* imagesData = &data->imagesData;

    ImageData* imageData = imagesData->getCurrentImageData();
    if (!isTurnable(imageData->getImagePath())) {
        return;
    }
    int orientation = imageData->getImageOrientation();

    switch (orientation) {
    case 1:
        orientation = 8;
        break;
    case 2:
        orientation = 5;
        break;
    case 3:
        orientation = 6;
        break;
    case 4:
        orientation = 7;
        break;
    case 5:
        orientation = 2;
        break;
    case 6:
        orientation = 1;
        break;
    case 7:
        orientation = 4;
        break;
    case 8:
        orientation = 3;
        break;
    default:
        break;
    }
    imageData->turnImage(orientation);

    imageData->saveMetaData();

    data->rotateImageCache(imageData->getImagePath(), -90);



    reload();


}
void ImageEditor::rotateRightJpg() {

    ImagesData* imagesData = &data->imagesData;

    ImageData* imageData = imagesData->getCurrentImageData();
    if (!isTurnable(imageData->getImagePath())) {
        return;
    }

    int orientation = imageData->getImageOrientation();

    switch (orientation) {
    case 1:
        orientation = 6;
        break;
    case 2:
        orientation = 5;
        break;
    case 3:
        orientation = 8;
        break;
    case 4:
        orientation = 7;
        break;
    case 5:
        orientation = 2;
        break;
    case 6:
        orientation = 3;
        break;
    case 7:
        orientation = 4;
        break;
    case 8:
        orientation = 1;
        break;
    default:
        break;
    }
    imageData->turnImage(orientation);

    imageData->saveMetaData();

    data->rotateImageCache(imageData->getImagePath(), 90);


    reload();
}

void ImageEditor::reload() {
    ImagesData* imagesData = &data->imagesData;

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

    if (imagesData->get().size() <= 0) {
        // TODO reactivate
        showInformationMessage(this, "no image data loaded");
        addSelectedFilesToFolders(this);

        return;
    }

    // setImage(imagesData->getCurrentImageData());
}

// void ImageEditor::reloadMainImage() {

//     imageLabel->update();

// }



void ImageEditor::createPreview() {


    ImagesData* imagesData = &data->imagesData;

    if (imagesData->get().size() <= 0) {
        return;
    }


    std::vector<std::string> imagePaths;

    int currentImageNumber = imagesData->getImageNumber();
    int totalImages = imagesData->get().size();

    int under = 0;
    for (int i = PREVIEW_NBR; i > 0; --i) {

        if (currentImageNumber - i >= 0) {
            imagePaths.push_back(imagesData->getImageData(currentImageNumber - i)->getImagePath());
            under += 1;
        }
    }

    imagePaths.push_back(imagesData->getCurrentImageData()->getImagePath());

    for (int i = 1; i <= PREVIEW_NBR; ++i) {


        if (currentImageNumber + i <= totalImages - 1) {
            imagePaths.push_back(imagesData->getImageData(currentImageNumber + i)->getImagePath());
        }
    }



    // Créer et ajouter les nouveaux boutons
    for (int i = 0; i < PREVIEW_NBR * 2 + 1; ++i) {
        if (i < imagePaths.size()) {
            int imageNbr = imagesData->getImageNumber() + i - under;
            if (imageNbr == imagesData->getImageNumber()) {
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
    ImagesData* imagesData = &data->imagesData;

    if (imagesData->get().size() <= 0) {
        return;
    }

    std::vector<std::string> imagePaths;

    int currentImageNumber = imagesData->getImageNumber();
    int totalImages = imagesData->get().size();

    int under = 0;
    for (int i = PREVIEW_NBR; i > 0; --i) {

        if (currentImageNumber - i >= 0) {
            imagePaths.push_back(imagesData->getImageData(currentImageNumber - i)->getImagePath());
            under += 1;
        }
    }

    imagePaths.push_back(imagesData->getCurrentImageData()->getImagePath());

    for (int i = 1; i <= PREVIEW_NBR; ++i) {


        if (currentImageNumber + i <= totalImages - 1) {
            imagePaths.push_back(imagesData->getImageData(currentImageNumber + i)->getImagePath());
        }
    }




    for (int i = 0; i < PREVIEW_NBR * 2 + 1; ++i) {
        if (i < imagePaths.size()) {
            int imageNbr = imagesData->getImageNumber() + i - under;
            if (imageNbr == imagesData->getImageNumber()) {
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

    imageMirrorLeftRight = createImageMirrorLeftRight();
    imageMirrorUpDown = createImageMirrorUpDown();

    imageDelete = createImageDelete();
    imageSave = createImageSave();
    imageEditExif = createImageEditExif();


    actionButtonLayout->addWidget(imageRotateRight);
    actionButtonLayout->addWidget(imageRotateLeft);
    actionButtonLayout->addWidget(imageMirrorLeftRight);
    actionButtonLayout->addWidget(imageMirrorUpDown);
    actionButtonLayout->addWidget(imageDelete);
    actionButtonLayout->addWidget(imageSave);
    actionButtonLayout->addWidget(imageEditExif);

    buttonImageBefore = createImageBefore();
    buttonImageNext = createImageNext();

    imageLabel = createImageLabel();

    buttonLayout->addWidget(buttonImageBefore);
    buttonLayout->addWidget(imageLabel);
    buttonLayout->addWidget(buttonImageNext);
    buttonLayout->setAlignment(Qt::AlignCenter);
}


void ImageEditor::updateButtons() {
    if (data->imagesData.get().size() <= 0) {
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
    if (imageMirrorLeftRight) {

        ClickableLabel* imageMirrorLeftRightNew = createImageMirrorLeftRight();

        actionButtonLayout->replaceWidget(imageMirrorLeftRight, imageMirrorLeftRightNew);

        imageMirrorLeftRight->hide();
        imageMirrorLeftRight->deleteLater();

        imageMirrorLeftRight = imageMirrorLeftRightNew;
    }
    if (imageMirrorUpDown) {

        ClickableLabel* imageMirrorUpDownNew = createImageMirrorUpDown();

        actionButtonLayout->replaceWidget(imageMirrorUpDown, imageMirrorUpDownNew);

        imageMirrorUpDown->hide();
        imageMirrorUpDown->deleteLater();

        imageMirrorUpDown = imageMirrorUpDownNew;
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
        if (data->imagesData.getImageNumber() == 0) {
            buttonImageBefore->setDisabled(true);
        }

    }


    restartImageLabel();

    if (buttonImageNext) {
        if (data->imagesData.getImageNumber() == data->imagesData.get().size() - 1) {
            buttonImageNext->setDisabled(true);
        }

    }


    if (data->imagesData.getImageNumber() != 0) {
        if (!buttonImageBefore->isEnabled())
            buttonImageBefore->setEnabled(true);
    }

    if (data->imagesData.getImageNumber() != data->imagesData.get().size() - 1) {
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

    stopImageOpenTimer();

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
 // ClickableLabel* ImageEditor::createImageDelete() {

 //     if (data->imagesData.get().size() <= 0) {
 //         return nullptr;
 //     }

 //     ClickableLabel* imageDelete = new ClickableLabel(data, ":/delete.png", this, actionSize);

 //     if (data->isDeleted(data->imagesData.getImageNumber())) {

 //         imageDelete->background_color = "#700c13";
 //         imageDelete->updateStyleSheet();
 //         connect(imageDelete, &ClickableLabel::clicked, [this, imageDelete]() { this->
 //             // unDeleteImage();
 //             data->unPreDeleteImage(data->imagesData.getImageNumber());
 //         // updateButtons();

 //         std::cerr << "undelete" << std::endl;
 //             });
 //     }
 //     else {
 //         connect(imageDelete, &ClickableLabel::clicked, [this, imageDelete] { this->
 //             // deleteImage();

 //             data->preDeleteImage(data->imagesData.getImageNumber());
 //         std::cerr << "update" << std::endl;
 //         // elle se rappel elle meme mais pas dans son etat actuel je crois
 //         // updateButtons();
 //         std::cerr << "delete" << std::endl;
 //             });

 //     }
 //     data->deletedImagesData.print();

 //     return imageDelete;
 // }
ClickableLabel* ImageEditor::createImageDelete() {

    if (data->imagesData.get().size() <= 0) {
        return nullptr;
    }

    ClickableLabel* imageDelete = new ClickableLabel(data, ":/delete.png", this, actionSize);

    if (data->isDeleted(data->imagesData.getImageNumber())) {

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

    if (data->imagesData.get().size() <= 0) {
        return nullptr;
    }

    ClickableLabel* imageSaveNew = new ClickableLabel(data, ":/save.png", this, actionSize);

    connect(imageSaveNew, &ClickableLabel::clicked, [this]() { this->
        saveImage();
        });


    return imageSaveNew;
}

ClickableLabel* ImageEditor::createImageRotateRight() {

    if (data->imagesData.get().size() <= 0) {
        return nullptr;
    }

    ClickableLabel* imageRotateRightNew = new ClickableLabel(data, ":/rotateRight.png", this, actionSize);

    if (!isTurnable(data->imagesData.getCurrentImageData()->getImagePath())) {
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

    if (data->imagesData.get().size() <= 0) {
        return nullptr;
    }

    ClickableLabel* imageRotateLeftNew = new ClickableLabel(data, ":/rotateLeft.png", this, actionSize);

    if (!isTurnable(data->imagesData.getCurrentImageData()->getImagePath())) {
        imageRotateLeftNew->setDisabled(true);
    }
    else {
        if (!imageRotateLeftNew->isEnabled())
            imageRotateLeftNew->setEnabled(true);
    }

    connect(imageRotateLeftNew, &ClickableLabel::clicked, [this]() { this->rotateLeft(); });

    return imageRotateLeftNew;
}

ClickableLabel* ImageEditor::createImageMirrorUpDown() {

    if (data->imagesData.get().size() <= 0) {
        return nullptr;
    }

    ClickableLabel* imageMirrorUpDownNew = new ClickableLabel(data, ":/mirrorUpDown.png", this, actionSize);

    if (!isMirrorable(data->imagesData.getCurrentImageData()->getImagePath())) {
        imageMirrorUpDownNew->setDisabled(true);
    }
    else {
        if (!imageMirrorUpDownNew->isEnabled())
            imageMirrorUpDownNew->setEnabled(true);
    }

    connect(imageMirrorUpDownNew, &ClickableLabel::clicked, [this]() { this->mirrorUpDown(); });


    return imageMirrorUpDownNew;
}


ClickableLabel* ImageEditor::createImageMirrorLeftRight() {

    if (data->imagesData.get().size() <= 0) {
        return nullptr;
    }

    ClickableLabel* imageMirrorLeftRightNew = new ClickableLabel(data, ":/mirrorLeftRight.png", this, actionSize);



    if (!isMirrorable(data->imagesData.getCurrentImageData()->getImagePath())) {
        imageMirrorLeftRightNew->setDisabled(true);
        // std::cerr << "disabled" << std::endl;
    }
    else {
        // std::cerr << "enabled" << std::endl;
        if (!imageMirrorLeftRightNew->isEnabled())
            imageMirrorLeftRightNew->setEnabled(true);
    }

    connect(imageMirrorLeftRightNew, &ClickableLabel::clicked, [this]() { this->mirrorLeftRight(); });

    return imageMirrorLeftRightNew;
}




ClickableLabel* ImageEditor::createImageEditExif() {
    if (data->imagesData.get().size() <= 0) {
        return nullptr;
    }

    ClickableLabel* imageEditExifNew = new ClickableLabel(data, ":/editExif.png", this, actionSize);

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

    if (data->imagesData.get().size() <= 0) {
        return nullptr;
    }




    ClickableLabel* buttonImageBeforeNew = new ClickableLabel(data, ":/before.png", this, actionSize);
    // ClickableLabel* buttonImageBeforeNew = new ClickableLabel(":/ressources/before.png", this, actionSize);

    buttonImageBeforeNew->setFixedSize(actionSize);

    if (data->imagesData.getImageNumber() == 0) {
        buttonImageBeforeNew->setDisabled(true);
    }

    connect(buttonImageBeforeNew, &ClickableLabel::clicked, [this]() { this->previousImage(); });
    buttonImageBeforeNew->setFixedSize(actionSize);

    return buttonImageBeforeNew;
}


ClickableLabel* ImageEditor::createImageNext() {

    if (data->imagesData.get().size() <= 0) {
        return nullptr;
    }

    ClickableLabel* buttonImageNextNew = new ClickableLabel(data, ":/next.png", this, actionSize);

    if (data->imagesData.getImageNumber() == data->imagesData.get().size() - 1) {
        buttonImageNextNew->setDisabled(true);
    }

    connect(buttonImageNextNew, &ClickableLabel::clicked, [this]() { this->nextImage(); });

    buttonImageNextNew->setFixedSize(actionSize);

    return buttonImageNextNew;
}


ClickableLabel* ImageEditor::createImagePreview(std::string imagePath, int imageNbr) {

    if (data->imagesData.get().size() <= 0) {
        return nullptr;
    }

    ClickableLabel* previewButton = new ClickableLabel(data, QString::fromStdString(imagePath), this, previewSize, false, 128);


    connect(previewButton, &ClickableLabel::clicked, [this, imageNbr]() {
        data->imagesData.setImageNumber(imageNbr);
        // setImage(data.imagesData->getImageData(data->imagesData.getImageNumber()));
        reload();
        });


    return previewButton;
}

ClickableLabel* ImageEditor::createImageLabel() {

    checkCache();

    if (data->imagesData.get().size() <= 0) {
        return nullptr;
    }

    ClickableLabel* imageLabelNew = new ClickableLabel(data, QString::fromStdString(data->imagesData.getCurrentImageData()->getImagePath()), this, mainImageSize, false);
    connect(imageLabelNew, &ClickableLabel::clicked, [this]() {
        // TODO zoom to be added
        });



    return imageLabelNew;
}

void ImageEditor::restartImageLabel(){

    if (imageLabel) {

        ClickableLabel* imageLabelNew = createImageLabel();

        buttonLayout->replaceWidget(imageLabel, imageLabelNew);

        imageLabel->hide();
        imageLabel->deleteLater();

        imageLabel = imageLabelNew;
    }
}


void ImageEditor::keyPressEvent(QKeyEvent* event) {
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
    case Qt::Key_End:
        imageRotateLeft->background_color = CLICK_BACKGROUND_COLOR;
        imageRotateLeft->updateStyleSheet();
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
    case Qt::Key_Home:
        imageRotateLeft->background_color = CLICK_BACKGROUND_COLOR;
        imageRotateLeft->updateStyleSheet();
        break;

    case Qt::Key_Up:
        if (event->modifiers() & Qt::ControlModifier) {
            imageMirrorLeftRight->background_color = CLICK_BACKGROUND_COLOR;
            imageMirrorLeftRight->updateStyleSheet();
        }
        break;
    case Qt::Key_Down:
        if (event->modifiers() & Qt::ControlModifier) {
            imageMirrorUpDown->background_color = CLICK_BACKGROUND_COLOR;
            imageMirrorUpDown->updateStyleSheet();
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
        else if (event->modifiers() & Qt::MetaModifier) {
            imageRotateLeft->background_color = CLICK_BACKGROUND_COLOR;
            imageRotateLeft->updateStyleSheet();
            data->imagesData.imageNumber = data->imagesData.get().size() - 1;
            reload();
        }
        else {
            buttonImageBefore->background_color = BACKGROUND_COLOR;
            buttonImageBefore->updateStyleSheet();
            previousImage();
        }
        break;
    case Qt::Key_Home:
        imageRotateLeft->background_color = BACKGROUND_COLOR;
        imageRotateLeft->updateStyleSheet();
        data->imagesData.imageNumber = 0;
        reload();
        break;


    case Qt::Key_Right:
        if (event->modifiers() & Qt::ControlModifier) {
            imageRotateRight->background_color = BACKGROUND_COLOR;
            imageRotateRight->updateStyleSheet();
            rotateRight();
        }
        else if (event->modifiers() & Qt::MetaModifier) {
            imageRotateLeft->background_color = CLICK_BACKGROUND_COLOR;
            imageRotateLeft->updateStyleSheet();
            data->imagesData.imageNumber = data->imagesData.get().size() - 1;
            reload();
        }
        else {
            buttonImageNext->background_color = BACKGROUND_COLOR;
            buttonImageNext->updateStyleSheet();
            nextImage();
        }
        break;
    case Qt::Key_End:
        imageRotateLeft->background_color = BACKGROUND_COLOR;
        imageRotateLeft->updateStyleSheet();
        data->imagesData.imageNumber = data->imagesData.get().size() - 1;
        reload();
        break;


    case Qt::Key_Up:
        if (event->modifiers() & Qt::ControlModifier) {
            imageMirrorLeftRight->background_color = BACKGROUND_COLOR;
            imageMirrorLeftRight->updateStyleSheet();
            mirrorLeftRight();
        }
        break;
    case Qt::Key_Down:
        if (event->modifiers() & Qt::ControlModifier) {
            imageMirrorUpDown->background_color = BACKGROUND_COLOR;
            imageMirrorUpDown->updateStyleSheet();
            mirrorUpDown();
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


void ImageEditor::wheelEvent(QWheelEvent* event) {
    int numDegrees = event->angleDelta().y() / 8;
    int numSteps = numDegrees / 15;

    if (numSteps > 0) {
        buttonImageBefore->background_color = BACKGROUND_COLOR;
        buttonImageBefore->updateStyleSheet();
        previousImage(numSteps);
    }
    else if (numSteps < 0) {
        buttonImageNext->background_color = BACKGROUND_COLOR;
        buttonImageNext->updateStyleSheet();
        nextImage(-numSteps);
    }
}




void ImageEditor::saveImage() {
    data->removeDeletedImages();
    if (data->imagesData.get().size() <= 0) {
        clear();
    }
    data->imagesData.saveImagesData(IMAGESDATA_SAVE_DAT_PATH);

    data->imagesData.setImageNumber(0);
    reload();
}


void ImageEditor::deleteImage() {
    // if (data->isDeleted(data->imagesData.getImageNumber())) {


    // }
    // else {


    // }
}


void ImageEditor::populateMetadataFields() {
    ImagesData* imagesData = &data->imagesData;
    ImageData* imageData = imagesData->getCurrentImageData();
    Exiv2::ExifData exifData = imageData->getMetaData()->getExifData();

    nameEdit->clear();
    dateEdit->setDateTime(QDateTime::currentDateTime());
    geoEdit->clear();
    descriptionEdit->clear();

    nameEdit->setText(QString::fromStdString(imageData->getImageName()));

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
    ImagesData* imagesData = &data->imagesData;
    ImageData* imageData = imagesData->getCurrentImageData();
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


void ImageEditor::startImageOpenTimer() {
    if (imageOpenTimer) {
        imageOpenTimer->disconnect();
        imageOpenTimer->stop();
    }

    connect(imageOpenTimer, &QTimer::timeout, this, [this]() {
        data->loadInCache(data->imagesData.getCurrentImageData()->getImagePath());
        restartImageLabel();
        imageOpenTimer->stop();
        });

    imageOpenTimer->setInterval(TIME_BEFORE_FULL_QUALITY);
    imageOpenTimer->start();


    // precharge les images des alentours
    for (int i = 0; i < PRE_LOAD_RADIUS; i++){

        if (imagePreviewOpenTimers[i]) {
            imagePreviewOpenTimers[i]->disconnect();
            imagePreviewOpenTimers[i]->stop();
            imagePreviewOpenTimers[i]->deleteLater();
        }
        imagePreviewOpenTimers[i] = new QTimer(this);

        connect(imagePreviewOpenTimers[i], &QTimer::timeout, this, [this, i]() {

            int done = 0;
            if (data->imagesData.getImageNumber() - (i + 1) < data->imagesData.get().size()
                && data->imagesData.getImageNumber() - (i + 1) >= 0) {
                if (!data->isInCache(data->imagesData.getImageData(data->imagesData.getImageNumber() - (i + 1))->getImagePath())){
                    data->loadInCache(data->imagesData.getImageData(data->imagesData.getImageNumber() - (i + 1))->getImagePath());
                    done += 1;


                    data->unloadFromCache(data->getThumbnailPath(data->imagesData.getImageData(data->imagesData.getImageNumber() - (i + 1))->getImagePath(), 128));
                    data->unloadFromCache(data->getThumbnailPath(data->imagesData.getImageData(data->imagesData.getImageNumber() - (i + 1))->getImagePath(), 256));
                    data->unloadFromCache(data->getThumbnailPath(data->imagesData.getImageData(data->imagesData.getImageNumber() - (i + 1))->getImagePath(), 512));

                }
            }

            if (data->imagesData.getImageNumber() + (i + 1) < data->imagesData.get().size()
                && data->imagesData.getImageNumber() + (i + 1) >= 0){
                if (!data->isInCache(data->imagesData.getImageData(data->imagesData.getImageNumber() + (i + 1))->getImagePath())){
                    data->loadInCache(data->imagesData.getImageData(data->imagesData.getImageNumber() + (i + 1))->getImagePath());
                    done += 1;

                    data->unloadFromCache(data->getThumbnailPath(data->imagesData.getImageData(data->imagesData.getImageNumber() + (i + 1))->getImagePath(), 128));
                    data->unloadFromCache(data->getThumbnailPath(data->imagesData.getImageData(data->imagesData.getImageNumber() + (i + 1))->getImagePath(), 256));
                    data->unloadFromCache(data->getThumbnailPath(data->imagesData.getImageData(data->imagesData.getImageNumber() + (i + 1))->getImagePath(), 512));

                }
            }
            if (done != 0){
                updatePreview();

            }

            imagePreviewOpenTimers[i]->stop();
            imagePreviewOpenTimers[i]->deleteLater();
            imagePreviewOpenTimers[i] = nullptr;
            });

        imagePreviewOpenTimers[i]->setInterval(TIME_BEFORE_PRE_LOAD_FULL_QUALITY * (i + 1));

        imagePreviewOpenTimers[i]->start();
    }

}

void ImageEditor::stopImageOpenTimer() {
    if (imageOpenTimer) {
        imageOpenTimer->stop();
        imageOpenTimer->deleteLater();
        imageOpenTimer = nullptr;
    }
    for (int i = 0; i < PRE_LOAD_RADIUS; i++){

        if (imagePreviewOpenTimers[i]) {
            imagePreviewOpenTimers[i]->stop();
            imagePreviewOpenTimers[i]->deleteLater();
            imagePreviewOpenTimers[i] = nullptr;
        }
    }
}




void ImageEditor::checkLoadedImage() {
    for (const auto& cache : *data->imageCache) {
        const std::string& imagePath = cache.second.imagePath;
        const std::string& imagePathBis = cache.first;

        // std::cerr << "Loaded image: " << imagePath << std::endl;
        // std::cerr << "Loaded image: " << imagePathBis << std::endl;

        int imageId = data->imagesData.getImageIdByName(imagePath);
        if (imageId != -1){
            // Si l'image est dans le cache et n'est pas une ressource
            // std::cerr << "Image ID: " << imageId << std::endl;
            if (std::abs(data->imagesData.imageNumber - imageId) > 2 * PRE_LOAD_RADIUS){

                // int distance = std::abs(data->imagesData.getImageNumber() - imageId);

                // std::cerr << "unload" << imagePath << " : " << imageId << std::endl;

                // cree des sgementation fault
                // data.unloadFromCache(imagePathBis);

            }
        }
    }
}

// permet de verifier si il y a des images a charger ou decharger du cache
void ImageEditor::checkCache() {
    startImageOpenTimer();
    checkLoadedImage();
}

void ImageEditor::rotateLeft(){
    std::string extension = data->imagesData.getCurrentImageData()->getImageExtension();
    if (extension == ".jpg" || extension == ".jpeg" || extension == ".JPG" || extension == ".JPEG"){
        rotateLeftJpg();
    }
    else if (extension == ".png" || extension == ".PNG"){
        rotateLeftPng();
    }

}
void ImageEditor::rotateRight(){
    std::string extension = data->imagesData.getCurrentImageData()->getImageExtension();
    if (extension == ".jpg" || extension == ".jpeg" || extension == ".JPG" || extension == ".JPEG"){
        rotateRightJpg();
    }
    else if (extension == ".png" || extension == ".PNG"){
        rotateRightPng();
    }
}


void ImageEditor::rotateLeftPng(){
    QString outputPath = QString::fromStdString(data->imagesData.getCurrentImageData()->imagePath);
    QImage image = data->loadImage(this, data->imagesData.getCurrentImageData()->imagePath, QSize(0, 0), false);
    image = image.transformed(QTransform().rotate(-90));
    if (!image.save(outputPath)) {
        std::cerr << "Erreur lors de la sauvegarde de l'image." << std::endl;
    }
    data->unloadFromCache(data->imagesData.getCurrentImageData()->imagePath);
    data->loadInCache(data->imagesData.getCurrentImageData()->imagePath);
    data->createAllThumbnail(data->imagesData.getCurrentImageData()->imagePath, 512);
    reload();

}
void ImageEditor::rotateRightPng(){
    QString outputPath = QString::fromStdString(data->imagesData.getCurrentImageData()->imagePath);
    QImage image = data->loadImage(this, data->imagesData.getCurrentImageData()->imagePath, QSize(0, 0), false);
    image = image.transformed(QTransform().rotate(90));
    if (!image.save(outputPath)) {
        std::cerr << "Erreur lors de la sauvegarde de l'image." << std::endl;
    }
    data->unloadFromCache(data->imagesData.getCurrentImageData()->imagePath);
    data->loadInCache(data->imagesData.getCurrentImageData()->imagePath);
    data->createAllThumbnail(data->imagesData.getCurrentImageData()->imagePath, 512);

    reload();
}

void ImageEditor::mirrorUpDown(){
    std::string extension = data->imagesData.getCurrentImageData()->getImageExtension();
    if (extension == ".jpg" || extension == ".jpeg" || extension == ".JPG" || extension == ".JPEG"){
        mirrorUpDownJpg();
    }
    else if (extension == ".png" || extension == ".PNG"){
        mirrorUpDownPng();
    }
}

void ImageEditor::mirrorLeftRight(){
    std::string extension = data->imagesData.getCurrentImageData()->getImageExtension();
    if (extension == ".jpg" || extension == ".jpeg" || extension == ".JPG" || extension == ".JPEG"){
        mirrorLeftRightJpg();
    }
    else if (extension == ".png" || extension == ".PNG"){
        mirrorLeftRightPng();

    }
}

void ImageEditor::mirrorUpDownJpg(){
    ImagesData* imagesData = &data->imagesData;

    ImageData* imageData = imagesData->getCurrentImageData();
    if (!isTurnable(imageData->getImagePath())) {
        return;
    }

    int orientation = imageData->getImageOrientation();

    std::cerr << "orientation" << orientation << std::endl;
    if (orientation == 1) {
        orientation = 4;
    }
    else if (orientation == 3) {
        orientation = 2;
    }
    else if (orientation == 6) {
        orientation = 5;
    }
    else if (orientation == 8) {
        orientation = 7;
    }
    else if (orientation == 2) {
        orientation = 3;
    }
    else if (orientation == 4) {
        orientation = 1;
    }
    else if (orientation == 5) {
        orientation = 6;
    }
    else if (orientation == 7) {
        orientation = 8;
    }
    std::cerr << "orientation" << orientation << std::endl;

    imageData->turnImage(orientation);

    imageData->saveMetaData();

    data->mirrorImageCache(imageData->getImagePath(), true);

    reload();
}

void ImageEditor::mirrorLeftRightJpg(){
    ImagesData* imagesData = &data->imagesData;

    ImageData* imageData = imagesData->getCurrentImageData();
    if (!isTurnable(imageData->getImagePath())) {
        return;
    }

    int orientation = imageData->getImageOrientation();
    if (orientation == 1) {
        orientation = 2;
    }
    else if (orientation == 3) {
        orientation = 4;
    }
    else if (orientation == 6) {
        orientation = 7;
    }
    else if (orientation == 8) {
        orientation = 5;
    }
    else if (orientation == 2) {
        orientation = 1;
    }
    else if (orientation == 4) {
        orientation = 3;
    }
    else if (orientation == 5) {
        orientation = 8;
    }
    else if (orientation == 7) {
        orientation = 6;
    }
    imageData->turnImage(orientation);

    imageData->saveMetaData();

    data->mirrorImageCache(imageData->getImagePath(), false);


    reload();
}

void ImageEditor::mirrorLeftRightPng(){
    QString outputPath = QString::fromStdString(data->imagesData.getCurrentImageData()->imagePath);
    QImage image = data->loadImage(this, data->imagesData.getCurrentImageData()->imagePath, QSize(0, 0), false);
    image = image.mirrored(true, false);
    if (!image.save(outputPath)) {
        std::cerr << "Erreur lors de la sauvegarde de l'image." << std::endl;
    }
    data->unloadFromCache(data->imagesData.getCurrentImageData()->imagePath);
    data->loadInCache(data->imagesData.getCurrentImageData()->imagePath);
    data->createAllThumbnail(data->imagesData.getCurrentImageData()->imagePath, 512);
    reload();

}
void ImageEditor::mirrorUpDownPng(){
    QString outputPath = QString::fromStdString(data->imagesData.getCurrentImageData()->imagePath);
    QImage image = data->loadImage(this, data->imagesData.getCurrentImageData()->imagePath, QSize(0, 0), false);
    image = image.mirrored(false, true);
    if (!image.save(outputPath)) {
        std::cerr << "Erreur lors de la sauvegarde de l'image." << std::endl;
    }
    data->unloadFromCache(data->imagesData.getCurrentImageData()->imagePath);
    data->loadInCache(data->imagesData.getCurrentImageData()->imagePath);
    data->createAllThumbnail(data->imagesData.getCurrentImageData()->imagePath, 512);

    reload();
}