#include "MainImage.hpp"

#include <QApplication>
#include <QInputDialog>
#include <QMouseEvent>
#include <QPainter>

#include "Box.hpp"
#include "Const.hpp"
#include "Data.hpp"
#include "Text.hpp"
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
MainImage::MainImage(std::shared_ptr<Data> data, const QString& imagePath, QSize size, int thumbnail, bool personsEditor)
    : ImageLabel(data),
      data(data),
      cropping(false),
      imagePath(imagePath),
      personsEditor(personsEditor) {
    if (isImage(imagePath.toStdString()) || isVideo(imagePath.toStdString())) {
        qImage = data->loadImage(this, imagePath.toStdString(), size, false, thumbnail, true, false, true, false);

    } else {
        qImage = QImage();
    }

    if (!qImage.isNull()) {
        setPixmap(QPixmap::fromImage(qImage).scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        setCursor(Qt::PointingHandCursor);
    } else {
        setText("Error : null image");
    }

    QSize scaledSize = qImage.size();
    scaledSize.scale(size, Qt::KeepAspectRatio);
    setFixedSize(scaledSize);

    this->setAlignment(Qt::AlignCenter);

    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    setMouseTracking(true);

    updateStyleSheet();
}

/**
 * @brief Handle mouse press events
 * @param event Pointer to the mouse event
 * @details This function is called when the mouse is pressed on the widget. It emits signals based on the mouse button pressed and the state of the widget.
 */
void MainImage::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        if (personsEditor) {
            auto detectedFaces = data->getImagesDataPtr()->getCurrentImageData()->getDetectedFacesPtr();
            if (detectedFaces) {
                QSize scaledPixmapSize = qImage.size();
                scaledPixmapSize.scale(this->size(), Qt::KeepAspectRatio);
                double xScale = static_cast<double>(scaledPixmapSize.width()) / qImage.width();
                double yScale = static_cast<double>(scaledPixmapSize.height()) / qImage.height();
                int xOffset = (this->width() - scaledPixmapSize.width()) / 2;
                int yOffset = (this->height() - scaledPixmapSize.height()) / 2;
                int faceIndex = 0;
                int clickedFaceIndex = -1;
                for (auto& faceData : *detectedFaces) {
                    cv::Rect rect = faceData.getFaceRect();
                    int adjustedX = static_cast<int>(rect.x * xScale) + xOffset;
                    int adjustedY = static_cast<int>(rect.y * yScale) + yOffset;
                    int adjustedWidth = static_cast<int>(rect.width * xScale);
                    int adjustedHeight = static_cast<int>(rect.height * yScale);
                    QRect qRect(adjustedX, adjustedY, adjustedWidth, adjustedHeight);
                    if (qRect.contains(event->pos())) {
                        clickedFaceIndex = faceIndex;
                        break;
                    }
                    faceIndex++;
                }
                if (clickedFaceIndex != -1) {
                    if (lastSelectedFaceIndex == clickedFaceIndex) {
                        auto& faceData = (*detectedFaces)[clickedFaceIndex];
                        bool ok = false;
                        QString currentName;
                        int personId = faceData.getPersonIdConst();
                        const auto& personIdNames = data->getPersonIdNames();
                        auto it = personIdNames.find(personId);
                        if (it != personIdNames.end()) {
                            currentName = QString::fromStdString(it->second);
                            QStringList nameList;
                            for (const auto& [id, nameStr] : personIdNames) {
                                if (QString::fromStdString(nameStr) != currentName) {
                                    nameList << QString::fromStdString(nameStr);
                                }
                            }
                            std::string namesString = "none|none|" + nameList.join("|").toStdString();
                            std::map<std::string, Option> option = {
                                {Text::Option::ImageEditor::renameFace().toStdString(), Option("text", currentName.toStdString())},
                                {Text::Option::ImageEditor::deleteFace().toStdString(), Option("bool", "false")},
                                {Text::Option::ImageEditor::mergeFace().toStdString(), Option("list", namesString)}};
                            std::map<std::string, std::string> optionDone = showOptionsDialog(this, "Face options", option);
                            if (optionDone[Text::Option::ImageEditor::deleteFace().toStdString()] == "true") {
                                auto it = std::find(detectedFaces->begin(), detectedFaces->end(), faceData);
                                if (it != detectedFaces->end()) {
                                    detectedFaces->erase(it);
                                }
                                update();
                                lastSelectedFaceIndex = -1;
                                return;
                            } else if (!optionDone[Text::Option::ImageEditor::renameFace().toStdString()].empty()) {
                                auto* personIdNamesPtr = data->getPersonIdNamesPtr();
                                (*personIdNamesPtr)[personId] = optionDone[Text::Option::ImageEditor::renameFace().toStdString()];
                                update();
                            }
                            if (!optionDone[Text::Option::ImageEditor::mergeFace().toStdString()].empty()) {
                                std::string mergeWithName = optionDone[Text::Option::ImageEditor::mergeFace().toStdString()];
                                size_t pipePos = mergeWithName.find('|');
                                if (pipePos != std::string::npos) {
                                    mergeWithName = mergeWithName.substr(0, pipePos);
                                }
                                if (mergeWithName != "none") {
                                    int mergeWithId = -1;
                                    for (const auto& [id, nameStr] : personIdNames) {
                                        if (nameStr == mergeWithName) {
                                            mergeWithId = id;
                                            break;
                                        }
                                    }
                                    if (mergeWithId != -1 && mergeWithId != personId) {
                                        for (ImageData* imageData : *data->getImagesDataPtr()->get()) {
                                            auto otherDetectedFaces = imageData->getDetectedFacesPtr();
                                            for (auto& otherFace : *otherDetectedFaces) {
                                                if (otherFace.getPersonIdConst() == personId) {
                                                    *otherFace.getPersonIdPtr() = mergeWithId;
                                                }
                                            }
                                        }
                                        auto* personIdNamesPtr = data->getPersonIdNamesPtr();
                                        personIdNamesPtr->erase(personId);
                                        update();
                                    }
                                }
                            }
                        }
                        lastSelectedFaceIndex = -1;
                        return;
                    } else {
                        lastSelectedFaceIndex = clickedFaceIndex;
                        update();
                        return;
                    }
                } else {
                    lastSelectedFaceIndex = -1;
                }
            }
        }

        if (QApplication::keyboardModifiers() & Qt::ControlModifier) {
            if (!drawingRectangle) {
                emit ctrlLeftClicked();
            }
        } else {
            if (cropping) {
            } else {
                if (!inFace(event->pos())) {
                    emit leftClicked();
                    setHoverBackgroundColor(CLICK_BACKGROUND_COLOR);
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

            qImage = data->loadImage(this, imagePath.toStdString(), QSize(), false, 0, true, false, false, false);

            if (!qImage.isNull()) {
                this->setPixmap(QPixmap::fromImage(qImage).scaled(size() - QSize(5, 5), Qt::KeepAspectRatio, Qt::SmoothTransformation));
            } else {
                this->setText("Erreur");
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
                setHoverBackgroundColor(HOVER_BACKGROUND_COLOR);
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
    ImageLabel::paintEvent(event);

    QPainter painter(this);
    if (drawingRectangle) {
        painter.setPen(Qt::DashLine);
        painter.drawRect(QRect(cropStart, cropEnd));
    }
    if (this->getPersonsEditorConst() && !data->getImagesDataPtr()->getCurrentImageData()->getDetectedObjects().empty()) {
        std::map<std::string, std::vector<std::pair<cv::Rect, float>>> detectedObjects = data->getImagesDataPtr()->getCurrentImageData()->getDetectedObjects();

        QSize scaledPixmapSize = qImage.size();
        scaledPixmapSize.scale(this->size(), Qt::KeepAspectRatio);
        double xScale = static_cast<double>(scaledPixmapSize.width()) / qImage.width();
        double yScale = static_cast<double>(scaledPixmapSize.height()) / qImage.height();
        int xOffset = (this->width() - scaledPixmapSize.width()) / 2;
        int yOffset = (this->height() - scaledPixmapSize.height()) / 2;
        for (const auto& [key, value] : detectedObjects) {
            for (const auto& [rect, confidence] : value) {
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
        int faceIdx = 0;
        for (const auto& faceData : *detectedFaces) {
            cv::Rect rect = faceData.getFaceRect();
            float confidence = faceData.getConfidence();
            int personId = faceData.getPersonIdConst();

            int adjustedX = static_cast<int>(rect.x * xScale) + xOffset;
            int adjustedY = static_cast<int>(rect.y * yScale) + yOffset;
            int adjustedWidth = static_cast<int>(rect.width * xScale);
            int adjustedHeight = static_cast<int>(rect.height * yScale);

            QRect qRect(adjustedX, adjustedY, adjustedWidth, adjustedHeight);

            if (faceIdx == lastSelectedFaceIndex) {
                QPen thickPen(Qt::green);
                thickPen.setWidth(4);
                painter.setPen(thickPen);
            } else {
                painter.setPen(Qt::green);
            }
            painter.drawRect(qRect);

            QString label;
            if (personId != -1) {
                auto it = personIdNames.find(personId);
                if (it != personIdNames.end()) {
                    label = QString::fromStdString(it->second);
                } else {
                    label = QString("Error with ID : %1").arg(personId);
                }
            } else {
                label = QString("Face %1%").arg(confidence * 100, 0, 'f', 2);
            }
            painter.drawText(qRect.topLeft(), label);
            faceIdx++;
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
 * @param point The point to check if it is within a detected face rectangle
 * @return True if the point is within a detected face rectangle, false otherwise
 */
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