#include "MainImage.h"

MainImage::MainImage(Data* data, const QString& imagePath, QWidget* parent, QSize size, bool setSize, int thumbnail, bool square)
    : QLabel(parent), cropping(true) {

    qImage = data->loadImage(this, imagePath.toStdString(), size, setSize, thumbnail, true, square);

    if (!qImage.isNull()) {
        this->setPixmap(QPixmap::fromImage(qImage).scaled(size - QSize(5, 5), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        this->setText("Erreur");
    }

    if (setSize)
        setFixedSize(size);
    else {

        QSize scaledSize = qImage.size();
        scaledSize.scale(size, Qt::KeepAspectRatio);
        setFixedSize(scaledSize);
    }
    this->setAlignment(Qt::AlignCenter);

    setMouseTracking(true);

    updateStyleSheet();

}



void MainImage::updateStyleSheet() {
    QString styleSheet = QString(R"(
        QLabel {
            border: %1px solid %3;
            border-radius: %2px;
            background-color: %5;
        }
        QLabel:hover {
            border: %1px solid %4;
            border-radius: %2px;
            background-color: %6; 
        }
        QLabel:disabled {
            background-color: rgba(200, 200, 200, 1);

        }
    )").arg(border).arg(border_radius).arg(border_color).arg(hover_border_color).arg(background_color).arg(hover_background_color);
    this->setStyleSheet(styleSheet);
}

void MainImage::enterEvent(QEvent* event) {
    QLabel::enterEvent(event);
}

void MainImage::leaveEvent(QEvent* event) {
    QLabel::leaveEvent(event);
}

void MainImage::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        if (QApplication::keyboardModifiers() & Qt::ControlModifier) {
            emit ctrlLeftClicked();

        } else {
            if (cropping) {

            } else {
                emit leftClicked();

                hover_background_color = CLICK_BACKGROUND_COLOR;
                updateStyleSheet();
            }
        }
    }
}


void MainImage::mouseReleaseEvent(QMouseEvent* event) {
    emit clicked();
    if (event->button() == Qt::LeftButton) {
        if (QApplication::keyboardModifiers() & Qt::ControlModifier) {
            emit ctrlLeftClicked();

            std::cerr << "start cropping" << std::endl;
            cropping = true;
        } else {
            if (cropping) {
                if (cropStart == QPoint(-1, -1)) {
                    cropStart = event->pos();
                    drawingRectangle = true;
                } else {
                    cropEnd = event->pos();
                    drawingRectangle = false;
                    cropImage();

                }
            } else {
                emit leftClicked();

                hover_background_color = HOVER_BACKGROUND_COLOR;
                updateStyleSheet();
            }
        }
    }
    QLabel::mouseReleaseEvent(event);
}

void MainImage::cropImage() {
    if (cropStart == QPoint(-1, -1) || cropEnd == QPoint(-1, -1)) {
        return;
    }
    QRect cropRect = QRect(cropStart, cropEnd).normalized();

    // Calcul de la taille réelle de l'image affichée (pixmap)
    QSize scaledPixmapSize = qImage.size();
    scaledPixmapSize.scale(this->size() - QSize(5, 5), Qt::KeepAspectRatio);

    // Calcul des décalages dus à l'alignement de l'image dans le QLabel
    int xOffset = (this->width() - scaledPixmapSize.width()) / 2;
    int yOffset = (this->height() - scaledPixmapSize.height()) / 2;

    // Ajustement du rectangle de découpe en tenant compte des décalages
    cropRect.translate(-xOffset, -yOffset);

    // Vérification que le rectangle de découpe est dans les limites de l'image affichée
    QRect pixmapRect(0, 0, scaledPixmapSize.width(), scaledPixmapSize.height());
    cropRect = cropRect.intersected(pixmapRect);

    if (cropRect.isEmpty() || !qImage.isNull()) {
        // Conversion des coordonnées vers l'image originale
        double xRatio = static_cast<double>(qImage.width()) / scaledPixmapSize.width();
        double yRatio = static_cast<double>(qImage.height()) / scaledPixmapSize.height();

        QRect imageCropRect(
            static_cast<int>(cropRect.left() * xRatio),
            static_cast<int>(cropRect.top() * yRatio),
            static_cast<int>(cropRect.width() * xRatio),
            static_cast<int>(cropRect.height() * yRatio)
        );

        if (imageCropRect.isValid()) {
            QImage croppedImage = qImage.copy(imageCropRect.normalized());
            this->setPixmap(QPixmap::fromImage(croppedImage).scaled(this->size() - QSize(5, 5), Qt::KeepAspectRatio, Qt::SmoothTransformation));
            qImage = croppedImage;
        }
    }
}


void MainImage::paintEvent(QPaintEvent* event) {
    QLabel::paintEvent(event);

    if (drawingRectangle) {
        QPainter painter(this);
        painter.setPen(Qt::DashLine);
        painter.drawRect(QRect(cropStart, cropEnd));
    }

}

void MainImage::mouseMoveEvent(QMouseEvent* event) {
    if (drawingRectangle) {
        cropEnd = event->pos();
        update();
    }
}
