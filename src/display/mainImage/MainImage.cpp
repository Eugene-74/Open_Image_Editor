#include "MainImage.h"

MainImage::MainImage(Data* data, const QString& imagePath, ImageEditor* parent, QSize size, bool setSize, int thumbnail, bool square, bool force)
    : parent(parent), data(data), cropping(false), imagePath(imagePath), mSize(size), setSize(setSize), thumbnail(thumbnail), square(square), force(force) {

    qImage = data->loadImage(this, imagePath.toStdString(), mSize, setSize, thumbnail, true, square, true, force);

    if (!qImage.isNull()) {
        this->setPixmap(QPixmap::fromImage(qImage).scaled(mSize - QSize(5, 5), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        this->setText("Erreur");
    }

    if (setSize)
        setFixedSize(mSize);
    else {

        QSize scaledSize = qImage.size();
        scaledSize.scale(mSize, Qt::KeepAspectRatio);
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

void MainImage::enterEvent(QEnterEvent* event) {
    QLabel::enterEvent(event);
}

void MainImage::leaveEvent(QEvent* event) {
    QLabel::leaveEvent(event);
}

void MainImage::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        if (QApplication::keyboardModifiers() & Qt::ControlModifier) {
            if (!drawingRectangle){
                emit ctrlLeftClicked();
            }
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
    if (!drawingRectangle){
        emit clicked();
    }
    if (event->button() == Qt::LeftButton) {
        if (QApplication::keyboardModifiers() & Qt::ControlModifier) {
            if (!drawingRectangle){
                emit ctrlLeftClicked();
            }
            cropping = true;


            qImage = data->loadImage(this, imagePath.toStdString(), mSize, setSize, thumbnail, true, square, false, force);

            if (!qImage.isNull()) {
                this->setPixmap(QPixmap::fromImage(qImage).scaled(mSize - QSize(5, 5), Qt::KeepAspectRatio, Qt::SmoothTransformation));
            } else {
                this->setText("Erreur");
            }
            if (setSize)
                setFixedSize(mSize);
            else {

                QSize scaledSize = qImage.size();
                scaledSize.scale(mSize, Qt::KeepAspectRatio);
                setFixedSize(scaledSize);
            }
            this->setAlignment(Qt::AlignCenter);

        } else {
            if (cropping) {

                if (cropStart == QPoint(-1, -1)) {
                    cropStart = event->pos();

                    drawingRectangle = true;
                } else {
                    if (drawingRectangle){
                        cropEnd = event->pos();

                        drawingRectangle = false;
                        cropImage();
                        cropping = false;
                    }
                }
            } else {
                if (!drawingRectangle){
                    emit leftClicked();
                }
                hover_background_color = HOVER_BACKGROUND_COLOR;
                updateStyleSheet();
            }
        }
    }
    QLabel::mouseReleaseEvent(event);
}

void MainImage::cropImage() {
    QImage qImageReel = qImage.copy();
    if (cropStart == QPoint(-1, -1) || cropEnd == QPoint(-1, -1)) {
        return;
    }

    // Créer un rectangle à partir de cropStart et cropEnd
    QRect cropRect = QRect(cropStart, cropEnd).normalized();

    // Calculer la taille de l'image affichée (avec le redimensionnement)
    QSize scaledPixmapSize = qImage.size();
    scaledPixmapSize.scale(this->size(), Qt::KeepAspectRatio);

    // Calculer les décalages si l'image est centrée dans le widget
    int xOffset = (this->width() - scaledPixmapSize.width()) / 2;
    int yOffset = (this->height() - scaledPixmapSize.height()) / 2;

    // Ajuster cropRect pour qu'il soit relatif à l'image affichée
    cropRect.translate(-xOffset, -yOffset);

    // S'assurer que cropRect est dans les limites de l'image affichée
    QRect displayedImageRect(0, 0, scaledPixmapSize.width(), scaledPixmapSize.height());
    cropRect = cropRect.intersected(displayedImageRect);

    if (cropRect.isEmpty() || qImage.isNull()) {
        return;
    }

    // Mapper cropRect vers les coordonnées réelles de l'image
    double xScale = static_cast<double>(qImage.width()) / scaledPixmapSize.width();
    double yScale = static_cast<double>(qImage.height()) / scaledPixmapSize.height();

    QRect imageCropRect(
        static_cast<int>(cropRect.left() * xScale),
        static_cast<int>(cropRect.top() * yScale),
        static_cast<int>(cropRect.width() * xScale),
        static_cast<int>(cropRect.height() * yScale)
    );

    if (!imageCropRect.isValid()) {
        return;
    }

    // Découper l'image en utilisant le rectangle calculé
    QImage croppedImage = qImage.copy(imageCropRect);

    // Mettre à jour qImage et afficher l'image découpée
    qImage = croppedImage;
    this->setPixmap(QPixmap::fromImage(qImage).scaled(
        this->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation
    ));

    // Sauvegarder les coordonnées de découpe dans les coordonnées réelles de l'image
    if (data && data->imagesData.getCurrentImageData()) {
        std::vector<QPoint> cropPoints = {
            QPoint(imageCropRect.left(), imageCropRect.top()),
            QPoint(imageCropRect.right(), imageCropRect.bottom())
        };
        int orientation = data->imagesData.getCurrentImageData()->orientation;

        // int orientation = data->imagesData.getCurrentImageData()->getImageOrientation();
        std::vector<QPoint> adjustedCropPoints = adjustPointsForOrientation(cropPoints, orientation, qImageReel.size());

        data->imagesData.getCurrentImageData()->cropSizes.push_back(adjustedCropPoints);

        int nbr = data->imagesData.imageNumber;

        bool saved = data->saved;

        data->addAction([this, nbr, saved]() {
            if (saved){
                data->saved = true;
            }
            data->imagesData.getCurrentImageData()->cropSizes.pop_back();
            data->imagesData.imageNumber = nbr;
            // TODO reload marche pas bien
            // parent->reload();
            },
            [this, nbr, adjustedCropPoints]() {
                data->saved = false;
                data->imagesData.getCurrentImageData()->cropSizes.push_back(adjustedCropPoints);
                data->imagesData.imageNumber = nbr;
                // TODO reload marche pas bien
                // parent->reload();
            });

        imageCropted();
    } else {
        qDebug() << "Erreur : data ou getCurrentImageData() est nul";
    }
    data->saved = false;
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

std::vector<QPoint> MainImage::adjustPointsForOrientation(const std::vector<QPoint>& points, int orientation, QSize imageSize) {
    std::vector<QPoint> adjustedPoints;

    int W = imageSize.width();
    int H = imageSize.height();

    for (const QPoint& point : points) {
        QPoint adjustedPoint = point;

        switch (orientation) {
        case 2: // Miroir vertical
            adjustedPoint.setX(W - point.x());
            break;
        case 3: // Rotation de 180°
            adjustedPoint.setX(W - point.x());
            adjustedPoint.setY(H - point.y());
            break;
        case 4: // Miroir horizontal
            adjustedPoint.setY(H - point.y());
            break;
        case 5: // Miroir horizontal puis rotation de 270°
            adjustedPoint.setX(point.y());
            adjustedPoint.setY(point.x());
            break;
        case 6: // Rotation de 90 degrés (sens horaire)
            adjustedPoint.setX(point.y());
            adjustedPoint.setY(W - point.x());
            break;
        case 7: // Miroir horizontal puis rotation de 90°
            adjustedPoint.setX(H - point.y() - 1);
            adjustedPoint.setY(W - point.x() - 1);
            break;
        case 8: // Rotation de 270 degrés (sens horaire)
            adjustedPoint.setX(H - point.y() - 1);
            adjustedPoint.setY(point.x());
            break;
        default: // Orientation normale, pas de changement
            break;
        }

        adjustedPoints.push_back(adjustedPoint);
    }

    return adjustedPoints;
}