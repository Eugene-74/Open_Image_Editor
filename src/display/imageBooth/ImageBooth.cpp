#include "ImageBooth.h"

#include <QScrollBar>



ImageBooth::ImageBooth(Data* dat, QWidget* parent)
    : QMainWindow(parent), data(dat) {
    parent->setWindowTitle(IMAGE_BOOTH_WINDOW_NAME);
    imageNumber = 0;

    data->clearCache();


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

    scrollWidget->setMinimumHeight(data->sizes.imagesBoothSizes->realImageSize.height() * (data->imagesData.get()->size() / data->sizes.imagesBoothSizes->widthImageNumber) + 1);

    linesLayout->setAlignment(Qt::AlignTop);
    linesLayout->setSpacing(data->sizes.imagesBoothSizes->linesLayoutSpacing);
    linesLayout->setContentsMargins(
        data->sizes.imagesBoothSizes->linesLayoutMargins[0],   // gauche
        data->sizes.imagesBoothSizes->linesLayoutMargins[1],   // haut
        data->sizes.imagesBoothSizes->linesLayoutMargins[2],   // droite
        data->sizes.imagesBoothSizes->linesLayoutMargins[3]);  // bas

    spacer = new QSpacerItem(0, 0);
    linesLayout->insertSpacerItem(0, spacer);

    int index = 0;
    for (int i = 0; i < maxVisibleLines; ++i) {
        createLine();
    }

    int spacerHeight = scrollArea->verticalScrollBar()->value();
    int imageHeight = data->sizes.imagesBoothSizes->realImageSize.height();
    int lineNbr = spacerHeight / imageHeight;

    connect(scrollArea->verticalScrollBar(), &QScrollBar::valueChanged, this, &ImageBooth::onScroll);
}

void ImageBooth::updateVisibleImages() {
    QRect visibleRect = scrollArea->viewport()->rect();

    int spacerHeight = scrollArea->verticalScrollBar()->value();
    int imageHeight = data->sizes.imagesBoothSizes->realImageSize.height();
    spacerHeight = (spacerHeight / imageHeight) * imageHeight;

    int lineNbr = spacerHeight / imageHeight;
    int difLineNbr = lineNbr - lastLineNbr;

    if (difLineNbr == 0){
        return;
    }

    // TODO changer si on a plus que 1 page de changement !

    else if (difLineNbr > 0){
        // MET LE PREMIER EN DERNIER
        for (int i = 0;i < difLineNbr;i++){
            QHBoxLayout* firstLineLayout = qobject_cast<QHBoxLayout*>(linesLayout->takeAt(1)->layout());
            linesLayout->insertLayout(linesLayout->count(), firstLineLayout);
            linesLayout->invalidate();
            for (int j = 0;j < firstLineLayout->count();j++){
                int imageNbr = (lineNbr - difLineNbr + i + maxVisibleLines) * data->sizes.imagesBoothSizes->widthImageNumber + j;
                ClickableLabel* label = qobject_cast<ClickableLabel*>(firstLineLayout->itemAt(j)->widget());
                if (label) {
                    if (imageNbr >= 0 && imageNbr < data->imagesData.get()->size()) {
                        label->clear();
                        imageNumber = imageNbr;
                        ClickableLabel* newLabel = createImage(data->imagesData.get()->at(imageNbr).folders.name, imageNbr);
                        // ClickableLabel* newLabel = createImage(IMAGE_PATH_LOADING.toStdString(), imageNbr);

                        firstLineLayout->replaceWidget(label, newLabel);
                        delete label;
                    } else{
                        label->hide();
                    }
                }
            }

        }
    } else if (difLineNbr < 0){
        //  MET LE DERNIER EN PREMIER
        for (int i = 0;i < -difLineNbr;i++){
            QHBoxLayout* lastLineLayout = qobject_cast<QHBoxLayout*>(linesLayout->takeAt(linesLayout->count() - 1)->layout());
            linesLayout->insertLayout(1, lastLineLayout);
            linesLayout->invalidate();
            for (int j = 0;j < lastLineLayout->count();j++){
                int imageNbr = (lineNbr - difLineNbr + i) * data->sizes.imagesBoothSizes->widthImageNumber + j;
                ClickableLabel* label = qobject_cast<ClickableLabel*>(lastLineLayout->itemAt(j)->widget());
                if (label) {
                    if (imageNbr >= 0 && imageNbr < data->imagesData.get()->size()) {
                        label->clear();
                        imageNumber = imageNbr;

                        ClickableLabel* newLabel = createImage(data->imagesData.get()->at(imageNbr).folders.name, imageNbr);

                        lastLineLayout->replaceWidget(label, newLabel);
                        delete label;
                    } else{
                        label->hide();
                    }
                }
            }
        }
    }
    spacer->changeSize(0, spacerHeight);
    linesLayout->invalidate();
    lastLineNbr = lineNbr;

}

bool ImageBooth::isLineVisible(int lineIndex) {
    QRect visibleRect = scrollArea->viewport()->rect();
    int lineTop = lineIndex * data->sizes.imagesBoothSizes->realImageSize.height();
    int lineBottom = lineTop + data->sizes.imagesBoothSizes->realImageSize.height();

    return (lineBottom > visibleRect.top() + scrollArea->verticalScrollBar()->value() && lineTop < visibleRect.bottom() + scrollArea->verticalScrollBar()->value());
}


void ImageBooth::createLine() {
    QHBoxLayout* lineLayout = new QHBoxLayout();
    lineLayout->setAlignment(Qt::AlignLeft);

    linesLayout->addLayout(lineLayout);
    lineLayouts.push_back(lineLayout);
    int nbr = data->sizes.imagesBoothSizes->widthImageNumber;

    for (int i = 0; i < nbr; i++) {
        if (imageNumber >= 0 && imageNumber < data->imagesData.get()->size()) {
            std::string imagePath = data->imagesData.get()->at(imageNumber).folders.name;


            if (data->imagesData.get()->at(imageNumber).orientation == 0) {
                data->imagesData.get()->at(imageNumber).loadData();
            }
            lineLayout->addWidget(createImage(imagePath, imageNumber));

            imageNumber += 1;
        }
    }
}

// Slot to handle scrolling
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
            this, imageSize, false, IMAGE_BOOTH_IMAGE_QUALITY, true);
        loadedImageNumber += 1;
    } else if (data->hasThumbnail(imagePath, IMAGE_BOOTH_IMAGE_QUALITY)) {
        imageButton = new ClickableLabel(data, QString::fromStdString(imagePath),
            this, imageSize, false, IMAGE_BOOTH_IMAGE_QUALITY, true);
        loadedImageNumber += 1;
    } else {
        imageButton = new ClickableLabel(data, IMAGE_PATH_LOADING, this, imageSize,
            false, 0, true);

        data->loadInCacheAsync(imagePath, [this, imagePath, imageButton]() {
            done += 1;

            data->createAllThumbnailIfNotExists(imagePath, 512);

            data->unloadFromCache(imagePath);

            // TODO Remplis la memoire !!! fait 3 Mo alors que l'image fait 0,166 Mo
            QImage qImage = data->loadImage(this, imagePath, this->size(), true, IMAGE_BOOTH_IMAGE_QUALITY, true, true);

            QMetaObject::invokeMethod(
                QApplication::instance(),
                [imageButton, qImage]() {
                    imageButton->setPixmap(QPixmap::fromImage(qImage).scaled(
                        imageButton->size(), Qt::KeepAspectRatio,
                        Qt::SmoothTransformation));
                },
                Qt::QueuedConnection);

            loadedImageNumber += 1;
            });
    }

    if (std::find(data->imagesSelected.begin(), data->imagesSelected.end(), nbr) != data->imagesSelected.end()) {
        imageButton->select(COLOR_BACKGROUND_IMAGE_BOOTH_SELECTED, COLOR_BACKGROUND_HOVER_IMAGE_BOOTH_SELECTED);
    }

    if (nbr == imageShiftSelected) {
        if (imageShiftSelectedSelect){
            imageButton->select(COLOR_BACKGROUND_IMAGE_BOOTH_SELECTED_MULTIPLE_SELECT, COLOR_BACKGROUND_HOVER_IMAGE_BOOTH_SELECTED_MULTIPLE_SELECT);
        } else{
            imageButton->select(COLOR_BACKGROUND_IMAGE_BOOTH_SELECTED_MULTIPLE_UNSELECT, COLOR_BACKGROUND_HOVER_IMAGE_BOOTH_SELECTED_MULTIPLE_UNSELECT);
        }
    }

    connect(imageButton, &ClickableLabel::leftClicked, [this, nbr]() {
        data->imagesData.setImageNumber(nbr);
        switchToImageEditor();
        });

    connect(imageButton, &ClickableLabel::ctrlLeftClicked, [this, nbr, imageButton]() {
        auto it = std::find(data->imagesSelected.begin(), data->imagesSelected.end(), nbr);
        if (it != data->imagesSelected.end()) {
            imageButton->unSelect();
            data->imagesSelected.erase(it);
        } else {
            imageButton->select(COLOR_BACKGROUND_IMAGE_BOOTH_SELECTED, COLOR_BACKGROUND_HOVER_IMAGE_BOOTH_SELECTED);
            data->imagesSelected.push_back(nbr);
        }
        });

    connect(imageButton, &ClickableLabel::shiftLeftClicked, [this, nbr, imageButton]() {
        // selectione tout entre les 2 click
        if (imageShiftSelected >= 0){

            int start = std::min(imageShiftSelected, nbr);
            int end = std::max(imageShiftSelected, nbr);
            for (int i = start; i <= end; ++i) {
                ClickableLabel* label = qobject_cast<ClickableLabel*>(linesLayout->itemAt(i / data->sizes.imagesBoothSizes->widthImageNumber)->layout()->itemAt(i % data->sizes.imagesBoothSizes->widthImageNumber)->widget());
                if (label) {
                    if (imageShiftSelectedSelect){
                        label->select(COLOR_BACKGROUND_IMAGE_BOOTH_SELECTED, COLOR_BACKGROUND_HOVER_IMAGE_BOOTH_SELECTED);
                        data->imagesSelected.push_back(i);
                    } else{
                        label->unSelect();
                        data->imagesSelected.erase(std::remove(data->imagesSelected.begin(), data->imagesSelected.end(), i), data->imagesSelected.end());
                    }
                }
            }
            imageShiftSelected = -1;
        } else{
            auto it = std::find(data->imagesSelected.begin(), data->imagesSelected.end(), nbr);
            if (it != data->imagesSelected.end()){
                imageButton->select(COLOR_BACKGROUND_IMAGE_BOOTH_SELECTED_MULTIPLE_UNSELECT, COLOR_BACKGROUND_HOVER_IMAGE_BOOTH_SELECTED_MULTIPLE_UNSELECT);
                imageShiftSelectedSelect = false;
            } else{
                imageButton->select(COLOR_BACKGROUND_IMAGE_BOOTH_SELECTED_MULTIPLE_SELECT, COLOR_BACKGROUND_HOVER_IMAGE_BOOTH_SELECTED_MULTIPLE_SELECT);
                imageShiftSelectedSelect = true;
            }
            imageShiftSelected = nbr;
        }
        });
    return imageButton;
}

void ImageBooth::setImageNumber(int nbr) {
    while (nbr < 0) {
        nbr += 1;
    }
    while (nbr >= data->imagesData.get()->size()) {
        nbr -= 1;
    }

    imageNumber = nbr;
}

void ImageBooth::clear() {
    QTimer::singleShot(100, this, [this]() {
        while (QLayoutItem* item = linesLayout->takeAt(0)) {
            if (QWidget* widget = item->widget()) {
                widget->deleteLater();
            }
            delete item;
        }
        delete scrollArea;
        scrollArea = nullptr;

        while (QLayoutItem* item = actionButtonLayout->takeAt(0)) {
            if (QWidget* widget = item->widget()) {
                widget->deleteLater();
            }
            delete item;
        }
        actionButtonLayout->deleteLater();
        });
}

void ImageBooth::keyReleaseEvent(QKeyEvent* event) {

    switch (event->key()) {

    case Qt::Key_Escape:
        switchToMainWindow();
        break;

    case Qt::Key_I:
        // Debug
        for (const auto& cache : *data->imageCache) {
            std::cout << "Image: " << cache.first << " Size: " << static_cast<double>(cache.second.image.sizeInBytes()) / (1024 * 1024) << " MB" << std::endl;
        }
        break;

    default:
        QWidget::keyReleaseEvent(event);
    }
}


void ImageBooth::createButtons(){
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

    ClickableLabel* imageDelete = new ClickableLabel(data, ICON_PATH_DELETE, this, actionSize);

    connect(imageDelete, &ClickableLabel::clicked, [this]() {
        });

    if (data->imagesSelected.empty()){
        imageDelete->setDisabled(true);
    }

    return imageDelete;
}

ClickableLabel* ImageBooth::createImageSave() {

    if (data->imagesData.get()->size() <= 0) {
        return nullptr;
    }

    ClickableLabel* imageSaveNew = new ClickableLabel(data, ICON_PATH_SAVE, this, actionSize);

    connect(imageSaveNew, &ClickableLabel::clicked, [this]() {
        });

    return imageSaveNew;
}

ClickableLabel* ImageBooth::createImageExport() {

    if (data->imagesData.get()->size() <= 0) {
        return nullptr;
    }

    ClickableLabel* imageExportNew = new ClickableLabel(data, ICON_PATH_EXPORT, this, actionSize);

    connect(imageExportNew, &ClickableLabel::clicked, [this]() {
        });

    if (data->imagesSelected.empty()){
        imageExportNew->setDisabled(true);
    }

    return imageExportNew;
}

ClickableLabel* ImageBooth::createImageRotateRight() {

    if (data->imagesData.get()->size() <= 0) {
        return nullptr;
    }

    ClickableLabel* imageRotateRightNew = new ClickableLabel(data, ICON_PATH_ROTATE_RIGHT, this, actionSize);

    connect(imageRotateRightNew, &ClickableLabel::clicked, [this]() {
        });
    if (data->imagesSelected.empty()){
        imageRotateRightNew->setDisabled(true);
    }

    return imageRotateRightNew;
}


ClickableLabel* ImageBooth::createImageRotateLeft() {

    if (data->imagesData.get()->size() <= 0) {
        return nullptr;
    }

    ClickableLabel* imageRotateLeftNew = new ClickableLabel(data, ICON_PATH_ROTATE_LEFT, this, actionSize);

    connect(imageRotateLeftNew, &ClickableLabel::clicked, [this]() {
        });

    if (data->imagesSelected.empty()){
        imageRotateLeftNew->setDisabled(true);
    }
    return imageRotateLeftNew;
}

ClickableLabel* ImageBooth::createImageMirrorUpDown() {

    if (data->imagesData.get()->size() <= 0) {
        return nullptr;
    }

    ClickableLabel* imageMirrorUpDownNew = new ClickableLabel(data, ICON_PATH_MIRROR_UP_DOWN, this, actionSize);

    connect(imageMirrorUpDownNew, &ClickableLabel::clicked, [this]() {
        });

    if (data->imagesSelected.empty()){
        imageMirrorUpDownNew->setDisabled(true);
    }
    return imageMirrorUpDownNew;
}


ClickableLabel* ImageBooth::createImageMirrorLeftRight() {

    if (data->imagesData.get()->size() <= 0) {
        return nullptr;
    }

    ClickableLabel* imageMirrorLeftRightNew = new ClickableLabel(data, ICON_PATH_MIRROR_LEFT_RIGHT, this, actionSize);

    connect(imageMirrorLeftRightNew, &ClickableLabel::clicked, [this]() {
        });

    if (data->imagesSelected.empty()){
        imageMirrorLeftRightNew->setDisabled(true);
    }

    return imageMirrorLeftRightNew;
}




ClickableLabel* ImageBooth::createImageEditExif() {
    if (data->imagesData.get()->size() <= 0) {
        return nullptr;
    }

    ClickableLabel* imageEditExifNew = new ClickableLabel(data, ICON_PATH_EDIT_EXIF, this, actionSize);

    connect(imageEditExifNew, &ClickableLabel::clicked, [this]() {
        });

    imageEditExifNew->setDisabled(true);

    return imageEditExifNew;
}

ClickableLabel* ImageBooth::createImageConversion() {
    if (data->imagesData.get()->size() <= 0) {
        return nullptr;
    }

    ClickableLabel* imageConversionNew = new ClickableLabel(data, ICON_PATH_CONVERSION, this, actionSize);

    connect(imageConversionNew, &ClickableLabel::clicked, [this]() {
        });

    if (data->imagesSelected.empty()){
        imageConversionNew->setDisabled(true);
    }

    return imageConversionNew;
}