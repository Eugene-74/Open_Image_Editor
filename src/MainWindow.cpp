#include "MainWindow.hpp"

#include <QHBoxLayout>

#include "ClickableLabel.hpp"
#include "Const.hpp"
#include "LoadImage.hpp"

/**
 * @brief Constructor for the MainWindow class
 * @param dat Pointer to the Data object containing application data
 * @param parent Pointer to the parent QWidget (usually the main window)
 * @details This constructor initializes the mainWindow of the application.
 */
MainWindow::MainWindow(std::shared_ptr<Data> dat, QWidget* parent)
    : QMainWindow(parent), data(dat) {
    parent->setWindowTitle(MAIN_WINDOW_WINDOW_NAME);

    QWidget* centralWidget = new QWidget(parent);
    setCentralWidget(centralWidget);
    mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setSpacing(data->sizes->imageEditorSizes->mainLayoutSpacing);
    mainLayout->setContentsMargins(data->sizes->imageEditorSizes->mainLayoutMargins[0],
                                   data->sizes->imageEditorSizes->mainLayoutMargins[1],
                                   data->sizes->imageEditorSizes->mainLayoutMargins[2],
                                   data->sizes->imageEditorSizes->mainLayoutMargins[3]);  // Marges autour des bords (gauche, haut, droite, bas)

    switchLayout = new QHBoxLayout();

    ClickableLabel* switchToBooth = new ClickableLabel(data, ICON_PATH_IMAGE_BOOTH, TOOL_TIP_OPEN_IMAGE_BOOTH, this, actionSize, false, 0, true);
    switchToBooth->setInitialBackground("transparent", "#b3b3b3");
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

    connect(addImages, &ClickableLabel::clicked, this, [this]() {
        addImagesFromFolder(data, this);
    });

    switchLayout->addWidget(switchToBooth);
    switchLayout->addWidget(addImages);

    switchLayout->setAlignment(Qt::AlignCenter);

    mainLayout->addLayout(switchLayout);
}

/**
 * @brief Set the focus to the mainWindow when the mouse enters
 * @param event Pointer to the enter event
 * @details It also calls the base class implementation of the enter event.
 */
void MainWindow::enterEvent(QEnterEvent* event) {
    this->setFocus();
    QMainWindow::enterEvent(event);
}