#include "ImageBooth.h"

#include <QScrollBar>

ImageBooth::ImageBooth(Data* dat, QWidget* parent)
    : QMainWindow(parent), data(dat) {
    parent->setWindowTitle(IMAGE_BOOTH_WINDOW_NAME);

    imageNumber = 0;

    QWidget* centralWidget = new QWidget(parent);
    setCentralWidget(centralWidget);

    mainLayout = new QHBoxLayout(centralWidget);

    scrollArea = new QScrollArea(centralWidget);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFixedSize(data->sizes.imagesBoothSizes->scrollAreaSize);

    mainLayout->addWidget(scrollArea);

    QWidget* scrollWidget = new QWidget();
    linesLayout = new QVBoxLayout(scrollWidget);
    scrollArea->setWidget(scrollWidget);

    linesLayout->setAlignment(Qt::AlignTop);
    linesLayout->setSpacing(data->sizes.imagesBoothSizes->linesLayoutSpacing);
    linesLayout->setContentsMargins(
        data->sizes.imagesBoothSizes->linesLayoutMargins[0],   // gauche
        data->sizes.imagesBoothSizes->linesLayoutMargins[1],   // haut
        data->sizes.imagesBoothSizes->linesLayoutMargins[2],   // droite
        data->sizes.imagesBoothSizes->linesLayoutMargins[3]);  // bas
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0;
        i < data->sizes.imagesBoothSizes->heightImageNumber * LINE_LOADED; i++) {
        createLine();
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    std::cout << "Execution time: " << duration.count() << " seconds pour " <<
        data->sizes.imagesBoothSizes->heightImageNumber * LINE_LOADED * data->sizes.imagesBoothSizes->widthImageNumber << " images"
        << std::endl;

    connect(scrollArea->verticalScrollBar(), &QScrollBar::valueChanged,
        [this](int value) {
            if (value >= scrollArea->verticalScrollBar()->maximum() - 500) {
                createLine();
            }
        });
}

void ImageBooth::createLine() {
    QHBoxLayout* lineLayout = new QHBoxLayout();
    lineLayout->setAlignment(Qt::AlignLeft);

    linesLayout->addLayout(lineLayout);

    int nbr = data->sizes.imagesBoothSizes->widthImageNumber;

    for (int i = 0; i < nbr; i++) {
        if (imageNumber >= 0 && imageNumber < data->imagesData.get()->size()) {
            std::string imagePath = data->imagesData.get()->at(imageNumber).folders.name;
            data->imagesData.get()->at(imageNumber).loadData();
            lineLayout->addWidget(createImage(imagePath, imageNumber));
            imageNumber += 1;
        }
    }
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
            data->createAllThumbnail(imagePath, 512);

            if (IMAGE_BOOTH_IMAGE_QUALITY == 128) {
                data->unloadFromCache(data->getThumbnailPath(imagePath, 256));
                data->unloadFromCache(data->getThumbnailPath(imagePath, 512));
            } else if (IMAGE_BOOTH_IMAGE_QUALITY == 256) {
                data->unloadFromCache(data->getThumbnailPath(imagePath, 128));
                data->unloadFromCache(data->getThumbnailPath(imagePath, 512));
            } else if (IMAGE_BOOTH_IMAGE_QUALITY == 512) {
                data->unloadFromCache(data->getThumbnailPath(imagePath, 128));
                data->unloadFromCache(data->getThumbnailPath(imagePath, 256));
            }
            data->unloadFromCache(imagePath);

            QImage qImage = data->loadImage(this, imagePath, this->size(), true,
                IMAGE_BOOTH_IMAGE_QUALITY, true, true);

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

    connect(imageButton, &ClickableLabel::clicked, [this, nbr]() {
        data->imagesData.setImageNumber(nbr);
        switchToImageEditor();
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
    // stopAndDeleteTimers();
    QTimer::singleShot(100, this, [this]() {
        while (QLayoutItem* item = linesLayout->takeAt(0)) {
            if (QWidget* widget = item->widget()) {
                widget->deleteLater();
            }
            delete item;
        }
        delete scrollArea;
        scrollArea = nullptr;
        });
}


void ImageBooth::keyReleaseEvent(QKeyEvent* event) {

    switch (event->key()) {

    case Qt::Key_Escape:
        // switchToImageEditor();
        switchToMainWindow();
        break;

    default:
        QWidget::keyReleaseEvent(event);
    }
}
