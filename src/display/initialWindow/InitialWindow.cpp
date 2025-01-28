#include "InitialWindow.h"



InitialWindow::InitialWindow() {

    data = new Data();

    ImagesData imagesData(std::vector<ImageData>{});
    ImagesData deletedImagesData(std::vector<ImageData>{});
    data->imageCache = new std::map<std::string, QImageAndPath>();

    data->imagesData = imagesData;
    data->deletedImagesData = deletedImagesData;

    data->loadData();

    const QList<QScreen*> screens = QGuiApplication::screens();
    QScreen* screen = QGuiApplication::primaryScreen();
    screen = screens[0];


    QRect screenR = screen->availableGeometry();

    pixelRatio = screen->devicePixelRatio();

    screenGeometry = screenR.size() / pixelRatio;

    setWindowTitle("EasyImageEditor : Initial Window");


    QVBoxLayout* windowLayout = new QVBoxLayout();

    centralWidget = new QWidget(this);
    centralWidget->setLayout(windowLayout);
    setCentralWidget(centralWidget);


    layout = new QVBoxLayout;
    windowLayout->addLayout(layout);


    QHBoxLayout* linkLayout = new QHBoxLayout();
    linkLayout->addWidget(createImageDiscord());
    linkLayout->addWidget(createImageGithub());
    linkLayout->addWidget(createImageOption());
    linkLayout->setAlignment(Qt::AlignRight);

    windowLayout->addLayout(linkLayout);

    createMainWindow(data);
}

void InitialWindow::closeEvent(QCloseEvent* event) {
    if (!data->saved){

        if (showQuestionMessage(this, "Do you want to save before quit ?")) {
            data->saveData();
        }
    }
    event->accept();
}

void InitialWindow::createImageEditor(Data* data) {

    imageEditor = new ImageEditor(data, this);

    layout->addWidget(imageEditor);

    imageEditor->setFocus();

    connect(imageEditor, &ImageEditor::switchToImageBooth, this, &InitialWindow::showImageBooth);
    connect(imageEditor, &ImageEditor::switchToMainWindow, this, &InitialWindow::showMainWindow);
}

void InitialWindow::createImageBooth(Data* data) {
    imageBooth = new ImageBooth(data, this);

    layout->addWidget(imageBooth);

    imageBooth->setFocus();

    connect(imageBooth, &ImageBooth::switchToImageEditor, this, &InitialWindow::showImageEditor);
    connect(imageBooth, &ImageBooth::switchToMainWindow, this, &InitialWindow::showMainWindow);
}



void InitialWindow::createMainWindow(Data* data) {
    std::cerr << "createMainWindow" << std::endl;
    mainWindow = new MainWindow(data, this);

    layout->addWidget(mainWindow);

    mainWindow->setFocus();

    connect(mainWindow, &MainWindow::switchToImageBooth, this, &InitialWindow::showImageBooth);
    connect(mainWindow, &MainWindow::switchToImageEditor, this, &InitialWindow::showImageEditor);

}
void InitialWindow::clearImageEditor() {
    layout->removeWidget(imageEditor);
    imageEditor->clear();
    // delete imageEditor;
    imageEditor = nullptr;
}

void InitialWindow::clearImageBooth() {
    layout->removeWidget(imageBooth);
    imageBooth->clear();
    // delete imageBooth;
    imageBooth = nullptr;
}

void InitialWindow::clearMainWindow() {
    layout->removeWidget(mainWindow);
    mainWindow->clear();
    // delete mainWindow;
    mainWindow = nullptr;
}



void InitialWindow::showImageEditor() {
    std::cerr << "showImageEditor" << std::endl;
    if (imageBooth != nullptr){
        clearImageBooth();
    }
    if (mainWindow != nullptr){
        clearMainWindow();
    }
    createImageEditor(data);
}

void InitialWindow::showImageBooth() {
    std::cerr << "showImageBooth" << std::endl;
    if (imageEditor != nullptr){
        clearImageEditor();
    }
    if (mainWindow != nullptr){
        clearMainWindow();
    }
    createImageBooth(data);
}

void InitialWindow::showMainWindow() {
    std::cerr << "showMainWindow" << std::endl;
    if (imageEditor != nullptr){
        clearImageEditor();
    }
    if (imageBooth != nullptr){
        clearImageBooth();
    }
    createMainWindow(data);
}




ClickableLabel* InitialWindow::createImageDiscord() {

    ClickableLabel* imageDiscord = new ClickableLabel(data, ":/discord.png", this, QSize(data->sizes.linkButtons.width() / 2, data->sizes.linkButtons.height()), false, 0, true);



    connect(imageDiscord, &ClickableLabel::clicked, [this]() {
        QDesktopServices::openUrl(QUrl("https://discord.gg/Q2HhZucmxU"));
        });


    return imageDiscord;
}



ClickableLabel* InitialWindow::createImageGithub() {

    ClickableLabel* imageGithub = new ClickableLabel(data, ":/github.png", this, QSize(data->sizes.linkButtons.width() / 2, data->sizes.linkButtons.height()), false, 0, true);



    connect(imageGithub, &ClickableLabel::clicked, [this]() {
        QDesktopServices::openUrl(QUrl("https://github.com/Eugene-74/Easy_Image_Editor"));
        });


    return imageGithub;
}
ClickableLabel* InitialWindow::createImageOption() {

    ClickableLabel* imageOption = new ClickableLabel(data, ":/option.png", this, QSize(50, 50), false, 0, true);



    connect(imageOption, &ClickableLabel::clicked, [this]() {
        openOption();
        });


    return imageOption;
}

void InitialWindow::openOption() {
    if (data->options.size() == 0) {
        data->options = DEFAULT_OPTIONS;
    }
    std::map<std::string, std::string> options = showOptionsDialog(this, "Options", DEFAULT_OPTIONS);

    for (const auto& [key, value] : options) {
        std::cerr << key << " : " << value << std::endl;
    }
}

