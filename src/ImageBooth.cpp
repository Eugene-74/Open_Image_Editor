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
ClickableLabel* ImageBooth::createImage(std::string imagePath, int nbrInCurrent) {
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
        data->createAllThumbnailsAsync(imagePath, [self, imagePath, nbrInCurrent](bool result) {
            if (self) {
                if (result) {
                    QHBoxLayout* lineLayout = nullptr;
                    ClickableLabel* lastImageButton = self->getClickableLabelIfExist(nbrInCurrent, lineLayout);
                    if (lastImageButton != nullptr) {
                        QMetaObject::invokeMethod(self, [lineLayout, lastImageButton, self, imagePath, nbrInCurrent]() {
                            ClickableLabel* newImageButton = self->createImage(imagePath, nbrInCurrent);
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

    int imageNumberInTotal = data->getImagesDataPtr()->getImageNumberInTotal(nbrInCurrent);
    if (data->isDeleted(imageNumberInTotal)) {
        imageButton->setOpacity(DELETE_IMAGE_OPACITY);
    }
    imageButton->setInitialBorder(Const::Color::TRANSPARENT1, Const::Color::LIGHT_GRAY);

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

    connect(imageButton, &ClickableLabel::leftClicked, [this, nbrInCurrent]() {
        data->getImagesDataPtr()->setImageNumber(nbrInCurrent);
        switchToImageEditor();
    });

    connect(imageButton, &ClickableLabel::ctrlLeftClicked, [this, nbrInCurrent, imageButton]() {
        int imageNumberInTotal = data->getImagesDataPtr()->getImageNumberInTotal(nbrInCurrent);
        auto it = std::find(data->getImagesSelectedPtr()->begin(), data->getImagesSelectedPtr()->end(), imageNumberInTotal);
        if (it != data->getImagesSelectedPtr()->end()) {
            imageButton->resetBorder();
            data->getImagesSelectedPtr()->erase(it);

            addActionWithDelay(
                [this, nbrInCurrent, imageNumberInTotal]() {
                    ClickableLabel* imageButton = getClickableLabelIfExist(nbrInCurrent);
                    if (imageButton != nullptr) {
                        imageButton->setBorder(COLOR_BACKGROUND_IMAGE_BOOTH_SELECTED, COLOR_BACKGROUND_HOVER_IMAGE_BOOTH_SELECTED);
                    }
                    addNbrToSelectedImages(imageNumberInTotal);
                },
                [this, nbrInCurrent, imageNumberInTotal]() {
                    ClickableLabel* imageButton = getClickableLabelIfExist(nbrInCurrent);
                    if (imageButton != nullptr) {
                        imageButton->resetBorder();
                    }
                    removeNbrToSelectedImages(imageNumberInTotal);
                },
                imageNumberInTotal);

        } else {
            imageButton->setBorder(COLOR_BACKGROUND_IMAGE_BOOTH_SELECTED, COLOR_BACKGROUND_HOVER_IMAGE_BOOTH_SELECTED);
            data->getImagesSelectedPtr()->push_back(imageNumberInTotal);

            addActionWithDelay(
                [this, nbrInCurrent, imageNumberInTotal]() {
                    ClickableLabel* imageButton = getClickableLabelIfExist(nbrInCurrent);
                    if (imageButton != nullptr) {
                        imageButton->resetBorder();
                    }
                    removeNbrToSelectedImages(imageNumberInTotal);
                },
                [this, nbrInCurrent, imageNumberInTotal]() {
                    ClickableLabel* imageButton = getClickableLabelIfExist(nbrInCurrent);
                    if (imageButton != nullptr) {
                        imageButton->setBorder(COLOR_BACKGROUND_IMAGE_BOOTH_SELECTED, COLOR_BACKGROUND_HOVER_IMAGE_BOOTH_SELECTED);
                    }
                    addNbrToSelectedImages(imageNumberInTotal);
                },
                imageNumberInTotal);
        }
    });

    connect(imageButton, &ClickableLabel::shiftLeftClicked, [this, nbrInCurrent, imageButton]() {
        // select all image beetwen the 2 nbr
        int imageNumberInTotal = data->getImagesDataPtr()->getImageNumberInTotal(nbrInCurrent);

        if (imageShiftSelected >= 0) {
            std::vector<int> modifiedNbr;
            std::vector<int> modifiedNbrInTotal;

            int imageShiftSelectedInCurrent = data->getImagesDataPtr()->getImageNumberInCurrent(imageShiftSelected);

            int start = std::min(imageShiftSelectedInCurrent, nbrInCurrent);
            int end = std::max(imageShiftSelectedInCurrent, nbrInCurrent);

            for (int i = start; i <= end; ++i) {
                int imageNumberInTotalBis = data->getImagesDataPtr()->getImageNumberInTotal(i);
                auto it = std::find(data->getImagesSelectedPtr()->begin(), data->getImagesSelectedPtr()->end(), imageNumberInTotalBis);
                if (it != data->getImagesSelectedPtr()->end()) {
                    if (!imageShiftSelectedSelect) {
                        data->getImagesSelectedPtr()->erase(it);
                        modifiedNbr.push_back(i);
                        modifiedNbrInTotal.push_back(imageNumberInTotalBis);
                    }
                } else {
                    if (imageShiftSelectedSelect) {
                        data->getImagesSelectedPtr()->push_back(imageNumberInTotalBis);
                        modifiedNbr.push_back(i);
                        modifiedNbrInTotal.push_back(imageNumberInTotalBis);
                    }
                }
            }
            reload();

            bool select = imageShiftSelectedSelect;

            addActionWithDelay(
                [this, nbrInCurrent, imageNumberInTotal, modifiedNbrInTotal, select]() {
                    for (auto nbr : modifiedNbrInTotal) {
                        if (select) {
                            removeNbrToSelectedImages(nbr);
                        } else {
                            addNbrToSelectedImages(nbr);
                        }
                    }
                },
                [this, nbrInCurrent, imageNumberInTotal, modifiedNbrInTotal, select]() {
                    qDebug() << "Adding or removing images from selection BIS";

                    for (auto nbr : modifiedNbrInTotal) {
                        if (select) {
                            addNbrToSelectedImages(nbr);
                        } else {
                            removeNbrToSelectedImages(nbr);
                        }
                    }
                },
                imageNumberInTotal);

            imageShiftSelected = -1;
        } else {
            // Select the first image

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
            int nbrInCurrent = (lineNbr - foldersLineNumber + i - 1) * data->getSizesPtr()->imagesBoothSizes->widthImageNumber + j;
            ClickableLabel* lastImageButton = qobject_cast<ClickableLabel*>(lineLayout->itemAt(j)->widget());
            if (nbrInCurrent >= data->getImagesDataPtr()->getCurrent()->size()) {
                lastImageButton->hide();
            } else {
                std::string imagePath;
                ClickableLabel* imageButton;
                if (data->getImagesDataPtr()->getImageDataInCurrent(nbrInCurrent)) {
                    imagePath = data->getImagesDataPtr()->getImageDataInCurrent(nbrInCurrent)->getImagePath();
                    // auto start = std::chrono::high_resolution_clock::now();
                    imageButton = createImage(imagePath, nbrInCurrent);
                    // auto end = std::chrono::high_resolution_clock::now();
                    // auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
                    // qDebug() << "onScroll() execution time:" << duration << "ms";
                } else {
                    imageButton = new ClickableLabel(data, Const::ImagePath::ERROR_PATH,
                                                     "", this, imageSize, false, 0, true);
                }

                lineLayout->replaceWidget(lastImageButton, imageButton);

                lastImageButton->deleteLater();

                int imageNumberInTotal = data->getImagesDataPtr()->getImageNumberInTotal(nbrInCurrent);

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
            // TODO add action
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
        case Qt::Key_A:
            if (event->modifiers() & Qt::ControlModifier) {
                if (data->getImagesDataPtr()->getCurrent()->size() <= 0) {
                    return;
                }
                std::vector<int> imagesSelectedBefore = *data->getImagesSelectedPtr();
                for (size_t i = 0; i < data->getImagesDataPtr()->getCurrent()->size(); ++i) {
                    int imageNumberInTotal = data->getImagesDataPtr()->getImageNumberInTotal(i);
                    if (std::find(data->getImagesSelectedPtr()->begin(), data->getImagesSelectedPtr()->end(), imageNumberInTotal) == data->getImagesSelectedPtr()->end()) {
                        data->getImagesSelectedPtr()->push_back(imageNumberInTotal);
                    }
                }
                reload();

                int nbrInTotal = data->getImagesDataPtr()->getImageDataId(data->getImagesDataPtr()->getCurrent()->at(0)->getImagePath());

                addActionWithDelay(
                    [this, imagesSelectedBefore]() {
                        data->getImagesSelectedPtr()->clear();
                        *data->getImagesSelectedPtr() = imagesSelectedBefore;
                    },
                    [this]() {
                        for (size_t i = 0; i < data->getImagesDataPtr()->getCurrent()->size(); ++i) {
                            int imageNumberInTotal = data->getImagesDataPtr()->getImageNumberInTotal(i);
                            if (std::find(data->getImagesSelectedPtr()->begin(), data->getImagesSelectedPtr()->end(), imageNumberInTotal) == data->getImagesSelectedPtr()->end()) {
                                data->getImagesSelectedPtr()->push_back(imageNumberInTotal);
                            }
                        }
                    },
                    nbrInTotal);
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

    imageEditMap = createImageEditMap();

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
    actionButtonLayout->addWidget(imageEditMap);
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
        int imageNumberInTotal = imagesSelectedBefore.at(0);
        addActionWithDelay(
            [this, imagesSelectedBefore, savedBefore]() {
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
            },
            [this, imagesSelectedBefore]() {
                for (int i = 0; i < imagesSelectedBefore.size(); i++) {
                    if (data->isDeleted(imagesSelectedBefore.at(i))) {
                        data->unPreDeleteImage(imagesSelectedBefore.at(i));
                    } else {
                        data->preDeleteImage(imagesSelectedBefore.at(i));
                    }
                }
                data->setSaved(false);
            },
            imageNumberInTotal);
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
        this->exportImage();
    });

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
            ImageData* imageData = data->getImagesDataPtr()->getImageData(imagesSelectedBefore.at(i));
            imageData->clearDetectedObjects();
            imageData->rotate(Const::Rotation::RIGHT);
        }
        data->getImagesSelectedPtr()->clear();
        reload();


        int imageNumberInTotal = imagesSelectedBefore.at(0);
        addActionWithDelay(
            [this, imagesSelectedBefore]() {
                for (int i = 0; i < imagesSelectedBefore.size(); i++) {
                    ImageData* imageData = data->getImagesDataPtr()->getImageData(imagesSelectedBefore.at(i));
                    imageData->clearDetectedObjects();
                    imageData->rotate(Const::Rotation::LEFT);
                }
                *data->getImagesSelectedPtr() = imagesSelectedBefore;
            },
            [this, imagesSelectedBefore]() {
                for (int i = 0; i < imagesSelectedBefore.size(); i++) {
                    ImageData* imageData = data->getImagesDataPtr()->getImageData(imagesSelectedBefore.at(i));
                    imageData->clearDetectedObjects();
                    imageData->rotate(Const::Rotation::RIGHT);
                }
                *data->getImagesSelectedPtr() = imagesSelectedBefore;
            },
            imageNumberInTotal);
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
            ImageData* imageData = data->getImagesDataPtr()->getImageData(imagesSelectedBefore.at(i));
            imageData->clearDetectedObjects();
            imageData->rotate(Const::Rotation::LEFT);
        }
        data->getImagesSelectedPtr()->clear();
        reload();

        int imageNumberInTotal = imagesSelectedBefore.at(0);
        addActionWithDelay(
            [this, imagesSelectedBefore]() {
                for (int i = 0; i < imagesSelectedBefore.size(); i++) {
                    ImageData* imageData = data->getImagesDataPtr()->getImageData(imagesSelectedBefore.at(i));
                    imageData->clearDetectedObjects();
                    imageData->rotate(Const::Rotation::RIGHT);
                }
                *data->getImagesSelectedPtr() = imagesSelectedBefore;
            },
            [this, imagesSelectedBefore]() {
                for (int i = 0; i < imagesSelectedBefore.size(); i++) {
                    ImageData* imageData = data->getImagesDataPtr()->getImageData(imagesSelectedBefore.at(i));
                    imageData->clearDetectedObjects();
                    imageData->rotate(Const::Rotation::LEFT);
                }
                *data->getImagesSelectedPtr() = imagesSelectedBefore;
            },
            imageNumberInTotal);
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
            ImageData* imageData = data->getImagesDataPtr()->getImageData(imagesSelectedBefore.at(i));
            imageData->clearDetectedObjects();
            imageData->mirror(true);
        }
        data->getImagesSelectedPtr()->clear();
        reload();

        int imageNumberInTotal = imagesSelectedBefore.at(0);
        addActionWithDelay(
            [this, imagesSelectedBefore]() {
                for (int i = 0; i < imagesSelectedBefore.size(); i++) {
                    ImageData* imageData = data->getImagesDataPtr()->getImageData(imagesSelectedBefore.at(i));
                    imageData->clearDetectedObjects();
                    imageData->mirror(true);
                }
                *data->getImagesSelectedPtr() = imagesSelectedBefore;
            },
            [this, imagesSelectedBefore]() {
                for (int i = 0; i < imagesSelectedBefore.size(); i++) {
                    ImageData* imageData = data->getImagesDataPtr()->getImageData(imagesSelectedBefore.at(i));
                    imageData->clearDetectedObjects();
                    imageData->mirror(true);
                }
                *data->getImagesSelectedPtr() = imagesSelectedBefore;
            },
            imageNumberInTotal);
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
            ImageData* imageData = data->getImagesDataPtr()->getImageData(imagesSelectedBefore.at(i));
            imageData->clearDetectedObjects();
            imageData->mirror(false);
        }
        data->getImagesSelectedPtr()->clear();
        reload();

        int imageNumberInTotal = imagesSelectedBefore.at(0);
        addActionWithDelay(
            [this, imagesSelectedBefore]() {
                for (int i = 0; i < imagesSelectedBefore.size(); i++) {
                    ImageData* imageData = data->getImagesDataPtr()->getImageData(imagesSelectedBefore.at(i));
                    imageData->clearDetectedObjects();
                    imageData->mirror(false);
                }
                *data->getImagesSelectedPtr() = imagesSelectedBefore;
            },
            [this, imagesSelectedBefore]() {
                for (int i = 0; i < imagesSelectedBefore.size(); i++) {
                    ImageData* imageData = data->getImagesDataPtr()->getImageData(imagesSelectedBefore.at(i));
                    imageData->clearDetectedObjects();
                    imageData->mirror(false);
                }
                *data->getImagesSelectedPtr() = imagesSelectedBefore;
            },
            imageNumberInTotal);
    });

    return imageMirrorLeftRightNew;
}

/**
 * @brief Create the edit exif image button
 * @return Pointer to the created ClickableLabel object
 */
ClickableLabel* ImageBooth::createImageEditMap() {
    if (data->getImagesDataPtr()->get()->size() <= 0) {
        return nullptr;
    }

    auto* imageEditMapNew = new ClickableLabel(data, Const::IconPath::MAP, Text::Tooltip::ImageBooth::map(), this, actionSize);
    imageEditMapNew->setInitialBackground(Const::Color::TRANSPARENT1, Const::Color::LIGHT_GRAY);

    connect(imageEditMapNew, &ClickableLabel::clicked, [this]() {
    });

    imageEditMapNew->hide();

    return imageEditMapNew;
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
        std::vector<QString> lastSelectedFormat;
        for (int i = 0; i < data->getImagesSelectedPtr()->size(); i++) {
            lastSelectedFormat.push_back(QString::fromStdString(data->getImagesDataPtr()->getImageData(data->getImagesSelectedPtr()->at(i))->getImageExtension()));
        }

        QString newSelectedFormat;
        if (data->getImagesSelectedPtr()->size() > 0) {
            newSelectedFormat = launchConversionDialog();
            if (newSelectedFormat != nullptr) {
                for (int i = 0; i < data->getImagesSelectedPtr()->size(); i++) {
                    QString inputImagePath = QString::fromStdString(data->getImagesDataPtr()->getImageData(data->getImagesSelectedPtr()->at(i))->getImagePath());
                    data->getImagesDataPtr()->getImageData(i)->setExtension(newSelectedFormat.toStdString());
                }

                data->getImagesSelectedPtr()->clear();
                reload();
            }
        }
        std::vector<int> imagesSelectedBefore = *data->getImagesSelectedPtr();
        int imageNumberInTotal = imagesSelectedBefore.at(0);
        addActionWithDelay(
            [this, imagesSelectedBefore, lastSelectedFormat]() {
                for (int i = 0; i < data->getImagesSelectedPtr()->size(); i++) {
                    QString inputImagePath = QString::fromStdString(data->getImagesDataPtr()->getImageData(data->getImagesSelectedPtr()->at(i))->getImagePath());
                    data->getImagesDataPtr()->getImageData(i)->setExtension(lastSelectedFormat.at(i).toStdString());
                }
                *data->getImagesSelectedPtr() = imagesSelectedBefore;
            },
            [this, imagesSelectedBefore, newSelectedFormat]() {
                for (int i = 0; i < data->getImagesSelectedPtr()->size(); i++) {
                    QString inputImagePath = QString::fromStdString(data->getImagesDataPtr()->getImageData(data->getImagesSelectedPtr()->at(i))->getImagePath());
                    data->getImagesDataPtr()->getImageData(i)->setExtension(newSelectedFormat.toStdString());
                }
                *data->getImagesSelectedPtr() = imagesSelectedBefore;
            },
            imageNumberInTotal);
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

/**
 * @brief Add an action with a delay for undo and redo operations
 * @param unDo The function to execute for undo
 * @param reDo The function to execute for redo
 * @param nbrInTotal The total number of images in the data
 */
void ImageBooth::addActionWithDelay(std::function<void()> unDo, std::function<void()> reDo, int nbrInTotal) {
    int nbrInCurrent = data->getImagesDataPtr()->getImageNumberInCurrent(nbrInTotal);
    data->addAction(
        [this, nbrInCurrent, unDo, nbrInTotal]() {
            int time = 0;
            if (!isImageVisible(nbrInCurrent)) {
                gotToImage(nbrInCurrent);
                time = TIME_UNDO_VISUALISATION;
            }
            QTimer::singleShot(time, [this, unDo, nbrInTotal]() {
                unDo();
                reload();
            });
        },
        [this, nbrInCurrent, reDo]() {
            int time = 0;
            if (!isImageVisible(nbrInCurrent)) {
                gotToImage(nbrInCurrent);
                time = TIME_UNDO_VISUALISATION;
            }
            QTimer::singleShot(time, [this, reDo]() {
                reDo();
                reload();
            });
        });
}

/**
 * @brief Export the image to a specified path
 * @details This function opens a dialog to select the export path and whether to include the date in the image name.
 */
void ImageBooth::exportImage() {
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