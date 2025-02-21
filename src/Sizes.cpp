#include "Sizes.hpp"

Sizes::Sizes() {
    screen = QGuiApplication::primaryScreen();

    screenR = screen->availableGeometry();
#ifdef _WIN32
    pixelRatio = 1;
#else
    pixelRatio = screen->devicePixelRatio();
#endif
    screenGeometry = screenR.size() / pixelRatio;
    linkButtons = QSize(screenGeometry.width() * 1 / 20 * 2, screenGeometry.height() * 1 / 20);

    imagesEditorSizes = new ImagesEditorSizes(this);
    imagesBoothSizes = new ImagesBoothSizes(this);
}

Sizes::ImagesEditorSizes::ImagesEditorSizes(Sizes* parent)
    : parentSizes(parent) {
    if (parentSizes->screenGeometry.width() < parentSizes->screenGeometry.height()) {
        actionSize = QSize((parentSizes->screenGeometry.width() * 1 / 24) / parentSizes->pixelRatio, (parentSizes->screenGeometry.width() * 1 / 24) / parentSizes->pixelRatio);
    } else {
        actionSize = QSize((parentSizes->screenGeometry.height() * 1 / 24) / parentSizes->pixelRatio, (parentSizes->screenGeometry.height() * 1 / 24) / parentSizes->pixelRatio);
    }

    previewSize = (parentSizes->screenGeometry * 1 / 12);
    mainImageSize = (parentSizes->screenGeometry * 4 / 6);

    bigImage = parent->screenGeometry - parent->linkButtons - QSize(mainLayoutMargins[0] + mainLayoutMargins[2], mainLayoutMargins[1] + mainLayoutMargins[3]) - QSize(mainLayoutSpacing * 0, mainLayoutSpacing * 2)
               // TODO pk 100 ???
               - QSize(0, 100);
}

Sizes::ImagesBoothSizes::ImagesBoothSizes(Sizes* parent)
    : parentSizes(parent) {
    int imageNumber = 10;
    if (parentSizes->screenGeometry.width() > parentSizes->screenGeometry.height()) {
        imageSize = QSize(parentSizes->screenGeometry.width() * 9 / 12 / imageNumber, parentSizes->screenGeometry.width() * 9 / 12 / imageNumber);

    } else {
        imageSize = QSize(parentSizes->screenGeometry.height() * 9 / 12 / imageNumber, parentSizes->screenGeometry.height() * 9 / 12 / imageNumber);
    }

    realImageSize = imageSize + QSize(linesLayoutSpacing, linesLayoutSpacing);

    // TODO calcul pas parfait +10 anormal probablement la barre pour faire défiler (sans +10 l'appli crash X( ))
    scrollAreaSize = QSize(parentSizes->screenGeometry.width() * 5 / 6 + linesLayoutMargins[0] + linesLayoutMargins[2]  // marge gauche et droite
                               - (parentSizes->screenGeometry.width() * 5 / 6) % realImageSize.width() + linesLayoutSpacing + 10,

                           parentSizes->screenGeometry.height() * 9 / 12 + linesLayoutMargins[1] + linesLayoutMargins[3]  // marge haut et bas
                               - (parentSizes->screenGeometry.height() * 9 / 12) % realImageSize.height() + linesLayoutSpacing);

    widthImageNumber = scrollAreaSize.width() / realImageSize.width();
    heightImageNumber = scrollAreaSize.height() / realImageSize.height();
}