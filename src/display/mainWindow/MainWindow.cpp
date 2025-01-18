#include "MainWindow.h"

MainWindow::MainWindow(Data* dat, QWidget* parent) : QMainWindow(parent), data(dat) {
    parent->setWindowTitle(IMAGE_EDITOR_WINDOW_NAME);


    QWidget* centralWidget = new QWidget(parent);
    setCentralWidget(centralWidget);
    mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setSpacing(data->sizes.imagesEditorSizes->mainLayoutSpacing);
    mainLayout->setContentsMargins(data->sizes.imagesEditorSizes->mainLayoutMargins[0],
        data->sizes.imagesEditorSizes->mainLayoutMargins[1],
        data->sizes.imagesEditorSizes->mainLayoutMargins[2],
        data->sizes.imagesEditorSizes->mainLayoutMargins[3]); // Marges autour des bords (gauche, haut, droite, bas)

    switchLayout = new QHBoxLayout();

    ClickableLabel* switchToBooth = new ClickableLabel(data, ":/before.png", this, QSize(50, 50), false, 0, true);
    ClickableLabel* switchToEditor = new ClickableLabel(data, ":/next.png", this, QSize(50, 50), false, 0, true);

    connect(switchToBooth, &ClickableLabel::clicked, this, switchToImageBooth);
    connect(switchToEditor, &ClickableLabel::clicked, this, switchToImageEditor);

    switchLayout->addWidget(switchToBooth);
    switchLayout->addWidget(switchToEditor);
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


