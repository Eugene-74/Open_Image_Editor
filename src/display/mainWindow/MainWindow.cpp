#include "MainWindow.h"

MainWindow::MainWindow(Data* dat, QWidget* parent) : QMainWindow(parent), data(dat) {
    parent->setWindowTitle(MAIN_WINDOW_WINDOW_NAME);


    QWidget* centralWidget = new QWidget(parent);
    setCentralWidget(centralWidget);
    mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setSpacing(data->sizes.imagesEditorSizes->mainLayoutSpacing);
    mainLayout->setContentsMargins(data->sizes.imagesEditorSizes->mainLayoutMargins[0],
        data->sizes.imagesEditorSizes->mainLayoutMargins[1],
        data->sizes.imagesEditorSizes->mainLayoutMargins[2],
        data->sizes.imagesEditorSizes->mainLayoutMargins[3]); // Marges autour des bords (gauche, haut, droite, bas)

    switchLayout = new QHBoxLayout();

    ClickableLabel* switchToBooth = new ClickableLabel(data, ":/imageBooth.png", this, QSize(300, 300), false, 0, true);
    ClickableLabel* switchToEditor = new ClickableLabel(data, ":/imageEditor.png", this, QSize(300, 300), false, 0, true);
    ClickableLabel* addImages = new ClickableLabel(data, ":/addImages.png", this, QSize(300, 300), false, 0, true);

    connect(switchToBooth, &ClickableLabel::clicked, this, [this](){
        if (data->imagesData.get()->size() <= 0){
            addImagesFromFolder(data, this);

        }
        if (data->imagesData.get()->size() > 0){
            switchToImageBooth();
        }
        });

    connect(switchToEditor, &ClickableLabel::clicked, this, [this](){
        if (data->imagesData.get()->size() <= 0){
            addImagesFromFolder(data, this);

        }
        if (data->imagesData.get()->size() > 0){

            switchToImageEditor();
        }
        });

    connect(addImages, &ClickableLabel::clicked, this, [this]() {
        addImagesFromFolder(data, this);

        });

    switchLayout->addWidget(switchToBooth);
    switchLayout->addWidget(switchToEditor);
    switchLayout->addWidget(addImages);

    switchLayout->setAlignment(Qt::AlignCenter);

    mainLayout->addLayout(switchLayout);


}

void MainWindow::clear(){
    QTimer::singleShot(100, this, [this]() {

        if (switchLayout) {
            QLayoutItem* item;
            while ((item = switchLayout->takeAt(0)) != nullptr) {
                if (item->widget()) {
                    item->widget()->disconnect();
                    item->widget()->hide();
                    item->widget()->deleteLater();
                }
                delete item;

            }
            delete switchLayout;
            switchLayout = nullptr;
        }
        });
}


