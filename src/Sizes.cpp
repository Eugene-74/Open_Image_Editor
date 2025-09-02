#include "Sizes.hpp"

#include <QApplication>
#include <QMainWindow>

/**
 * @brief Update the sizes of the application
 */
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

    if (mainWin != nullptr) {
        screenR = mainWin->geometry();
    } else {
        screen = QGuiApplication::primaryScreen();
        if (screen != nullptr) {
            screenR = screen->availableGeometry();
        } else {
            qFatal() << "Aucun écran disponible";
            screenR = QRect();
        }
    }
#ifdef _WIN32
    pixelRatio = 1;
#else
    pixelRatio = screen->devicePixelRatio();
#endif
    screenGeometry = screenR.size() / pixelRatio;
    // qDebug() << "Screen geometry:" << screenGeometry;

    int minSize = std::min(screenGeometry.width(), screenGeometry.height());
    // qDebug() << "Max size:" << maxSize;
    linkButton = QSize(minSize * 1 / 48, minSize * 1 / 48);

    fontSize = (screenGeometry.height()) / 100;

    imageEditorSizes->update();
    imagesBoothSizes->update();
    mainWindowSizes->update();
}

/**
 * @brief Update the sizes of the imageEditor
 */
void Sizes::ImageEditorSizes::update() {
    int minSize = std::min(parentSizes->screenGeometry.width(), parentSizes->screenGeometry.height());

    actionSize = QSize((minSize * 1 / 32) / parentSizes->pixelRatio, (minSize * 1 / 32) / parentSizes->pixelRatio);

    float ratio = static_cast<float>(parentSizes->screenGeometry.width()) / parentSizes->screenGeometry.height();
    previewSize = QSize(minSize * ratio, minSize) * 1 / 12;
    mainImageSize = QSize(minSize * ratio, minSize) * 4 / 6;

    bigImage = parentSizes->screenGeometry - QSize(0, parentSizes->linkButton.height()) * 2 - QSize(mainLayoutMargins[0] + mainLayoutMargins[2], mainLayoutMargins[1] + mainLayoutMargins[3]) - QSize(mainLayoutSpacing * 0, mainLayoutSpacing * 2);
}

/**
 * @brief Update the sizes of the imageBooth
 */
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

/**
 * @brief Update the sizes of the mainWindow
 */
void Sizes::MainWindowSizes::update() {
    if (parentSizes->screenGeometry.width() < parentSizes->screenGeometry.height()) {
        actionSize = QSize((parentSizes->screenGeometry.width() * 1 / 3) / parentSizes->pixelRatio, (parentSizes->screenGeometry.width() * 1 / 3) / parentSizes->pixelRatio);
    } else {
        actionSize = QSize((parentSizes->screenGeometry.height() * 1 / 3) / parentSizes->pixelRatio, (parentSizes->screenGeometry.height() * 1 / 3) / parentSizes->pixelRatio);
    }
}

/**
 * @brief Constructor for the Sizes class
 */
Sizes::Sizes() {
    imageEditorSizes = std::make_unique<ImageEditorSizes>(this);
    imagesBoothSizes = std::make_unique<ImagesBoothSizes>(this);
    mainWindowSizes = std::make_unique<MainWindowSizes>(this);
    update();
}

/**
 * @brief Set the number of images per line in the imageBooth
 * @param imagesPerLine The number of images per line
 */
void Sizes::ImagesBoothSizes::setimagesPerLine(int imagesPerLine) {
    this->imagesPerLine = imagesPerLine;
    while (imagesPerLine < 4) {
        imagesPerLine += 1;
    }
    update();
}

/**
 * @brief Change the number of images per line in the imageBooth
 * @param imagesPerLine The number of images per line to change
 */
void Sizes::ImagesBoothSizes::changeimagesPerLine(int imagesPerLine) {
    setimagesPerLine(imagesPerLine);
}