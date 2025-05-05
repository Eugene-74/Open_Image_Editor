#include "ImageEditor.hpp"

#include <QApplication>
#include <QCalendarWidget>
#include <QComboBox>
#include <QDateTime>
#include <QDateTimeEdit>
#include <QFrame>
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
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>
#include <unordered_set>

#include "ClickableLabel.hpp"
#include "Const.hpp"
#include "Conversion.hpp"
#include "Download.hpp"
#include "LoadImage.hpp"
#include "MainImage.hpp"
#include "ObjectRecognition.hpp"
#include "Verification.hpp"

/**
 * @brief Constructor for the ImageEditor class
 * @param dat Pointer to the Data object containing application data
 * @param parent Pointer to the parent QWidget (usually the main window)
 */
ImageEditor::ImageEditor(std::shared_ptr<Data> dat, QWidget* parent)
    : QMainWindow(parent), data(dat) {
    this->installEventFilter(this);
    parent->setWindowTitle(IMAGE_EDITOR_WINDOW_NAME);

    QWidget* centralWidget = new QWidget(parent);
    setCentralWidget(centralWidget);
    mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setSpacing(data->sizes->imageEditorSizes->mainLayoutSpacing);
    mainLayout->setContentsMargins(data->sizes->imageEditorSizes->mainLayoutMargins[0],
                                   data->sizes->imageEditorSizes->mainLayoutMargins[1],
                                   data->sizes->imageEditorSizes->mainLayoutMargins[2],
                                   data->sizes->imageEditorSizes->mainLayoutMargins[3]);  // Marges autour des bords (gauche, haut, droite, bas)

    fixedFrame = new QFrame();
    fixedFrame->setFixedSize(data->sizes->imageEditorSizes->mainImageSize);
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

    dateEdit = new QDateTimeEdit(this);
    dateEdit->setDisplayFormat("dd/MM/yyyy, HH:mm");
    dateEdit->setCalendarPopup(true);

    QCalendarWidget* calendarWidget = dateEdit->calendarWidget();
    calendarWidget->setGridVisible(true);
    calendarWidget->setFirstDayOfWeek(Qt::Monday);
    calendarWidget->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
    calendarWidget->setNavigationBarVisible(true);

    editGeo = createMapWidget();

    validateButton = new QPushButton("Valider", this);
    connect(validateButton, &QPushButton::clicked, this, &ImageEditor::validateMetadata);

    infoLayout->addWidget(new QLabel("Date:", this));
    infoLayout->addWidget(dateEdit);
    infoLayout->addWidget(new QLabel("Géolocalisation:", this));
    infoLayout->addWidget(editGeo);

    infoLayout->addWidget(validateButton);
    if (exifEditor && !bigImage) {
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

/**
 * @brief Go to the next image in the list
 * @param nbr Number of images to skip (default is 1)
 */
void ImageEditor::nextImage(int nbr) {
    if (nbr < 0) {
        nbr = 0;
    }
    data->imagesData.setImageNumber(data->imagesData.getImageNumber() + nbr);
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
    data->imagesData.setImageNumber(data->imagesData.getImageNumber() - nbr);
    reload();
}

/**
 * @brief Reload imageEditor window
 */
void ImageEditor::reload() {
    checkCache();

    if (bigImage) {
        MainImage* bigImageLabelNew = new MainImage(data, QString::fromStdString(data->imagesData.getCurrentImageData()->getImagePath()), (data->sizes->imageEditorSizes->bigImage), false, personsEditor);

        bigImageLabelNew->setFixedSize(data->sizes->imageEditorSizes->bigImage);
        connect(bigImageLabelNew, &MainImage::leftClicked, [this]() {
            closeBigImageLabel(bigImageLabel);
        });
        mainLayout->replaceWidget(bigImageLabel, bigImageLabelNew);
        bigImageLabel->deleteLater();
        bigImageLabel = bigImageLabelNew;
    } else {
        ImagesData* imagesData = &data->imagesData;

        updateButtons();

        updatePreview();

        if (exifEditor && !bigImage) {
            imagesData->getCurrentImageData()->loadData();
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
}

/**
 * @brief Initialise a preview of the images in the editor
 */
void ImageEditor::createPreview() {
    ImagesData* imagesData = &data->imagesData;

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
    ImagesData* imagesData = &data->imagesData;

    if (imagesData->get()->size() <= 0) {
        return;
    }

    std::vector<std::string> imagePaths;

    int currentImageNumber = imagesData->getImageNumber();
    int totalImages = data->imagesData.getCurrent()->size();

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

    imagePersons = createImagePersons();

    actionButtonLayout->addWidget(imageRotateRight);
    actionButtonLayout->addWidget(imageRotateLeft);
    actionButtonLayout->addWidget(imageMirrorLeftRight);
    actionButtonLayout->addWidget(imageMirrorUpDown);
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

    buttonLayout->addWidget(buttonImageBefore);
    buttonLayout->addLayout(imageLabelLayout);
    buttonLayout->addWidget(buttonImageNext);
    buttonLayout->setAlignment(Qt::AlignCenter);
}

/**
 * @brief Update the buttons in the image editor based on the current image state
 */
void ImageEditor::updateButtons() {
    if (data->imagesData.get()->size() <= 0) {
        return;
    } else if (bigImage) {
        return;
    }

    if (imageRotateRight) {
        if (!isTurnable(data->imagesData.getCurrentImageData()->getImagePath())) {
            imageRotateRight->setDisabled(true);
        } else {
            if (!imageRotateRight->isEnabled())
                imageRotateRight->setEnabled(true);
        }
    }

    if (imageRotateLeft) {
        if (!isTurnable(data->imagesData.getCurrentImageData()->getImagePath())) {
            imageRotateLeft->setDisabled(true);
        } else {
            if (!imageRotateLeft->isEnabled())
                imageRotateLeft->setEnabled(true);
        }
    }
    if (imageMirrorLeftRight) {
        if (!isMirrorable(data->imagesData.getCurrentImageData()->getImagePath())) {
            imageMirrorLeftRight->setDisabled(true);
        } else {
            if (!imageMirrorLeftRight->isEnabled())
                imageMirrorLeftRight->setEnabled(true);
        }
    }
    if (imageMirrorUpDown) {
        if (!isMirrorable(data->imagesData.getCurrentImageData()->getImagePath())) {
            imageMirrorUpDown->setDisabled(true);
        } else {
            if (!imageMirrorUpDown->isEnabled())
                imageMirrorUpDown->setEnabled(true);
        }
    }
    if (imageDelete) {
        if (data->isDeleted(data->imagesData.getImageNumberInTotal())) {
            imageDelete->setBackground("#700c13", "#F00c13");
        } else {
            imageDelete->resetBackground();
        }
    }
    if (buttonImageBefore) {
        if (data->imagesData.getImageNumber() == 0) {
            buttonImageBefore->setDisabled(true);
        } else {
            buttonImageBefore->setEnabled(true);
        }
    }

    reloadImageLabel();

    if (buttonImageNext) {
        if (data->imagesData.getImageNumber() == data->imagesData.getCurrent()->size() - 1) {
            buttonImageNext->setDisabled(true);
        } else {
            buttonImageNext->setEnabled(true);
        }
    }

    if (imagePersons) {
        qDebug() << "Image persons : " << data->imagesData.getCurrentImageData()->getDetectedObjects()["person"].size();
        if (data->imagesData.getCurrentImageData()->isDetectionStatusLoaded()) {
            qDebug() << "Image persons : loaded";
            imagePersons->setLogoNumber(data->imagesData.getCurrentImageData()->getDetectedObjects()["person"].size());
        } else {
            qDebug() << "Image persons not : loaded";

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
    if (data->imagesData.get()->size() <= 0) {
        return nullptr;
    }

    ClickableLabel* imageDeleteNew = new ClickableLabel(data, Const::IconPath::DELETE_ICON, Const::Tooltip::ImageEditor::DELETE_TIP, this, actionSize);
    imageDeleteNew->setInitialBackground("transparent", "#b3b3b3");

    if (data->isDeleted(data->imagesData.getImageNumberInTotal())) {
        imageDeleteNew->setBackground("#700c13", "#F00c13");
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
    if (data->imagesData.get()->size() <= 0) {
        return nullptr;
    }

    ClickableLabel* imageSaveNew = new ClickableLabel(data, Const::IconPath::SAVE, Const::Tooltip::ImageEditor::SAVE, this, actionSize);
    imageSaveNew->setInitialBackground("transparent", "#b3b3b3");

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
    if (data->imagesData.get()->size() <= 0) {
        return nullptr;
    }

    ClickableLabel* imageExportNew = new ClickableLabel(data, Const::IconPath::EXPORT, Const::Tooltip::ImageEditor::EXPORT, this, actionSize);
    imageExportNew->setInitialBackground("transparent", "#b3b3b3");

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
    ClickableLabel* imageRotateRightNew = new ClickableLabel(data, Const::IconPath::ROTATE_RIGHT, Const::Tooltip::ImageEditor::ROTATE_RIGHT, this, actionSize);
    imageRotateRightNew->setInitialBackground("transparent", "#b3b3b3");

    if (!isTurnable(data->getImagesData()->getCurrentImageData()->getImagePath())) {
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
    if (data->imagesData.get()->size() <= 0) {
        return nullptr;
    }

    ClickableLabel* imageRotateLeftNew = new ClickableLabel(data, Const::IconPath::ROTATE_LEFT, Const::Tooltip::ImageEditor::ROTATE_LEFT, this, actionSize);
    imageRotateLeftNew->setInitialBackground("transparent", "#b3b3b3");

    if (!isTurnable(data->imagesData.getCurrentImageData()->getImagePath())) {
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
    if (data->imagesData.get()->size() <= 0) {
        return nullptr;
    }

    ClickableLabel* imageMirrorUpDownNew = new ClickableLabel(data, Const::IconPath::MIRROR_UP_DOWN, Const::Tooltip::ImageEditor::MIRROR_UP_DOWN, this, actionSize);
    imageMirrorUpDownNew->setInitialBackground("transparent", "#b3b3b3");

    if (!isMirrorable(data->imagesData.getCurrentImageData()->getImagePath())) {
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
    if (data->imagesData.get()->size() <= 0) {
        return nullptr;
    }

    ClickableLabel* imageMirrorLeftRightNew = new ClickableLabel(data, Const::IconPath::MIRROR_LEFT_RIGHT, Const::Tooltip::ImageEditor::MIRROR_LEFT_RIGHT, this, actionSize);
    imageMirrorLeftRightNew->setInitialBackground("transparent", "#b3b3b3");

    if (!isMirrorable(data->imagesData.getCurrentImageData()->getImagePath())) {
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
    if (data->imagesData.get()->size() <= 0) {
        return nullptr;
    }

    ClickableLabel* imageEditExifNew = new ClickableLabel(data, Const::IconPath::EDIT_EXIF, Const::Tooltip::ImageEditor::EDIT_EXIF, this, actionSize);
    imageEditExifNew->setInitialBackground("transparent", "#b3b3b3");

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

/**
 * @brief Create a button to open the convertion editor for the image editor
 * @return A pointer to the ClickableLabel object representing the conversion button
 */
ClickableLabel* ImageEditor::createImageConversion() {
    if (data->imagesData.get()->size() <= 0) {
        return nullptr;
    }

    ClickableLabel* imageConversionNew = new ClickableLabel(data, Const::IconPath::CONVERSION, Const::Tooltip::ImageEditor::CONVERSION, this, actionSize);
    imageConversionNew->setInitialBackground("transparent", "#b3b3b3");

    connect(imageConversionNew, &ClickableLabel::clicked, [this]() {
        launchConversionDialogAndConvert(QString::fromStdString(data->imagesData.getCurrentImageData()->getImagePath()));
    });

    return imageConversionNew;
}

/**
 * @brief Create a button to open the persons editor for the image editor
 * @return A pointer to the ClickableLabel object representing the persons editor button
 */
ClickableLabel* ImageEditor::createImagePersons() {
    if (data->getImagesData()->get()->size() <= 0) {
        return nullptr;
    }

    ClickableLabel* imagePersonsNew = new ClickableLabel(data, Const::IconPath::EDIT_PERSONS, Const::Tooltip::ImageEditor::EDIT_PERSONS, this, actionSize);
    imagePersonsNew->setInitialBackground("transparent", "#b3b3b3");
    imagePersonsNew->addLogo("#700c13", "#ffffff");

    if (data->imagesData.getCurrentImageData()->isDetectionStatusLoaded()) {
        imagePersonsNew->setLogoNumber(data->imagesData.getCurrentImageData()->getDetectedObjects()["person"].size());
    } else {
        imagePersonsNew->setLogoNumber(-1);
    }

    connect(imagePersonsNew, &ClickableLabel::leftClicked, [this, imagePersonsNew]() {
        if (!personsEditor) {
            personsEditor = true;
            imagePersonsNew->setBackground("#700c13", "#f00c13");

            imageLabel->personsEditor = true;
        } else {
            personsEditor = false;
            imagePersonsNew->resetBackground();
            imageLabel->personsEditor = false;
        }
        imageLabel->update();
    });

    connect(imagePersonsNew, &ClickableLabel::rightClicked, [this, imagePersonsNew]() {
        QStringList yoloModels = {"yolov5n - Nano model, fastest but less accurate",
                                  "yolov5s - Small model, good balance of speed and accuracy",
                                  "yolov5m - Medium model, more accurate but slower",
                                  "yolov5l - Large model, higher accuracy, slower",
                                  "yolov5x - Extra-large model, most accurate but slowest"};
        QString currentModel = QString::fromStdString(data->model.getModelName());
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
        confidenceSlider->setRange(0, 100);                                               // Confidence range from 0 to 100
        confidenceSlider->setValue(static_cast<int>(data->model.getConfidence() * 100));  // Set initial value
        QLabel* confidenceLabel = new QLabel(QString("Confidence: %1%").arg(confidenceSlider->value()), &dialog);

        connect(confidenceSlider, &QSlider::valueChanged, [this, confidenceLabel](int value) {
            float confidence = value / 100.0f;  // Convert slider value to float
            confidenceLabel->setText(QString("Confidence: %1%").arg(value));
            data->model.setConfidence(confidence);  // Update confidence in the model
            qDebug() << "Confidence updated to:" << confidence;
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
            data->model.setModelName(modelName);
            downloadModelIfNotExists(modelName + ".onnx");

            ImageData* imageData = data->imagesData.getCurrentImageData();
            if (imageData) {
                // QImage qImage(QString::fromStdString(imageData->getImagePath()));
                QImage qImage = data->loadImageNormal(nullptr, imageData->getImagePath(), QSize(0, 0), false);
                qImage = data->rotateQImage(qImage, imageData);

                auto detectedObjects = data->detect(imageData->getImagePath(), qImage, data->model.getModelName()).getDetectedObjects();
                imageData->setDetectedObjects(detectedObjects);
                qDebug() << "Object detection re-run with model:" << QString::fromStdString(data->model.getModelName());
            }

            reload();
        }
    });

    return imagePersonsNew;
}

/**
 * @brief Create a button to go to the previous image in the image editor
 * @return A pointer to the ClickableLabel object representing the previous image button
 */
ClickableLabel* ImageEditor::createImageBefore() {
    if (data->imagesData.get()->size() <= 0) {
        return nullptr;
    }

    ClickableLabel* buttonImageBeforeNew = new ClickableLabel(data, Const::IconPath::BEFORE, "", this, actionSize);
    buttonImageBeforeNew->setInitialBackground("transparent", "#b3b3b3");

    // buttonImageBeforeNew->setFixedSize(&actionSize);

    if (data->imagesData.getImageNumber() == 0) {
        buttonImageBeforeNew->setDisabled(true);
    }

    connect(buttonImageBeforeNew, &ClickableLabel::clicked, [this]() { this->previousImage(); });
    // buttonImageBeforeNew->setFixedSize(&actionSize);

    return buttonImageBeforeNew;
}

/**
 * @brief Create a button to go to the next image in the image editor
 * @return A pointer to the ClickableLabel object representing the next image button
 */
ClickableLabel* ImageEditor::createImageNext() {
    if (data->imagesData.get()->size() <= 0) {
        return nullptr;
    }

    ClickableLabel* buttonImageNextNew = new ClickableLabel(data, Const::IconPath::NEXT, "", this, actionSize);
    buttonImageNextNew->setInitialBackground("transparent", "#b3b3b3");

    if (data->imagesData.getImageNumber() == data->imagesData.getCurrent()->size() - 1) {
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
    if (data->imagesData.get()->size() <= 0) {
        return nullptr;
    }

    ClickableLabel* previewButton = new ClickableLabel(data, QString::fromStdString(imagePath), "", this, previewSize, false, Const::Thumbnail::NORMAL_QUALITY, false);
    previewButton->setInitialBorder("transparent", "#b3b3b3");

    connect(previewButton, &ClickableLabel::leftClicked, [this, imageNbr]() {
        data->imagesData.setImageNumber(imageNbr);
        reload();
    });

    return previewButton;
}

/**
 * @brief Ceate a MainImage for the image label
 * @return The widget it self with is a MainImage
 */
MainImage* ImageEditor::createImageLabel() {
    if (data->imagesData.get()->size() <= 0) {
        return nullptr;
    }

    MainImage* imageLabelNew = new MainImage(data, QString::fromStdString(data->imagesData.getCurrentImageData()->getImagePath()), *mainImageSize, false, personsEditor);

    std::string currentImagePath = data->imagesData.getCurrentImageData()->getImagePath();

    ImageData* imageData = data->imagesData.getCurrentImageData();

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

    int imageNbr = data->imagesData.getImageNumber();
    if (imageData->isDetectionStatusNotLoaded()) {
        imageData->setDetectionStatusLoading();
        qInfo() << "starting face recognition";
        QImage image = data->loadImageNormal(nullptr, data->imagesData.getCurrentImageData()->getImagePath(), QSize(0, 0), false);

        image = data->rotateQImage(image, imageData);

        QPointer<ImageEditor> self = this;

        detectObjectsAsync(data, currentImagePath, image, [self, imageNbr, currentImagePath](DetectedObjects detectedObject) {
            if (!self.isNull()) {
                ImageData* imageData = self->data->getImagesData()->getImageData(currentImagePath);

                if (imageData) {
                    qInfo() << "recognition done";
                    imageData->setDetectedObjects(detectedObject.getDetectedObjects());
                    imageData->setDetectionStatusLoaded();
                }
                if (self->data->imagesData.getImageNumber() == imageNbr) {
                    if (self->imagePersons) {
                        self->imagePersons->setLogoNumber(detectedObject.getDetectedObjects()["person"].size());

                        self->imagePersons->update();

                        if (self->imageLabel) {
                            self->imageLabel->update();
                        }
                    }
                }
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
    if (dateEdit->hasFocus()) {
        return;
    }

    switch (event->key()) {
        case Qt::Key_Left:
            if (event->modifiers() & Qt::ControlModifier) {
                imageRotateLeft->setBackground(CLICK_BACKGROUND_COLOR, CLICK_BACKGROUND_COLOR);
            } else {
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
                data->getImagesData()->setImageNumber(data->imagesData.get()->size() - 1);
                reload();
            } else {
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
                data->getImagesData()->setImageNumber(data->imagesData.get()->size() - 1);
                reload();
            } else {
                buttonImageNext->setBackground(BACKGROUND_COLOR, BACKGROUND_COLOR);
                nextImage();
            }
            break;
        case Qt::Key_End:
            imageRotateLeft->setBackground(BACKGROUND_COLOR, BACKGROUND_COLOR);
            data->getImagesData()->setImageNumber(data->imagesData.get()->size() - 1);
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
    int id = data->getImagesData()->getImageNumber();
    for (int i = 0; i <= id; ++i) {
        if (data->isDeleted(data->getImagesData()->getImageNumberInTotal(i))) {
            id--;
        }
    }
    data->imagesData.setImageNumber(id);
    data->removeDeletedImages();
    if (data->imagesData.get()->size() <= 0) {
        switchToMainWindow();
    }
    data->saveData();

    data->saved = true;

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
    int nbr = data->imagesData.getImageNumber();
    int nbrInTotal = data->getImagesData()->getImageDataId(data->getImagesData()->getImageDataInCurrent(nbr)->getImagePathConst());
    bool saved = data->saved;

    if (data->isDeleted(nbrInTotal)) {
        data->unPreDeleteImage(nbrInTotal);
        data->addAction(
            [this, nbrInTotal, saved]() {
                int time = 0;
                if (data->getImagesData()->getImageNumber() != nbrInTotal) {
                    data->getImagesData()->setImageNumber(nbrInTotal);
                    reload();
                    time = TIME_UNDO_VISUALISATION;
                }
                QTimer::singleShot(time, [this, nbrInTotal, saved]() {
                    if (saved) {
                        data->saved = true;
                    }
                    data->preDeleteImage(nbrInTotal);
                    reload();
                });
            },
            [this, nbrInTotal]() {
                int time = 0;
                if (data->getImagesData()->getImageNumber() != nbrInTotal) {
                    data->getImagesData()->setImageNumber(nbrInTotal);
                    reload();
                    time = TIME_UNDO_VISUALISATION;
                }
                QTimer::singleShot(time, [this, nbrInTotal]() {
                    data->saved = false;
                    data->unPreDeleteImage(nbrInTotal);
                });
            });

        updateButtons();
    } else {
        data->preDeleteImage(nbrInTotal);
        data->addAction(
            [this, nbrInTotal, saved]() {
                int time = 0;
                if (data->getImagesData()->getImageNumber() != nbrInTotal) {
                    data->getImagesData()->setImageNumber(nbrInTotal);
                    reload();
                    time = TIME_UNDO_VISUALISATION;
                }
                QTimer::singleShot(time, [this, nbrInTotal, saved]() {
                    if (saved) {
                        data->saved = true;
                    }
                    data->unPreDeleteImage(nbrInTotal);
                    reload();
                });
            },
            [this, nbrInTotal]() {
                int time = 0;
                if (data->getImagesData()->getImageNumber() != nbrInTotal) {
                    data->getImagesData()->setImageNumber(nbrInTotal);
                    reload();
                    time = TIME_UNDO_VISUALISATION;
                }
                QTimer::singleShot(time, [this, nbrInTotal]() {
                    data->saved = false;
                    data->preDeleteImage(nbrInTotal);
                    reload();
                });
            });
        updateButtons();
    }
    data->saved = false;
}

/**
 * @brief Populate the metadata fields with data from the current image
 * @details This function retrieves the metadata from the current image and populates the corresponding fields in the UI.
 */
void ImageEditor::populateMetadataFields() {
    ImagesData* imagesData = &data->imagesData;
    ImageData* imageData = imagesData->getCurrentImageData();
    Exiv2::ExifData exifData = imageData->getMetaDataPtr()->getExifData();

    dateEdit->setDateTime(QDateTime::currentDateTime());

    if (exifData["Exif.Image.DateTime"].count() != 0) {
        QString dateTimeStr = QString::fromStdString(exifData["Exif.Image.DateTime"].toString());
        QDateTime dateTime = QDateTime::fromString(dateTimeStr, "yyyy:MM:dd HH:mm:ss");
        dateEdit->setDateTime(dateTime);
    }
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

/**
 * @brief Validate and save the metadata fields
 * @details This function retrieves the values from the metadata fields and saves them to the current image's metadata.
 */
void ImageEditor::validateMetadata() {
    ImagesData* imagesData = &data->imagesData;
    ImageData* imageData = imagesData->getCurrentImageData();
    MetaData* metaData = imageData->getMetaDataPtr();

    QString dateTimeStr = dateEdit->dateTime().toString("yyyy:MM:dd HH:mm:ss");
    metaData->modifyExifValue("Exif.Image.DateTime", dateTimeStr.toStdString());

    imageData->saveMetaData();
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
            data->loadInCacheAsync(data->imagesData.getCurrentImageData()->getImagePath(), [self]() {
                if (self) {
                    self->reloadImageLabel();
                } }, false, QSize(0, 0), 0, true);

            imageOpenTimer->stop();
            for (int i = 0; i < PRE_LOAD_RADIUS; i++) {
                if (data->imagesData.getImageNumber() - (i + 1) < data->imagesData.getCurrent()->size() && data->imagesData.getImageNumber() - (i + 1) >= 0) {
                    data->loadInCacheAsync(data->imagesData.getImageDataInCurrent(data->imagesData.getImageNumber() - (i + 1))->getImagePath(), nullptr,
                                           false, QSize(0, 0), 0, true);
                }

                if (data->imagesData.getImageNumber() + (i + 1) < data->imagesData.getCurrent()->size() && data->imagesData.getImageNumber() + (i + 1) >= 0) {
                    data->loadInCacheAsync(data->imagesData.getImageDataInCurrent(data->imagesData.getImageNumber() + (i + 1))->getImagePath(), nullptr,
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
    data->checkToUnloadImages(data->getImagesData()->getImageNumber(), PRE_LOAD_RADIUS);
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
    ImageData* imageData = data->getImagesData()->getCurrentImageData();
    imageData->clearDetectedObjects();

    std::string extension = imageData->getImageExtension();
    int nbr = data->getImagesData()->getImageNumber();
    int imageInTotal = data->getImagesData()->getImageNumberInTotal(nbr);
    data->rotateLeft(imageInTotal, extension, [this]() { reload(); });
}

/**
 * @brief Rotate the image to the right
 * @details This function rotates the current image to the right and updates the image label.
 */
void ImageEditor::rotateRight() {
    ImageData* imageData = data->getImagesData()->getCurrentImageData();
    imageData->clearDetectedObjects();

    std::string extension = imageData->getImageExtension();
    int nbr = data->getImagesData()->getImageNumber();
    int imageInTotal = data->getImagesData()->getImageNumberInTotal(nbr);

    data->rotateRight(imageInTotal, extension, [this]() { reload(); });
}

/**
 * @brief Mirror the image up and down
 * @details This function mirrors the current image up-down and updates the image label.
 */
void ImageEditor::mirrorUpDown() {
    ImageData* imageData = data->getImagesData()->getCurrentImageData();
    imageData->clearDetectedObjects();

    std::string extension = imageData->getImageExtension();
    int nbr = data->getImagesData()->getImageNumber();
    int imageInTotal = data->getImagesData()->getImageNumberInTotal(nbr);

    data->mirrorUpDown(imageInTotal, extension, [this]() { reload(); });
}

/**
 * @brief Mirror the image left and right
 * @details This function mirrors the current image left-right and updates the image label.
 */
void ImageEditor::mirrorLeftRight() {
    ImageData* imageData = data->getImagesData()->getCurrentImageData();
    imageData->clearDetectedObjects();
    std::string extension = imageData->getImageExtension();
    int nbr = data->getImagesData()->getImageNumber();
    int imageInTotal = data->getImagesData()->getImageNumberInTotal(nbr);

    data->mirrorLeftRight(imageInTotal, extension, [this]() { reload(); });
}

/**
 * @brief Open a large image label for viewing
 * @details This function creates a new MainImage object for displaying a larger version of the current image.
 */
void ImageEditor::openBigImageLabel() {
    bigImage = true;
    hide();

    bigImageLabel = new MainImage(data, QString::fromStdString(data->imagesData.getCurrentImageData()->getImagePath()), (data->sizes->imageEditorSizes->bigImage), false, personsEditor);
    bigImageLabel->setFixedSize(data->sizes->imageEditorSizes->bigImage);

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
        if (exifEditor && !bigImage) {
            if (dateEdit->hasFocus()) {
                QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
                if (!dateEdit->geometry().contains(mouseEvent->pos())) {
                    dateEdit->clearFocus();
                    this->setFocus();
                }
            }
        }
    }
    return QMainWindow::eventFilter(obj, event);
}

MapWidget* ImageEditor::createMapWidget() {
    ImageData* imageData = data.get()->getImagesData()->getCurrentImageData();

    MapWidget* mapWidget = new MapWidget(this, imageData);
    if (imageData->getLatitude() != 0 && imageData->getLongitude() != 0) {
        mapWidget->setMapCenter(imageData->getLatitude(), imageData->getLongitude());
        mapWidget->moveMapPoint(imageData->getLatitude(), imageData->getLongitude());
    }
    data->addThreadToFront([this, mapWidget]() {
        for (ImageData* imageData : data->getImagesData()->getConst()) {
            if (imageData->getLatitude() != 0 && imageData->getLongitude() != 0) {
                // qDebug() << "Adding point for others:" << imageData->getLatitude() << imageData->getLongitude();
                mapWidget->addMapPointForOthers(imageData->getLatitude(), imageData->getLongitude());
            }
        }
    });

    return mapWidget;
}