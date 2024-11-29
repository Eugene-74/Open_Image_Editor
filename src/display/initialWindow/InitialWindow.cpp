#include "InitialWindow.h"


InitialWindow::InitialWindow() {
    // Supposons que vous ayez une instance de ImageBooth appelée imageBooth

    // TODO connexion a imageEditor pour les action des bouto,s
    // connect(imageBooth, &ImageBooth::changeToImageEditor, this, &InitialWindow::openImageEditor);

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

    // createImageBooth(data);

    createImageEditor(data);

    // std::cerr << "imagesData 1" << std::endl;
    // data.imagesData->print();


}

void InitialWindow::createImageEditor(Data* data) {
    ImageEditor* imageEditor = new ImageEditor(data, this);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(imageEditor);
    QWidget* centralWidget = new QWidget(this);
    centralWidget->setLayout(layout);
    setCentralWidget(centralWidget);
    imageEditor->setFocus();
}

void InitialWindow::createImageBooth(Data* data) {

    ImageBooth* imageBooth = new ImageBooth(data, this);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(imageBooth);
    QWidget* centralWidget = new QWidget(this);
    centralWidget->setLayout(layout);
    setCentralWidget(centralWidget);
    imageBooth->setFocus();
}

void InitialWindow::clearImageEditor() {
    // imageEditor->clear();
    // delete imageEditor;
}


void InitialWindow::openImageEditor() {
    // Code pour créer et afficher la fenêtre imageEditor
    createImageEditor(data);
}