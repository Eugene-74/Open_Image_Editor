#include "InitialWindow.h"


InitialWindow::InitialWindow() {
    data = new Data();

    ImagesData imagesData(std::vector<ImageData>{});
    ImagesData deletedImagesData(std::vector<ImageData>{});
    data->imageCache = new std::map<std::string, QImageAndPath>();



    data->imagesData = imagesData;
    data->deletedImagesData = deletedImagesData;

    std::string path = "/home/eugene/Documents/photo trié";


    data->loadData();

    const QList<QScreen*> screens = QGuiApplication::screens();
    QScreen* screen = QGuiApplication::primaryScreen();
    screen = screens[0];


    QRect screenR = screen->availableGeometry();

    pixelRatio = screen->devicePixelRatio();

    screenGeometry = screenR.size() / pixelRatio;



    setWindowTitle("EasyImageEditor : Initial Window");


    while (data->imagesData.get()->size() <= 0) {
        std::cerr << "No images loaded" << std::endl;
        // demander un fichier a l'utilisateur si aucune images n'est chargé
        data->imagesData = addSelectedFilesToFolders(data, this);
        data->sortImagesData();

        data->saveData();

    }
    data->imagesData.setImageNumber(0);


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



    createImageBooth(data);
}

void InitialWindow::closeEvent(QCloseEvent* event) {
    if (showQuestionMessage(this, "Do you want to save before quit ?")) {
        data->saveData();
        event->accept();

    } else {
        event->accept();
    }
}

void InitialWindow::createImageEditor(Data* data) {
    if (imageEditor) {
        delete imageEditor;
    }

    imageEditor = new ImageEditor(data, this);

    layout->removeWidget(imageBooth);

    layout->addWidget(imageEditor);

    imageEditor->setFocus();

    connect(imageEditor, &ImageEditor::switchToImageBooth, this, &InitialWindow::showImageBooth);


}

void InitialWindow::createImageBooth(Data* data) {
    if (imageBooth) {
        delete imageBooth;
    }
    if (imageEditor) {
        layout->removeWidget(imageEditor);
    }
    imageBooth = new ImageBooth(data, this);



    layout->addWidget(imageBooth);

    imageBooth->setFocus();

    connect(imageBooth, &ImageBooth::switchToImageEditor, this, &InitialWindow::showImageEditor);

}

void InitialWindow::clearImageEditor() {
    imageEditor->clear();
}

void InitialWindow::clearImageBooth() {
    imageBooth->clear();
}


void InitialWindow::showImageEditor() {
    // data->cancelTasks();
    clearImageBooth();
    createImageEditor(data);
}

void InitialWindow::showImageBooth() {
    std::cerr << "showImageBooth" << std::endl;
    clearImageEditor();
    createImageBooth(data);
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