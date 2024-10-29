#include "InitialWindow.h"
#include <iostream>


InitialWindow::InitialWindow() {
    Data data;
    ImagesData imagesData(std::vector<ImageData>{});
    ImagesData deletedImagesData(std::vector<ImageData>{});
    using ImagesData = std::vector<ImageData*>;

    data.imagesData = imagesData;
    data.deletedImagesData = deletedImagesData;

    data.imagesData = loadImagesData(IMAGESDATA_SAVE_DAT_PATH);
    data.deletedImagesData = loadImagesData(DELETED_IMAGESDATA_SAVE_DAT_PATH);



    const QList<QScreen*> screens = QGuiApplication::screens();
    QScreen* screen = QGuiApplication::primaryScreen();
    screen = screens[0];


    QRect screenR = screen->availableGeometry();

    pixelRatio = screen->devicePixelRatio();

    screenGeometry = screenR.size() / pixelRatio;



    setWindowTitle("EasyImageEditor : Initial Window");


    while (data.imagesData.get().size() <= 0) {
        std::cerr << "No images loaded" << std::endl;
        // demander un fichier a l'utilisateur si aucune images n'est chargé
        data.imagesData = addSelectedFilesToFolders(this);
    }
    data.imagesData.setImageNumber(0);

    createImageEditor(data);

}

void InitialWindow::createImageEditor(Data& data) {
    ImageEditor* imageEditor = new ImageEditor(data, this);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(imageEditor);
    QWidget* centralWidget = new QWidget(this);
    centralWidget->setLayout(layout);
    setCentralWidget(centralWidget);
}

void InitialWindow::clearImageEditor() {
    // imageEditor->clear();
    // delete imageEditor;
}
