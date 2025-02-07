#include "ImageEditor.hpp"

ImageEditor::ImageEditor(Data* dat, QWidget* parent)
    : QMainWindow(parent), data(dat) {
    parent->setWindowTitle(IMAGE_EDITOR_WINDOW_NAME);

    QWidget* centralWidget = new QWidget(parent);
    setCentralWidget(centralWidget);
    mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setSpacing(data->sizes.imagesEditorSizes->mainLayoutSpacing);
    mainLayout->setContentsMargins(data->sizes.imagesEditorSizes->mainLayoutMargins[0],
                                   data->sizes.imagesEditorSizes->mainLayoutMargins[1],
                                   data->sizes.imagesEditorSizes->mainLayoutMargins[2],
                                   data->sizes.imagesEditorSizes->mainLayoutMargins[3]);  // Marges autour des bords (gauche, haut, droite, bas)

    fixedFrame = new QFrame();
    fixedFrame->setFixedSize(data->sizes.imagesEditorSizes->mainImageSize);  // Set the desired fixed size
    QVBoxLayout* fixedFrameLayout = new QVBoxLayout(fixedFrame);
    fixedFrameLayout->setAlignment(Qt::AlignCenter);
    fixedFrame->setLayout(fixedFrameLayout);

    imageLabelLayout = new QVBoxLayout();
    imageLabelLayout->setAlignment(Qt::AlignCenter);

    imageLabelLayout->addWidget(fixedFrame);

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
    updatePreview();

    nameEdit = new QLineEdit(this);

    dateEdit = new QDateTimeEdit(this);
    dateEdit->setDisplayFormat("dd/MM/yyyy, HH:mm");
    dateEdit->setCalendarPopup(true);

    QCalendarWidget* calendarWidget = dateEdit->calendarWidget();
    calendarWidget->setGridVisible(true);
    calendarWidget->setFirstDayOfWeek(Qt::Monday);
    calendarWidget->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
    calendarWidget->setNavigationBarVisible(true);

    geoEdit = new QLineEdit(this);
    descriptionEdit = new QLineEdit(this);
    validateButton = new QPushButton("Valider", this);
    connect(validateButton, &QPushButton::clicked, this, &ImageEditor::validateMetadata);

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
    } else {
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

void ImageEditor::reload() {
    ImagesData* imagesData = &data->imagesData;

    updateButtons();
    updatePreview();

    if (exifEditor) {
        populateMetadataFields();
    } else {
        for (int i = 0; i < infoLayout->count(); ++i) {
            QWidget* widget = infoLayout->itemAt(i)->widget();
            if (widget) {
                widget->hide();
            }
        }
    }

    if (imagesData->get()->size() <= 0) {
        addImagesFromFolder(data, this);
        return;
    }
}

void ImageEditor::createPreview() {
    ImagesData* imagesData = &data->imagesData;

    if (imagesData->get()->size() <= 0) {
        return;
    }

    // initialisation des boutons preview
    for (int i = 0; i < PREVIEW_NBR * 2 + 1; ++i) {
        ClickableLabel* previewButton = createImagePreview(IMAGE_PATH_LOADING.toStdString(), 0);
        previewButtonLayout->addWidget(previewButton);
        previewButtons.push_back(previewButton);
    }

    previewButtonLayout->setAlignment(Qt::AlignCenter);
}

void ImageEditor::updatePreview() {
    ImagesData* imagesData = &data->imagesData;

    if (imagesData->get()->size() <= 0) {
        return;
    }

    std::vector<std::string> imagePaths;

    int currentImageNumber = imagesData->getImageNumber();
    int totalImages = imagesData->get()->size();

    int under = 0;
    for (int i = PREVIEW_NBR; i > 0; --i) {
        if (currentImageNumber - i >= 0) {
            imagesData->getImageData(currentImageNumber - i)->loadData();
            imagePaths.push_back(imagesData->getImageData(currentImageNumber - i)->getImagePath());
            under += 1;
        }
    }

    imagePaths.push_back(imagesData->getCurrentImageData()->getImagePath());

    for (int i = 1; i <= PREVIEW_NBR; ++i) {
        if (currentImageNumber + i <= totalImages - 1) {
            imagesData->getImageData(currentImageNumber + i)->loadData();
            imagePaths.push_back(imagesData->getImageData(currentImageNumber + i)->getImagePath());
        }
    }

    for (int i = 0; i < PREVIEW_NBR * 2 + 1; ++i) {
        if (i < imagePaths.size()) {
            int imageNbr = imagesData->getImageNumber() + i - under;
            if (imageNbr == imagesData->getImageNumber()) {
                ClickableLabel* previewButtonNew = createImagePreview(imagePaths[i], imageNbr);
                previewButtonNew->background_color = "#b3b3b3";
                previewButtonNew->updateStyleSheet();

                previewButtonLayout->replaceWidget(previewButtons[i], previewButtonNew);

                previewButtons[i]->hide();
                previewButtons[i]->deleteLater();

                previewButtons[i] = previewButtonNew;
            } else {
                ClickableLabel* previewButtonNew = createImagePreview(imagePaths[i], imageNbr);

                previewButtonLayout->replaceWidget(previewButtons[i], previewButtonNew);

                previewButtons[i]->hide();
                previewButtons[i]->deleteLater();

                previewButtons[i] = previewButtonNew;
            }
        } else {
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
    imageExport = createImageExport();

    imageEditExif = createImageEditExif();

    imageConversion = createImageConversion();

    actionButtonLayout->addWidget(imageRotateRight);
    actionButtonLayout->addWidget(imageRotateLeft);
    actionButtonLayout->addWidget(imageMirrorLeftRight);
    actionButtonLayout->addWidget(imageMirrorUpDown);
    actionButtonLayout->addWidget(imageDelete);
    actionButtonLayout->addWidget(imageSave);
    actionButtonLayout->addWidget(imageExport);
    actionButtonLayout->addWidget(imageConversion);
    actionButtonLayout->addWidget(imageEditExif);

    buttonImageBefore = createImageBefore();
    buttonImageNext = createImageNext();

    imageLabel = createImageLabel();
    fixedFrame->layout()->addWidget(imageLabel);

    checkCache();

    buttonLayout->addWidget(buttonImageBefore);
    buttonLayout->addLayout(imageLabelLayout);
    buttonLayout->addWidget(buttonImageNext);
    buttonLayout->setAlignment(Qt::AlignCenter);
}

void ImageEditor::updateButtons() {
    if (data->imagesData.get()->size() <= 0) {
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
    if (imageExport) {
        ClickableLabel* imageExportNew = createImageExport();

        actionButtonLayout->replaceWidget(imageExport, imageExportNew);

        imageExport->hide();
        imageExport->deleteLater();

        imageExport = imageExportNew;
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

    reloadImageLabel();
    checkCache();

    if (buttonImageNext) {
        if (data->imagesData.getImageNumber() == data->imagesData.get()->size() - 1) {
            buttonImageNext->setDisabled(true);
        }
    }

    if (data->imagesData.getImageNumber() != 0) {
        if (!buttonImageBefore->isEnabled())
            buttonImageBefore->setEnabled(true);
    }

    if (data->imagesData.getImageNumber() != data->imagesData.get()->size() - 1) {
        if (!buttonImageNext->isEnabled())
            buttonImageNext->setEnabled(true);
    }
}

void ImageEditor::clear() {
    stopImageOpen();

    QTimer::singleShot(100, this, [this]() {
        if (imageLabelLayout) {
            QLayoutItem* item;
            while ((item = imageLabelLayout->takeAt(0)) != nullptr) {
                if (item->widget()) {
                    item->widget()->disconnect();
                    item->widget()->hide();
                    item->widget()->deleteLater();
                }
                delete item;
            }
            delete imageLabelLayout;
            imageLabelLayout = nullptr;
        }

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

void ImageEditor::hide() {
    stopImageOpen();

    if (imageLabelLayout) {
        for (int i = 0; i < imageLabelLayout->count(); ++i) {
            QLayoutItem* item = imageLabelLayout->itemAt(i);
            if (item && item->widget()) {
                item->widget()->hide();
            }
        }
    }

    if (actionButtonLayout) {
        for (int i = 0; i < actionButtonLayout->count(); ++i) {
            QLayoutItem* item = actionButtonLayout->itemAt(i);
            if (item && item->widget()) {
                item->widget()->hide();
            }
        }
    }

    if (previewButtonLayout) {
        for (int i = 0; i < previewButtonLayout->count(); ++i) {
            QLayoutItem* item = previewButtonLayout->itemAt(i);
            if (item && item->widget()) {
                item->widget()->hide();
            }
        }
    }

    if (buttonLayout) {
        for (int i = 0; i < buttonLayout->count(); ++i) {
            QLayoutItem* item = buttonLayout->itemAt(i);
            if (item && item->widget()) {
                item->widget()->hide();
            }
        }
    }

    if (mainLayout) {
        for (int i = 0; i < mainLayout->count(); ++i) {
            QLayoutItem* item = mainLayout->itemAt(i);
            if (item && item->widget()) {
                item->widget()->hide();
            }
        }
    }
}

void ImageEditor::unHide() {
    if (imageLabelLayout) {
        for (int i = 0; i < imageLabelLayout->count(); ++i) {
            QLayoutItem* item = imageLabelLayout->itemAt(i);
            if (item && item->widget()) {
                item->widget()->show();
            }
        }
    }
    if (actionButtonLayout) {
        for (int i = 0; i < actionButtonLayout->count(); ++i) {
            QLayoutItem* item = actionButtonLayout->itemAt(i);
            if (item && item->widget()) {
                item->widget()->show();
            }
        }
    }

    if (previewButtonLayout) {
        for (int i = 0; i < previewButtonLayout->count(); ++i) {
            QLayoutItem* item = previewButtonLayout->itemAt(i);
            if (item && item->widget()) {
                item->widget()->show();
            }
        }
    }

    if (buttonLayout) {
        for (int i = 0; i < buttonLayout->count(); ++i) {
            QLayoutItem* item = buttonLayout->itemAt(i);
            if (item && item->widget()) {
                item->widget()->show();
            }
        }
    }

    if (mainLayout) {
        for (int i = 0; i < mainLayout->count(); ++i) {
            QLayoutItem* item = mainLayout->itemAt(i);
            if (item && item->widget()) {
                item->widget()->show();
            }
        }
    }
}

ClickableLabel* ImageEditor::createImageDelete() {
    if (data->imagesData.get()->size() <= 0) {
        return nullptr;
    }

    ClickableLabel* imageDelete = new ClickableLabel(data, ICON_PATH_DELETE, TOOL_IMAGE_EDITOR_DELETE, this, actionSize);

    if (data->isDeleted(data->imagesData.getImageNumber())) {
        imageDelete->background_color = "#700c13";
        imageDelete->updateStyleSheet();
    }
    connect(imageDelete, &ClickableLabel::clicked, [this]() {
        this->deleteImage();
    });

    return imageDelete;
}

ClickableLabel* ImageEditor::createImageSave() {
    if (data->imagesData.get()->size() <= 0) {
        return nullptr;
    }

    ClickableLabel* imageSaveNew = new ClickableLabel(data, ICON_PATH_SAVE, TOOL_IMAGE_EDITOR_SAVE, this, actionSize);

    connect(imageSaveNew, &ClickableLabel::clicked, [this]() {
        this->saveImage();
    });

    return imageSaveNew;
}

ClickableLabel* ImageEditor::createImageExport() {
    if (data->imagesData.get()->size() <= 0) {
        return nullptr;
    }

    ClickableLabel* imageExportNew = new ClickableLabel(data, ICON_PATH_EXPORT, TOOL_IMAGE_EDITOR_EXPORT, this, actionSize);

    connect(imageExportNew, &ClickableLabel::clicked, [this]() {
        this->exportImage();
    });

    return imageExportNew;
}

ClickableLabel* ImageEditor::createImageRotateRight() {
    if (data->imagesData.get()->size() <= 0) {
        return nullptr;
    }

    ClickableLabel* imageRotateRightNew = new ClickableLabel(data, ICON_PATH_ROTATE_RIGHT, TOOL_IMAGE_EDITOR_ROTATE_RIGHT, this, actionSize);

    if (!isTurnable(data->imagesData.getCurrentImageData()->getImagePath())) {
        imageRotateRightNew->setDisabled(true);
    } else {
        if (!imageRotateRightNew->isEnabled())
            imageRotateRightNew->setEnabled(true);
    }

    connect(imageRotateRightNew, &ClickableLabel::clicked, [this]() { this->rotateRight(); });

    return imageRotateRightNew;
}

ClickableLabel* ImageEditor::createImageRotateLeft() {
    if (data->imagesData.get()->size() <= 0) {
        return nullptr;
    }

    ClickableLabel* imageRotateLeftNew = new ClickableLabel(data, ICON_PATH_ROTATE_LEFT, TOOL_IMAGE_EDITOR_ROTATE_LEFT, this, actionSize);

    if (!isTurnable(data->imagesData.getCurrentImageData()->getImagePath())) {
        imageRotateLeftNew->setDisabled(true);
    } else {
        if (!imageRotateLeftNew->isEnabled())
            imageRotateLeftNew->setEnabled(true);
    }

    connect(imageRotateLeftNew, &ClickableLabel::clicked, [this]() { this->rotateLeft(); });

    return imageRotateLeftNew;
}

ClickableLabel* ImageEditor::createImageMirrorUpDown() {
    if (data->imagesData.get()->size() <= 0) {
        return nullptr;
    }

    ClickableLabel* imageMirrorUpDownNew = new ClickableLabel(data, ICON_PATH_MIRROR_UP_DOWN, TOOL_IMAGE_EDITOR_MIRROR_UP_DOWN, this, actionSize);

    if (!isMirrorable(data->imagesData.getCurrentImageData()->getImagePath())) {
        imageMirrorUpDownNew->setDisabled(true);
    } else {
        if (!imageMirrorUpDownNew->isEnabled())
            imageMirrorUpDownNew->setEnabled(true);
    }

    connect(imageMirrorUpDownNew, &ClickableLabel::clicked, [this]() { this->mirrorUpDown(); });

    return imageMirrorUpDownNew;
}

ClickableLabel* ImageEditor::createImageMirrorLeftRight() {
    if (data->imagesData.get()->size() <= 0) {
        return nullptr;
    }

    ClickableLabel* imageMirrorLeftRightNew = new ClickableLabel(data, ICON_PATH_MIRROR_LEFT_RIGHT, TOOL_IMAGE_EDITOR_MIRROR_LEFT_RIGHT, this, actionSize);

    if (!isMirrorable(data->imagesData.getCurrentImageData()->getImagePath())) {
        imageMirrorLeftRightNew->setDisabled(true);

    } else {
        if (!imageMirrorLeftRightNew->isEnabled())
            imageMirrorLeftRightNew->setEnabled(true);
    }

    connect(imageMirrorLeftRightNew, &ClickableLabel::clicked, [this]() { this->mirrorLeftRight(); });

    return imageMirrorLeftRightNew;
}

ClickableLabel* ImageEditor::createImageEditExif() {
    if (data->imagesData.get()->size() <= 0) {
        return nullptr;
    }

    ClickableLabel* imageEditExifNew = new ClickableLabel(data, ICON_PATH_EDIT_EXIF, TOOL_IMAGE_EDITOR_EDIT_EXIF, this, actionSize);

    connect(imageEditExifNew, &ClickableLabel::clicked, [this]() {
        if (exifEditor) {
            exifEditor = false;
        } else {
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

ClickableLabel* ImageEditor::createImageConversion() {
    if (data->imagesData.get()->size() <= 0) {
        return nullptr;
    }

    ClickableLabel* imageConversionNew = new ClickableLabel(data, ICON_PATH_CONVERSION, TOOL_IMAGE_EDITOR_CONVERSION, this, actionSize);

    connect(imageConversionNew, &ClickableLabel::clicked, [this]() {
        launchConversionDialog(QString::fromStdString(data->imagesData.getCurrentImageData()->getImagePath()));
    });

    return imageConversionNew;
}

ClickableLabel* ImageEditor::createImageBefore() {
    if (data->imagesData.get()->size() <= 0) {
        return nullptr;
    }

    ClickableLabel* buttonImageBeforeNew = new ClickableLabel(data, ICON_PATH_BEFORE, "", this, actionSize);

    buttonImageBeforeNew->setFixedSize(actionSize);

    if (data->imagesData.getImageNumber() == 0) {
        buttonImageBeforeNew->setDisabled(true);
    }

    connect(buttonImageBeforeNew, &ClickableLabel::clicked, [this]() { this->previousImage(); });
    buttonImageBeforeNew->setFixedSize(actionSize);

    return buttonImageBeforeNew;
}

ClickableLabel* ImageEditor::createImageNext() {
    if (data->imagesData.get()->size() <= 0) {
        return nullptr;
    }

    ClickableLabel* buttonImageNextNew = new ClickableLabel(data, ICON_PATH_NEXT, "", this, actionSize);

    if (data->imagesData.getImageNumber() == data->imagesData.get()->size() - 1) {
        buttonImageNextNew->setDisabled(true);
    }

    connect(buttonImageNextNew, &ClickableLabel::clicked, [this]() { this->nextImage(); });

    buttonImageNextNew->setFixedSize(actionSize);

    return buttonImageNextNew;
}

ClickableLabel* ImageEditor::createImagePreview(std::string imagePath, int imageNbr) {
    if (data->imagesData.get()->size() <= 0) {
        return nullptr;
    }

    ClickableLabel* previewButton = new ClickableLabel(data, QString::fromStdString(imagePath), "", this, previewSize, false, 128, false);

    connect(previewButton, &ClickableLabel::leftClicked, [this, imageNbr]() {
        data->imagesData.setImageNumber(imageNbr);
        reload();
    });

    return previewButton;
}

MainImage* ImageEditor::createImageLabel() {
    if (data->imagesData.get()->size() <= 0) {
        return nullptr;
    }

    MainImage* imageLabelNew = new MainImage(data, QString::fromStdString(data->imagesData.getCurrentImageData()->getImagePath()), this, mainImageSize, false);

    connect(imageLabelNew, &MainImage::leftClicked, [this]() {
        if (!bigImage) {
            openBigImageLabel();
        }
    });

    connect(imageLabelNew, &MainImage::imageCropted, [this]() {
        updatePreview();
    });

    return imageLabelNew;
}

void ImageEditor::reloadImageLabel() {
    if (imageLabel) {
        MainImage* imageLabelNew = createImageLabel();

        fixedFrame->layout()->replaceWidget(imageLabel, imageLabelNew);

        imageLabel->hide();
        imageLabel->deleteLater();

        imageLabel = imageLabelNew;
    }
}

void ImageEditor::keyPressEvent(QKeyEvent* event) {
    qDebug() << "keyPressEvent called with key:" << event->key();
    if (bigImage || exifEditor) {
        return;
    }
    switch (event->key()) {
        case Qt::Key_Left:
            if (event->modifiers() & Qt::ControlModifier) {
                imageRotateLeft->background_color = CLICK_BACKGROUND_COLOR;
                imageRotateLeft->updateStyleSheet();
            } else {
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
            } else {
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
            QWidget::keyPressEvent(event);
    }
}

void ImageEditor::keyReleaseEvent(QKeyEvent* event) {
    if (bigImage || exifEditor) {
        return;
    }
    switch (event->key()) {
        case Qt::Key_Left:
            if (event->modifiers() & Qt::ControlModifier) {
                imageRotateLeft->background_color = BACKGROUND_COLOR;
                imageRotateLeft->updateStyleSheet();
                rotateLeft();
            } else if (event->modifiers() & Qt::MetaModifier) {
                imageRotateLeft->background_color = CLICK_BACKGROUND_COLOR;
                imageRotateLeft->updateStyleSheet();
                data->imagesData.imageNumber = data->imagesData.get()->size() - 1;
                reload();
            } else {
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
            } else if (event->modifiers() & Qt::MetaModifier) {
                imageRotateLeft->background_color = CLICK_BACKGROUND_COLOR;
                imageRotateLeft->updateStyleSheet();
                data->imagesData.imageNumber = data->imagesData.get()->size() - 1;
                reload();
            } else {
                buttonImageNext->background_color = BACKGROUND_COLOR;
                buttonImageNext->updateStyleSheet();
                nextImage();
            }
            break;
        case Qt::Key_End:
            imageRotateLeft->background_color = BACKGROUND_COLOR;
            imageRotateLeft->updateStyleSheet();
            data->imagesData.imageNumber = data->imagesData.get()->size() - 1;
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

        case Qt::Key_Z:
            if (event->modifiers() & Qt::ControlModifier) {
                if (event->modifiers() & Qt::ShiftModifier) {
                    data->reDoAction();
                } else {
                    data->unDoAction();
                }
            }
            break;

        case Qt::Key_Escape:
            switchToImageBooth();
            break;

        default:
            QWidget::keyReleaseEvent(event);
    }
}

void ImageEditor::wheelEvent(QWheelEvent* event) {
    if (bigImage) {
        return;
    }
    int numDegrees = event->angleDelta().y() / 8;
    int numSteps = numDegrees / 15;

    if (numSteps > 0) {
        buttonImageBefore->background_color = BACKGROUND_COLOR;
        buttonImageBefore->updateStyleSheet();
        previousImage(numSteps);
    } else if (numSteps < 0) {
        buttonImageNext->background_color = BACKGROUND_COLOR;
        buttonImageNext->updateStyleSheet();
        nextImage(-numSteps);
    }
}

void ImageEditor::saveImage() {
    int id = data->imagesData.imageNumber;
    for (int i = 0; i <= data->imagesData.imageNumber; ++i) {
        if (data->isDeleted(i)) {
            id--;
        }
    }
    data->removeDeletedImages();
    if (data->imagesData.get()->size() <= 0) {
        clear();
    }
    data->saveData();

    data->saved = true;

    data->imagesData.setImageNumber(id);

    reload();
}

void ImageEditor::exportImage() {
    std::map<std::string, std::string> result;
    std::map<std::string, Option> map = {
        {"Date in image Name", Option("bool", "false")},
        {"Export path", Option("directory", PICTURES_PATH.toStdString())}};
    result = showOptionsDialog(this, "export", map);
    std::string exportPath = result["Export path"];
    bool dateInName = (result["Date in image Name"] == "true");

    if (exportPath == "") {
        qDebug() << "No export path selected";
        return;
    }

    data->exportImages(exportPath, dateInName);
}

void ImageEditor::deleteImage() {
    int nbr = data->imagesData.getImageNumber();

    bool saved = data->saved;

    if (data->isDeleted(nbr)) {
        data->unPreDeleteImage(nbr);
        data->addAction(
            [this, nbr, saved]() {
                int time = 0;
                if (data->imagesData.imageNumber != nbr) {
                    data->imagesData.imageNumber = nbr;
                    reload();
                    time = TIME_UNDO_VISUALISATION;
                }
                QTimer::singleShot(time, [this, nbr, saved]() {
                    if (saved) {
                        data->saved = true;
                    }
                    data->preDeleteImage(nbr);
                    reload();
                });
            },
            [this, nbr]() {
                int time = 0;
                if (data->imagesData.imageNumber != nbr) {
                    data->imagesData.imageNumber = nbr;
                    reload();
                    time = TIME_UNDO_VISUALISATION;
                }
                QTimer::singleShot(time, [this, nbr]() {
                    data->saved = false;
                    data->unPreDeleteImage(nbr);
                });
            });

        updateButtons();
    } else {
        data->preDeleteImage(nbr);
        data->addAction(
            [this, nbr, saved]() {
                int time = 0;
                if (data->imagesData.imageNumber != nbr) {
                    data->imagesData.imageNumber = nbr;
                    reload();
                    time = TIME_UNDO_VISUALISATION;
                }
                QTimer::singleShot(time, [this, nbr, saved]() {
                    if (saved) {
                        data->saved = true;
                    }
                    data->unPreDeleteImage(nbr);
                    reload();
                });
            },
            [this, nbr]() {
                int time = 0;
                if (data->imagesData.imageNumber != nbr) {
                    data->imagesData.imageNumber = nbr;
                    reload();
                    time = TIME_UNDO_VISUALISATION;
                }
                QTimer::singleShot(time, [this, nbr]() {
                    data->saved = false;
                    data->preDeleteImage(nbr);
                    reload();
                });
            });
        updateButtons();
    }
    data->saved = false;
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

    // imageData->folders.print();

    imageData->saveMetaData();
}

void ImageEditor::startImageOpen() {
    if (imageOpenTimer) {
        imageOpenTimer->disconnect();
        imageOpenTimer->stop();
    }

    connect(imageOpenTimer, &QTimer::timeout, this, [this]() {
        data->loadInCacheAsync(data->imagesData.getCurrentImageData()->getImagePath(), [this]() {
            reloadImageLabel();
        });
        imageOpenTimer->stop();
        for (int i = 0; i < PRE_LOAD_RADIUS; i++) {
            if (data->imagesData.getImageNumber() - (i + 1) < data->imagesData.get()->size() && data->imagesData.getImageNumber() - (i + 1) >= 0) {
                data->loadInCacheAsync(data->imagesData.getImageData(data->imagesData.getImageNumber() - (i + 1))->getImagePath(), nullptr);
            }

            if (data->imagesData.getImageNumber() + (i + 1) < data->imagesData.get()->size() && data->imagesData.getImageNumber() + (i + 1) >= 0) {
                data->loadInCacheAsync(data->imagesData.getImageData(data->imagesData.getImageNumber() + (i + 1))->getImagePath(), nullptr);
            }
        }
    });

    imageOpenTimer->setInterval(TIME_BEFORE_FULL_QUALITY);
    imageOpenTimer->start();
}

void ImageEditor::stopImageOpen() {
    if (imageOpenTimer) {
        imageOpenTimer->disconnect();
        imageOpenTimer->stop();
    }
}

void ImageEditor::checkLoadedImage() {
    std::vector<std::string> toUnload;
    for (const auto& cache : *data->imageCache) {
        const std::string& imagePath = cache.second.imagePath;
        const std::string& imagePathBis = cache.first;

        int imageId = data->imagesData.getImageDataId(imagePath);
        if (imageId != -1) {
            if (std::abs(data->imagesData.imageNumber - imageId) > 2 * PRE_LOAD_RADIUS) {
                toUnload.push_back(imagePathBis);
            }
        }
    }
    for (const auto& imagePath : toUnload) {
        data->unloadFromCache(imagePath);
        data->unloadFromFutures(imagePath);
    }
}

void ImageEditor::checkCache() {
    startImageOpen();
    checkLoadedImage();
}

void ImageEditor::rotateLeft() {
    std::string extension = data->imagesData.getCurrentImageData()->getImageExtension();
    int nbr = data->imagesData.imageNumber;
    data->rotateLeft(nbr, extension, [this]() { reload(); });
}
void ImageEditor::rotateRight() {
    std::string extension = data->imagesData.getCurrentImageData()->getImageExtension();
    int nbr = data->imagesData.imageNumber;
    data->rotateRight(nbr, extension, [this]() { reload(); });
}

void ImageEditor::mirrorUpDown() {
    std::string extension = data->imagesData.getCurrentImageData()->getImageExtension();
    int nbr = data->imagesData.imageNumber;

    data->mirrorUpDown(nbr, extension, [this]() { reload(); });
}

void ImageEditor::mirrorLeftRight() {
    std::string extension = data->imagesData.getCurrentImageData()->getImageExtension();
    int nbr = data->imagesData.imageNumber;

    data->mirrorLeftRight(nbr, extension, [this]() { reload(); });
}

void ImageEditor::openBigImageLabel() {
    bigImage = true;
    hide();

    bigImageLabel = new MainImage(data, QString::fromStdString(data->imagesData.getCurrentImageData()->getImagePath()), this, (data->sizes.imagesEditorSizes->bigImage), false, 0, false, true);
    bigImageLabel->setFixedSize(data->sizes.imagesEditorSizes->bigImage);

    mainLayout->addWidget(bigImageLabel);

    bool oldExifEditor = exifEditor;

    exifEditor = false;
    for (int i = 0; i < infoLayout->count(); ++i) {
        QWidget* widget = infoLayout->itemAt(i)->widget();
        if (widget) {
            widget->hide();
        }
    }

    connect(bigImageLabel, &MainImage::leftClicked, [this, oldExifEditor]() {
        closeBigImageLabel(bigImageLabel, oldExifEditor);
    });
}

void ImageEditor::closeBigImageLabel(MainImage* bigImageLabel, bool oldExifEditor) {
    bigImage = false;

    mainLayout->removeWidget(bigImageLabel);

    bigImageLabel->hide();
    bigImageLabel->deleteLater();

    unHide();

    exifEditor = oldExifEditor;
    for (int i = 0; i < infoLayout->count(); ++i) {
        QWidget* widget = infoLayout->itemAt(i)->widget();
        if (widget) {
            widget->setHidden(false);
        }
    }
    reload();
}