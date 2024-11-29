#include "ImageBooth.h"


ImageBooth::ImageBooth(Data* dat, QWidget* parent) : QMainWindow(parent), data(dat) {

    screenGeometry = data->screenR.size() / data->pixelRatio;
    imageSize = QSize(screenGeometry.height() * 5 / 6 * 1 / 10, screenGeometry.height() * 5 / 6 * 1 / 10);

    imageNumber = 0;

    QWidget* centralWidget = new QWidget(parent);
    setCentralWidget(centralWidget);

    // Créer un layout vertical pour toute la fenêtre
    mainLayout = new QHBoxLayout(centralWidget);


    scrollArea = new QScrollArea(centralWidget);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFixedSize(screenGeometry.width() * 5 / 6, screenGeometry.height() * 5 / 6);
    mainLayout->addWidget(scrollArea);



    QWidget* scrollWidget = new QWidget();
    linesLayout = new QVBoxLayout(scrollWidget);
    linesLayout->setAlignment(Qt::AlignTop);

    scrollArea->setWidget(scrollWidget);

    createLine();
    createLine();
    createLine();
    createLine();
    createLine();
    createLine();
    createLine();
    createLine();
    createLine();
    createLine();
    createLine();
    createLine();





}

void ImageBooth::createLine(){
    QVBoxLayout* lineLayout = new QVBoxLayout();
    lineLayout->setAlignment(Qt::AlignLeft);

    linesLayout->addLayout(lineLayout);

    int nbr = screenGeometry.width() / imageSize.width();
    // int nbr = 100;

    for (int i = 0; i < nbr;i++){
        if (imageNumber >= 0 && imageNumber < data->imagesData.get()->size()){

            std::string imagePath = data->imagesData.get()->at(imageNumber).imagePath;
            // scrollArea->setWidget(createImage(imagePath));
            lineLayout->addWidget(createImage(imagePath));
            imageNumber += 1;
        }
    }
}

ClickableLabel* ImageBooth::createImage(std::string imagePath) {

    if (data->imagesData.get()->size() <= 0) {
        return nullptr;
    }

    ClickableLabel* imageButton = new ClickableLabel(data, QString::fromStdString(imagePath), this, imageSize, false, 128);
    // ClickableLabel* imageButton = new ClickableLabel(data, QString::fromStdString(imagePath), this, imageSize, false, 0);


    connect(imageButton, &ClickableLabel::clicked, [this]() {
        std::cerr << "fezfz" << std::endl;

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