#include "Sizes.hpp"

#include <QApplication>
#include <QMainWindow>

void Sizes::update() {
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
    } else {
        linkButton = QSize(screenGeometry.height() * 1 / 20, screenGeometry.height() * 1 / 20);
    }

    imagesEditorSizes->update();
    imagesBoothSizes->update();
    mainWindowSizes->update();
}

void Sizes::ImagesEditorSizes::update() {
    if (parentSizes->screenGeometry.width() < parentSizes->screenGeometry.height()) {
        actionSize = QSize((parentSizes->screenGeometry.width() * 1 / 24) / parentSizes->pixelRatio, (parentSizes->screenGeometry.width() * 1 / 24) / parentSizes->pixelRatio);
    } else {
        actionSize = QSize((parentSizes->screenGeometry.height() * 1 / 24) / parentSizes->pixelRatio, (parentSizes->screenGeometry.height() * 1 / 24) / parentSizes->pixelRatio);
    }

    previewSize = (parentSizes->screenGeometry * 1 / 12);
    mainImageSize = (parentSizes->screenGeometry * 4 / 6);

    bigImage = parentSizes->screenGeometry - QSize(0, parentSizes->linkButton.height()) - QSize(mainLayoutMargins[0] + mainLayoutMargins[2], mainLayoutMargins[1] + mainLayoutMargins[3]) - QSize(mainLayoutSpacing * 0, mainLayoutSpacing * 2);
}

void Sizes::ImagesBoothSizes::update() {
    // int imagesPerLine = 10;
    if (parentSizes->screenGeometry.width() > parentSizes->screenGeometry.height()) {
        imageSize = QSize(parentSizes->screenGeometry.width() * 9 / 12 / imagesPerLine, parentSizes->screenGeometry.width() * 9 / 12 / imagesPerLine);

    } else {
        imageSize = QSize(parentSizes->screenGeometry.height() * 9 / 12 / imagesPerLine, parentSizes->screenGeometry.height() * 9 / 12 / imagesPerLine);
    }

    realImageSize = imageSize + QSize(linesLayoutSpacing, linesLayoutSpacing);

    scrollAreaSize = QSize(parentSizes->screenGeometry.width() * 5 / 6 + linesLayoutMargins[0] + linesLayoutMargins[2]             // marge gauche et droite
                               - (parentSizes->screenGeometry.width() * 5 / 6) % realImageSize.width() + linesLayoutSpacing + 10,  // +10 for the scroll bar

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

// Sizes::Sizes(Data* parent)
//     : data(parent) {
Sizes::Sizes() {
    imagesEditorSizes = new ImagesEditorSizes(this);
    imagesBoothSizes = new ImagesBoothSizes(this);
    mainWindowSizes = new MainWindowSizes(this);
    update();
}

void Sizes::ImagesBoothSizes::setimagesPerLine(int imagesPerLine) {
    this->imagesPerLine = imagesPerLine;
    while (imagesPerLine < 4) {
        imagesPerLine += 1;
    }
    update();
}

void Sizes::ImagesBoothSizes::changeimagesPerLine(int imagesPerLine) {
    setimagesPerLine(this->imagesPerLine + imagesPerLine);
}