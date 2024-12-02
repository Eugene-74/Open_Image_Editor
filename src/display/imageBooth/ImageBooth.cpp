#include "ImageBooth.h"
#include <QScrollBar>


ImageBooth::ImageBooth(Data* dat, QWidget* parent) : QMainWindow(parent), data(dat) {

    parent->setWindowTitle(IMAGE_BOOTH_WINDOW_NAME);
    // for (int i = 0; i < data->sizes.imagesBoothSizes->heightImageNumber * LINE_LOADED; i++){
        // imageOpenTimers.push_back(new QTimer(this));
    // }

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
            for (int i = 0; i < data->sizes.imagesBoothSizes->heightImageNumber; i++) {
                createLine();
            }
        }
        });

    // startImageOpenTimer();


}

void ImageBooth::createLine(){

    QHBoxLayout* lineLayout = new QHBoxLayout();
    lineLayout->setAlignment(Qt::AlignLeft);

    linesLayout->addLayout(lineLayout);

    int nbr = data->sizes.imagesBoothSizes->widthImageNumber;

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
    if (data->isInCache(imagePath)){
        imageButton = new ClickableLabel(data, QString::fromStdString(imagePath), this, imageSize, false, 0, true);
    }
    else if (data->isInCache(data->getThumbnailPath(imagePath, 128))){
        imageButton = new ClickableLabel(data, QString::fromStdString(imagePath), this, imageSize, false, 128, true);
    }
    else if (data->hasThumbnail(imagePath, 128)){
        imageButton = new ClickableLabel(data, QString::fromStdString(imagePath), this, imageSize, false, 128, true);
    }
    // else if (preload){
        // imageButton = new ClickableLabel(data, IMAGE_PATH_LOADING, this, imageSize, false, 128, true);
    // }


    connect(imageButton, &ClickableLabel::clicked, [this, nbr]() {
        data->imagesData.setImageNumber(nbr);
        switchToImageEditor();
        });

    // std::cerr << "timer " << imagePath << std::endl;
    // QTimer* timer = new QTimer(this);
    // connect(timer, &QTimer::timeout, this, [this, imagePath, imageButton, timer]() {
    //     data->loadInCache(data->getThumbnailPath(imagePath, 128));
    //     std::cerr << "load image " << imagePath << std::endl;

    //     QImage qImage = data->imageCache->at(data->getThumbnailPath(imagePath, 128)).image;
    //     QSize size = QSize(100, 100);
    //     imageButton->setPixmap(QPixmap::fromImage(qImage).scaled(size - QSize(5, 5), Qt::KeepAspectRatio, Qt::SmoothTransformation));

    //     // auto it = std::find(imageOpenTimers.begin(), imageOpenTimers.end(), this);
    //     // if (it != imageOpenTimers.end()) {
    //     //     imageOpenTimers.erase(it);
    //     // }
    //     timer->stop();
    //     timer->deleteLater();

    //     });
    // if (data->hasThumbnail(imagePath, 128)){
    //     timer->setInterval(TIME_BEFORE_LOAD_128 * (nbr + 1));
    // }
    // else {
    //     timer->setInterval(TIME_BEFORE_LOAD * (nbr + 1));
    // }
    // timer->start();

    // imageOpenTimers.push_back(timer);


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

// void ImageBooth::startImageOpenTimer() {
//     // int i = 0;
//     for (int i = 0; i < data->sizes.imagesBoothSizes->heightImageNumber * LINE_LOADED; i++){

//         QTimer* imageOpenTimer = imageOpenTimers[i];
//         // precharge les images des alentours
//         if (imageOpenTimers[i]) {
//             imageOpenTimers[i]->disconnect();
//             imageOpenTimers[i]->stop();
//             imageOpenTimers[i]->deleteLater();
//         }
//         imageOpenTimers[i] = new QTimer(this);

//         connect(imageOpenTimers[i], &QTimer::timeout, this, [this, i]() {
//             for (int j = 0; j < data->sizes.imagesBoothSizes->widthImageNumber; j++){
//                 data->loadInCache(data->imagesData.getImageData(i * data->sizes.imagesBoothSizes->widthImageNumber + j)->imagePath);
//                 std::cerr << "load image " << i * data->sizes.imagesBoothSizes->widthImageNumber + j << std::endl;
//                 imageNumber = 0;
//                 createLine(true);

//             }

//             imageOpenTimers[i]->stop();
//             imageOpenTimers[i]->deleteLater();
//             imageOpenTimers[i] = nullptr;
//             });

//         imageOpenTimers[i]->setInterval(TIME_BEFORE_PRE_LOAD_FULL_QUALITY * data->sizes.imagesBoothSizes->widthImageNumber * (i + 1));

//         imageOpenTimers[i]->start();
//     }
// }

