#include "MainImage.hpp"

#include <QApplication>
#include <QInputDialog>
#include <QMouseEvent>
#include <QPainter>

#include "Box.hpp"
#include "Const.hpp"
#include "Data.hpp"
#include "Verification.hpp"

/**
 * @brief Constructor for the MainImage class
 * @param data Pointer to the Data object containing application data
 * @param imagePath Path to the image file
 * @param size Size of the image to be displayed
 * @param setSize Set if the image should be resized
 * @param thumbnail Thumbnail size (0 for no thumbnail)
 * @param square Set if the image should be displayed as a square
 * @param force Force the image to be loaded even if it is already in the cache
 * @details This constructor initializes the MainImage widget with the specified image and size.
 */
MainImage::MainImage(std::shared_ptr<Data> data, const QString& imagePath, QSize size, bool setSize, bool personsEditor, bool square, bool force)
    : data(data), cropping(false), imagePath(imagePath), mSize(size), setSize(setSize), personsEditor(personsEditor), square(square), force(force) {
    if (data->isInCache(data->getThumbnailPath(imagePath.toStdString(), 0))) {
        if (isImage(imagePath.toStdString()) || isVideo(imagePath.toStdString())) {
            qImage = data->loadImage(this, imagePath.toStdString(), mSize, setSize, 0, true, square, true, force);
        } else {
            qImage = QImage();
        }
    } else {
        qImage = data->loadImage(this, imagePath.toStdString(), mSize, setSize, Const::Thumbnail::NORMAL_QUALITY, true, square, true, force);
    }

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

/**
 * @brief Update the style sheet of the MainImage widget
 */
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

/**
 * @brief Handle mouse press events
 * @param event Pointer to the mouse event
 * @details This function is called when the mouse is pressed on the widget. It emits signals based on the mouse button pressed and the state of the widget.
 */
void MainImage::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        if (QApplication::keyboardModifiers() & Qt::ControlModifier) {
            if (!drawingRectangle) {
                emit ctrlLeftClicked();
            }
        } else {
            if (cropping) {
            } else {
                if (inFace(event->pos())) {
                    qDebug() << "Clicked on a face";
                    // QTimer::singleShot(QApplication::doubleClickInterval(), this, [this]() {
                    //     emit leftClicked();
                    // });
                } else {
                    qDebug() << "Clicked on the image";
                    emit leftClicked();
                    hover_background_color = QString::fromStdString(CLICK_BACKGROUND_COLOR);
                    updateStyleSheet();
                }
            }
        }
    }
}

/**
 * @brief Handle mouse move events
 * @param event Pointer to the mouse event
 * @details This function is called when the mouse is moved over the widget. It updates the hover background color and style sheet.
 * @details It also handles the cropping rectangle drawing if the mouse is pressed and the cropping mode is enabled.
 */
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
                    if (!inFace(event->pos())) {
                        emit leftClicked();
                    }
                }
                hover_background_color = QString::fromStdString(HOVER_BACKGROUND_COLOR);
                updateStyleSheet();
            }
        }
    }
    QLabel::mouseReleaseEvent(event);
}

/**
 * @brief Crop the image based on the selected rectangle
 * @details This function crops the image based on the selected rectangle and updates the displayed image.
 */
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

    if (data && data->getImagesDataPtr()->getCurrentImageData()) {
        std::vector<QPoint> cropPoints = {
            QPoint(imageCropRect.left(), imageCropRect.top()),
            QPoint(imageCropRect.right(), imageCropRect.bottom())};
        int orientation = data->getImagesDataPtr()->getCurrentImageData()->getOrientation();

        std::vector<QPoint> adjustedCropPoints = adjustPointsForOrientation(cropPoints, orientation, qImageReel.size());

        std::vector<std::vector<QPoint>> cropSizes = data->getImagesDataPtr()->getCurrentImageData()->getCropSizes();
        cropSizes.push_back(adjustedCropPoints);
        data->getImagesDataPtr()->getCurrentImageData()->setCropSizes(cropSizes);

        int nbr = data->getImagesDataPtr()->getImageNumber();

        bool saved = data->getSaved();

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
                //         data->setSaved(true);
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
                //     data->setSaved(false);
                //     data->imagesData.getCurrentImageData()->cropSizes.push_back(adjustedCropPoints);
                //     // parent->reload();

                //     });
            });

        imageCropted();
    } else {
        qWarning() << "Erreur : data ou getCurrentImageData() est nul";
    }
    data->setSaved(false);
}

/**
 * @brief Paint event handler for the MainImage widget
 * @param event Pointer to the paint event
 * @details This function is called when the widget needs to be repainted. It draws the cropping rectangle.
 * @details It also draws detected objects if the personsEditor is enabled and there are detected objects in the image data.
 */
void MainImage::paintEvent(QPaintEvent* event) {
    QLabel::paintEvent(event);

    QPainter painter(this);
    if (drawingRectangle) {
        painter.setPen(Qt::DashLine);
        painter.drawRect(QRect(cropStart, cropEnd));
    }
    if (this->getPersonsEditorConst() && !data->getImagesDataPtr()->getCurrentImageData()->getDetectedObjects().empty()) {
        std::map<std::string, std::vector<std::pair<cv::Rect, float>>> detectedObjects = data->getImagesDataPtr()->getCurrentImageData()->getDetectedObjects();

        // Calculate the scale factors
        QSize scaledPixmapSize = qImage.size();
        scaledPixmapSize.scale(this->size(), Qt::KeepAspectRatio);
        double xScale = static_cast<double>(scaledPixmapSize.width()) / qImage.width();
        double yScale = static_cast<double>(scaledPixmapSize.height()) / qImage.height();

        // Calculate the offsets
        int xOffset = (this->width() - scaledPixmapSize.width()) / 2;
        int yOffset = (this->height() - scaledPixmapSize.height()) / 2;

        for (const auto& [key, value] : detectedObjects) {
            for (const auto& [rect, confidence] : value) {
                // Adjust the rectangle coordinates
                int adjustedX = static_cast<int>(rect.x * xScale) + xOffset;
                int adjustedY = static_cast<int>(rect.y * yScale) + yOffset;
                int adjustedWidth = static_cast<int>(rect.width * xScale);
                int adjustedHeight = static_cast<int>(rect.height * yScale);

                QRect qRect(adjustedX, adjustedY, adjustedWidth, adjustedHeight);

                if (key == "person") {
                    painter.setPen(Qt::blue);
                } else {
                    painter.setPen(Qt::red);
                }

                painter.drawRect(qRect);
                painter.drawText(qRect.topLeft(), QString::fromStdString(key) + " " + QString::number(confidence * 100, 'f', 2) + "%");
            }
        }
        auto detectedFaces = data->getImagesDataPtr()->getCurrentImageData()->getDetectedFacesPtr();
        const auto& personIdNames = data->getPersonIdNames();
        for (const auto& faceData : *detectedFaces) {
            cv::Rect rect = faceData.getFaceRect();
            float confidence = faceData.getConfidence();
            int personId = faceData.getPersonIdConst();

            // Adjust the rectangle coordinates
            int adjustedX = static_cast<int>(rect.x * xScale) + xOffset;
            int adjustedY = static_cast<int>(rect.y * yScale) + yOffset;
            int adjustedWidth = static_cast<int>(rect.width * xScale);
            int adjustedHeight = static_cast<int>(rect.height * yScale);

            QRect qRect(adjustedX, adjustedY, adjustedWidth, adjustedHeight);

            painter.setPen(Qt::green);
            painter.drawRect(qRect);

            QString label;
            if (personId != -1) {
                auto it = personIdNames.find(personId);
                if (it != personIdNames.end()) {
                    label = QString::fromStdString(it->second);
                } else {
                    label = QString("ID %1").arg(personId);
                }
            } else {
                label = QString("Face %1%").arg(confidence * 100, 0, 'f', 2);
            }
            painter.drawText(qRect.topLeft(), label);
        }
    }
}

/**
 * @brief Handle mouse move events for cropping
 * @param event Pointer to the mouse event
 */
void MainImage::mouseMoveEvent(QMouseEvent* event) {
    if (drawingRectangle) {
        cropEnd = event->pos();
        update();
    }
}

/**
 * @brief Adjust points for image orientation
 * @param points Vector of points to adjust
 * @param orientation Image orientation value (1-8)
 * @param imageSize Size of the image
 * @return Adjusted points based on the image orientation
 */
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

/**
 * @brief Get the personsEditor status
 * @return True if the personsEditor is enabled, false otherwise
 */
bool MainImage::getPersonsEditorConst() const {
    return this->personsEditor;
}

/**
 * @brief Set the personsEditor status
 * @param personsEditor True to enable the personsEditor, false to disable it
 */
void MainImage::setPersonsEditor(bool personsEditor) {
    this->personsEditor = personsEditor;
}

/**
 * @brief Handle double-click events
 * @param event Pointer to the mouse event
 * @details This function is called when the widget is double-clicked. It opens a dialog to enter the name of the detected face.
 */
void MainImage::mouseDoubleClickEvent(QMouseEvent* event) {
    if (!personsEditor) return;

    auto detectedFaces = data->getImagesDataPtr()->getCurrentImageData()->getDetectedFacesPtr();
    if (!detectedFaces) return;

    QSize scaledPixmapSize = qImage.size();
    scaledPixmapSize.scale(this->size(), Qt::KeepAspectRatio);
    double xScale = static_cast<double>(scaledPixmapSize.width()) / qImage.width();
    double yScale = static_cast<double>(scaledPixmapSize.height()) / qImage.height();
    int xOffset = (this->width() - scaledPixmapSize.width()) / 2;
    int yOffset = (this->height() - scaledPixmapSize.height()) / 2;

    for (auto& faceData : *detectedFaces) {
        cv::Rect rect = faceData.getFaceRect();
        int adjustedX = static_cast<int>(rect.x * xScale) + xOffset;
        int adjustedY = static_cast<int>(rect.y * yScale) + yOffset;
        int adjustedWidth = static_cast<int>(rect.width * xScale);
        int adjustedHeight = static_cast<int>(rect.height * yScale);
        QRect qRect(adjustedX, adjustedY, adjustedWidth, adjustedHeight);

        if (qRect.contains(event->pos())) {
            bool ok = false;
            QString currentName;
            int personId = faceData.getPersonIdConst();
            const auto& personIdNames = data->getPersonIdNames();
            auto it = personIdNames.find(personId);
            if (it != personIdNames.end()) {
                currentName = QString::fromStdString(it->second);
            }
            // TODO translate
            QString name = QInputDialog::getText(this, "Nom du visage", "Entrer le nom :", QLineEdit::Normal, currentName, &ok);
            if (ok && !name.isEmpty()) {
                auto* personIdNamesPtr = data->getPersonIdNamesPtr();
                (*personIdNamesPtr)[personId] = name.toStdString();
                update();
            }
            break;
        }
    }
}

bool MainImage::inFace(QPoint point) {
    bool clickedOnFace = false;
    if (personsEditor) {
        auto detectedFaces = data->getImagesDataPtr()->getCurrentImageData()->getDetectedFacesPtr();
        if (detectedFaces) {
            QSize scaledPixmapSize = qImage.size();
            scaledPixmapSize.scale(this->size(), Qt::KeepAspectRatio);
            double xScale = static_cast<double>(scaledPixmapSize.width()) / qImage.width();
            double yScale = static_cast<double>(scaledPixmapSize.height()) / qImage.height();
            int xOffset = (this->width() - scaledPixmapSize.width()) / 2;
            int yOffset = (this->height() - scaledPixmapSize.height()) / 2;
            for (const auto& faceData : *detectedFaces) {
                cv::Rect rect = faceData.getFaceRect();
                int adjustedX = static_cast<int>(rect.x * xScale) + xOffset;
                int adjustedY = static_cast<int>(rect.y * yScale) + yOffset;
                int adjustedWidth = static_cast<int>(rect.width * xScale);
                int adjustedHeight = static_cast<int>(rect.height * yScale);
                QRect qRect(adjustedX, adjustedY, adjustedWidth, adjustedHeight);
                if (qRect.contains(point)) {
                    clickedOnFace = true;
                    break;
                }
            }
        }
    }
    return clickedOnFace;
}