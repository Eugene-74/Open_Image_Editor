#include "Sizes.hpp"

#include <QMainWindow>
#include <QWidget>

void Sizes::update() {
    qDebug() << "Sizes::update";

    QWidget* activeWindow = QApplication::activeWindow();

    QMainWindow* mainWin = qobject_cast<QMainWindow*>(activeWindow);

    // Get the main active window to avoir progessBar
    if (!mainWin) {
        const auto topWidgets = QApplication::topLevelWidgets();
        for (QWidget* widget : topWidgets) {
            if ((mainWin = qobject_cast<QMainWindow*>(widget))) {
            }
        }
    }

    if (mainWin) {
        screenR = mainWin->geometry();
        qDebug() << "Utilisation de la géométrie de la fenêtre active:" << screenR;
    } else {
        screen = QGuiApplication::primaryScreen();
        if (screen) {
            screenR = screen->availableGeometry();
            qDebug() << "Aucune fenêtre active, utilisation de la géométrie de l'écran:" << screenR;
        } else {
            qDebug() << "Aucun écran disponible";
            screenR = QRect();
        }
    }
#ifdef _WIN32
    pixelRatio = 1;
#else
    pixelRatio = screen->devicePixelRatio();
#endif
    screenGeometry = screenR.size() / pixelRatio;
    if (screenGeometry.width() < screenGeometry.height()) {
        linkButton = QSize(screenGeometry.width() * 1 / 20, screenGeometry.width() * 1 / 20);
        // linkButtons = QSize(screenGeometry.width() * 1 / 20 * 2, screenGeometry.width() * 1 / 20);
    } else {
        linkButton = QSize(screenGeometry.height() * 1 / 20, screenGeometry.height() * 1 / 20);
        // linkButtons = QSize(screenGeometry.height() * 1 / 20 * 2, screenGeometry.height() * 1 / 20);
    }
    qDebug() << "Sizes::update done";

    imagesEditorSizes->update();
    qDebug() << "Sizes::update done1";

    imagesBoothSizes->update();
    qDebug() << "Sizes::update done2";

    mainWindowSizes->update();
    qDebug() << "Sizes::update done3";
}

void Sizes::ImagesEditorSizes::update() {
    qDebug() << "Sizes::ImagesEditorSizes::update";
    if (parentSizes->screenGeometry.width() < parentSizes->screenGeometry.height()) {
        actionSize = QSize((parentSizes->screenGeometry.width() * 1 / 24) / parentSizes->pixelRatio, (parentSizes->screenGeometry.width() * 1 / 24) / parentSizes->pixelRatio);
    } else {
        actionSize = QSize((parentSizes->screenGeometry.height() * 1 / 24) / parentSizes->pixelRatio, (parentSizes->screenGeometry.height() * 1 / 24) / parentSizes->pixelRatio);
    }
    qDebug() << "Sizes::ImagesEditorSizes::update 1";

    previewSize = (parentSizes->screenGeometry * 1 / 12);
    mainImageSize = (parentSizes->screenGeometry * 4 / 6);

    bigImage = parentSizes->screenGeometry - QSize(0, parentSizes->linkButton.height()) - QSize(mainLayoutMargins[0] + mainLayoutMargins[2], mainLayoutMargins[1] + mainLayoutMargins[3]) - QSize(mainLayoutSpacing * 0, mainLayoutSpacing * 2)
               // TODO pk 100 ???
               - QSize(0, 100);
}

void Sizes::ImagesBoothSizes::update() {
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

void Sizes::MainWindowSizes::update() {
    if (parentSizes->screenGeometry.width() < parentSizes->screenGeometry.height()) {
        actionSize = QSize((parentSizes->screenGeometry.width() * 1 / 3) / parentSizes->pixelRatio, (parentSizes->screenGeometry.width() * 1 / 3) / parentSizes->pixelRatio);
    } else {
        actionSize = QSize((parentSizes->screenGeometry.height() * 1 / 3) / parentSizes->pixelRatio, (parentSizes->screenGeometry.height() * 1 / 3) / parentSizes->pixelRatio);
    }
}

Sizes::Sizes() {
    qDebug() << "Sizes::Sizes";
    imagesEditorSizes = new ImagesEditorSizes(this);
    qDebug() << "Sizes::Sizes 1";
    imagesBoothSizes = new ImagesBoothSizes(this);
    mainWindowSizes = new MainWindowSizes(this);
    update();
}