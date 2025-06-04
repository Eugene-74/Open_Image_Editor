#include "ImageBooth.hpp"

#include <QApplication>
#include <QCheckBox>
#include <QDebug>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QMetaObject>
#include <QPointer>
#include <QProgressDialog>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QSpacerItem>
#include <QTimer>

#include "ClickableFolderLabel.hpp"
#include "ClickableLabel.hpp"
#include "Const.hpp"
#include "Conversion.hpp"
#include "Text.hpp"
#include "Verification.hpp"

/**
 * @brief Constructor for the ImageBooth class
 * @param dat Pointer to the Data object
 * @param parent Pointer to the parent widget
 */
ImageBooth::ImageBooth(std::shared_ptr<Data> dat, QWidget* parent)
    : QMainWindow(parent), data(dat) {
    data->getImagesDataPtr()->getCurrent()->clear();

    imageQuality = Const::Thumbnail::POOR_QUALITY;
    int i = Const::Thumbnail::THUMBNAIL_SIZES.size() - 2;
    while (i > 0 && imageQuality == Const::Thumbnail::POOR_QUALITY) {
        if (imageSize->height() + imageSize->width() >= Const::Thumbnail::THUMBNAIL_SIZES[i]) {
            imageQuality = Const::Thumbnail::THUMBNAIL_SIZES[i + 1];
        }
        i--;
    }

    if (data->getCurrentFolders()->getName() == "*") {
        qInfo() << "Opening all images folder";
        Folders* firstFolder = data->findFirstFolderWithAllImages();
        Folders* allImagesFolder = new Folders("*");

        allImagesFolder->setParent(firstFolder);

        auto images = data->getImagesDataPtr()->get();
        for (auto it = images->begin(); it != images->end(); ++it) {
            ImageData* imageData = *it;
            if (imageData->respectFilters(data->getImagesDataPtr()->getFilters())) {
                data->getImagesDataPtr()->getCurrent()->push_back(imageData);
            } else {
                qInfo() << "Image " << imageData->getImagePath() << " does not respect filters";
            }
        }
        data->setCurrentFolders(allImagesFolder);
    } else {
        qInfo() << "Opening folder, with " << data->getCurrentFolders()->getFilesPtr()->size() << " images" << " and " << data->getCurrentFolders()->getFolders()->size() << " folders";

        for (auto it = data->getCurrentFolders()->getFilesPtr()->begin(); it != data->getCurrentFolders()->getFilesPtr()->end(); ++it) {
            std::string imagePath = *it;
            ImageData* imageData = data->getImagesDataPtr()->getImageData(imagePath);

            if (imageData == nullptr) {
                qCritical() << "imageData is null";
            } else {
                if (imageData->respectFilters(data->getImagesDataPtr()->getFilters())) {
                    data->getImagesDataPtr()->getCurrent()->push_back(imageData);
                }
            }
        }
    }

    data->sortCurrentImagesData();

    data->clearCache();

    lastLineNbr = 0;

    auto* centralWidget = new QWidget(parent);
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
    scrollArea->setFixedSize(data->getSizesPtr()->imagesBoothSizes->scrollAreaSize);
    actionButtonLayout->setAlignment(Qt::AlignCenter);
    scrollLayout->addWidget(scrollArea);

    scrollWidget = new QWidget();
    linesLayout = new QVBoxLayout(scrollWidget);
    scrollArea->setWidget(scrollWidget);

    int minTotalImagesHeight = data->getSizesPtr()->imagesBoothSizes->realImageSize.height() * (data->getImagesDataPtr()->getCurrent()->size() / data->getSizesPtr()->imagesBoothSizes->widthImageNumber + 1);
    int minTotalFoldersHeight = data->getSizesPtr()->imagesBoothSizes->realImageSize.height() * (getCurrentFoldersSize() / data->getSizesPtr()->imagesBoothSizes->imagesPerLine + 1);

    int minHeight = minTotalImagesHeight + minTotalFoldersHeight;
    scrollWidget->setMinimumHeight(minHeight);

    linesLayout->setAlignment(Qt::AlignTop);
    linesLayout->setSpacing(data->getSizesPtr()->imagesBoothSizes->linesLayoutSpacing);
    linesLayout->setContentsMargins(
        data->getSizesPtr()->imagesBoothSizes->linesLayoutMargins[0],   // gauche
        data->getSizesPtr()->imagesBoothSizes->linesLayoutMargins[1],   // haut
        data->getSizesPtr()->imagesBoothSizes->linesLayoutMargins[2],   // droite
        data->getSizesPtr()->imagesBoothSizes->linesLayoutMargins[3]);  // bas

    spacer = new QSpacerItem(0, 0);
    linesLayout->insertSpacerItem(0, spacer);

    createFirstImages();

    connect(scrollArea->verticalScrollBar(), &QScrollBar::valueChanged, this, &ImageBooth::onScroll);

    QTimer::singleShot(100, this, [this]() {
        // wait because else it doesn't work
        gotToImage(data->getImagesDataPtr()->getImageNumber(), true);
    });
}

/**
 * @brief Open a folder and load its images
 * @param index Index of the folder to open (in the current folder) (-2 for parent folder, -1 for all images)
 */
void ImageBooth::openFolder(int index) {
    data->getImagesDataPtr()->getCurrent()->clear();

    if (data->getCurrentFolders()->getFolders()->size() > index || index == -2) {
        if (index == -2) {
            if (data->getFolderPath(data.get()->getCurrentFolders()) == data->getFolderPath(data->findFirstFolderWithAllImages())) {
                switchToMainWindow();
                return;
            }

            if (data->getCurrentFolders()->getParent() == nullptr) {
                switchToMainWindow();
            }
            data->setCurrentFolders(data->getCurrentFolders()->getParent());
        } else {
            data->setCurrentFolders(data->getCurrentFolders()->getFolder(index));
        }
        for (auto it = data->getCurrentFolders()->getFilesPtr()->begin(); it != data->getCurrentFolders()->getFilesPtr()->end(); ++it) {
            std::string imagePath = *it;
            ImageData* imageData = data->getImagesDataPtr()->getImageData(imagePath);
            if (imageData->respectFilters(data->getImagesDataPtr()->getFilters())) {
                data->getImagesDataPtr()->getCurrent()->push_back(imageData);
            }
        }
    } else {
        auto* allImagesFolder = new Folders("*");
        allImagesFolder->setParent(data->findFirstFolderWithAllImages());

        auto* images = data->getImagesDataPtr()->get();
        for (auto it = images->begin(); it != images->end(); ++it) {
            ImageData* imageData = *it;
            if (imageData->respectFilters(data->getImagesDataPtr()->getFilters())) {
                data->getImagesDataPtr()->getCurrent()->push_back(imageData);
            }
        }

        data->setCurrentFolders(allImagesFolder);
    }

    int minTotalImagesHeight = data->getSizesPtr()->imagesBoothSizes->realImageSize.height() * (data->getImagesDataPtr()->getCurrent()->size() / data->getSizesPtr()->imagesBoothSizes->widthImageNumber + 1);

    int minTotalFoldersHeight = data->getSizesPtr()->imagesBoothSizes->realImageSize.height() * (getCurrentFoldersSize() / data->getSizesPtr()->imagesBoothSizes->widthImageNumber + 1);

    int minHeight = minTotalImagesHeight + minTotalFoldersHeight;
    scrollWidget->setMinimumHeight(minHeight);

    data->getImagesDataPtr()->setImageNumber(0);
    data->clearCache();

    data->sortCurrentImagesData();

    reload();

    data->addAction(
        [this, index]() {
            openFolder(-2);
        },
        [this, index]() {
            openFolder(index);
        });
}

/**
 * @brief Update the visible images in the image booth
 * @param force Force the update even if the number of visible images has not changed
 */
void ImageBooth::updateVisibleImages(bool force) {
    int spacerHeight = scrollArea->verticalScrollBar()->value();
    int imageHeight = data->getSizesPtr()->imagesBoothSizes->realImageSize.height();
    spacerHeight = (spacerHeight / imageHeight) * imageHeight;

    int imageNumber = (spacerHeight / realImageSize->height()) * data->getSizesPtr()->imagesBoothSizes->imagesPerLine;
    data->getImagesDataPtr()->setImageNumber(imageNumber);

    int lineNbr = spacerHeight / imageHeight;
    int difLineNbr = lineNbr - lastLineNbr;

    if (difLineNbr == 0 && !force) {
        return;
    }
    spacer->changeSize(0, spacerHeight);

    updateImages();
    linesLayout->invalidate();
    lastLineNbr = lineNbr;

    data->checkToUnloadImages(imageNumber, IMAGE_BOOTH_PRE_LOAD_RADIUS * maxVisibleLines * data->getSizesPtr()->imagesBoothSizes->imagesPerLine);
    data->checkToLoadImages(imageNumber, IMAGE_BOOTH_PRE_LOAD_RADIUS * maxVisibleLines * data->getSizesPtr()->imagesBoothSizes->imagesPerLine, imageQuality);
}

/**
 * @brief Check if the image is visible in the current view
 * @param imageIndex Index of the image to check
 * @return True if the image is visible, false otherwise
 */
bool ImageBooth::isImageVisible(int imageIndex) {
    int spacerHeight = scrollArea->verticalScrollBar()->value();
    int imageHeight = data->getSizesPtr()->imagesBoothSizes->realImageSize.height();
    spacerHeight = (spacerHeight / imageHeight) * imageHeight;

    int firstImageNbr = spacerHeight / imageHeight * data->getSizesPtr()->imagesBoothSizes->widthImageNumber;
    int lastImageNbr = (spacerHeight / imageHeight + maxVisibleLines) * data->getSizesPtr()->imagesBoothSizes->widthImageNumber;

    return (imageIndex >= firstImageNbr && imageIndex <= lastImageNbr);
}

/**
 * @brief Create the first images in the image booth
 */
void ImageBooth::createFirstImages() {
    for (int line = 0; line < maxVisibleLines; line++) {
        auto* lineLayout = new QHBoxLayout();
        lineLayout->setAlignment(Qt::AlignLeft);

        linesLayout->addLayout(lineLayout);
        lineLayouts.push_back(lineLayout);
        int imagePerLine = data->getSizesPtr()->imagesBoothSizes->imagesPerLine;

        int nbrInLine = 0;
        int folderNumber = (line * imagePerLine) + nbrInLine;

        for (int i = 0; i < imagePerLine; i++) {
            auto* imageButton = new ClickableLabel(data, Const::ImagePath::LOADING,
                                                   "", this, imageSize, false, 0, true);
            imageButton->hide();
            lineLayout->addWidget(imageButton);
            nbrInLine++;
            folderNumber = line * imagePerLine + nbrInLine;
        }
    }
}

/**
 * @brief Update the images in the image booth when scrolling
 * @param value Value of the scroll bar
 */
void ImageBooth::onScroll(int value) {
    updateVisibleImages();
}

/**
 * @brief Create an image button in the image booth
 * @param imagePath Path to the image
 * @param nbr Index of the image in the current folder
 * @return Pointer to the created ClickableLabel object
 */
ClickableLabel* ImageBooth::createImage(std::string imagePath, int nbr) {
    if (data->getImagesDataPtr()->getCurrent()->size() <= 0) {
        return nullptr;
    }
    ClickableLabel* imageButton;

    if (data->isInCache(data->getThumbnailPath(imagePath, imageQuality)) || imagePath.rfind(":", 0) == 0) {
        imageButton = new ClickableLabel(data, QString::fromStdString(imagePath),
                                         "", this, imageSize, false, imageQuality, true);
    } else if (data->hasThumbnail(imagePath, Const::Thumbnail::POOR_QUALITY)) {
        imageButton = new ClickableLabel(data, QString::fromStdString(imagePath),
                                         "", this, imageSize, false, imageQuality, true);
    } else {
        imageButton = new ClickableLabel(data, Const::ImagePath::LOADING,
                                         "", this, imageSize, false, 0, true);
        QPointer<ImageBooth> self = this;
        data->createAllThumbnailsAsync(imagePath, [self, imagePath, nbr](bool result) {
            if (self) {
                if (result) {
                    QHBoxLayout* lineLayout = nullptr;
                    ClickableLabel* lastImageButton = self->getClickableLabelIfExist(nbr, lineLayout);
                    if (lastImageButton != nullptr) {
                        QMetaObject::invokeMethod(self, [lineLayout, lastImageButton, self, imagePath, nbr]() {
                            ClickableLabel* newImageButton = self->createImage(imagePath, nbr);
                            if (lineLayout != nullptr) {
                                lineLayout->replaceWidget(lastImageButton, newImageButton);
                                lastImageButton->deleteLater();
                            } else {
                                newImageButton->deleteLater();
                        } }, Qt::QueuedConnection);
                    }
                }
            } }, true);
    }

    imageButton->setInitialBorder(Const::Color::TRANSPARENT1, Const::Color::LIGHT_GRAY);

    int imageNumberInTotal = data->getImagesDataPtr()->getImageNumberInTotal(nbr);

    if (std::find(data->getImagesSelectedPtr()->begin(), data->getImagesSelectedPtr()->end(), imageNumberInTotal) != data->getImagesSelectedPtr()->end()) {
        imageButton->setBorder(COLOR_BACKGROUND_IMAGE_BOOTH_SELECTED, COLOR_BACKGROUND_HOVER_IMAGE_BOOTH_SELECTED);
    }

    if (imageNumberInTotal == imageShiftSelected) {
        if (imageShiftSelectedSelect) {
            imageButton->setBorder(COLOR_BACKGROUND_IMAGE_BOOTH_SELECTED_MULTIPLE_SELECT, COLOR_BACKGROUND_HOVER_IMAGE_BOOTH_SELECTED_MULTIPLE_SELECT);
        } else {
            imageButton->setBorder(COLOR_BACKGROUND_IMAGE_BOOTH_SELECTED_MULTIPLE_UNSELECT, COLOR_BACKGROUND_HOVER_IMAGE_BOOTH_SELECTED_MULTIPLE_UNSELECT);
        }
    }

    connect(imageButton, &ClickableLabel::leftClicked, [this, nbr]() {
        data->getImagesDataPtr()->setImageNumber(nbr);
        switchToImageEditor();
    });

    connect(imageButton, &ClickableLabel::ctrlLeftClicked, [this, nbr, imageButton]() {
        int imageNumberInTotal = data->getImagesDataPtr()->getImageNumberInTotal(nbr);
        auto it = std::find(data->getImagesSelectedPtr()->begin(), data->getImagesSelectedPtr()->end(), imageNumberInTotal);
        if (it != data->getImagesSelectedPtr()->end()) {
            imageButton->resetBorder();
            data->getImagesSelectedPtr()->erase(it);

            data->addAction(
                [this, nbr, imageNumberInTotal]() {
                    if (!isImageVisible(nbr)) {
                        gotToImage(nbr);
                    }
                    QTimer::singleShot(TIME_UNDO_VISUALISATION, [this, nbr, imageNumberInTotal]() {
                        ClickableLabel* imageButton = getClickableLabelIfExist(nbr);
                        if (imageButton != nullptr) {
                            imageButton->setBorder(COLOR_BACKGROUND_IMAGE_BOOTH_SELECTED, COLOR_BACKGROUND_HOVER_IMAGE_BOOTH_SELECTED);
                        }
                        addNbrToSelectedImages(imageNumberInTotal);
                    });
                },
                [this, nbr, imageNumberInTotal]() {
                    if (!isImageVisible(nbr)) {
                        gotToImage(nbr);
                    }
                    QTimer::singleShot(TIME_UNDO_VISUALISATION, [this, nbr, imageNumberInTotal]() {
                        ClickableLabel* imageButton = getClickableLabelIfExist(nbr);
                        if (imageButton != nullptr) {
                            imageButton->resetBorder();
                        }
                        removeNbrToSelectedImages(imageNumberInTotal);
                    });
                });
        } else {
            imageButton->setBorder(COLOR_BACKGROUND_IMAGE_BOOTH_SELECTED, COLOR_BACKGROUND_HOVER_IMAGE_BOOTH_SELECTED);
            data->getImagesSelectedPtr()->push_back(imageNumberInTotal);

            data->addAction(
                [this, nbr, imageNumberInTotal]() {
                    int time = 0;
                    if (!isImageVisible(nbr)) {
                        gotToImage(nbr);
                        time = TIME_UNDO_VISUALISATION;
                    }
                    QTimer::singleShot(time, [this, nbr, imageNumberInTotal]() {
                        ClickableLabel* imageButton = getClickableLabelIfExist(nbr);
                        if (imageButton != nullptr) {
                            imageButton->resetBorder();
                        }
                        removeNbrToSelectedImages(imageNumberInTotal);
                    });
                },
                [this, nbr, imageNumberInTotal]() {
                    int time = 0;
                    if (!isImageVisible(nbr)) {
                        gotToImage(nbr);
                        time = TIME_UNDO_VISUALISATION;
                    }
                    QTimer::singleShot(time, [this, nbr, imageNumberInTotal]() {
                        ClickableLabel* imageButton = getClickableLabelIfExist(nbr);
                        if (imageButton != nullptr) {
                            imageButton->setBorder(COLOR_BACKGROUND_IMAGE_BOOTH_SELECTED, COLOR_BACKGROUND_HOVER_IMAGE_BOOTH_SELECTED);
                        }
                        addNbrToSelectedImages(imageNumberInTotal);
                    });
                });
        }
    });

    connect(imageButton, &ClickableLabel::shiftLeftClicked, [this, nbr, imageButton]() {
        // select all image beetwen the 2 nbr
        if (imageShiftSelected >= 0) {
            std::vector<int> modifiedNbr;
            std::vector<int> modifiedNbrInTotal;

            int imageShiftSelectedInCurrent = data->getImagesDataPtr()->getImageNumberInCurrent(imageShiftSelected);

            int start = std::min(imageShiftSelectedInCurrent, nbr);
            int end = std::max(imageShiftSelectedInCurrent, nbr);

            for (int i = start; i <= end; ++i) {
                int imageNumberInTotal = data->getImagesDataPtr()->getImageNumberInTotal(i);
                auto it = std::find(data->getImagesSelectedPtr()->begin(), data->getImagesSelectedPtr()->end(), imageNumberInTotal);
                if (it != data->getImagesSelectedPtr()->end()) {
                    if (!imageShiftSelectedSelect) {
                        data->getImagesSelectedPtr()->erase(it);
                        modifiedNbr.push_back(i);
                        modifiedNbrInTotal.push_back(imageNumberInTotal);
                    }
                } else {
                    if (imageShiftSelectedSelect) {
                        data->getImagesSelectedPtr()->push_back(imageNumberInTotal);
                        modifiedNbr.push_back(i);
                        modifiedNbrInTotal.push_back(imageNumberInTotal);
                    }
                }
            }
            reload();

            bool select = imageShiftSelectedSelect;
            data->addAction(
                [this, modifiedNbr, modifiedNbrInTotal, select]() {
                    int time = 0;
                    int minNbr = *std::min_element(modifiedNbr.begin(), modifiedNbr.end());
                    if (!isImageVisible(minNbr)) {
                        gotToImage(minNbr);
                        time = TIME_UNDO_VISUALISATION;
                    }
                    QTimer::singleShot(time, [this, modifiedNbrInTotal, select]() {
                        for (auto nbr : modifiedNbrInTotal) {
                            if (select) {
                                removeNbrToSelectedImages(nbr);
                            } else {
                                addNbrToSelectedImages(nbr);
                            }
                        }
                        reload();
                    });
                },
                [this, modifiedNbr, modifiedNbrInTotal, select]() {
                    int time = 0;
                    int minNbr = *std::min_element(modifiedNbr.begin(), modifiedNbr.end());
                    if (!isImageVisible(minNbr)) {
                        gotToImage(minNbr);
                        time = TIME_UNDO_VISUALISATION;
                    }
                    QTimer::singleShot(time, [this, modifiedNbrInTotal, select]() {
                        for (auto nbr : modifiedNbrInTotal) {
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
            int imageNumberInTotal = data->getImagesDataPtr()->getImageNumberInTotal(nbr);

            auto it = std::find(data->getImagesSelectedPtr()->begin(), data->getImagesSelectedPtr()->end(), imageNumberInTotal);
            if (it != data->getImagesSelectedPtr()->end()) {
                imageButton->setBorder(COLOR_BACKGROUND_IMAGE_BOOTH_SELECTED_MULTIPLE_UNSELECT, COLOR_BACKGROUND_HOVER_IMAGE_BOOTH_SELECTED_MULTIPLE_UNSELECT);
                imageShiftSelectedSelect = false;
            } else {
                imageButton->setBorder(COLOR_BACKGROUND_IMAGE_BOOTH_SELECTED_MULTIPLE_SELECT, COLOR_BACKGROUND_HOVER_IMAGE_BOOTH_SELECTED_MULTIPLE_SELECT);
                imageShiftSelectedSelect = true;
            }
            imageShiftSelected = imageNumberInTotal;
        }
    });
    return imageButton;
}

/**
 * @brief Go to a specific image in the image booth
 * @param imageNumberInCurrent The image number in the current folder
 * @param force Force the update even if the image is already visible
 */
void ImageBooth::gotToImage(int imageNumberInCurrent, bool force) {
    int imageLine = imageNumberInCurrent / data->getSizesPtr()->imagesBoothSizes->widthImageNumber;
    int spacerHeight = imageLine * data->getSizesPtr()->imagesBoothSizes->realImageSize.height();

    scrollArea->verticalScrollBar()->setValue(spacerHeight);

    updateVisibleImages(force);
    QCoreApplication::processEvents();
}

/**
 * @brief Add a number to the selected images
 * @param nbr The number to add (image number in imagesData)
 */
void ImageBooth::addNbrToSelectedImages(int nbr) {
    auto it = std::find(data->getImagesSelectedPtr()->begin(), data->getImagesSelectedPtr()->end(), nbr);
    if (it == data->getImagesSelectedPtr()->end()) {
        data->getImagesSelectedPtr()->push_back(nbr);
    }
}

/**
 * @brief Remove a number from the selected images
 * @param nbr The number to remove (image number in imagesData)
 */
void ImageBooth::removeNbrToSelectedImages(int nbr) {
    auto it = std::find(data->getImagesSelectedPtr()->begin(), data->getImagesSelectedPtr()->end(), nbr);
    if (it != data->getImagesSelectedPtr()->end()) {
        data->getImagesSelectedPtr()->erase(it);
    }
}

/**
 * @brief Get the clickableLabel (and lineLayout) of imageNbr if it exists (visible in the current view)
 * @param imageNbr The image number in the current folder
 * @param lineLayout Reference to the line layout containing the image button
 * @return Pointer to the ClickableLabel object if it exists, nullptr otherwise
 */
ClickableLabel* ImageBooth::getClickableLabelIfExist(int imageNbr, QHBoxLayout*& lineLayout) {
    int foldersLineNumber = (getCurrentFoldersSize() / data->getSizesPtr()->imagesBoothSizes->widthImageNumber) + 1;
    imageNbr += foldersLineNumber * data->getSizesPtr()->imagesBoothSizes->widthImageNumber;

    int spacerHeight = scrollArea->verticalScrollBar()->value();
    int imageHeight = data->getSizesPtr()->imagesBoothSizes->realImageSize.height();
    spacerHeight = (spacerHeight / imageHeight) * imageHeight;

    int firstImageNbr = spacerHeight / imageHeight * data->getSizesPtr()->imagesBoothSizes->widthImageNumber;
    int lastImageNbr = (spacerHeight / imageHeight + maxVisibleLines) * data->getSizesPtr()->imagesBoothSizes->widthImageNumber;

    if (imageNbr >= firstImageNbr && imageNbr <= lastImageNbr) {
        int firstImageLine = firstImageNbr / data->getSizesPtr()->imagesBoothSizes->widthImageNumber;
        int imageLine = imageNbr / data->getSizesPtr()->imagesBoothSizes->widthImageNumber;

        int imageRelativeLine = imageLine - firstImageLine;
        int imageNbrInLine = imageNbr - imageLine * data->getSizesPtr()->imagesBoothSizes->widthImageNumber;
        lineLayout = qobject_cast<QHBoxLayout*>(linesLayout->itemAt(imageRelativeLine + 1)->layout());
        ClickableLabel* imageButton = qobject_cast<ClickableLabel*>(lineLayout->itemAt(imageNbrInLine)->widget());

        return imageButton;
    }
    return nullptr;
}

/**
 * @brief Get the clickableLabel of imageNbr if it exists (visible in the current view)
 * @param imageNbr The image number in the current folder
 * @return Pointer to the ClickableLabel object if it exists, nullptr otherwise
 */
ClickableLabel* ImageBooth::getClickableLabelIfExist(int imageNbr) {
    QHBoxLayout* lineLayout = nullptr;
    return getClickableLabelIfExist(imageNbr, lineLayout);
}

/**
 * @brief Update the images in the image booth
 */
void ImageBooth::updateImages() {
    int spacerHeight = scrollArea->verticalScrollBar()->value();
    int imageHeight = data->getSizesPtr()->imagesBoothSizes->realImageSize.height();
    spacerHeight = (spacerHeight / imageHeight) * imageHeight;

    int foldersLineNumber = (getCurrentFoldersSize() / data->getSizesPtr()->imagesBoothSizes->widthImageNumber) + 1;
    int lineNbr = spacerHeight / imageHeight;

    int folderLinesNbr = std::min(std::max(foldersLineNumber - lineNbr, 0), maxVisibleLines);
    int imageLinesNbr = std::min(std::max(maxVisibleLines - folderLinesNbr, 0), maxVisibleLines);

    for (int i = 1; i < 1 + folderLinesNbr; i++) {
        QHBoxLayout* lineLayout = qobject_cast<QHBoxLayout*>(linesLayout->itemAt(i)->layout());
        for (int j = 0; j < lineLayout->count(); j++) {
            int folderNbr = ((lineNbr + i - 1) * data->getSizesPtr()->imagesBoothSizes->widthImageNumber) + j;

            ClickableLabel* lastFolderButton = qobject_cast<ClickableLabel*>(lineLayout->itemAt(j)->widget());

            if (folderNbr < getCurrentFoldersSize()) {
                if (folderNbr == 0) {
                    std::string firstFolderName = data->findFirstFolderWithAllImages()->getName();

                    ClickableLabel* folderButton;
                    if (data->getCurrentFolders()->getName() == firstFolderName) {
                        folderButton = new ClickableLabel(data, Const::IconPath::ALL_IMAGES,
                                                          Text::Tooltip::ImageBooth::all_images(),
                                                          this, imageSize, false, 0, true);
                        folderButton->addLogo(QColor::fromString(Const::Color::GREEN), QColor::fromString(Const::Color::DARK), data->getImagesDataPtr()->get()->size());
                    } else {
                        folderButton = new ClickableLabel(data, Const::IconPath::BACK,
                                                          Text::Tooltip::back_folder() + " : " + QString::fromStdString(data->getCurrentFolders()->getParent()->getName()),
                                                          this, imageSize, false, 0, true);
                    }

                    connect(folderButton, &ClickableLabel::leftClicked, [this]() {
                        std::string firstFolderName = data->findFirstFolderWithAllImages()->getName();

                        if (data->getCurrentFolders()->getName() == firstFolderName) {
                            openFolder(-1);
                        } else {
                            openFolder(-2);
                        }
                    });
                    lineLayout->replaceWidget(lastFolderButton, folderButton);
                    lastFolderButton->deleteLater();
                } else {
                    auto* folderButton = new ClickableFolderLabel(data, Const::IconPath::FOLDER,
                                                                  QString::fromStdString(data->getCurrentFolders()->getFolder(folderNbr - 1)->getName()),
                                                                  this, imageSize, false, 0, true);
                    folderButton->setText(data->getCurrentFolders()->getFolder(folderNbr - 1)->getName());
                    int totalImages = 0;
                    auto* currentFolder = data->getCurrentFolders()->getFolder(folderNbr - 1);
                    if (currentFolder) {
                        countImagesInFolder(currentFolder, totalImages);
                    }
                    folderButton->addLogo(QColor::fromString(Const::Color::GREEN), QColor::fromString(Const::Color::DARK), totalImages);

                    connect(folderButton, &ClickableLabel::leftClicked, [this, folderNbr]() {
                        openFolder(folderNbr - 1);
                    });
                    lineLayout->replaceWidget(lastFolderButton, folderButton);
                    lastFolderButton->deleteLater();
                }

            } else {
                lastFolderButton->hide();
            }
        }
    }
    // TODO opti trop long
    for (int i = 1 + folderLinesNbr; i < 1 + imageLinesNbr + folderLinesNbr; i++) {
        QHBoxLayout* lineLayout = qobject_cast<QHBoxLayout*>(linesLayout->itemAt(i)->layout());
        for (int j = 0; j < lineLayout->count(); j++) {
            int imageNbr = (lineNbr - foldersLineNumber + i - 1) * data->getSizesPtr()->imagesBoothSizes->widthImageNumber + j;
            ClickableLabel* lastImageButton = qobject_cast<ClickableLabel*>(lineLayout->itemAt(j)->widget());
            if (imageNbr >= data->getImagesDataPtr()->getCurrent()->size()) {
                lastImageButton->hide();
            } else {
                std::string imagePath;
                ClickableLabel* imageButton;
                if (data->getImagesDataPtr()->getImageDataInCurrent(imageNbr)) {
                    imagePath = data->getImagesDataPtr()->getImageDataInCurrent(imageNbr)->getImagePath();
                    // auto start = std::chrono::high_resolution_clock::now();
                    imageButton = createImage(imagePath, imageNbr);
                    // auto end = std::chrono::high_resolution_clock::now();
                    // auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
                    // qDebug() << "onScroll() execution time:" << duration << "ms";
                } else {
                    imageButton = new ClickableLabel(data, Const::ImagePath::ERROR_PATH,
                                                     "", this, imageSize, false, 0, true);
                }

                lineLayout->replaceWidget(lastImageButton, imageButton);

                lastImageButton->deleteLater();

                int imageNumberInTotal = data->getImagesDataPtr()->getImageNumberInTotal(imageNbr);

                auto it = std::find(data->getImagesSelectedPtr()->begin(), data->getImagesSelectedPtr()->end(), imageNumberInTotal);
                if (it != data->getImagesSelectedPtr()->end()) {
                    imageButton->setBorder(COLOR_BACKGROUND_IMAGE_BOOTH_SELECTED, COLOR_BACKGROUND_HOVER_IMAGE_BOOTH_SELECTED);
                } else {
                    imageButton->resetBorder();
                }
            }
        }
    }
}

/**
 * @brief Handle key release events in the image booth
 * @param event Pointer to the key event
 */
void ImageBooth::keyReleaseEvent(QKeyEvent* event) {
    switch (event->key()) {
        case Qt::Key_Escape: {
            std::string firstFolderName = data->findFirstFolderWithAllImages()->getName();
            if (firstFolderName == data->getCurrentFolders()->getName()) {
                switchToMainWindow();
            } else {
                openFolder(-2);
            }
        } break;

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

/**
 * @brief Create the action buttons in the image booth
 */
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

    editFilters = createEditFilters();

    actionButtonLayout->addWidget(imageRotateRight);
    actionButtonLayout->addWidget(imageRotateLeft);
    actionButtonLayout->addWidget(imageMirrorLeftRight);
    actionButtonLayout->addWidget(imageMirrorUpDown);
    actionButtonLayout->addWidget(imageDelete);
    actionButtonLayout->addWidget(imageSave);
    actionButtonLayout->addWidget(imageExport);
    actionButtonLayout->addWidget(imageConversion);
    actionButtonLayout->addWidget(imageEditExif);
    actionButtonLayout->addWidget(editFilters);
}

/**
 * @brief Create the delete image button
 * @return Pointer to the created ClickableLabel object
 */
ClickableLabel* ImageBooth::createImageDelete() {
    if (data->getImagesDataPtr()->get()->size() <= 0) {
        return nullptr;
    }

    ClickableLabel* imageDeleteNew = new ClickableLabel(data, Const::IconPath::DELETE_ICON, Text::Tooltip::ImageBooth::delete_tip(), this, actionSize);
    imageDeleteNew->setInitialBackground(Const::Color::TRANSPARENT1, Const::Color::LIGHT_GRAY);

    connect(imageDeleteNew, &ClickableLabel::clicked, [this]() {
        if (data->getImagesSelectedPtr()->empty()) {
            showInformationMessage(this, "No image selected", "You need to select an image to delete it");
            return;
        }
        std::vector<int> imagesSelectedBefore = *data->getImagesSelectedPtr();
        for (int i = 0; i < data->getImagesSelectedPtr()->size(); i++) {
            if (data->isDeleted(data->getImagesSelectedPtr()->at(i))) {
                data->unPreDeleteImage(data->getImagesSelectedPtr()->at(i));
            } else {
                data->preDeleteImage(data->getImagesSelectedPtr()->at(i));
            }
        }
        data->getImagesSelectedPtr()->clear();
        reload();

        data->setSaved(false);

        bool savedBefore = data->getSaved();

        data->addAction(
            [this, imagesSelectedBefore, savedBefore]() {
                if (!isImageVisible(imagesSelectedBefore.at(0))) {
                    gotToImage(imagesSelectedBefore.at(0));
                }
                for (int i = 0; i < imagesSelectedBefore.size(); i++) {
                    if (data->isDeleted(imagesSelectedBefore.at(i))) {
                        data->unPreDeleteImage(imagesSelectedBefore.at(i));
                    } else {
                        data->preDeleteImage(imagesSelectedBefore.at(i));
                    }
                }
                if (savedBefore) {
                    data->setSaved(true);
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
                    } else {
                        data->preDeleteImage(imagesSelectedBefore.at(i));
                    }
                }
                data->setSaved(false);
                reload();
            });
    });

    return imageDeleteNew;
}

/**
 * @brief Create the save image button
 * @return Pointer to the created ClickableLabel object
 */
ClickableLabel* ImageBooth::createImageSave() {
    if (data->getImagesDataPtr()->get()->size() <= 0) {
        return nullptr;
    }

    ClickableLabel* imageSaveNew = new ClickableLabel(data, Const::IconPath::SAVE, Text::Tooltip::ImageBooth::save(), this, actionSize);
    imageSaveNew->setInitialBackground(Const::Color::TRANSPARENT1, Const::Color::LIGHT_GRAY);

    connect(imageSaveNew, &ClickableLabel::clicked, [this]() {
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
    });

    return imageSaveNew;
}

/**
 * @brief Create the export image button
 * @return Pointer to the created ClickableLabel object
 */
ClickableLabel* ImageBooth::createImageExport() {
    if (data->getImagesDataPtr()->get()->size() <= 0) {
        return nullptr;
    }

    auto* imageExportNew = new ClickableLabel(data, Const::IconPath::EXPORT, Text::Tooltip::ImageBooth::export_tip(), this, actionSize);
    imageExportNew->setInitialBackground(Const::Color::TRANSPARENT1, Const::Color::LIGHT_GRAY);

    connect(imageExportNew, &ClickableLabel::clicked, [this]() {
    });

    imageExportNew->setDisabled(true);

    return imageExportNew;
}

/**
 * @brief Create the rotate right image button
 * @return Pointer to the created ClickableLabel object
 */
ClickableLabel* ImageBooth::createImageRotateRight() {
    if (data->getImagesDataPtr()->get()->size() <= 0) {
        return nullptr;
    }

    auto* imageRotateRightNew = new ClickableLabel(data, Const::IconPath::ROTATE_RIGHT, Text::Tooltip::ImageBooth::rotate_right(), this, actionSize);
    imageRotateRightNew->setInitialBackground(Const::Color::TRANSPARENT1, Const::Color::LIGHT_GRAY);

    connect(imageRotateRightNew, &ClickableLabel::clicked, [this]() {
        if (data->getImagesSelectedPtr()->empty()) {
            showInformationMessage(this, "No image selected", "You need to select an image to rotate it");
            return;
        }
        std::vector<int> imagesSelectedBefore = *data->getImagesSelectedPtr();
        for (int i = 0; i < data->getImagesSelectedPtr()->size(); i++) {
            std::string extension = data->getImagesDataPtr()->get()->at(data->getImagesSelectedPtr()->at(i))->getImageExtension();
            data->rotateRight(data->getImagesSelectedPtr()->at(i), extension, [this]() {}, false);
        }
        data->getImagesSelectedPtr()->clear();
        reload();

        data->addAction(
            [this, imagesSelectedBefore]() {
                int imagesSelectedBeforeInCurrent0 = data->getImagesDataPtr()->getImageNumberInCurrent(imagesSelectedBefore.at(0));
                if (!isImageVisible(imagesSelectedBeforeInCurrent0)) {
                    gotToImage(imagesSelectedBeforeInCurrent0);
                }
                for (int i = 0; i < imagesSelectedBefore.size(); i++) {
                    std::string extension = data->getImagesDataPtr()->get()->at(imagesSelectedBefore.at(i))->getImageExtension();
                    data->rotateLeft(imagesSelectedBefore.at(i), extension, [this]() {}, false);
                }
                reload();
            },
            [this, imagesSelectedBefore]() {
                int imagesSelectedBeforeInCurrent0 = data->getImagesDataPtr()->getImageNumberInCurrent(imagesSelectedBefore.at(0));
                if (!isImageVisible(imagesSelectedBeforeInCurrent0)) {
                    gotToImage(imagesSelectedBeforeInCurrent0);
                }
                for (int i = 0; i < imagesSelectedBefore.size(); i++) {
                    std::string extension = data->getImagesDataPtr()->get()->at(imagesSelectedBefore.at(i))->getImageExtension();
                    data->rotateRight(imagesSelectedBefore.at(i), extension, [this]() {}, false);
                }
                reload();
            });
    });

    return imageRotateRightNew;
}

/**
 * @brief Create the rotate left image button
 * @return Pointer to the created ClickableLabel object
 */
ClickableLabel* ImageBooth::createImageRotateLeft() {
    if (data->getImagesDataPtr()->get()->size() <= 0) {
        return nullptr;
    }

    auto* imageRotateLeftNew = new ClickableLabel(data, Const::IconPath::ROTATE_LEFT, Text::Tooltip::ImageBooth::rotate_left(), this, actionSize);
    imageRotateLeftNew->setInitialBackground(Const::Color::TRANSPARENT1, Const::Color::LIGHT_GRAY);

    connect(imageRotateLeftNew, &ClickableLabel::clicked, [this]() {
        if (data->getImagesSelectedPtr()->empty()) {
            showInformationMessage(this, "No image selected", "You need to select an image to rotate it");
            return;
        }
        std::vector<int> imagesSelectedBefore = *data->getImagesSelectedPtr();
        for (int i = 0; i < data->getImagesSelectedPtr()->size(); i++) {
            std::string extension = data->getImagesDataPtr()->get()->at(data->getImagesSelectedPtr()->at(i))->getImageExtension();
            data->rotateLeft(data->getImagesSelectedPtr()->at(i), extension, [this]() {}, false);
        }
        data->getImagesSelectedPtr()->clear();
        reload();

        data->addAction(
            [this, imagesSelectedBefore]() {
                int imagesSelectedBeforeInCurrent0 = data->getImagesDataPtr()->getImageNumberInCurrent(imagesSelectedBefore.at(0));
                if (!isImageVisible(imagesSelectedBeforeInCurrent0)) {
                    gotToImage(imagesSelectedBeforeInCurrent0);
                }
                for (int i = 0; i < imagesSelectedBefore.size(); i++) {
                    std::string extension = data->getImagesDataPtr()->get()->at(imagesSelectedBefore.at(i))->getImageExtension();
                    data->rotateRight(imagesSelectedBefore.at(i), extension, [this]() {}, false);
                }
                reload();
            },
            [this, imagesSelectedBefore]() {
                int imagesSelectedBeforeInCurrent0 = data->getImagesDataPtr()->getImageNumberInCurrent(imagesSelectedBefore.at(0));
                if (!isImageVisible(imagesSelectedBeforeInCurrent0)) {
                    gotToImage(imagesSelectedBeforeInCurrent0);
                }
                for (int i = 0; i < imagesSelectedBefore.size(); i++) {
                    std::string extension = data->getImagesDataPtr()->get()->at(imagesSelectedBefore.at(i))->getImageExtension();
                    data->rotateLeft(imagesSelectedBefore.at(i), extension, [this]() {}, false);
                }
                reload();
            });
    });

    return imageRotateLeftNew;
}

/**
 * @brief Create the mirror up down image button
 * @return Pointer to the created ClickableLabel object
 */
ClickableLabel* ImageBooth::createImageMirrorUpDown() {
    if (data->getImagesDataPtr()->get()->size() <= 0) {
        return nullptr;
    }

    auto* imageMirrorUpDownNew = new ClickableLabel(data, Const::IconPath::MIRROR_UP_DOWN, Text::Tooltip::ImageBooth::mirror_up_down(), this, actionSize);
    imageMirrorUpDownNew->setInitialBackground(Const::Color::TRANSPARENT1, Const::Color::LIGHT_GRAY);

    connect(imageMirrorUpDownNew, &ClickableLabel::clicked, [this]() {
        if (data->getImagesSelectedPtr()->empty()) {
            showInformationMessage(this, "No image selected", "You need to select an image to mirror it");
            return;
        }
        std::vector<int> imagesSelectedBefore = *data->getImagesSelectedPtr();

        for (int i = 0; i < data->getImagesSelectedPtr()->size(); i++) {
            std::string extension = data->getImagesDataPtr()->get()->at(data->getImagesSelectedPtr()->at(i))->getImageExtension();
            data->mirrorUpDown(data->getImagesSelectedPtr()->at(i), extension, [this]() {}, false);
        }
        data->getImagesSelectedPtr()->clear();
        reload();

        data->addAction(
            [this, imagesSelectedBefore]() {
                int imagesSelectedBeforeInCurrent0 = data->getImagesDataPtr()->getImageNumberInCurrent(imagesSelectedBefore.at(0));
                if (!isImageVisible(imagesSelectedBeforeInCurrent0)) {
                    gotToImage(imagesSelectedBeforeInCurrent0);
                }
                for (int i = 0; i < imagesSelectedBefore.size(); i++) {
                    std::string extension = data->getImagesDataPtr()->get()->at(imagesSelectedBefore.at(i))->getImageExtension();
                    data->mirrorUpDown(imagesSelectedBefore.at(i), extension, [this]() {}, false);
                }
                reload();
            },
            [this, imagesSelectedBefore]() {
                int imagesSelectedBeforeInCurrent0 = data->getImagesDataPtr()->getImageNumberInCurrent(imagesSelectedBefore.at(0));
                if (!isImageVisible(imagesSelectedBeforeInCurrent0)) {
                    gotToImage(imagesSelectedBeforeInCurrent0);
                }
                for (int i = 0; i < imagesSelectedBefore.size(); i++) {
                    std::string extension = data->getImagesDataPtr()->get()->at(imagesSelectedBefore.at(i))->getImageExtension();
                    data->mirrorUpDown(imagesSelectedBefore.at(i), extension, [this]() {}, false);
                }
                reload();
            });
    });

    return imageMirrorUpDownNew;
}

/**
 * @brief Create the mirror left right image button
 * @return Pointer to the created ClickableLabel object
 */
ClickableLabel* ImageBooth::createImageMirrorLeftRight() {
    if (data->getImagesDataPtr()->get()->size() <= 0) {
        return nullptr;
    }

    auto* imageMirrorLeftRightNew = new ClickableLabel(data, Const::IconPath::MIRROR_LEFT_RIGHT, Text::Tooltip::ImageBooth::mirror_left_right(), this, actionSize);
    imageMirrorLeftRightNew->setInitialBackground(Const::Color::TRANSPARENT1, Const::Color::LIGHT_GRAY);

    connect(imageMirrorLeftRightNew, &ClickableLabel::clicked, [this]() {
        if (data->getImagesSelectedPtr()->empty()) {
            showInformationMessage(this, "No image selected", "You need to select an image to mirror it");
            return;
        }
        std::vector<int> imagesSelectedBefore = *data->getImagesSelectedPtr();
        for (int i = 0; i < data->getImagesSelectedPtr()->size(); i++) {
            std::string extension = data->getImagesDataPtr()->get()->at(data->getImagesSelectedPtr()->at(i))->getImageExtension();
            data->mirrorLeftRight(data->getImagesSelectedPtr()->at(i), extension, [this]() {}, false);
        }
        data->getImagesSelectedPtr()->clear();
        reload();

        data->addAction(
            [this, imagesSelectedBefore]() {
                int imagesSelectedBeforeInCurrent0 = data->getImagesDataPtr()->getImageNumberInCurrent(imagesSelectedBefore.at(0));
                if (!isImageVisible(imagesSelectedBeforeInCurrent0)) {
                    gotToImage(imagesSelectedBeforeInCurrent0);
                }
                for (int i = 0; i < imagesSelectedBefore.size(); i++) {
                    std::string extension = data->getImagesDataPtr()->get()->at(imagesSelectedBefore.at(i))->getImageExtension();
                    data->mirrorLeftRight(imagesSelectedBefore.at(i), extension, [this]() {}, false);
                }
                reload();
            },
            [this, imagesSelectedBefore]() {
                int imagesSelectedBeforeInCurrent0 = data->getImagesDataPtr()->getImageNumberInCurrent(imagesSelectedBefore.at(0));
                if (!isImageVisible(imagesSelectedBeforeInCurrent0)) {
                    gotToImage(imagesSelectedBeforeInCurrent0);
                }
                for (int i = 0; i < imagesSelectedBefore.size(); i++) {
                    std::string extension = data->getImagesDataPtr()->get()->at(imagesSelectedBefore.at(i))->getImageExtension();
                    data->mirrorLeftRight(imagesSelectedBefore.at(i), extension, [this]() {}, false);
                }
                reload();
            });
    });

    return imageMirrorLeftRightNew;
}

/**
 * @brief Create the edit exif image button
 * @return Pointer to the created ClickableLabel object
 */
ClickableLabel* ImageBooth::createImageEditExif() {
    if (data->getImagesDataPtr()->get()->size() <= 0) {
        return nullptr;
    }

    auto* imageEditExifNew = new ClickableLabel(data, Const::IconPath::MAP, Text::Tooltip::ImageBooth::map(), this, actionSize);
    imageEditExifNew->setInitialBackground(Const::Color::TRANSPARENT1, Const::Color::LIGHT_GRAY);

    connect(imageEditExifNew, &ClickableLabel::clicked, [this]() {
    });

    imageEditExifNew->setDisabled(true);

    return imageEditExifNew;
}

/**
 * @brief Create the image conversion button
 * @return Pointer to the created ClickableLabel object
 */
ClickableLabel* ImageBooth::createImageConversion() {
    if (data->getImagesDataPtr()->get()->size() <= 0) {
        return nullptr;
    }

    auto* imageConversionNew = new ClickableLabel(data, Const::IconPath::CONVERSION, Text::Tooltip::ImageBooth::conversion(), this, actionSize);
    imageConversionNew->setInitialBackground(Const::Color::TRANSPARENT1, Const::Color::LIGHT_GRAY);

    connect(imageConversionNew, &ClickableLabel::clicked, [this]() {
        if (data->getImagesSelectedPtr()->empty()) {
            showInformationMessage(this, "No image selected", "You need to select an image to convert it");
            return;
        }
        if (data->getImagesSelectedPtr()->size() > 0) {
            QString selectedFormat = launchConversionDialog();
            if (selectedFormat != nullptr) {
                QProgressDialog* progressDialog = new QProgressDialog(QString("Converting images : "), QString("Cancel"), 0, 3);
                progressDialog->setWindowModality(Qt::WindowModal);
                progressDialog->setAutoClose(false);
                progressDialog->show();

                for (int i = 0; i < data->getImagesSelectedPtr()->size(); i++) {
                    QString inputImagePath = QString::fromStdString(data->getImagesDataPtr()->getImageData(data->getImagesSelectedPtr()->at(i))->getImagePath());
                    if (progressDialog->wasCanceled()) {
                        break;
                    }
                    convertion(inputImagePath, selectedFormat, progressDialog);
                    progressDialog->setLabelText(QString("Converting image %1/%2").arg(i + 1).arg(data->getImagesSelectedPtr()->size()));
                }
                progressDialog->close();
                delete progressDialog;

                data->getImagesSelectedPtr()->clear();
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

/**
 * @brief Create the edit filters button
 * @return Pointer to the created ClickableLabel object
 */
ClickableLabel* ImageBooth::createEditFilters() {
    if (data->getImagesDataPtr()->get()->size() <= 0) {
        return nullptr;
    }

    auto* editFiltersNew = new ClickableLabel(data, Const::IconPath::EDIT_FILTERS, Text::Tooltip::ImageBooth::edit_filters(), this, actionSize);
    editFiltersNew->setInitialBackground(Const::Color::TRANSPARENT1, Const::Color::LIGHT_GRAY);
    int activeFiltersCount = 0;
    for (const auto& filter : data->getImagesDataPtr()->getFilters()) {
        if (filter.first != "image" && filter.first != "video") {
            if (filter.second) {
                activeFiltersCount++;
            }
        }
    }
    if (activeFiltersCount > 0) {
        editFiltersNew->addLogo(QColor::fromString(Const::Color::GRAY), QColor::fromString(Const::Color::DARK), activeFiltersCount);
    }

    connect(editFiltersNew, &ClickableLabel::clicked, [this]() {
        openFiltersPopup();
        switchToImageBooth();
    });

    return editFiltersNew;
}

/**
 * @brief Opens a popup to edit filters with checkboxes
 */
void ImageBooth::openFiltersPopup() {
    QDialog* dialog = new QDialog(this);
    dialog->setWindowTitle("Edit Filters");
    dialog->setModal(true);

    QVBoxLayout* mainLayout = new QVBoxLayout(dialog);

    QHBoxLayout* imageVideoLayout = new QHBoxLayout();
    QCheckBox* imageCheckbox = new QCheckBox("image", dialog);
    QCheckBox* videoCheckbox = new QCheckBox("video", dialog);

    std::map<std::string, bool> filters = data->getImagesDataPtr()->getFilters();
    imageCheckbox->setChecked(filters["image"]);
    videoCheckbox->setChecked(filters["video"]);

    imageVideoLayout->addWidget(imageCheckbox);
    imageVideoLayout->addWidget(videoCheckbox);
    mainLayout->addLayout(imageVideoLayout);

    QPushButton* toggleAllFiltersButton = new QPushButton("Enable/Disable All Filters", dialog);
    mainLayout->addWidget(toggleAllFiltersButton);

    QGridLayout* gridLayout = new QGridLayout();
    mainLayout->addLayout(gridLayout);

    QMap<QString, QCheckBox*> checkboxes;
    int row = 0, col = 0;
    const int maxColumns = 3;

    for (auto it = filters.begin(); it != filters.end(); ++it) {
        if (it->first == "image" || it->first == "video") {
            continue;
        }

        QCheckBox* checkbox = new QCheckBox(QString::fromStdString(it->first), dialog);
        checkbox->setChecked(it->second);
        gridLayout->addWidget(checkbox, row, col);
        checkboxes.insert(QString::fromStdString(it->first), checkbox);

        col++;
        if (col >= maxColumns) {
            col = 0;
            row++;
        }
    }

    connect(toggleAllFiltersButton, &QPushButton::clicked, [checkboxes, imageCheckbox, videoCheckbox]() {
        bool allChecked = false;

        for (auto checkbox : checkboxes) {
            allChecked = checkbox->isChecked();
        }

        bool newState = !allChecked;
        for (auto checkbox : checkboxes) {
            checkbox->setChecked(newState);
        }
    });

    QPushButton* validateButton = new QPushButton("Validate", dialog);
    mainLayout->addWidget(validateButton);

    connect(validateButton, &QPushButton::clicked, [this, dialog, checkboxes, imageCheckbox, videoCheckbox]() {
        std::map<std::string, bool> updatedFilters;

        updatedFilters["image"] = imageCheckbox->isChecked();
        updatedFilters["video"] = videoCheckbox->isChecked();

        for (auto it = checkboxes.begin(); it != checkboxes.end(); ++it) {
            updatedFilters[it.key().toStdString()] = it.value()->isChecked();
        }

        data->getImagesDataPtr()->setFilters(updatedFilters);
        dialog->accept();
    });

    dialog->setLayout(mainLayout);
    dialog->exec();
}

/**
 * @brief reload the images of the scroll area of imageBooth window
 */
void ImageBooth::reload() {
    // auto start = std::chrono::high_resolution_clock::now();
    updateImages();
    // auto end = std::chrono::high_resolution_clock::now();
    // auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    // qDebug() << "reload() execution time:" << duration << "ms";
}

/**
 * @brief Set the focus to imageBooth when entering the window
 * @param event QEnterEvent
 */
void ImageBooth::enterEvent(QEnterEvent* event) {
    this->setFocus();
    QMainWindow::enterEvent(event);
}

/**
 * @brief Give you the size of the currentFodler
 * @return the size of the current folders
 */
int ImageBooth::getCurrentFoldersSize() {
    if (data->getCurrentFolders()) {
        if (data->getCurrentFolders()->getFolders()) {
            return data->getCurrentFolders()->getFolders()->size() + 1;
        }
    }
    qCritical() << "getCurrentFoldersSize : Folder doesn't work";
    return 1;
}

/**
 * @brief Get the number of images in the current folder and its subfolders
 * @param currentFolder The current folder to count images in
 * @return The number of images in the current folder and its subfolders
 */
void ImageBooth::countImagesInFolder(Folders* currentFolder, int& totalImages) {
    auto* files = currentFolder->getFilesPtr();
    for (const auto& file : *files) {
        ImageData* imageData = data->getImagesDataPtr()->getImageData(file);
        if (imageData && imageData->respectFilters(data->getImagesDataPtr()->getFilters())) {
            totalImages++;
        }
    }

    const auto& subFolders = currentFolder->getFolders();
    for (auto it = subFolders->begin(); it != subFolders->end(); ++it) {
        countImagesInFolder(&(*it), totalImages);
    }
}
