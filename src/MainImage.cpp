#include "MainImage.hpp"

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
    )")
                             .arg(border)
                             .arg(border_radius)
                             .arg(border_color)
                             .arg(hover_border_color)
                             .arg(background_color)
                             .arg(hover_background_color);
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
            if (!drawingRectangle) {
                emit ctrlLeftClicked();
            }
        } else {
            if (cropping) {
            } else {
                emit leftClicked();
                hover_background_color = QString::fromStdString(CLICK_BACKGROUND_COLOR);
                updateStyleSheet();
            }
        }
    }
}

void MainImage::mouseReleaseEvent(QMouseEvent* event) {
    if (!drawingRectangle) {
        emit clicked();
    }
    if (event->button() == Qt::LeftButton) {
        if (QApplication::keyboardModifiers() & Qt::ControlModifier) {
            if (!drawingRectangle) {
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
                    if (drawingRectangle) {
                        cropEnd = event->pos();

                        drawingRectangle = false;
                        cropImage();
                        cropping = false;
                    }
                }
            } else {
                if (!drawingRectangle) {
                    emit leftClicked();
                }
                hover_background_color = QString::fromStdString(HOVER_BACKGROUND_COLOR);
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

    QRect cropRect = QRect(cropStart, cropEnd).normalized();

    QSize scaledPixmapSize = qImage.size();
    scaledPixmapSize.scale(this->size(), Qt::KeepAspectRatio);

    int xOffset = (this->width() - scaledPixmapSize.width()) / 2;
    int yOffset = (this->height() - scaledPixmapSize.height()) / 2;

    cropRect.translate(-xOffset, -yOffset);

    QRect displayedImageRect(0, 0, scaledPixmapSize.width(), scaledPixmapSize.height());
    cropRect = cropRect.intersected(displayedImageRect);

    if (cropRect.isEmpty() || qImage.isNull()) {
        return;
    }

    double xScale = static_cast<double>(qImage.width()) / scaledPixmapSize.width();
    double yScale = static_cast<double>(qImage.height()) / scaledPixmapSize.height();

    QRect imageCropRect(
        static_cast<int>(cropRect.left() * xScale),
        static_cast<int>(cropRect.top() * yScale),
        static_cast<int>(cropRect.width() * xScale),
        static_cast<int>(cropRect.height() * yScale));

    if (!imageCropRect.isValid()) {
        return;
    }

    QImage croppedImage = qImage.copy(imageCropRect);

    qImage = croppedImage;
    this->setPixmap(QPixmap::fromImage(qImage).scaled(
        this->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));

    if (data && data->imagesData.getCurrentImageData()) {
        std::vector<QPoint> cropPoints = {
            QPoint(imageCropRect.left(), imageCropRect.top()),
            QPoint(imageCropRect.right(), imageCropRect.bottom())};
        int orientation = data->imagesData.getCurrentImageData()->orientation;

        std::vector<QPoint> adjustedCropPoints = adjustPointsForOrientation(cropPoints, orientation, qImageReel.size());

        data->imagesData.getCurrentImageData()->cropSizes.push_back(adjustedCropPoints);

        int nbr = data->imagesData.imageNumber;

        bool saved = data->saved;

        data->addAction(
            [this, nbr, saved]() {
                showErrorMessage(this, "Erreur : impossible de unDo (it's normal)");

                // int time = 0;
                // if (data->imagesData.imageNumber != nbr){
                //     data->imagesData.imageNumber = nbr;
                //     // parent->reload();
                //     time = TIME_UNDO_VISUALISATION;
                // }
                // QTimer::singleShot(time, [this, nbr, saved]() {
                //     if (saved){
                //         data->saved = true;
                //     }
                //     data->imagesData.getCurrentImageData()->cropSizes.pop_back();
                //     // parent->reload();

                //     });
            },
            [this, nbr, adjustedCropPoints]() {
                showErrorMessage(this, "Erreur : impossible de reDo (it's normal)");
                // int time = 0;
                // if (data->imagesData.imageNumber != nbr){
                //     data->imagesData.imageNumber = nbr;
                //     // TODO reload fait crash apres le premier ctrl + Z
                //     // parent->reload();
                //     time = TIME_UNDO_VISUALISATION;
                // }
                // QTimer::singleShot(time, [this, nbr, adjustedCropPoints]() {
                //     data->saved = false;
                //     data->imagesData.getCurrentImageData()->cropSizes.push_back(adjustedCropPoints);
                //     // parent->reload();

                //     });
            });

        imageCropted();
    } else {
        qDebug() << "Erreur : data ou getCurrentImageData() est nul";
    }
    data->saved = false;
}

void MainImage::paintEvent(QPaintEvent* event) {
    QLabel::paintEvent(event);

    QPainter painter(this);
    if (drawingRectangle) {
        painter.setPen(Qt::DashLine);
        painter.drawRect(QRect(cropStart, cropEnd));
    }
    if (personsEditor && !data->imagesData.getCurrentImageData()->persons.empty()) {
        painter.setPen(QPen(Qt::blue, 2));

        QSize scaledSize = qImage.size();
        scaledSize.scale(this->size(), Qt::KeepAspectRatio);

        double xScale = static_cast<double>(scaledSize.width()) / qImage.width();
        double yScale = static_cast<double>(scaledSize.height()) / qImage.height();

        int xOffset = (this->width() - scaledSize.width()) / 2;
        int yOffset = (this->height() - scaledSize.height()) / 2;

        for (const auto& person : data->imagesData.getCurrentImageData()->persons) {
            cv::Rect face = person.face;
            int x = static_cast<int>(face.x * xScale) + xOffset;
            int y = static_cast<int>(face.y * yScale) + yOffset;
            int width = static_cast<int>(face.width * xScale);
            int height = static_cast<int>(face.height * yScale);

            QRect rect(x, y, width, height);
            painter.drawRect(rect);

            QString name = QString::fromStdString(person.name);
            painter.drawText(x, y + height + 15, name);
        }
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
            case 2:
                adjustedPoint.setX(W - point.x());
                break;
            case 3:
                adjustedPoint.setX(W - point.x());
                adjustedPoint.setY(H - point.y());
                break;
            case 4:
                adjustedPoint.setY(H - point.y());
                break;
            case 5:
                adjustedPoint.setX(point.y());
                adjustedPoint.setY(point.x());
                break;
            case 6:
                adjustedPoint.setX(point.y());
                adjustedPoint.setY(W - point.x());
                break;
            case 7:
                adjustedPoint.setX(H - point.y() - 1);
                adjustedPoint.setY(W - point.x() - 1);
                break;
            case 8:
                adjustedPoint.setX(H - point.y() - 1);
                adjustedPoint.setY(point.x());
                break;
            default:
                break;
        }

        adjustedPoints.push_back(adjustedPoint);
    }

    return adjustedPoints;
}