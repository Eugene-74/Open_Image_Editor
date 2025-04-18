#include "MainWindow.hpp"

#include <QHBoxLayout>

#include "ClickableLabel.hpp"
#include "Const.hpp"
#include "LoadImage.hpp"

MainWindow::MainWindow(Data* dat, QWidget* parent)
    : QMainWindow(parent), data(dat) {
    parent->setWindowTitle(MAIN_WINDOW_WINDOW_NAME);

    QWidget* centralWidget = new QWidget(parent);
    setCentralWidget(centralWidget);
    mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setSpacing(data->sizes->imagesEditorSizes->mainLayoutSpacing);
    mainLayout->setContentsMargins(data->sizes->imagesEditorSizes->mainLayoutMargins[0],
                                   data->sizes->imagesEditorSizes->mainLayoutMargins[1],
                                   data->sizes->imagesEditorSizes->mainLayoutMargins[2],
                                   data->sizes->imagesEditorSizes->mainLayoutMargins[3]);  // Marges autour des bords (gauche, haut, droite, bas)

    switchLayout = new QHBoxLayout();

    ClickableLabel* switchToBooth = new ClickableLabel(data, ICON_PATH_IMAGE_BOOTH, TOOL_TIP_OPEN_IMAGE_BOOTH, this, actionSize, false, 0, true);
    switchToBooth->setInitialBackground("transparent", "#b3b3b3");
    // ClickableLabel* switchToEditor = new ClickableLabel(data, ICON_PATH_IMAGE_EDITOR, TOOL_TIP_OPEN_IMAGE_EDITOR, this, actionSize, false, 0, true);
    // switchToEditor->setInitialBackground("transparent", "#b3b3b3");
    ClickableLabel* addImages = new ClickableLabel(data, ICON_PATH_OPTION_ADD_IMAGES, TOOL_TIP_ADD_IMAGES, this, actionSize, false, 0, true);
    addImages->setInitialBackground("transparent", "#b3b3b3");

    connect(switchToBooth, &ClickableLabel::clicked, this, [this]() {
        if (data->imagesData.get()->size() <= 0) {
            addImagesFromFolder(data, this);
        }
        if (data->imagesData.get()->size() > 0) {
            switchToImageBooth();
        }
    });

    // connect(switchToEditor, &ClickableLabel::clicked, this, [this]() {
    //     if (data->imagesData.get()->size() <= 0) {
    //         addImagesFromFolder(data, this);
    //     }
    //     if (data->imagesData.get()->size() > 0) {
    //         switchToImageEditor();
    //     }
    // });

    connect(addImages, &ClickableLabel::clicked, this, [this]() {
        addImagesFromFolder(data, this);
    });

    switchLayout->addWidget(switchToBooth);
    // switchLayout->addWidget(switchToEditor);
    switchLayout->addWidget(addImages);

    switchLayout->setAlignment(Qt::AlignCenter);

    mainLayout->addLayout(switchLayout);
}

void MainWindow::enterEvent(QEnterEvent* event) {
    this->setFocus();
    QMainWindow::enterEvent(event);
}