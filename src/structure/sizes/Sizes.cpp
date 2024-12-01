#include "Sizes.h"

Sizes::Sizes() {
    // Initialisation des membres
    screen = QGuiApplication::primaryScreen();
    screenR = screen->availableGeometry();
    pixelRatio = screen->devicePixelRatio();
    screenGeometry = screenR.size() / pixelRatio;

    // Création des instances des classes imbriquées en passant 'this'
    imagesEditorSizes = new ImagesEditorSizes(this);
    imagesBoothSizes = new ImagesBoothSizes(this);
}

Sizes::ImagesEditorSizes::ImagesEditorSizes(Sizes* parent) : parentSizes(parent) {
    // Utilisation de parentSizes pour accéder à screenGeometry

    if (parentSizes->screenGeometry.width() < parentSizes->screenGeometry.height()) {
        actionSize = QSize((parentSizes->screenGeometry.width() * 1 / 48) * parentSizes->pixelRatio, (parentSizes->screenGeometry.width() * 1 / 48) * parentSizes->pixelRatio);
    }
    else {
        actionSize = QSize((parentSizes->screenGeometry.height() * 1 / 48) * parentSizes->pixelRatio, (parentSizes->screenGeometry.height() * 1 / 48) * parentSizes->pixelRatio);

    }

    previewSize = (parentSizes->screenGeometry * 1 / 12);
    mainImageSize = (parentSizes->screenGeometry * 4 / 6);
}

Sizes::ImagesBoothSizes::ImagesBoothSizes(Sizes* parent) : parentSizes(parent) {
    // Accès aux membres de Sizes si nécessaire
}