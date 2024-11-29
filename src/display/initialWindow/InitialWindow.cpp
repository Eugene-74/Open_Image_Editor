#include "InitialWindow.h"


InitialWindow::InitialWindow() {
    // Supposons que vous ayez une instance de ImageBooth appelée imageBooth

    // TODO connexion a imageEditor pour les action des bouto,s
    // connect(imageBooth, &ImageBooth::changeToImageEditor, this, &InitialWindow::switchToImageBooth);

    data = new Data();

    ImagesData imagesData(std::vector<ImageData>{});
    ImagesData deletedImagesData(std::vector<ImageData>{});
    data->imageCache = new std::map<std::string, QImageAndPath>();



    data->imagesData = imagesData;
    data->deletedImagesData = deletedImagesData;

    data->screenR = data->screen->availableGeometry();
    data->pixelRatio = data->screen->devicePixelRatio();

    std::string path = "/home/eugene/Documents/photo trié";
    startLoadingImagesFromFolder(data, path, &data->imagesData);


    // TODO sauvegarde orientation marche pas 
    // data->imagesData = loadImagesData(IMAGESDATA_SAVE_DAT_PATH);
    // data->deletedImagesData = loadImagesData(DELETED_IMAGESDATA_SAVE_DAT_PATH);

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
    }
    data->imagesData.setImageNumber(0);


    // imageBooth = new ImageBooth(data, this);
    // imageEditor = new ImageEditor(data, this);

    // Set the initial central widget
    setCentralWidget(imageBooth);

    // Connect signals to slots
    centralWidget = new QWidget(this);
    layout = new QVBoxLayout;
    centralWidget->setLayout(layout);
    setCentralWidget(centralWidget);

    createImageBooth(data);
    // createImageEditor(data);
}

void InitialWindow::createImageEditor(Data* data) {
    if (imageEditor) {
        delete imageEditor;
    }

    // pose probleme
    imageEditor = new ImageEditor(data, this);

    layout->removeWidget(imageBooth);

    layout->addWidget(imageEditor);

    imageEditor->setFocus();
    // connect(imageEditor, &ImageEditor::switchToImageBooth, this, &InitialWindow::showImageBooth);

}

void InitialWindow::createImageBooth(Data* data) {
    if (imageBooth) {
        delete imageBooth;
    }


    imageBooth = new ImageBooth(data, this);

    layout->addWidget(imageBooth);


    imageBooth->setFocus();

    connect(imageBooth, &ImageBooth::switchToImageEditor, this, &InitialWindow::showImageEditor);

}

void InitialWindow::clearImageEditor() {
    imageEditor->clear();
    // delete imageEditor;
}

void InitialWindow::clearImageBooth() {
    imageBooth->clear();
    // delete imageBooth;
}


void InitialWindow::showImageEditor() {
    std::cerr << "showImageEditor" << std::endl;
    clearImageBooth();
    createImageEditor(data);
}

void InitialWindow::showImageBooth() {
    std::cerr << "showImageBooth" << std::endl;
    clearImageEditor();
    createImageBooth(data);
}