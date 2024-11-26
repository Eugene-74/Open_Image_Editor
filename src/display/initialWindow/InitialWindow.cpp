#include "InitialWindow.h"


InitialWindow::InitialWindow() {
    Data* data = new Data();



    ImagesData imagesData(std::vector<ImageData>{});
    ImagesData deletedImagesData(std::vector<ImageData>{});
    data->imageCache = new std::map<std::string, QImageAndPath>();


    data->imagesData = imagesData;
    data->deletedImagesData = deletedImagesData;

    std::string path = "/home/eugene/Documents/photo trié";
    startLoadingImagesFromFolder(path, &data->imagesData);


    // TODO sauvegarde orientation marche pas 
        // data.imagesData = loadImagesData(IMAGESDATA_SAVE_DAT_PATH);
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
        data->imagesData = addSelectedFilesToFolders(this);
    }
    data->imagesData.setImageNumber(0);

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

void InitialWindow::clearImageEditor() {
    // imageEditor->clear();
    // delete imageEditor;
}
