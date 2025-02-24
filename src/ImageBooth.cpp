#include "ImageBooth.hpp"

ImageBooth::ImageBooth(Data* dat, QWidget* parent)
    : QMainWindow(parent), data(dat) {
    parent->setWindowTitle(IMAGE_BOOTH_WINDOW_NAME);
    imageNumber = data->imagesData.getImageNumber();
    data->clearCache();

    lastLineNbr = 0;

    QWidget* centralWidget = new QWidget(parent);
    setCentralWidget(centralWidget);

    mainLayout = new QVBoxLayout(centralWidget);
    actionButtonLayout = new QHBoxLayout();
    actionButtonLayout->setAlignment(Qt::AlignCenter);
    mainLayout->addLayout(actionButtonLayout);

    createButtons();

    scrollLayout = new QHBoxLayout();
    mainLayout->addLayout(scrollLayout);

    scrollArea = new QScrollArea(centralWidget);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFixedSize(data->sizes.imagesBoothSizes->scrollAreaSize);
    actionButtonLayout->setAlignment(Qt::AlignCenter);
    scrollLayout->addWidget(scrollArea);

    QWidget* scrollWidget = new QWidget();
    linesLayout = new QVBoxLayout(scrollWidget);
    scrollArea->setWidget(scrollWidget);

    // marche pas jsp pk il manque 1/2 lignes a la fin
    scrollWidget->setMinimumHeight(data->sizes.imagesBoothSizes->realImageSize.height() * (data->imagesData.get()->size() / data->sizes.imagesBoothSizes->widthImageNumber));

    linesLayout->setAlignment(Qt::AlignTop);
    linesLayout->setSpacing(data->sizes.imagesBoothSizes->linesLayoutSpacing);
    linesLayout->setContentsMargins(
        data->sizes.imagesBoothSizes->linesLayoutMargins[0],   // gauche
        data->sizes.imagesBoothSizes->linesLayoutMargins[1],   // haut
        data->sizes.imagesBoothSizes->linesLayoutMargins[2],   // droite
        data->sizes.imagesBoothSizes->linesLayoutMargins[3]);  // bas

    spacer = new QSpacerItem(0, 0);
    linesLayout->insertSpacerItem(0, spacer);

    createFirstImages();

    connect(scrollArea->verticalScrollBar(), &QScrollBar::valueChanged, this, &ImageBooth::onScroll);

    // Make sure that the scrollArea is well initialized (doesn't work without)
    QTimer::singleShot(1, this, [this]() {
        gotToImage(imageNumber, true);
    });
}

void ImageBooth::updateVisibleImages(bool force) {
    int spacerHeight = scrollArea->verticalScrollBar()->value();
    int imageHeight = data->sizes.imagesBoothSizes->realImageSize.height();
    spacerHeight = (spacerHeight / imageHeight) * imageHeight;

    int lineNbr = spacerHeight / imageHeight;
    int difLineNbr = lineNbr - lastLineNbr;

    if (difLineNbr == 0 && !force) {
        return;
    }
    spacer->changeSize(0, spacerHeight);
    updateImages();
    linesLayout->invalidate();
    lastLineNbr = lineNbr;
}

// Check if an image is visible
bool ImageBooth::isImageVisible(int imageIndex) {
    int spacerHeight = scrollArea->verticalScrollBar()->value();
    int imageHeight = data->sizes.imagesBoothSizes->realImageSize.height();
    spacerHeight = (spacerHeight / imageHeight) * imageHeight;

    int firstImageNbr = spacerHeight / imageHeight * data->sizes.imagesBoothSizes->widthImageNumber;
    int lastImageNbr = (spacerHeight / imageHeight + maxVisibleLines) * data->sizes.imagesBoothSizes->widthImageNumber;

    if (imageIndex >= firstImageNbr && imageIndex <= lastImageNbr) {
        return true;
    }

    return false;
}

void ImageBooth::createFirstImages() {
    for (int j = 0; j < maxVisibleLines; j++) {
        QHBoxLayout* lineLayout = new QHBoxLayout();
        lineLayout->setAlignment(Qt::AlignLeft);

        linesLayout->addLayout(lineLayout);
        lineLayouts.push_back(lineLayout);
        int nbr = data->sizes.imagesBoothSizes->widthImageNumber;

        for (int i = 0; i < nbr; i++) {
            int imageNbr = j * nbr + i;
            if (i < data->imagesData.get()->size()) {
                std::string imagePath = data->imagesData.get()->at(imageNbr).folders.name;

                lineLayout->addWidget(createImage(imagePath, imageNbr));
            }
        }
    }
}

void ImageBooth::onScroll(int value) {
    updateVisibleImages();
}

ClickableLabel* ImageBooth::createImage(std::string imagePath, int nbr) {
    if (data->imagesData.get()->size() <= 0) {
        return nullptr;
    }
    ClickableLabel* imageButton;
    if (data->isInCache(data->getThumbnailPath(imagePath, IMAGE_BOOTH_IMAGE_QUALITY))) {
        imageButton = new ClickableLabel(data, QString::fromStdString(imagePath),
                                         "", this, imageSize, false, IMAGE_BOOTH_IMAGE_QUALITY, true);
    } else if (data->hasThumbnail(imagePath, IMAGE_BOOTH_IMAGE_POOR_QUALITY)) {
        imageButton = new ClickableLabel(data, QString::fromStdString(data->getThumbnailPath(imagePath, IMAGE_BOOTH_IMAGE_POOR_QUALITY)),
                                         "", this, imageSize, false, 0, true);

        QPointer<ImageBooth> self = this;
        Data* dataPtr = data;
        data->loadInCacheAsync(data->getThumbnailPath(imagePath, IMAGE_BOOTH_IMAGE_QUALITY), [self, dataPtr, imagePath, nbr]() {
            if (!self.isNull()) {
                QHBoxLayout* lineLayout = nullptr;
                ClickableLabel* lastImageButton = self->getClickableLabelIfExist(nbr, lineLayout);
                if (lastImageButton != nullptr) {
                    ClickableLabel* newImageButton = self->createImage(imagePath, nbr);
                    if (lineLayout != nullptr) {
                        lineLayout->replaceWidget(lastImageButton, newImageButton);
                        lastImageButton->deleteLater();
                    } else {
                        newImageButton->deleteLater();
                    }
                }
            }
        });
    } else {
        imageButton = new ClickableLabel(data, IMAGE_PATH_LOADING,
                                         "", this, imageSize, false, 0, true);

        QPointer<ImageBooth> self = this;
        Data* dataPtr = data;

        data->loadInCacheAsync(imagePath, [self, dataPtr, imagePath, nbr]() {
            qDebug() << "Load image : " << imagePath;
            dataPtr->createAllThumbnailIfNotExists(imagePath, 512);

            dataPtr->unloadFromCache(imagePath);

            if (!self.isNull()) {
                QHBoxLayout* lineLayout = nullptr;
                ClickableLabel* lastImageButton = self->getClickableLabelIfExist(nbr, lineLayout);
                if (lastImageButton != nullptr) {
                    ClickableLabel* newImageButton = self->createImage(imagePath, nbr);
                    if (lineLayout != nullptr) {
                        lineLayout->replaceWidget(lastImageButton, newImageButton);
                        lastImageButton->deleteLater();
                    } else {
                        newImageButton->deleteLater();
                    }
                }
            }
        });
    }

    if (std::find(data->imagesSelected.begin(), data->imagesSelected.end(), nbr) != data->imagesSelected.end()) {
        imageButton->setBorder(COLOR_BACKGROUND_IMAGE_BOOTH_SELECTED, COLOR_BACKGROUND_HOVER_IMAGE_BOOTH_SELECTED);
    }

    if (nbr == imageShiftSelected) {
        if (imageShiftSelectedSelect) {
            imageButton->setBorder(COLOR_BACKGROUND_IMAGE_BOOTH_SELECTED_MULTIPLE_SELECT, COLOR_BACKGROUND_HOVER_IMAGE_BOOTH_SELECTED_MULTIPLE_SELECT);
        } else {
            imageButton->setBorder(COLOR_BACKGROUND_IMAGE_BOOTH_SELECTED_MULTIPLE_UNSELECT, COLOR_BACKGROUND_HOVER_IMAGE_BOOTH_SELECTED_MULTIPLE_UNSELECT);
        }
    }

    connect(imageButton, &ClickableLabel::leftClicked, [this, nbr]() {
        data->imagesData.setImageNumber(nbr);
        switchToImageEditor();
    });

    connect(imageButton, &ClickableLabel::ctrlLeftClicked, [this, nbr, imageButton]() {
        auto it = std::find(data->imagesSelected.begin(), data->imagesSelected.end(), nbr);
        if (it != data->imagesSelected.end()) {
            imageButton->resetBorder();
            data->imagesSelected.erase(it);

            data->addAction(
                [this, nbr]() {
                    if (!isImageVisible(nbr)) {
                        gotToImage(nbr);
                    }
                    QTimer::singleShot(TIME_UNDO_VISUALISATION, [this, nbr]() {
                        ClickableLabel* imageButton = getClickableLabelIfExist(nbr);
                        if (imageButton != nullptr) {
                            imageButton->setBorder(COLOR_BACKGROUND_IMAGE_BOOTH_SELECTED, COLOR_BACKGROUND_HOVER_IMAGE_BOOTH_SELECTED);
                        }
                        addNbrToSelectedImages(nbr);
                    });
                },
                [this, nbr]() {
                    if (!isImageVisible(nbr)) {
                        gotToImage(nbr);
                    }
                    QTimer::singleShot(TIME_UNDO_VISUALISATION, [this, nbr]() {
                        ClickableLabel* imageButton = getClickableLabelIfExist(nbr);
                        if (imageButton != nullptr) {
                            imageButton->resetBorder();
                        }
                        removeNbrToSelectedImages(nbr);
                    });
                });
        } else {
            imageButton->setBorder(COLOR_BACKGROUND_IMAGE_BOOTH_SELECTED, COLOR_BACKGROUND_HOVER_IMAGE_BOOTH_SELECTED);
            data->imagesSelected.push_back(nbr);

            data->addAction(
                [this, nbr]() {
                    int time = 0;
                    if (!isImageVisible(nbr)) {
                        gotToImage(nbr);
                        int time = TIME_UNDO_VISUALISATION;
                    }
                    QTimer::singleShot(time, [this, nbr]() {
                        ClickableLabel* imageButton = getClickableLabelIfExist(nbr);
                        if (imageButton != nullptr) {
                            imageButton->resetBorder();
                        }
                        removeNbrToSelectedImages(nbr);
                    });
                },
                [this, nbr]() {
                    int time = 0;
                    if (!isImageVisible(nbr)) {
                        gotToImage(nbr);
                        int time = TIME_UNDO_VISUALISATION;
                    }
                    QTimer::singleShot(time, [this, nbr]() {
                        ClickableLabel* imageButton = getClickableLabelIfExist(nbr);
                        if (imageButton != nullptr) {
                            imageButton->setBorder(COLOR_BACKGROUND_IMAGE_BOOTH_SELECTED, COLOR_BACKGROUND_HOVER_IMAGE_BOOTH_SELECTED);
                        }
                        addNbrToSelectedImages(nbr);
                    });
                });
        }
    });

    connect(imageButton, &ClickableLabel::shiftLeftClicked, [this, nbr, imageButton]() {
        // select all image beetwen the 2 nbr
        if (imageShiftSelected >= 0) {
            std::vector<int> modifiedNbr;

            int start = std::min(imageShiftSelected, nbr);
            int end = std::max(imageShiftSelected, nbr);
            for (int i = start; i <= end; ++i) {
                auto it = std::find(data->imagesSelected.begin(), data->imagesSelected.end(), i);
                if (it != data->imagesSelected.end()) {
                    if (!imageShiftSelectedSelect) {
                        data->imagesSelected.erase(it);
                        modifiedNbr.push_back(i);
                    }
                } else {
                    if (imageShiftSelectedSelect) {
                        data->imagesSelected.push_back(i);
                        modifiedNbr.push_back(i);
                    }
                }
            }
            reload();

            bool select = *&imageShiftSelectedSelect;
            data->addAction(
                [this, modifiedNbr, select]() {
                    int time = 0;
                    int minNbr = *std::min_element(modifiedNbr.begin(), modifiedNbr.end());
                    if (!isImageVisible(minNbr)) {
                        gotToImage(minNbr);
                        int time = TIME_UNDO_VISUALISATION;
                    }
                    QTimer::singleShot(time, [this, modifiedNbr, select]() {
                        for (auto nbr : modifiedNbr) {
                            if (select) {
                                removeNbrToSelectedImages(nbr);
                            } else {
                                addNbrToSelectedImages(nbr);
                            }
                        }
                        reload();
                    });
                },
                [this, modifiedNbr, select]() {
                    int time = 0;
                    int minNbr = *std::min_element(modifiedNbr.begin(), modifiedNbr.end());
                    if (!isImageVisible(minNbr)) {
                        gotToImage(minNbr);
                        int time = TIME_UNDO_VISUALISATION;
                    }
                    QTimer::singleShot(time, [this, modifiedNbr, select]() {
                        for (auto nbr : modifiedNbr) {
                            if (select) {
                                addNbrToSelectedImages(nbr);
                            } else {
                                removeNbrToSelectedImages(nbr);
                            }
                        }
                        reload();
                    });
                });

            imageShiftSelected = -1;
        } else {
            // Select the first image
            auto it = std::find(data->imagesSelected.begin(), data->imagesSelected.end(), nbr);
            if (it != data->imagesSelected.end()) {
                imageButton->setBorder(COLOR_BACKGROUND_IMAGE_BOOTH_SELECTED_MULTIPLE_UNSELECT, COLOR_BACKGROUND_HOVER_IMAGE_BOOTH_SELECTED_MULTIPLE_UNSELECT);
                imageShiftSelectedSelect = false;
            } else {
                imageButton->setBorder(COLOR_BACKGROUND_IMAGE_BOOTH_SELECTED_MULTIPLE_SELECT, COLOR_BACKGROUND_HOVER_IMAGE_BOOTH_SELECTED_MULTIPLE_SELECT);
                imageShiftSelectedSelect = true;
            }
            imageShiftSelected = nbr;
        }
    });
    imageButton->setInitialBorder("transparent", "#b3b3b3");
    return imageButton;
}

// Go to the line of the image nbr
void ImageBooth::gotToImage(int nbr, bool force) {
    int imageLine = nbr / data->sizes.imagesBoothSizes->widthImageNumber;
    int spacerHeight = imageLine * data->sizes.imagesBoothSizes->realImageSize.height();

    scrollArea->verticalScrollBar()->setValue(spacerHeight);

    updateVisibleImages(force);
    QCoreApplication::processEvents();
}

// Add a nbr to the selected images
void ImageBooth::addNbrToSelectedImages(int nbr) {
    auto it = std::find(data->imagesSelected.begin(), data->imagesSelected.end(), nbr);
    if (it == data->imagesSelected.end()) {
        data->imagesSelected.push_back(nbr);
    }
}

// remove a nbr to the selected images
void ImageBooth::removeNbrToSelectedImages(int nbr) {
    auto it = std::find(data->imagesSelected.begin(), data->imagesSelected.end(), nbr);
    if (it != data->imagesSelected.end()) {
        data->imagesSelected.erase(it);
    }
}

// Get the clickableLabel + lineLayout of imageNbr if exist
ClickableLabel* ImageBooth::getClickableLabelIfExist(int imageNbr, QHBoxLayout*& lineLayout) {
    int spacerHeight = scrollArea->verticalScrollBar()->value();
    int imageHeight = data->sizes.imagesBoothSizes->realImageSize.height();
    spacerHeight = (spacerHeight / imageHeight) * imageHeight;

    int firstImageNbr = spacerHeight / imageHeight * data->sizes.imagesBoothSizes->widthImageNumber;
    int lastImageNbr = (spacerHeight / imageHeight + maxVisibleLines) * data->sizes.imagesBoothSizes->widthImageNumber;

    if (imageNbr >= firstImageNbr && imageNbr <= lastImageNbr) {
        int firstImageLine = firstImageNbr / data->sizes.imagesBoothSizes->widthImageNumber;
        int imageLine = imageNbr / data->sizes.imagesBoothSizes->widthImageNumber;

        int imageRelativeLine = imageLine - firstImageLine;
        int imageNbrInLine = imageNbr - imageLine * data->sizes.imagesBoothSizes->widthImageNumber;
        lineLayout = qobject_cast<QHBoxLayout*>(linesLayout->itemAt(imageRelativeLine + 1)->layout());
        ClickableLabel* imageButton = qobject_cast<ClickableLabel*>(lineLayout->itemAt(imageNbrInLine)->widget());

        return imageButton;
    }
    return nullptr;
}

// Get the clickable label of imageNbr if exist
ClickableLabel* ImageBooth::getClickableLabelIfExist(int imageNbr) {
    QHBoxLayout* lineLayout = nullptr;
    return getClickableLabelIfExist(imageNbr, lineLayout);
}

// Update all visible images
void ImageBooth::updateImages() {
    int spacerHeight = scrollArea->verticalScrollBar()->value();
    int imageHeight = data->sizes.imagesBoothSizes->realImageSize.height();
    spacerHeight = (spacerHeight / imageHeight) * imageHeight;

    int lineNbr = spacerHeight / imageHeight;

    for (int i = 1; i < linesLayout->count(); i++) {
        QHBoxLayout* lineLayout = qobject_cast<QHBoxLayout*>(linesLayout->itemAt(i)->layout());
        for (int j = 0; j < lineLayout->count(); j++) {
            int imageNbr = (lineNbr + i - 1) * data->sizes.imagesBoothSizes->widthImageNumber + j;

            ClickableLabel* lastImageButton = qobject_cast<ClickableLabel*>(lineLayout->itemAt(j)->widget());
            ClickableLabel* imageButton = createImage(data->imagesData.get()->at(imageNbr).folders.name, imageNbr);

            lineLayout->replaceWidget(lastImageButton, imageButton);
            lastImageButton->deleteLater();

            auto it = std::find(data->imagesSelected.begin(), data->imagesSelected.end(), imageNbr);
            if (it != data->imagesSelected.end()) {
                imageButton->setBorder(COLOR_BACKGROUND_IMAGE_BOOTH_SELECTED, COLOR_BACKGROUND_HOVER_IMAGE_BOOTH_SELECTED);
            } else {
                imageButton->resetBorder();
            }
        }
    }
}

void ImageBooth::keyReleaseEvent(QKeyEvent* event) {
    switch (event->key()) {
        case Qt::Key_Escape:
            switchToMainWindow();
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

        default:
            QWidget::keyReleaseEvent(event);
    }
}

// Create all the buttons
void ImageBooth::createButtons() {
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
}

ClickableLabel* ImageBooth::createImageDelete() {
    if (data->imagesData.get()->size() <= 0) {
        return nullptr;
    }

    ClickableLabel* imageDeleteNew = new ClickableLabel(data, ICON_PATH_DELETE, TOOL_TIP_IMAGE_BOOTH_DELETE, this, actionSize);
    imageDeleteNew->setInitialBackground("transparent", "#b3b3b3");

    connect(imageDeleteNew, &ClickableLabel::clicked, [this]() {
        if (data->imagesSelected.empty()) {
            showInformationMessage(this, "No image selected", "You need to select an image to delete it");
            return;
        }
        std::vector<int> imagesSelectedBefore = *&data->imagesSelected;
        for (int i = 0; i < data->imagesSelected.size(); i++) {
            if (data->isDeleted(data->imagesSelected.at(i))) {
                data->unPreDeleteImage(data->imagesSelected.at(i));
                data->imagesDeleted.erase(std::remove(data->imagesDeleted.begin(), data->imagesDeleted.end(), data->imagesSelected.at(i)), data->imagesDeleted.end());
            } else {
                data->preDeleteImage(data->imagesSelected.at(i));
                data->imagesDeleted.push_back(data->imagesSelected.at(i));
            }
        }
        data->imagesSelected.clear();
        reload();

        data->saved = false;

        bool savedBefore = data->saved;

        data->addAction(
            [this, imagesSelectedBefore, savedBefore]() {
                if (!isImageVisible(imagesSelectedBefore.at(0))) {
                    gotToImage(imagesSelectedBefore.at(0));
                }
                for (int i = 0; i < imagesSelectedBefore.size(); i++) {
                    if (data->isDeleted(imagesSelectedBefore.at(i))) {
                        data->unPreDeleteImage(imagesSelectedBefore.at(i));
                        auto it = std::find(data->imagesSelected.begin(), data->imagesSelected.end(), imagesSelectedBefore.at(i));
                        if (it != data->imagesSelected.end()) {
                            data->imagesDeleted.erase(it);
                        }
                    } else {
                        data->preDeleteImage(imagesSelectedBefore.at(i));
                        data->imagesDeleted.push_back(imagesSelectedBefore.at(i));
                    }
                }
                if (savedBefore) {
                    data->saved = true;
                }
                reload();
            },
            [this, imagesSelectedBefore]() {
                if (!isImageVisible(imagesSelectedBefore.at(0))) {
                    gotToImage(imagesSelectedBefore.at(0));
                }
                for (int i = 0; i < imagesSelectedBefore.size(); i++) {
                    if (data->isDeleted(imagesSelectedBefore.at(i))) {
                        data->unPreDeleteImage(imagesSelectedBefore.at(i));
                        auto it = std::find(data->imagesSelected.begin(), data->imagesSelected.end(), imagesSelectedBefore.at(i));
                        if (it != data->imagesSelected.end()) {
                            data->imagesDeleted.erase(it);
                        }
                    } else {
                        data->preDeleteImage(imagesSelectedBefore.at(i));
                        data->imagesDeleted.push_back(imagesSelectedBefore.at(i));
                    }
                }
                data->saved = false;
                reload();
            });
    });

    return imageDeleteNew;
}

ClickableLabel* ImageBooth::createImageSave() {
    if (data->imagesData.get()->size() <= 0) {
        return nullptr;
    }

    ClickableLabel* imageSaveNew = new ClickableLabel(data, ICON_PATH_SAVE, TOOL_TIP_IMAGE_BOOTH_SAVE, this, actionSize);
    imageSaveNew->setInitialBackground("transparent", "#b3b3b3");

    connect(imageSaveNew, &ClickableLabel::clicked, [this]() {
        data->saveData();
    });

    return imageSaveNew;
}

ClickableLabel* ImageBooth::createImageExport() {
    if (data->imagesData.get()->size() <= 0) {
        return nullptr;
    }

    ClickableLabel* imageExportNew = new ClickableLabel(data, ICON_PATH_EXPORT, TOOL_TIP_IMAGE_BOOTH_EXPORT, this, actionSize);
    imageExportNew->setInitialBackground("transparent", "#b3b3b3");

    connect(imageExportNew, &ClickableLabel::clicked, [this]() {
    });

    imageExportNew->setDisabled(true);

    return imageExportNew;
}

ClickableLabel* ImageBooth::createImageRotateRight() {
    if (data->imagesData.get()->size() <= 0) {
        return nullptr;
    }

    ClickableLabel* imageRotateRightNew = new ClickableLabel(data, ICON_PATH_ROTATE_RIGHT, TOOL_TIP_IMAGE_BOOTH_ROTATE_RIGHT, this, actionSize);
    imageRotateRightNew->setInitialBackground("transparent", "#b3b3b3");

    connect(imageRotateRightNew, &ClickableLabel::clicked, [this]() {
        if (data->imagesSelected.empty()) {
            showInformationMessage(this, "No image selected", "You need to select an image to rotate it");
            return;
        }
        std::vector<int> imagesSelectedBefore = *&data->imagesSelected;
        for (int i = 0; i < data->imagesSelected.size(); i++) {
            std::string extension = data->imagesData.get()->at(data->imagesSelected.at(i)).getImageExtension();
            data->rotateRight(data->imagesSelected.at(i), extension, [this]() {}, false);
        }
        data->imagesSelected.clear();
        reload();

        data->addAction(
            [this, imagesSelectedBefore]() {
                if (!isImageVisible(imagesSelectedBefore.at(0))) {
                    gotToImage(imagesSelectedBefore.at(0));
                }
                for (int i = 0; i < imagesSelectedBefore.size(); i++) {
                    std::string extension = data->imagesData.get()->at(imagesSelectedBefore.at(i)).getImageExtension();
                    data->rotateLeft(imagesSelectedBefore.at(i), extension, [this]() {}, false);
                }
                reload();
            },
            [this, imagesSelectedBefore]() {
                if (!isImageVisible(imagesSelectedBefore.at(0))) {
                    gotToImage(imagesSelectedBefore.at(0));
                }
                for (int i = 0; i < imagesSelectedBefore.size(); i++) {
                    std::string extension = data->imagesData.get()->at(imagesSelectedBefore.at(i)).getImageExtension();
                    data->rotateRight(imagesSelectedBefore.at(i), extension, [this]() {}, false);
                }
                reload();
            });
    });

    return imageRotateRightNew;
}

ClickableLabel* ImageBooth::createImageRotateLeft() {
    if (data->imagesData.get()->size() <= 0) {
        return nullptr;
    }

    ClickableLabel* imageRotateLeftNew = new ClickableLabel(data, ICON_PATH_ROTATE_LEFT, TOOL_TIP_IMAGE_BOOTH_ROTATE_LEFT, this, actionSize);
    imageRotateLeftNew->setInitialBackground("transparent", "#b3b3b3");

    connect(imageRotateLeftNew, &ClickableLabel::clicked, [this]() {
        if (data->imagesSelected.empty()) {
            showInformationMessage(this, "No image selected", "You need to select an image to rotate it");
            return;
        }
        std::vector<int> imagesSelectedBefore = *&data->imagesSelected;
        for (int i = 0; i < data->imagesSelected.size(); i++) {
            std::string extension = data->imagesData.get()->at(data->imagesSelected.at(i)).getImageExtension();
            data->rotateLeft(data->imagesSelected.at(i), extension, [this]() {}, false);
        }
        data->imagesSelected.clear();
        reload();

        data->addAction(
            [this, imagesSelectedBefore]() {
                if (!isImageVisible(imagesSelectedBefore.at(0))) {
                    gotToImage(imagesSelectedBefore.at(0));
                }
                for (int i = 0; i < imagesSelectedBefore.size(); i++) {
                    std::string extension = data->imagesData.get()->at(imagesSelectedBefore.at(i)).getImageExtension();
                    data->rotateRight(imagesSelectedBefore.at(i), extension, [this]() {}, false);
                }
                reload();
            },
            [this, imagesSelectedBefore]() {
                if (!isImageVisible(imagesSelectedBefore.at(0))) {
                    gotToImage(imagesSelectedBefore.at(0));
                }
                for (int i = 0; i < imagesSelectedBefore.size(); i++) {
                    std::string extension = data->imagesData.get()->at(imagesSelectedBefore.at(i)).getImageExtension();
                    data->rotateLeft(imagesSelectedBefore.at(i), extension, [this]() {}, false);
                }
                reload();
            });
    });

    return imageRotateLeftNew;
}

ClickableLabel* ImageBooth::createImageMirrorUpDown() {
    if (data->imagesData.get()->size() <= 0) {
        return nullptr;
    }

    ClickableLabel* imageMirrorUpDownNew = new ClickableLabel(data, ICON_PATH_MIRROR_UP_DOWN, TOOL_TIP_IMAGE_BOOTH_MIRROR_UP_DOWN, this, actionSize);
    imageMirrorUpDownNew->setInitialBackground("transparent", "#b3b3b3");

    connect(imageMirrorUpDownNew, &ClickableLabel::clicked, [this]() {
        if (data->imagesSelected.empty()) {
            showInformationMessage(this, "No image selected", "You need to select an image to mirror it");
            return;
        }
        std::vector<int> imagesSelectedBefore = *&data->imagesSelected;

        for (int i = 0; i < data->imagesSelected.size(); i++) {
            std::string extension = data->imagesData.get()->at(data->imagesSelected.at(i)).getImageExtension();
            data->mirrorUpDown(data->imagesSelected.at(i), extension, [this]() {}, false);
        }
        data->imagesSelected.clear();
        reload();

        data->addAction(
            [this, imagesSelectedBefore]() {
                if (!isImageVisible(imagesSelectedBefore.at(0))) {
                    gotToImage(imagesSelectedBefore.at(0));
                }
                for (int i = 0; i < imagesSelectedBefore.size(); i++) {
                    std::string extension = data->imagesData.get()->at(imagesSelectedBefore.at(i)).getImageExtension();
                    data->mirrorUpDown(imagesSelectedBefore.at(i), extension, [this]() {}, false);
                }
                reload();
            },
            [this, imagesSelectedBefore]() {
                if (!isImageVisible(imagesSelectedBefore.at(0))) {
                    gotToImage(imagesSelectedBefore.at(0));
                }
                for (int i = 0; i < imagesSelectedBefore.size(); i++) {
                    std::string extension = data->imagesData.get()->at(imagesSelectedBefore.at(i)).getImageExtension();
                    data->mirrorUpDown(imagesSelectedBefore.at(i), extension, [this]() {}, false);
                }
                reload();
            });
    });

    return imageMirrorUpDownNew;
}

ClickableLabel* ImageBooth::createImageMirrorLeftRight() {
    if (data->imagesData.get()->size() <= 0) {
        return nullptr;
    }

    ClickableLabel* imageMirrorLeftRightNew = new ClickableLabel(data, ICON_PATH_MIRROR_LEFT_RIGHT, TOOL_TIP_IMAGE_BOOTH_MIRROR_LEFT_RIGHT, this, actionSize);
    imageMirrorLeftRightNew->setInitialBackground("transparent", "#b3b3b3");

    connect(imageMirrorLeftRightNew, &ClickableLabel::clicked, [this]() {
        if (data->imagesSelected.empty()) {
            showInformationMessage(this, "No image selected", "You need to select an image to mirror it");
            return;
        }
        std::vector<int> imagesSelectedBefore = *&data->imagesSelected;
        for (int i = 0; i < data->imagesSelected.size(); i++) {
            std::string extension = data->imagesData.get()->at(data->imagesSelected.at(i)).getImageExtension();
            data->mirrorLeftRight(data->imagesSelected.at(i), extension, [this]() {}, false);
        }
        data->imagesSelected.clear();
        reload();

        data->addAction(
            [this, imagesSelectedBefore]() {
                if (!isImageVisible(imagesSelectedBefore.at(0))) {
                    gotToImage(imagesSelectedBefore.at(0));
                }
                for (int i = 0; i < imagesSelectedBefore.size(); i++) {
                    std::string extension = data->imagesData.get()->at(imagesSelectedBefore.at(i)).getImageExtension();
                    data->mirrorLeftRight(imagesSelectedBefore.at(i), extension, [this]() {}, false);
                }
                reload();
            },
            [this, imagesSelectedBefore]() {
                if (!isImageVisible(imagesSelectedBefore.at(0))) {
                    gotToImage(imagesSelectedBefore.at(0));
                }
                for (int i = 0; i < imagesSelectedBefore.size(); i++) {
                    std::string extension = data->imagesData.get()->at(imagesSelectedBefore.at(i)).getImageExtension();
                    data->mirrorLeftRight(imagesSelectedBefore.at(i), extension, [this]() {}, false);
                }
                reload();
            });
    });

    return imageMirrorLeftRightNew;
}

ClickableLabel* ImageBooth::createImageEditExif() {
    if (data->imagesData.get()->size() <= 0) {
        return nullptr;
    }

    ClickableLabel* imageEditExifNew = new ClickableLabel(data, ICON_PATH_EDIT_EXIF, TOOL_TIP_IMAGE_BOOTH_EDIT_EXIF, this, actionSize);
    imageEditExifNew->setInitialBackground("transparent", "#b3b3b3");

    connect(imageEditExifNew, &ClickableLabel::clicked, [this]() {
    });

    imageEditExifNew->setDisabled(true);

    return imageEditExifNew;
}

ClickableLabel* ImageBooth::createImageConversion() {
    if (data->imagesData.get()->size() <= 0) {
        return nullptr;
    }

    ClickableLabel* imageConversionNew = new ClickableLabel(data, ICON_PATH_CONVERSION, TOOL_TIP_IMAGE_BOOTH_CONVERSION, this, actionSize);
    imageConversionNew->setInitialBackground("transparent", "#b3b3b3");

    connect(imageConversionNew, &ClickableLabel::clicked, [this]() {
        if (data->imagesSelected.empty()) {
            showInformationMessage(this, "No image selected", "You need to select an image to convert it");
            return;
        }
        if (data->imagesSelected.size() > 0) {
            QString selectedFormat = launchConversionDialog();
            if (selectedFormat != nullptr) {
                for (int i = 0; i < data->imagesSelected.size(); i++) {
                    QString inputImagePath = QString::fromStdString(data->imagesData.getImageData(data->imagesSelected.at(i))->getImagePath());
                    convertion(inputImagePath, selectedFormat);
                }
                data->imagesSelected.clear();
                reload();
            }
        }
        data->addAction(
            [this]() {
                showErrorMessage(this, "Impossible de revenir en arrière");
            },
            [this]() {
                showErrorMessage(this, "Impossible de revenir en arrière");
            });
    });

    return imageConversionNew;
}

void ImageBooth::reload() {
    updateImages();
}
void ImageBooth::enterEvent(QEnterEvent* event) {
    qDebug() << "enterEvent 2 ";
    this->setFocus();
    QMainWindow::enterEvent(event);
}