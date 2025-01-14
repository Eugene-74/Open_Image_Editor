#include "Sizes.h"

Sizes::Sizes() {
    // Initialisation des membres
    screen = QGuiApplication::primaryScreen();
    screenR = screen->availableGeometry();
    pixelRatio = screen->devicePixelRatio();
    screenGeometry = screenR.size() / pixelRatio;
    linkButtons = QSize(screenGeometry.width() * 1 / 20 * 2, screenGeometry.height() * 1 / 20);


    // Création des instances des classes imbriquées en passant 'this'
    imagesEditorSizes = new ImagesEditorSizes(this);
    imagesBoothSizes = new ImagesBoothSizes(this);
}

Sizes::ImagesEditorSizes::ImagesEditorSizes(Sizes* parent) : parentSizes(parent) {
    // Utilisation de parentSizes pour accéder à screenGeometry

    if (parentSizes->screenGeometry.width() < parentSizes->screenGeometry.height()) {
        actionSize = QSize((parentSizes->screenGeometry.width() * 1 / 48) * parentSizes->pixelRatio, (parentSizes->screenGeometry.width() * 1 / 48) * parentSizes->pixelRatio);
    } else {
        actionSize = QSize((parentSizes->screenGeometry.height() * 1 / 48) * parentSizes->pixelRatio, (parentSizes->screenGeometry.height() * 1 / 48) * parentSizes->pixelRatio);
    }

    previewSize = (parentSizes->screenGeometry * 1 / 12);
    mainImageSize = (parentSizes->screenGeometry * 4 / 6);

    bigImage = parent->screenGeometry
        - parent->linkButtons
        - QSize(mainLayoutMargins[0] + mainLayoutMargins[2],
            mainLayoutMargins[1] + mainLayoutMargins[3])
        - QSize(mainLayoutSpacing * 0, mainLayoutSpacing * 2)
        // TODO pk 100 ???
        - QSize(0, 100);
}

Sizes::ImagesBoothSizes::ImagesBoothSizes(Sizes* parent) : parentSizes(parent) {



    if (parentSizes->screenGeometry.width() < parentSizes->screenGeometry.height()) {
        imageSize = QSize(parentSizes->screenGeometry.width() * 5 / 6 * 1 / 10, parentSizes->screenGeometry.width() * 5 / 6 * 1 / 10);

    } else {
        imageSize = QSize(parentSizes->screenGeometry.height() * 5 / 6 * 1 / 10, parentSizes->screenGeometry.height() * 5 / 6 * 1 / 10);
    }

    realImageSize = imageSize + QSize(linesLayoutSpacing, linesLayoutSpacing);

    // TODO calcul pas parfait +10 anormal probablement la barre pour faire défiler
    scrollAreaSize = QSize(parentSizes->screenGeometry.width() * 5 / 6 + linesLayoutMargins[0] + linesLayoutMargins[2] // marge gauche et droite
        - (parentSizes->screenGeometry.width() * 5 / 6) % realImageSize.width() + linesLayoutSpacing + 10,

        parentSizes->screenGeometry.height() * 5 / 6 + linesLayoutMargins[1] + linesLayoutMargins[3] // marge haut et bas
        - (parentSizes->screenGeometry.height() * 5 / 6) % realImageSize.height() + linesLayoutSpacing);


    widthImageNumber = scrollAreaSize.width() / realImageSize.width();
    heightImageNumber = scrollAreaSize.height() / realImageSize.height();
}