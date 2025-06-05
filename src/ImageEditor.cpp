#include "ImageEditor.hpp"

#include <QApplication>
#include <QCalendarWidget>
#include <QColor>
#include <QComboBox>
#include <QDateTime>
#include <QDateTimeEdit>
#include <QDesktopServices>
#include <QFrame>
#include <QFuture>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QKeyEvent>
#include <QLayout>
#include <QLineEdit>
#include <QMainWindow>
#include <QObject>
#include <QPointer>
#include <QPushButton>
#include <QRegularExpression>
#include <QSize>
#include <QSlider>
#include <QString>
#include <QThread>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>
#include <unordered_set>

#include "ClickableLabel.hpp"
#include "Const.hpp"
#include "Conversion.hpp"
#include "Download.hpp"
#include "FacesRecognition.hpp"
#include "Gimp.hpp"
#include "LoadImage.hpp"
#include "MainImage.hpp"
#include "ObjectRecognition.hpp"
#include "Text.hpp"
#include "Verification.hpp"

/**
 * @brief Constructor for the ImageEditor class
 * @param dat Pointer to the Data object containing application data
 * @param parent Pointer to the parent QWidget (usually the main window)
 */
ImageEditor::ImageEditor(std::shared_ptr<Data> dat, QWidget* parent)
    : QMainWindow(parent), data(dat) {
    this->installEventFilter(this);

    QWidget* centralWidget = new QWidget(parent);
    setCentralWidget(centralWidget);
    mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setSpacing(data->getSizesPtr()->imageEditorSizes->mainLayoutSpacing);
    mainLayout->setContentsMargins(data->getSizesPtr()->imageEditorSizes->mainLayoutMargins[0],
                                   data->getSizesPtr()->imageEditorSizes->mainLayoutMargins[1],
                                   data->getSizesPtr()->imageEditorSizes->mainLayoutMargins[2],
                                   data->getSizesPtr()->imageEditorSizes->mainLayoutMargins[3]);  // Marges autour des bords (gauche, haut, droite, bas)

    fixedFrame = new QFrame();
    fixedFrame->setFixedSize(data->getSizesPtr()->imageEditorSizes->mainImageSize);
    QVBoxLayout* fixedFrameLayout = new QVBoxLayout(fixedFrame);
    fixedFrameLayout->setAlignment(Qt::AlignCenter);
    fixedFrame->setLayout(fixedFrameLayout);

    nameAndDateLayout = new QHBoxLayout();
    nameAndDateLayout->setAlignment(Qt::AlignCenter);
    QWidget* nameAndDateWidget = new QWidget(this);
    nameAndDateWidget->setLayout(nameAndDateLayout);
    nameAndDateWidget->setFixedWidth(fixedFrame->width());

    QLabel* nameLabel = new QLabel(Text::name() + " : ", this);
    nameEdit = new QLabel("", this);
    dateEdit = new QDateTimeEdit(this);
    QLabel* dateLabel = new QLabel("     " + Text::date() + " : ", this);

    nameAndDateLayout->addWidget(nameLabel);
    nameAndDateLayout->addWidget(nameEdit);

    dateEdit->setCalendarPopup(true);
    nameAndDateLayout->addWidget(dateLabel);
    nameAndDateLayout->addWidget(dateEdit);

    QCalendarWidget* calendarWidget = dateEdit->calendarWidget();
    calendarWidget->setGridVisible(true);
    calendarWidget->setFirstDayOfWeek(Qt::Monday);
    calendarWidget->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
    calendarWidget->setNavigationBarVisible(true);

    validateButton = new QPushButton(Text::validate(), this);
    connect(validateButton, &QPushButton::clicked, this, &ImageEditor::validateMetadata);

    buttonLayout = new QVBoxLayout();
    buttonLayout->setAlignment(Qt::AlignCenter);

    if (data.get()->getSizesPtr()->fontSize <= 5) {
        qInfo() << "text size is too small : " << data.get()->getSizesPtr()->fontSize;
        nameLabel->hide();
        nameEdit->hide();
        dateLabel->hide();
        dateEdit->hide();
        validateButton->hide();
    } else {
        QFont font = nameLabel->font();
        font.setPointSize(data.get()->getSizesPtr()->fontSize);
        nameLabel->setFont(font);

        font = nameEdit->font();
        font.setPointSize(data.get()->getSizesPtr()->fontSize);
        nameEdit->setFont(font);

        font = dateLabel->font();
        font.setPointSize(data.get()->getSizesPtr()->fontSize);
        dateLabel->setFont(font);

        font = dateEdit->font();
        font.setPointSize(data.get()->getSizesPtr()->fontSize);
        dateEdit->setFont(font);

        nameAndDateLayout->addWidget(validateButton);
        buttonLayout->addWidget(nameAndDateWidget);
    }

    imageLabelLayout = new QVBoxLayout();
    imageLabelLayout->setAlignment(Qt::AlignCenter);

    imageLabelLayout->addWidget(fixedFrame);

    editionLayout = new QVBoxLayout();
    editionLayout->setAlignment(Qt::AlignCenter);

    actionButtonLayout = new QHBoxLayout();
    actionButtonLayout->setAlignment(Qt::AlignCenter);

    imageLayout = new QHBoxLayout();
    imageLayout->setAlignment(Qt::AlignCenter);

    buttonLayout->addLayout(imageLayout);

    previewButtonLayout = new QHBoxLayout();
    previewButtonLayout->setAlignment(Qt::AlignCenter);

    infoLayout = new QVBoxLayout();
    infoLayout->setAlignment(Qt::AlignCenter);

    mainLayout->addLayout(editionLayout);
    editionLayout->addLayout(actionButtonLayout);
    editionLayout->addLayout(buttonLayout);
    editionLayout->addLayout(previewButtonLayout);
    mainLayout->addLayout(infoLayout);

    editGeo = createMapWidget();

    infoLayout->addWidget(new QLabel(Text::geolocation() + " : ", this));
    infoLayout->addWidget(editGeo);

    createButtons();
    updateButtons();

    createPreview();
    updatePreview();
}

/**
 * @brief Go to the next image in the list
 * @param nbr Number of images to skip (default is 1)
 */
void ImageEditor::nextImage(int nbr) {
    if (nbr < 0) {
        nbr = 0;
    }
    data->getImagesDataPtr()->setImageNumber(data->getImagesDataPtr()->getImageNumber() + nbr);
    reload();
}

/**
 * @brief Go to the previous image in the list
 * @param nbr Number of images to skip (default is 1)
 */
void ImageEditor::previousImage(int nbr) {
    if (nbr < 0) {
        nbr = 0;
    }
    data->getImagesDataPtr()->setImageNumber(data->getImagesDataPtr()->getImageNumber() - nbr);
    reload();
}

/**
 * @brief Reload imageEditor window
 */
void ImageEditor::reload() {
    checkCache();

    if (bigImage) {
        MainImage* bigImageLabelNew = new MainImage(data, QString::fromStdString(data->getImagesDataPtr()->getCurrentImageData()->getImagePath()), (data->getSizesPtr()->imageEditorSizes->bigImage), false, personsEditor);

        bigImageLabelNew->setFixedSize(data->getSizesPtr()->imageEditorSizes->bigImage);
        connect(bigImageLabelNew, &MainImage::leftClicked, [this]() {
            closeBigImageLabel(bigImageLabel);
        });
        mainLayout->replaceWidget(bigImageLabel, bigImageLabelNew);
        bigImageLabel->deleteLater();
        bigImageLabel = bigImageLabelNew;
    } else {
        ImagesData* imagesData = data->getImagesDataPtr();

        updateButtons();
        updatePreview();

        // if (bigImage || mapEditor) {
        //     for (int i = 0; i < infoLayout->count(); ++i) {
        //         QWidget* widget = infoLayout->itemAt(i)->widget();
        //         if (widget) {
        //             widget->hide();
        //         }
        //     }
        // }

        if (imagesData->get()->size() <= 0) {
            addImagesFromFolder(data, this);
            return;
        }
    }
}

/**
 * @brief Initialise a preview of the images in the editor
 */
void ImageEditor::createPreview() {
    ImagesData* imagesData = data->getImagesDataPtr();

    if (imagesData->get()->size() <= 0) {
        return;
    }

    for (int i = 0; i < PREVIEW_NBR * 2 + 1; ++i) {
        ClickableLabel* previewButton = createImagePreview(Const::ImagePath::LOADING.toStdString(), 0);
        previewButtonLayout->addWidget(previewButton);
        previewButtons.push_back(previewButton);
    }

    previewButtonLayout->setAlignment(Qt::AlignCenter);
}

/**
 * @brief Update the preview of the images in the editor
 */
void ImageEditor::updatePreview() {
    ImagesData* imagesData = data->getImagesDataPtr();

    if (imagesData->get()->size() <= 0) {
        return;
    }

    std::vector<std::string> imagePaths;

    int currentImageNumber = imagesData->getImageNumber();
    int totalImages = data->getImagesDataPtr()->getCurrent()->size();

    int under = 0;
    for (int i = PREVIEW_NBR; i > 0; --i) {
        if (currentImageNumber - i >= 0) {
            ImageData* imageData = imagesData->getImageDataInCurrent(currentImageNumber - i);
            imageData->loadData();
            imagePaths.push_back(imageData->getImagePath());
            under += 1;
        }
    }

    imagePaths.push_back(imagesData->getCurrentImageData()->getImagePath());

    for (int i = 1; i <= PREVIEW_NBR; ++i) {
        if (currentImageNumber + i <= totalImages - 1) {
            ImageData* imageData = imagesData->getImageDataInCurrent(currentImageNumber + i);
            imageData->loadData();
            imagePaths.push_back(imageData->getImagePath());
        }
    }

    for (int i = 0; i < PREVIEW_NBR * 2 + 1; ++i) {
        if (i < imagePaths.size()) {
            int imageNbr = imagesData->getImageNumber() + i - under;
            if (imageNbr == imagesData->getImageNumber()) {
                ClickableLabel* previewButtonNew = createImagePreview(imagePaths[i], imageNbr);

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

/**
 * @brief Create buttons for the imageEditor
 */
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

    imagePersons = createImageDetection();
    imageGimp = createImageGimp();

    actionButtonLayout->addWidget(imageRotateRight);
    actionButtonLayout->addWidget(imageRotateLeft);
    actionButtonLayout->addWidget(imageMirrorLeftRight);
    actionButtonLayout->addWidget(imageMirrorUpDown);
    actionButtonLayout->addWidget(imageGimp);

    actionButtonLayout->addWidget(imageDelete);
    actionButtonLayout->addWidget(imageSave);
    actionButtonLayout->addWidget(imageExport);
    actionButtonLayout->addWidget(imageConversion);

    actionButtonLayout->addWidget(imageEditExif);
    actionButtonLayout->addWidget(imagePersons);

    buttonImageBefore = createImageBefore();
    buttonImageNext = createImageNext();

    imageLabel = createImageLabel();
    fixedFrame->layout()->addWidget(imageLabel);

    checkCache();

    imageLayout->addWidget(buttonImageBefore);
    imageLayout->addLayout(imageLabelLayout);
    imageLayout->addWidget(buttonImageNext);
    imageLayout->setAlignment(Qt::AlignCenter);
}

/**
 * @brief Update the buttons in the image editor based on the current image state
 */
void ImageEditor::updateButtons() {
    if (data->getImagesDataPtr()->get()->size() <= 0) {
        return;
    } else if (bigImage) {
        return;
    }
    data->getImagesDataPtr()->getCurrentImageData()->loadData();

    if (!mapEditor) {
        for (int i = 0; i < infoLayout->count(); ++i) {
            QWidget* widget = infoLayout->itemAt(i)->widget();
            if (widget) {
                widget->hide();
            }
        }
    }

    if (nameEdit) {
        nameEdit->setText(QString::fromStdString(data->getImagesDataPtr()->getCurrentImageData()->getExportImageName()));
    }

    if (dateEdit) {
        ImagesData* imagesData = data->getImagesDataPtr();
        ImageData* imageData = imagesData->getCurrentImageData();
        Exiv2::ExifData exifData = imageData->getMetaDataPtr()->getExifData();

        if (data->getDarkMode()) {
            dateEdit->setStyleSheet("QDateTimeEdit { color: white; }");
        } else {
            dateEdit->setStyleSheet("QDateTimeEdit { color: black; }");
        }

        if (exifData["Exif.Image.DateTime"].count() != 0) {
            QString dateTimeStr = QString::fromStdString(exifData["Exif.Image.DateTime"].toString());
            QDateTime dateTime = QDateTime::fromString(dateTimeStr, "yyyy:MM:dd HH:mm:ss");
            dateEdit->setDateTime(dateTime);
            if (!dateTime.isValid()) {
                dateEdit->setDateTime(QDateTime::currentDateTime());
                dateEdit->setStyleSheet("QDateTimeEdit { color: red; }");
            }
        } else {
            dateEdit->setDateTime(QDateTime::currentDateTime());
            dateEdit->setStyleSheet("QDateTimeEdit { color: red; }");
        }
        dateEdit->update();
    }

    if (imageRotateRight) {
        if (!isTurnable(data->getImagesDataPtr()->getCurrentImageData()->getImagePath())) {
            imageRotateRight->setDisabled(true);
        } else {
            if (!imageRotateRight->isEnabled())
                imageRotateRight->setEnabled(true);
        }
    }

    if (imageRotateLeft) {
        if (!isTurnable(data->getImagesDataPtr()->getCurrentImageData()->getImagePath())) {
            imageRotateLeft->setDisabled(true);
        } else {
            if (!imageRotateLeft->isEnabled())
                imageRotateLeft->setEnabled(true);
        }
    }
    if (imageMirrorLeftRight) {
        if (!isMirrorable(data->getImagesDataPtr()->getCurrentImageData()->getImagePath())) {
            imageMirrorLeftRight->setDisabled(true);
        } else {
            if (!imageMirrorLeftRight->isEnabled())
                imageMirrorLeftRight->setEnabled(true);
        }
    }
    if (imageMirrorUpDown) {
        if (!isMirrorable(data->getImagesDataPtr()->getCurrentImageData()->getImagePath())) {
            imageMirrorUpDown->setDisabled(true);
        } else {
            if (!imageMirrorUpDown->isEnabled())
                imageMirrorUpDown->setEnabled(true);
        }
    }
    if (imageDelete) {
        if (data->isDeleted(data->getImagesDataPtr()->getImageNumberInTotal())) {
            imageDelete->setBackground(Const::Color::DARK_RED, Const::Color::LIGHT_RED);
        } else {
            imageDelete->resetBackground();
        }
    }
    if (buttonImageBefore) {
        if (data->getImagesDataPtr()->getImageNumber() == 0) {
            buttonImageBefore->setDisabled(true);
        } else {
            buttonImageBefore->setEnabled(true);
        }
    }

    reloadImageLabel();

    if (buttonImageNext) {
        if (data->getImagesDataPtr()->getImageNumber() == data->getImagesDataPtr()->getCurrent()->size() - 1) {
            buttonImageNext->setDisabled(true);
        } else {
            buttonImageNext->setEnabled(true);
        }
    }

    if (imagePersons) {
        if (data->getImagesDataPtr()->getCurrentImageData()->isDetectionStatusLoaded()) {
            imagePersons->setLogoNumber(data->getImagesDataPtr()->getCurrentImageData()->getDetectedObjects()["person"].size());
        } else {
            imagePersons->setLogoNumber(-1);
        }
    }
}

/**
 * @brief Clear the image editor and stop any image opening process
 */
void ImageEditor::clear() {
    stopImageOpen();
}

/**
 * @brief Hide imageEditor and its components
 */
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

    if (imageLayout) {
        for (int i = 0; i < imageLayout->count(); ++i) {
            QLayoutItem* item = imageLayout->itemAt(i);
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

/**
 * @brief Unhide imageEditor and its components
 */
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

    if (imageLayout) {
        for (int i = 0; i < imageLayout->count(); ++i) {
            QLayoutItem* item = imageLayout->itemAt(i);
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

/**
 * @brief Create a delete button for the image editor
 * @return A pointer to the ClickableLabel object representing the delete button
 */
ClickableLabel* ImageEditor::createImageDelete() {
    if (data->getImagesDataPtr()->get()->size() <= 0) {
        return nullptr;
    }

    ClickableLabel* imageDeleteNew = new ClickableLabel(data, Const::IconPath::DELETE_ICON, Text::Tooltip::ImageEditor::delete_tip(), this, actionSize);
    imageDeleteNew->setInitialBackground(Const::Color::TRANSPARENT1, Const::Color::LIGHT_GRAY);

    if (data->isDeleted(data->getImagesDataPtr()->getImageNumberInTotal())) {
        imageDeleteNew->setBackground(Const::Color::DARK_RED, Const::Color::LIGHT_RED);
    }
    connect(imageDeleteNew, &ClickableLabel::clicked, [this]() {
        this->deleteImage();
    });

    return imageDeleteNew;
}

/**
 * @brief Create a save button for the image editor
 * @return A pointer to the ClickableLabel object representing the save button
 */
ClickableLabel* ImageEditor::createImageSave() {
    if (data->getImagesDataPtr()->get()->size() <= 0) {
        return nullptr;
    }

    ClickableLabel* imageSaveNew = new ClickableLabel(data, Const::IconPath::SAVE, Text::Tooltip::ImageEditor::save(), this, actionSize);
    imageSaveNew->setInitialBackground(Const::Color::TRANSPARENT1, Const::Color::LIGHT_GRAY);

    connect(imageSaveNew, &ClickableLabel::clicked, [this]() {
        this->saveImage();
    });

    return imageSaveNew;
}

/**
 * @brief Create an export button for the image editor
 * @return A pointer to the ClickableLabel object representing the export button
 */
ClickableLabel* ImageEditor::createImageExport() {
    if (data->getImagesDataPtr()->get()->size() <= 0) {
        return nullptr;
    }

    ClickableLabel* imageExportNew = new ClickableLabel(data, Const::IconPath::EXPORT, Text::Tooltip::ImageEditor::export_tip(), this, actionSize);
    imageExportNew->setInitialBackground(Const::Color::TRANSPARENT1, Const::Color::LIGHT_GRAY);

    connect(imageExportNew, &ClickableLabel::clicked, [this]() {
        this->exportImage();
    });

    return imageExportNew;
}

/**
 * @brief Create a rotate right button for the image editor
 * @return A pointer to the ClickableLabel object representing the rotate right button
 */
ClickableLabel* ImageEditor::createImageRotateRight() {
    ClickableLabel* imageRotateRightNew = new ClickableLabel(data, Const::IconPath::ROTATE_RIGHT, Text::Tooltip::ImageEditor::rotate_right(), this, actionSize);
    imageRotateRightNew->setInitialBackground(Const::Color::TRANSPARENT1, Const::Color::LIGHT_GRAY);

    if (!isTurnable(data->getImagesDataPtr()->getCurrentImageData()->getImagePath())) {
        imageRotateRightNew->setDisabled(true);
    } else {
        if (!imageRotateRightNew->isEnabled())
            imageRotateRightNew->setEnabled(true);
    }

    connect(imageRotateRightNew, &ClickableLabel::clicked, [this]() { this->rotateRight(); });

    return imageRotateRightNew;
}

/**
 * @brief Create a rotate left button for the image editor
 * @return A pointer to the ClickableLabel object representing the rotate left button
 */
ClickableLabel* ImageEditor::createImageRotateLeft() {
    if (data->getImagesDataPtr()->get()->size() <= 0) {
        return nullptr;
    }

    ClickableLabel* imageRotateLeftNew = new ClickableLabel(data, Const::IconPath::ROTATE_LEFT, Text::Tooltip::ImageEditor::rotate_left(), this, actionSize);
    imageRotateLeftNew->setInitialBackground(Const::Color::TRANSPARENT1, Const::Color::LIGHT_GRAY);

    if (!isTurnable(data->getImagesDataPtr()->getCurrentImageData()->getImagePath())) {
        imageRotateLeftNew->setDisabled(true);
    } else {
        if (!imageRotateLeftNew->isEnabled())
            imageRotateLeftNew->setEnabled(true);
    }

    connect(imageRotateLeftNew, &ClickableLabel::clicked, [this]() { this->rotateLeft(); });

    return imageRotateLeftNew;
}

/**
 * @brief Create a mirror up-down button for the image editor
 * @return A pointer to the ClickableLabel object representing the mirror up-down button
 */
ClickableLabel* ImageEditor::createImageMirrorUpDown() {
    if (data->getImagesDataPtr()->get()->size() <= 0) {
        return nullptr;
    }

    ClickableLabel* imageMirrorUpDownNew = new ClickableLabel(data, Const::IconPath::MIRROR_UP_DOWN, Text::Tooltip::ImageEditor::mirror_up_down(), this, actionSize);
    imageMirrorUpDownNew->setInitialBackground(Const::Color::TRANSPARENT1, Const::Color::LIGHT_GRAY);

    if (!isMirrorable(data->getImagesDataPtr()->getCurrentImageData()->getImagePath())) {
        imageMirrorUpDownNew->setDisabled(true);
    } else {
        if (!imageMirrorUpDownNew->isEnabled())
            imageMirrorUpDownNew->setEnabled(true);
    }

    connect(imageMirrorUpDownNew, &ClickableLabel::clicked, [this]() { this->mirrorUpDown(); });

    return imageMirrorUpDownNew;
}

/**
 * @brief Create a mirror left-right button for the image editor
 * @return A pointer to the ClickableLabel object representing the mirror left-right button
 */
ClickableLabel* ImageEditor::createImageMirrorLeftRight() {
    if (data->getImagesDataPtr()->get()->size() <= 0) {
        return nullptr;
    }

    ClickableLabel* imageMirrorLeftRightNew = new ClickableLabel(data, Const::IconPath::MIRROR_LEFT_RIGHT, Text::Tooltip::ImageEditor::mirror_left_right(), this, actionSize);
    imageMirrorLeftRightNew->setInitialBackground(Const::Color::TRANSPARENT1, Const::Color::LIGHT_GRAY);

    if (!isMirrorable(data->getImagesDataPtr()->getCurrentImageData()->getImagePath())) {
        imageMirrorLeftRightNew->setDisabled(true);

    } else {
        if (!imageMirrorLeftRightNew->isEnabled())
            imageMirrorLeftRightNew->setEnabled(true);
    }

    connect(imageMirrorLeftRightNew, &ClickableLabel::clicked, [this]() { this->mirrorLeftRight(); });

    return imageMirrorLeftRightNew;
}

/**
 * @brief Create a button to open the editor for EXIF metadata for the image editor
 * @return A pointer to the ClickableLabel object representing the edit EXIF button
 */
ClickableLabel* ImageEditor::createImageEditExif() {
    if (data->getImagesDataPtr()->get()->size() <= 0) {
        return nullptr;
    }

    ClickableLabel* imageEditExifNew = new ClickableLabel(data, Const::IconPath::MAP, Text::Tooltip::ImageEditor::map(), this, actionSize);
    imageEditExifNew->setInitialBackground(Const::Color::TRANSPARENT1, Const::Color::LIGHT_GRAY);

    connect(imageEditExifNew, &ClickableLabel::clicked, [this]() {
        if (mapEditor) {
            mapEditor = false;
        } else {
            mapEditor = true;
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

/**
 * @brief Create a button to open the convertion editor for the image editor
 * @return A pointer to the ClickableLabel object representing the conversion button
 */
ClickableLabel* ImageEditor::createImageConversion() {
    if (data->getImagesDataPtr()->get()->size() <= 0) {
        return nullptr;
    }

    ClickableLabel* imageConversionNew = new ClickableLabel(data, Const::IconPath::CONVERSION, Text::Tooltip::ImageEditor::conversion(), this, actionSize);
    imageConversionNew->setInitialBackground(Const::Color::TRANSPARENT1, Const::Color::LIGHT_GRAY);

    connect(imageConversionNew, &ClickableLabel::clicked, [this]() {
        launchConversionDialog(data->getImagesDataPtr()->getCurrentImageData());
        reload();
    });

    return imageConversionNew;
}

/**
 * @brief Create a button to open the persons editor for the image editor
 * @return A pointer to the ClickableLabel object representing the persons editor button
 */
ClickableLabel* ImageEditor::createImageDetection() {
    if (data->getImagesDataPtr()->get()->size() <= 0) {
        return nullptr;
    }

    ClickableLabel* imagePersonsNew = new ClickableLabel(data, Const::IconPath::EDIT_PERSONS, Text::Tooltip::ImageEditor::edit_persons(), this, actionSize);
    imagePersonsNew->setInitialBackground(Const::Color::TRANSPARENT1, Const::Color::LIGHT_GRAY);
    imagePersonsNew->addLogo(QColor::fromString(Const::Color::DARK_RED), QColor::fromString(Const::Color::WHITE));

    if (data->getImagesDataPtr()->getCurrentImageData()->isDetectionStatusLoaded()) {
        imagePersonsNew->setLogoNumber(data->getImagesDataPtr()->getCurrentImageData()->getDetectedObjects()["person"].size());
    } else {
        imagePersonsNew->setLogoNumber(-1);
    }

    connect(imagePersonsNew, &ClickableLabel::leftClicked, [this, imagePersonsNew]() {
        if (!personsEditor) {
            personsEditor = true;
            imagePersonsNew->setBackground(Const::Color::DARK_RED, Const::Color::LIGHT_RED);

            imageLabel->setPersonsEditor(true);
        } else {
            personsEditor = false;
            imagePersonsNew->resetBackground();
            imageLabel->setPersonsEditor(false);
        }
        imageLabel->update();
    });

    connect(imagePersonsNew, &ClickableLabel::rightClicked, [this, imagePersonsNew]() {
        QStringList yoloModels = {"yolov5n.onnx - Nano model, fastest but less accurate",
                                  "yolov5s.onnx - Small model, good balance of speed and accuracy",
                                  "yolov5m.onnx - Medium model, more accurate but slower",
                                  "yolov5l.onnx - Large model, higher accuracy, slower",
                                  "yolov5x.onnx - Extra-large model, most accurate but slowest"};
        QString currentModel = QString::fromStdString(data->getModelConst().getModelName());
        for (int i = 0; i < yoloModels.size(); ++i) {
            if (yoloModels[i].startsWith(currentModel)) {
                yoloModels[i] = yoloModels[i] + " -> [Selected]";
            }
        }

        QDialog dialog(this);
        dialog.setWindowTitle("Select YOLO Model and Confidence");

        QVBoxLayout* dialogLayout = new QVBoxLayout(&dialog);

        QLabel* modelLabel = new QLabel("Choose a YOLO model:", &dialog);
        QComboBox* modelComboBox = new QComboBox(&dialog);
        modelComboBox->addItems(yoloModels);
        modelComboBox->setCurrentText(currentModel);

        dialogLayout->addWidget(modelLabel);
        dialogLayout->addWidget(modelComboBox);

        QSlider* confidenceSlider = new QSlider(Qt::Horizontal, &dialog);
        confidenceSlider->setRange(0, 100);                                                         // Confidence range from 0 to 100
        confidenceSlider->setValue(static_cast<int>(data->getModelConst().getConfidence() * 100));  // Set initial value
        QLabel* confidenceLabel = new QLabel(QString("Confidence: %1%").arg(confidenceSlider->value()), &dialog);

        connect(confidenceSlider, &QSlider::valueChanged, [this, confidenceLabel](int value) {
            float confidence = value / 100.0f;  // Convert slider value to float
            confidenceLabel->setText(QString("Confidence: %1%").arg(value));
            data->getModelPtr()->setConfidence(confidence);  // Update confidence in the model
        });

        dialogLayout->addWidget(confidenceLabel);
        dialogLayout->addWidget(confidenceSlider);

        QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
        dialogLayout->addWidget(buttonBox);

        connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

        if (dialog.exec() == QDialog::Accepted) {
            QString selectedModel = modelComboBox->currentText();
            selectedModel = selectedModel.remove(QRegularExpression("<[^>]*>"));  // Remove HTML tags
            qInfo() << "Selected YOLO Model:" << selectedModel;

            const std::string modelName = selectedModel.split(" - ").first().toStdString();
            data->getModelPtr()->setModelName(modelName);

            ImageData* imageData = data->getImagesDataPtr()->getCurrentImageData();
            int imageNbr = data->getImagesDataPtr()->getImageNumber();
            qDebug() << "createImageDetection";

            QImage image = data->loadImageNormal(data->getImagesDataPtr()->getCurrentImageData()->getImagePath());
            image = rotateQImage(image, imageData);
            std::string currentImagePath = data->getImagesDataPtr()->getCurrentImageData()->getImagePath();

            QPointer<ImageEditor> self = this;
            detectObjectsAsync(data, currentImagePath, image, [self, imageNbr, currentImagePath](DetectedObjects detectedObject) {
                if (!self.isNull()) {
                    ImageData* imageData = self->data->getImagesDataPtr()->getImageData(currentImagePath);

                    if (imageData) {
                        imageData->setDetectedObjects(detectedObject.getDetectedObjects());
                        imageData->setDetectionStatusLoaded();
                    }
                    if (self->data->getImagesDataPtr()->getImageNumber() == imageNbr) {
                        if (self->imagePersons) {
                            self->imagePersons->setLogoNumber(detectedObject.getDetectedObjects()["person"].size());

                            self->imagePersons->update();

                            if (self->imageLabel) {
                                self->imageLabel->update();
                            }
                        }
                    }
                    self->data->detectAndRecognizeFaces(imageData);
                } }, true);
        }
    });

    return imagePersonsNew;
}

/**
 * @brief Create a button to open the image on Gimp
 * @return A pointer to the ClickableLabel object representing the Gimp
 */
ClickableLabel* ImageEditor::createImageGimp() {
    if (data->getImagesDataPtr()->get()->size() <= 0) {
        return nullptr;
    }

    ClickableLabel* imageGimpNew = new ClickableLabel(data, Const::IconPath::GIMP, Text::Tooltip::ImageEditor::gimp(), this, actionSize);
    imageGimpNew->setInitialBackground(Const::Color::TRANSPARENT1, Const::Color::LIGHT_GRAY);

    connect(imageGimpNew, &ClickableLabel::clicked, [this]() {
        if (Gimp::exist()) {
            Gimp::launchWithImage(QString::fromStdString(data->getImagesDataPtr()->getCurrentImageData()->getImagePath()).toStdWString());
        } else {
            QDesktopServices::openUrl(QUrl("https://www.gimp.org/downloads/"));
        }
    });

    return imageGimpNew;
}

/**
 * @brief Create a button to go to the previous image in the image editor
 * @return A pointer to the ClickableLabel object representing the previous image button
 */
ClickableLabel* ImageEditor::createImageBefore() {
    if (data->getImagesDataPtr()->get()->size() <= 0) {
        return nullptr;
    }

    ClickableLabel* buttonImageBeforeNew = new ClickableLabel(data, Const::IconPath::BEFORE, "", this, actionSize);
    buttonImageBeforeNew->setInitialBackground(Const::Color::TRANSPARENT1, Const::Color::LIGHT_GRAY);

    if (data->getImagesDataPtr()->getImageNumber() == 0) {
        buttonImageBeforeNew->setDisabled(true);
    }

    connect(buttonImageBeforeNew, &ClickableLabel::clicked, [this]() { this->previousImage(); });

    return buttonImageBeforeNew;
}

/**
 * @brief Create a button to go to the next image in the image editor
 * @return A pointer to the ClickableLabel object representing the next image button
 */
ClickableLabel* ImageEditor::createImageNext() {
    if (data->getImagesDataPtr()->get()->size() <= 0) {
        return nullptr;
    }

    ClickableLabel* buttonImageNextNew = new ClickableLabel(data, Const::IconPath::NEXT, "", this, actionSize);
    buttonImageNextNew->setInitialBackground(Const::Color::TRANSPARENT1, Const::Color::LIGHT_GRAY);

    if (data->getImagesDataPtr()->getImageNumber() == data->getImagesDataPtr()->getCurrent()->size() - 1) {
        buttonImageNextNew->setDisabled(true);
    }

    connect(buttonImageNextNew, &ClickableLabel::clicked, [this]() { this->nextImage(); });

    // buttonImageNextNew->setFixedSize(&actionSize);

    return buttonImageNextNew;
}

/**
 * @brief Create a ClickableLabel for preview image
 * @param imagePath Path to the image to preview
 * @param imageNbr Number of the image to preview
 * @return The ClickableLabel for the preview
 */
ClickableLabel* ImageEditor::createImagePreview(std::string imagePath, int imageNbr) {
    if (data->getImagesDataPtr()->get()->size() <= 0) {
        return nullptr;
    }

    ClickableLabel* previewButton = new ClickableLabel(data, QString::fromStdString(imagePath), "", this, previewSize, false, Const::Thumbnail::NORMAL_QUALITY, false);
    previewButton->setInitialBorder(Const::Color::TRANSPARENT1, Const::Color::LIGHT_GRAY);
    connect(previewButton, &ClickableLabel::leftClicked, [this, imageNbr]() {
        data->getImagesDataPtr()->setImageNumber(imageNbr);
        reload();
    });

    return previewButton;
}

/**
 * @brief Ceate a MainImage for the image label
 * @return The widget it self with is a MainImage
 */
MainImage* ImageEditor::createImageLabel() {
    if (data->getImagesDataPtr()->get()->size() <= 0) {
        return nullptr;
    }

    MainImage* imageLabelNew = new MainImage(data, QString::fromStdString(data->getImagesDataPtr()->getCurrentImageData()->getImagePath()), *mainImageSize, false, personsEditor);

    std::string currentImagePath = data->getImagesDataPtr()->getCurrentImageData()->getImagePath();

    ImageData* imageData = data->getImagesDataPtr()->getCurrentImageData();

    connect(imageLabelNew, &MainImage::imageCropted, [this]() {
        updatePreview();
    });

    if (!data->isInCache(currentImagePath)) {
        return imageLabelNew;
    }

    connect(imageLabelNew, &MainImage::leftClicked, [this]() {
        if (!bigImage) {
            openBigImageLabel();
        }
    });

    int imageNbr = data->getImagesDataPtr()->getImageNumber();
    if (imageData->isDetectionStatusNotLoaded() && data->getConnectionEnabled()) {
        imageData->setDetectionStatusLoading();

        QImage image = data->loadImageNormal(data->getImagesDataPtr()->getCurrentImageData()->getImagePath());

        image = rotateQImage(image, imageData);

        QPointer<ImageEditor> self = this;

        detectObjectsAsync(data, currentImagePath, image, [self, imageNbr, currentImagePath](DetectedObjects detectedObject) {
            if (!self.isNull()) {
                ImageData* imageData = self->data->getImagesDataPtr()->getImageData(currentImagePath);

                if (imageData) {
                    imageData->setDetectedObjects(detectedObject.getDetectedObjects());
                    imageData->setDetectionStatusLoaded();
                }
                if (self->data->getImagesDataPtr()->getImageNumber() == imageNbr) {
                    if (self->imagePersons) {
                        self->imagePersons->setLogoNumber(detectedObject.getDetectedObjects()["person"].size());

                        self->imagePersons->update();

                        if (self->imageLabel) {
                            self->imageLabel->update();
                        }
                    }
                }
                self->data->detectAndRecognizeFaces(imageData);
            }
        });
    }

    return imageLabelNew;
}

/**
 * @brief Reload the image label in the image editor
 */
void ImageEditor::reloadImageLabel() {
    if (imageLabel) {
        MainImage* imageLabelNew = createImageLabel();
        populateMetadataFields();

        fixedFrame->layout()->replaceWidget(imageLabel, imageLabelNew);

        imageLabel->hide();
        imageLabel->deleteLater();

        imageLabel = imageLabelNew;
    }
}

/**
 * @brief Handle key press events in the image editor
 * @param event The key event to handle
 */
void ImageEditor::keyPressEvent(QKeyEvent* event) {
    switch (event->key()) {
        case Qt::Key_Left:
            if (event->modifiers() & Qt::ControlModifier) {
                imageRotateLeft->setBackground(CLICK_BACKGROUND_COLOR, CLICK_BACKGROUND_COLOR);
            } else {
                if (dateEdit->hasFocus()) {
                    return;
                }
                buttonImageBefore->setBackground(CLICK_BACKGROUND_COLOR, CLICK_BACKGROUND_COLOR);
            }
            break;
        case Qt::Key_End:
            imageRotateLeft->setBackground(CLICK_BACKGROUND_COLOR, CLICK_BACKGROUND_COLOR);
            break;

        case Qt::Key_Right:
            if (event->modifiers() & Qt::ControlModifier) {
                imageRotateRight->setBackground(CLICK_BACKGROUND_COLOR, CLICK_BACKGROUND_COLOR);
            } else {
                if (dateEdit->hasFocus()) {
                    return;
                }
                buttonImageNext->setBackground(CLICK_BACKGROUND_COLOR, CLICK_BACKGROUND_COLOR);
            }
            break;
        case Qt::Key_Home:
            imageRotateLeft->setBackground(CLICK_BACKGROUND_COLOR, CLICK_BACKGROUND_COLOR);
            break;

        case Qt::Key_Up:
            if (event->modifiers() & Qt::ControlModifier) {
                imageMirrorLeftRight->setBackground(CLICK_BACKGROUND_COLOR, CLICK_BACKGROUND_COLOR);
            }
            break;
        case Qt::Key_Down:
            if (event->modifiers() & Qt::ControlModifier) {
                imageMirrorUpDown->setBackground(CLICK_BACKGROUND_COLOR, CLICK_BACKGROUND_COLOR);
            }
            break;

        case Qt::Key_S:
            if (event->modifiers() & Qt::ControlModifier) {
                imageSave->setBackground(CLICK_BACKGROUND_COLOR, CLICK_BACKGROUND_COLOR);
            }
            break;
        case Qt::Key_Delete:
            imageDelete->setBackground(CLICK_BACKGROUND_COLOR, CLICK_BACKGROUND_COLOR);
            break;
        case Qt::Key_Backspace:
            imageDelete->setBackground(CLICK_BACKGROUND_COLOR, CLICK_BACKGROUND_COLOR);

            break;
        case Qt::Key_D:
            if (event->modifiers() & Qt::ControlModifier) {
                imageDelete->setBackground(CLICK_BACKGROUND_COLOR, CLICK_BACKGROUND_COLOR);
            }
            break;

        default:
            QWidget::keyPressEvent(event);
    }
}

/**
 * @brief Handle key release events in the image editor
 * @param event The key event to handle
 */
void ImageEditor::keyReleaseEvent(QKeyEvent* event) {
    if (dateEdit->hasFocus()) {
        return;
    }

    switch (event->key()) {
        case Qt::Key_Left:
            if (event->modifiers() & Qt::ControlModifier) {
                imageRotateLeft->setBackground(BACKGROUND_COLOR, BACKGROUND_COLOR);
                rotateLeft();
            } else if (event->modifiers() & Qt::MetaModifier) {
                imageRotateLeft->setBackground(CLICK_BACKGROUND_COLOR, CLICK_BACKGROUND_COLOR);
                data->getImagesDataPtr()->setImageNumber(data->getImagesDataPtr()->get()->size() - 1);
                reload();
            } else {
                if (dateEdit->hasFocus()) {
                    return;
                }
                buttonImageBefore->setBackground(BACKGROUND_COLOR, BACKGROUND_COLOR);
                previousImage();
            }
            break;
        case Qt::Key_Home:
            imageRotateLeft->setBackground(BACKGROUND_COLOR, BACKGROUND_COLOR);
            reload();
            break;

        case Qt::Key_Right:
            if (event->modifiers() & Qt::ControlModifier) {
                imageRotateRight->setBackground(BACKGROUND_COLOR, BACKGROUND_COLOR);
                rotateRight();
            } else if (event->modifiers() & Qt::MetaModifier) {
                imageRotateLeft->setBackground(CLICK_BACKGROUND_COLOR, CLICK_BACKGROUND_COLOR);
                data->getImagesDataPtr()->setImageNumber(data->getImagesDataPtr()->get()->size() - 1);
                reload();
            } else {
                if (dateEdit->hasFocus()) {
                    return;
                }
                buttonImageNext->setBackground(BACKGROUND_COLOR, BACKGROUND_COLOR);
                nextImage();
            }
            break;
        case Qt::Key_End:
            imageRotateLeft->setBackground(BACKGROUND_COLOR, BACKGROUND_COLOR);
            data->getImagesDataPtr()->setImageNumber(data->getImagesDataPtr()->get()->size() - 1);
            reload();
            break;

        case Qt::Key_Up:
            if (event->modifiers() & Qt::ControlModifier) {
                imageMirrorLeftRight->setBackground(BACKGROUND_COLOR, BACKGROUND_COLOR);
                mirrorLeftRight();
            }
            break;
        case Qt::Key_Down:
            if (event->modifiers() & Qt::ControlModifier) {
                imageMirrorUpDown->setBackground(BACKGROUND_COLOR, BACKGROUND_COLOR);
                mirrorUpDown();
            }
            break;

        case Qt::Key_S:
            if (event->modifiers() & Qt::ControlModifier) {
                imageSave->setBackground(BACKGROUND_COLOR, BACKGROUND_COLOR);
                saveImage();
            }
            break;
        case Qt::Key_Delete:
            imageDelete->setBackground(BACKGROUND_COLOR, BACKGROUND_COLOR);
            deleteImage();
            break;
        case Qt::Key_Backspace:
            imageDelete->setBackground(BACKGROUND_COLOR, BACKGROUND_COLOR);
            deleteImage();
            break;
        case Qt::Key_D:
            if (event->modifiers() & Qt::ControlModifier) {
                imageDelete->setBackground(BACKGROUND_COLOR, BACKGROUND_COLOR);
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

/**
 * @brief Handle wheel events in the image editor
 * @param event The wheel event to handle
 */
void ImageEditor::wheelEvent(QWheelEvent* event) {
    int numDegrees = event->angleDelta().y() / 8;
    int numSteps = numDegrees / 15;

    if (numSteps > 0) {
        buttonImageBefore->setBackground(BACKGROUND_COLOR, BACKGROUND_COLOR);
        previousImage(numSteps);
    } else if (numSteps < 0) {
        buttonImageNext->setBackground(BACKGROUND_COLOR, BACKGROUND_COLOR);
        nextImage(-numSteps);
    }
}

/**
 * @brief Save the image and update deleted images
 * @details This function updates the image number and removes deleted images from the data.
 */
void ImageEditor::saveImage() {
    // Move image Id when deleting images
    int id = data->getImagesDataPtr()->getImageNumber();
    for (int i = 0; i <= id; ++i) {
        if (data->isDeleted(data->getImagesDataPtr()->getImageNumberInTotal(i))) {
            id--;
        }
    }
    data->getImagesDataPtr()->setImageNumber(id);

    data->removeDeletedImages();
    if (data->getImagesDataPtr()->get()->size() <= 0) {
        switchToMainWindow();
    }
    data->saveData();

    data->setSaved(true);

    reload();
}

/**
 * @brief Export the image to a specified path
 * @details This function opens a dialog to select the export path and whether to include the date in the image name.
 */
void ImageEditor::exportImage() {
    std::map<std::string, std::string> result;
    std::map<std::string, Option> map = {
        {"Date in image Name", Option("bool", "false")},
        {"Export path", Option("directory", PICTURES_PATH.toStdString())}};
    result = showOptionsDialog(this, "export", map);
    std::string exportPath = result["Export path"];
    bool dateInName = (result["Date in image Name"] == "true");

    if (exportPath == "") {
        return;
    }

    data->exportImages(exportPath, dateInName);
}

/**
 * @brief (Pre)Delete the current image
 */
void ImageEditor::deleteImage() {
    int nbr = data->getImagesDataPtr()->getImageNumber();
    int nbrInTotal = data->getImagesDataPtr()->getImageDataId(data->getImagesDataPtr()->getImageDataInCurrent(nbr)->getImagePathConst());
    bool saved = data->getSaved();

    if (data->isDeleted(nbrInTotal)) {
        data->unPreDeleteImage(nbrInTotal);
        data->addAction(
            [this, nbrInTotal, saved]() {
                int time = 0;
                if (data->getImagesDataPtr()->getImageNumberInTotal() != nbrInTotal) {
                    data->getImagesDataPtr()->setImageNumber(nbrInTotal);
                    reload();
                    time = TIME_UNDO_VISUALISATION;
                }
                QTimer::singleShot(time, [this, nbrInTotal, saved]() {
                    if (saved) {
                        data->setSaved(true);
                    }
                    data->preDeleteImage(nbrInTotal);
                    reload();
                });
            },
            [this, nbrInTotal]() {
                int time = 0;
                if (data->getImagesDataPtr()->getImageNumberInTotal() != nbrInTotal) {
                    data->getImagesDataPtr()->setImageNumber(nbrInTotal);
                    reload();
                    time = TIME_UNDO_VISUALISATION;
                }
                QTimer::singleShot(time, [this, nbrInTotal]() {
                    data->setSaved(false);
                    data->unPreDeleteImage(nbrInTotal);
                });
            });

        updateButtons();
    } else {
        data->preDeleteImage(nbrInTotal);
        data->addAction(
            [this, nbrInTotal, saved]() {
                int time = 0;
                if (data->getImagesDataPtr()->getImageNumberInTotal() != nbrInTotal) {
                    data->getImagesDataPtr()->setImageNumber(nbrInTotal);
                    reload();
                    time = TIME_UNDO_VISUALISATION;
                }
                QTimer::singleShot(time, [this, nbrInTotal, saved]() {
                    if (saved) {
                        data->setSaved(true);
                    }
                    data->unPreDeleteImage(nbrInTotal);
                    reload();
                });
            },
            [this, nbrInTotal]() {
                int time = 0;
                if (data->getImagesDataPtr()->getImageNumberInTotal() != nbrInTotal) {
                    data->getImagesDataPtr()->setImageNumber(nbrInTotal);
                    reload();
                    time = TIME_UNDO_VISUALISATION;
                }
                QTimer::singleShot(time, [this, nbrInTotal]() {
                    data->setSaved(false);
                    data->preDeleteImage(nbrInTotal);
                    reload();
                });
            });
        updateButtons();
    }
    data->setSaved(false);
}

/**
 * @brief Populate the metadata fields with data from the current image
 * @details This function retrieves the metadata from the current image and populates the corresponding fields in the UI.
 */
void ImageEditor::populateMetadataFields() {
    ImagesData* imagesData = data->getImagesDataPtr();
    ImageData* imageData = imagesData->getCurrentImageData();
    Exiv2::ExifData exifData = imageData->getMetaDataPtr()->getExifData();

    if (mapEditor) {
        double latitude = imageData->getLatitude();
        double longitude = imageData->getLongitude();
        editGeo->setImageData(imageData);
        if (latitude == 0 && longitude == 0) {
            editGeo->removeMapPoint();
        } else {
            editGeo->moveMapPoint(latitude, longitude);
            editGeo->setMapCenter(latitude, longitude);
        }
    }
}

/**
 * @brief Validate and save the metadata fields
 * @details This function retrieves the values from the metadata fields and saves them to the current image's metadata.
 */
void ImageEditor::validateMetadata() {
    ImagesData* imagesData = data->getImagesDataPtr();
    ImageData* imageData = imagesData->getCurrentImageData();
    MetaData* metaData = imageData->getMetaDataPtr();

    QString lastDate = metaData->getTimestampString();

    QString dateTimeStr = dateEdit->dateTime().toString("yyyy:MM:dd HH:mm:ss");
    metaData->modifyExifValue("Exif.Image.DateTime", dateTimeStr.toStdString());
    imageData->saveMetaData();

    data.get()->addAction(
        [this, lastDate, metaData]() {
            metaData->modifyExifValue("Exif.Image.DateTime", lastDate.toStdString());
            data->getImagesDataPtr()->getCurrentImageData()->saveMetaData();
            reload();
        },
        [this, dateTimeStr, metaData]() {
            metaData->modifyExifValue("Exif.Image.DateTime", dateTimeStr.toStdString());
            data->getImagesDataPtr()->getCurrentImageData()->saveMetaData();
            reload();
        });
}

/**
 * @brief Start the image open process
 * @details This function sets up a timer to load the image in the background and pre-load surrounding images.
 */
void ImageEditor::startImageOpen() {
    if (imageOpenTimer) {
        imageOpenTimer->stop();
    }

    connect(imageOpenTimer, &QTimer::timeout, this, [this]() {
        QPointer<ImageEditor> self = this;
        if (self && data != nullptr) {
            data->loadInCacheAsync(data->getImagesDataPtr()->getCurrentImageData()->getImagePath(), [self]() {
                if (self) {
                    self->reloadImageLabel();
                } }, false, QSize(0, 0), 0, true);

            imageOpenTimer->stop();
            for (int i = 0; i < PRE_LOAD_RADIUS; i++) {
                if (data->getImagesDataPtr()->getImageNumber() - (i + 1) < data->getImagesDataPtr()->getCurrent()->size() && data->getImagesDataPtr()->getImageNumber() - (i + 1) >= 0) {
                    data->loadInCacheAsync(data->getImagesDataPtr()->getImageDataInCurrent(data->getImagesDataPtr()->getImageNumber() - (i + 1))->getImagePath(), nullptr,
                                           false, QSize(0, 0), 0, true);
                }

                if (data->getImagesDataPtr()->getImageNumber() + (i + 1) < data->getImagesDataPtr()->getCurrent()->size() && data->getImagesDataPtr()->getImageNumber() + (i + 1) >= 0) {
                    data->loadInCacheAsync(data->getImagesDataPtr()->getImageDataInCurrent(data->getImagesDataPtr()->getImageNumber() + (i + 1))->getImagePath(), nullptr,
                                           false, QSize(0, 0), 0, true);
                }
            }
        }
    });
    if (imageOpenTimer) {
        imageOpenTimer->setInterval(TIME_BEFORE_FULL_QUALITY);
        imageOpenTimer->start();
    }
}

/**
 * @brief Stop the image open process
 * @details This function stops the timer that loads the image in the background and pre-loads surrounding images.
 */
void ImageEditor::stopImageOpen() {
    if (imageOpenTimer) {
        imageOpenTimer->disconnect();
        imageOpenTimer->stop();
    }
}

/**
 * @brief Check if the loaded image is still in the cache and unload it if not
 * @details This function checks the loaded images in the cache and unloads any images that are not currently being used.
 */
void ImageEditor::checkLoadedImage() {
    data->checkToUnloadImages(data->getImagesDataPtr()->getImageNumber(), PRE_LOAD_RADIUS);
}

/**
 * @brief Check the cache for loaded images and unload any that are not currently being used
 */
void ImageEditor::checkCache() {
    startImageOpen();
    checkLoadedImage();
}

/**
 * @brief Rotate the image to the left
 * @details This function rotates the current image to the left and updates the image label.
 */
void ImageEditor::rotateLeft() {
    ImageData* imageData = data->getImagesDataPtr()->getCurrentImageData();
    imageData->clearDetectedObjects();

    std::string extension = imageData->getImageExtension();
    int nbr = data->getImagesDataPtr()->getImageNumber();
    int imageInTotal = data->getImagesDataPtr()->getImageNumberInTotal(nbr);
    data->rotateLeft(imageInTotal, extension, [this]() { reload(); });
}

/**
 * @brief Rotate the image to the right
 * @details This function rotates the current image to the right and updates the image label.
 */
void ImageEditor::rotateRight() {
    ImageData* imageData = data->getImagesDataPtr()->getCurrentImageData();
    imageData->clearDetectedObjects();

    std::string extension = imageData->getImageExtension();
    int nbr = data->getImagesDataPtr()->getImageNumber();
    int imageInTotal = data->getImagesDataPtr()->getImageNumberInTotal(nbr);

    data->rotateRight(imageInTotal, extension, [this]() { reload(); });
}

/**
 * @brief Mirror the image up and down
 * @details This function mirrors the current image up-down and updates the image label.
 */
void ImageEditor::mirrorUpDown() {
    ImageData* imageData = data->getImagesDataPtr()->getCurrentImageData();
    imageData->clearDetectedObjects();

    std::string extension = imageData->getImageExtension();
    int nbr = data->getImagesDataPtr()->getImageNumber();
    int imageInTotal = data->getImagesDataPtr()->getImageNumberInTotal(nbr);

    data->mirrorUpDown(imageInTotal, extension, [this]() { reload(); });
}

/**
 * @brief Mirror the image left and right
 * @details This function mirrors the current image left-right and updates the image label.
 */
void ImageEditor::mirrorLeftRight() {
    ImageData* imageData = data->getImagesDataPtr()->getCurrentImageData();
    imageData->clearDetectedObjects();
    std::string extension = imageData->getImageExtension();
    int nbr = data->getImagesDataPtr()->getImageNumber();
    int imageInTotal = data->getImagesDataPtr()->getImageNumberInTotal(nbr);

    data->mirrorLeftRight(imageInTotal, extension, [this]() { reload(); });
}

/**
 * @brief Open a large image label for viewing
 * @details This function creates a new MainImage object for displaying a larger version of the current image.
 */
void ImageEditor::openBigImageLabel() {
    bigImage = true;
    hide();

    bigImageLabel = new MainImage(data, QString::fromStdString(data->getImagesDataPtr()->getCurrentImageData()->getImagePath()), (data->getSizesPtr()->imageEditorSizes->bigImage), false, personsEditor);
    bigImageLabel->setFixedSize(data->getSizesPtr()->imageEditorSizes->bigImage);

    mainLayout->addWidget(bigImageLabel);

    for (int i = 0; i < infoLayout->count(); ++i) {
        QWidget* widget = infoLayout->itemAt(i)->widget();
        if (widget) {
            widget->hide();
        }
    }

    connect(bigImageLabel, &MainImage::leftClicked, [this]() {
        closeBigImageLabel(bigImageLabel);
    });
}

/**
 * @brief Close the large image label and restore the original image editor state
 * @param bigImageLabel The MainImage object representing the large image label
 * @details This function removes the large image label from the layout and restores the original image editor state.
 * @details It also unhides the metadata fields and reloads the image label.
 */
void ImageEditor::closeBigImageLabel(MainImage* bigImageLabel) {
    bigImage = false;

    mainLayout->removeWidget(bigImageLabel);

    bigImageLabel->hide();
    bigImageLabel->deleteLater();

    unHide();

    for (int i = 0; i < infoLayout->count(); ++i) {
        QWidget* widget = infoLayout->itemAt(i)->widget();
        if (widget) {
            widget->setHidden(false);
        }
    }
    reload();
}

/**
 * @brief Handle the enter event for the image editor
 * @param event The enter event to handle
 * @details This function sets the focus to the image editor when the mouse enters the widget.
 * @details It also calls the base class implementation of the enter event.
 */
void ImageEditor::enterEvent(QEnterEvent* event) {
    this->setFocus();
    QMainWindow::enterEvent(event);
}

/**
 * @brief Handle the leave event for the imageEditor. (it avoid key problems with exif editor)
 * @param obj The object that triggered the leave event
 * @param event The leave event to handle
 * @return true if the event was handled, false otherwise
 */
bool ImageEditor::eventFilter(QObject* obj, QEvent* event) {
    if (event->type() == QEvent::MouseButtonPress) {
        if (dateEdit->hasFocus()) {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            if (!dateEdit->geometry().contains(mouseEvent->pos())) {
                dateEdit->clearFocus();
                this->setFocus();
            }
        }
    }
    return QMainWindow::eventFilter(obj, event);
}

/**
 * @brief Create a MapWidget for displaying the map
 * @return A pointer to the MapWidget object
 */
MapWidget* ImageEditor::createMapWidget() {
    ImageData* imageData = data->getImagesDataPtr()->getCurrentImageData();

    MapWidget* mapWidget = new MapWidget(this, imageData);
    if (imageData->getLatitude() != 0 && imageData->getLongitude() != 0) {
        mapWidget->setMapCenter(imageData->getLatitude(), imageData->getLongitude());
        mapWidget->moveMapPoint(imageData->getLatitude(), imageData->getLongitude());
    }
    data->addThreadToFront([this, mapWidget]() {
        for (ImageData* imageData : data->getImagesDataPtr()->getConst()) {
            if (imageData->getLatitude() != 0 && imageData->getLongitude() != 0) {
                mapWidget->addMapPointForOthers(imageData->getLatitude(), imageData->getLongitude());
            }
        }
    });

    return mapWidget;
}