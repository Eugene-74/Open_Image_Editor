#include "ImageBooth.h"


ImageBooth::ImageBooth(Data* dat, QWidget* parent) : QMainWindow(parent), data(dat) {

    parent->setWindowTitle("EasyImageEditor : Image Booth Window");

    screenGeometry = data->screenR.size() / data->pixelRatio;
    imageSize = QSize(screenGeometry.height() * 5 / 6 * 1 / 10, screenGeometry.height() * 5 / 6 * 1 / 10);
    // ajout des marges
    int space = 1;
    int marge = 5;


    realImageSize = imageSize + QSize(space * 2, space * 2);

    imageNumber = 0;

    QWidget* centralWidget = new QWidget(parent);
    setCentralWidget(centralWidget);

    // Créer un layout vertical pour toute la fenêtre
    mainLayout = new QHBoxLayout(centralWidget);
    // std::cerr << "test" << std::endl;

    scrollArea = new QScrollArea(centralWidget);
    scrollArea->setWidgetResizable(true);

    scrollArea->setFixedSize(screenGeometry.width() * 5 / 6 + marge * 2 - (screenGeometry.width() * 5 / 6) % realImageSize.width() + space, screenGeometry.height() * 5 / 6 + marge * 2 - (screenGeometry.height() * 5 / 6) % realImageSize.height() + space);


    mainLayout->addWidget(scrollArea);


    QWidget* scrollWidget = new QWidget();
    linesLayout = new QVBoxLayout(scrollWidget);
    linesLayout->setAlignment(Qt::AlignTop);
    linesLayout->setSpacing(space);
    linesLayout->setContentsMargins(marge, marge, marge, marge);



    scrollArea->setWidget(scrollWidget);

    while (imageNumber < data->imagesData.get()->size()) {
        createLine();
    }




}

void ImageBooth::createLine(){
    // std::cerr << "createLine" << std::endl;

    QHBoxLayout* lineLayout = new QHBoxLayout();
    lineLayout->setAlignment(Qt::AlignLeft);
    // lineLayout->setContentsMargins(100, 2, 2, 2);


    linesLayout->addLayout(lineLayout);

    int nbr = scrollArea->geometry().width() / realImageSize.width();

    for (int i = 0; i < nbr;i++){
        if (imageNumber >= 0 && imageNumber < data->imagesData.get()->size()){

            std::string imagePath = data->imagesData.get()->at(imageNumber).imagePath;
            // scrollArea->setWidget(createImage(imagePath));
            lineLayout->addWidget(createImage(imagePath, imageNumber));
            imageNumber += 1;
        }
    }
}

ClickableLabel* ImageBooth::createImage(std::string imagePath, int nbr) {

    if (data->imagesData.get()->size() <= 0) {
        return nullptr;
    }
    // std::cerr << "create" << std::endl;
    ClickableLabel* imageButton = new ClickableLabel(data, QString::fromStdString(imagePath), this, imageSize, false, 128, true);
    // ClickableLabel* imageButton = new ClickableLabel(data, QString::fromStdString(imagePath), this, imageSize, false, 0);



    connect(imageButton, &ClickableLabel::clicked, [this, nbr]() {
        data->imagesData.setImageNumber(nbr - 1);


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