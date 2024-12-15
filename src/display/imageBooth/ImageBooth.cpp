#include "ImageBooth.h"
#include <QScrollBar>


ImageBooth::ImageBooth(Data* dat, QWidget* parent) : QMainWindow(parent), data(dat) {

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
    linesLayout->setContentsMargins(data->sizes.imagesBoothSizes->linesLayoutMargins[0], // gauche
        data->sizes.imagesBoothSizes->linesLayoutMargins[1], // haut
        data->sizes.imagesBoothSizes->linesLayoutMargins[2], // droite 
        data->sizes.imagesBoothSizes->linesLayoutMargins[3]); // bas


    for (int i = 0; i < data->sizes.imagesBoothSizes->heightImageNumber * LINE_LOADED; i++) {
        createLine();
    }


    connect(scrollArea->verticalScrollBar(), &QScrollBar::valueChanged, [this](int value) {
        if (value >= scrollArea->verticalScrollBar()->maximum() - 500) {
            // for (int i = 0; i < data->sizes.imagesBoothSizes->heightImageNumber; i++) {
            createLine();
            // }
        }
        });



}

void ImageBooth::createLine(){

    QHBoxLayout* lineLayout = new QHBoxLayout();
    lineLayout->setAlignment(Qt::AlignLeft);

    linesLayout->addLayout(lineLayout);

    int nbr = data->sizes.imagesBoothSizes->widthImageNumber;
    // int loadedImageNumber = 0;


    for (int i = 0; i < nbr;i++){
        if (imageNumber >= 0 && imageNumber < data->imagesData.get()->size()){

            std::string imagePath = data->imagesData.get()->at(imageNumber).imagePath;
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
    if (data->isInCache(data->getThumbnailPath(imagePath, 128))){
        imageButton = new ClickableLabel(data, QString::fromStdString(imagePath), this, imageSize, false, 128, true);
        loadedImageNumber += 1;
    }
    else if (data->hasThumbnail(imagePath, 128)){
        imageButton = new ClickableLabel(data, QString::fromStdString(imagePath), this, imageSize, false, 128, true);
        loadedImageNumber += 1;
    }
    else{
        imageButton = new ClickableLabel(data, IMAGE_PATH_LOADING, this, imageSize, false, 0, true);

        data->loadInCacheAsync(imagePath, [this, imagePath, imageButton]() {
            data->createAllThumbnail(imagePath, 512);
            data->unloadFromCache(imagePath);
            data->unloadFromCache(data->getThumbnailPath(imagePath, 256));
            data->unloadFromCache(data->getThumbnailPath(imagePath, 512));

            QImage qImage = data->loadImage(this, imagePath, this->size(), true, 128, true, true);

            QMetaObject::invokeMethod(QApplication::instance(), [imageButton, qImage]() {
                imageButton->setPixmap(QPixmap::fromImage(qImage).scaled(imageButton->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
                }, Qt::QueuedConnection);

            loadedImageNumber += 1;
            });
    }


    connect(imageButton, &ClickableLabel::clicked, [this, nbr]() {
        data->imagesData.setImageNumber(nbr);
        switchToImageEditor();
        });



    return imageButton;
}

void ImageBooth::setImageNumber(int nbr){
    while (nbr < 0) {
        nbr += 1;
    }
    while (nbr >= data->imagesData.get()->size()) {
        nbr -= 1;
    }

    imageNumber = nbr;
}
void ImageBooth::clear(){
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

