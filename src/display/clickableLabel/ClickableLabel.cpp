#include "ClickableLabel.h"
ClickableLabel::ClickableLabel(const QString& imagePath, QWidget* parent, QSize size, bool setSize)
    : QLabel(parent) {
    // Load image using OpenCV with alpha channel
    // Check if the resource exists in the Qt resource system

    QFile file(imagePath);

    cv::Mat image;
    if (file.exists()) {
        // Load image from Qt resources
        if (file.open(QIODevice::ReadOnly)) {
            QByteArray imageData = file.readAll();
            std::vector<uchar> data(imageData.begin(), imageData.end());
            image = cv::imdecode(data, cv::IMREAD_UNCHANGED);
        }
    }
    else {
        // Load image from file system
        image = cv::imread(imagePath.toStdString(), cv::IMREAD_UNCHANGED);
    }


    if (!image.empty()) {

        // Convert BGR to RGB (if needed) and keep alpha channel
        if (image.channels() == 4) {


            // Create QImage from OpenCV Mat with alpha channel
            QImage qImage(image.data, image.cols, image.rows, image.step[0], QImage::Format_ARGB32);
            if (!setSize) {
                float ratio;
                if (size.height() / qImage.height() > size.width() / qImage.width()) {
                    ratio = static_cast<float>(size.width()) / qImage.width();
                }
                else {
                    ratio = static_cast<float>(size.height()) / qImage.height();
                }

                size.setWidth(qImage.width() * ratio);
                size.setHeight(qImage.height() * ratio);
            }
            this->setPixmap(QPixmap::fromImage(qImage).scaled(size - QSize(5, 5), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
        else {
            // Handle images without an alpha channel (optional)
            cv::cvtColor(image, image, cv::COLOR_BGR2RGB);

            QImage qImage(image.data, image.cols, image.rows, image.step[0], QImage::Format_RGB888);
            if (!setSize) {
                float ratio;
                if (size.height() / qImage.height() > size.width() / qImage.width()) {
                    ratio = static_cast<float>(size.width()) / qImage.width();
                }
                else {
                    ratio = static_cast<float>(size.height()) / qImage.height();
                }

                size.setWidth(qImage.width() * ratio);
                size.setHeight(qImage.height() * ratio);
            }
            this->setPixmap(QPixmap::fromImage(qImage).scaled(size - QSize(5, 5), Qt::KeepAspectRatio, Qt::SmoothTransformation));

        }

    }
    else {
        // Handle the case where the image is not valid (optional)
        this->setText("Erreur : Image non valide !");
    }

    setFixedSize(size);

    this->setAlignment(Qt::AlignCenter);

    // Enable mouse tracking
    setMouseTracking(true);

    updateStyleSheet();

}

// Gérer l'entrée de la souris
void ClickableLabel::enterEvent(QEvent* event) {
    QLabel::enterEvent(event);
}

// Gérer la sortie de la souris
void ClickableLabel::leaveEvent(QEvent* event) {
    QLabel::leaveEvent(event);
}

void ClickableLabel::mousePressEvent(QMouseEvent* event) {

    if (event->button() == Qt::LeftButton) {

        hover_background_color = CLICK_BACKGROUND_COLOR;

        updateStyleSheet();

    }
    QLabel::mousePressEvent(event);  // Call the base class implementation

}

void ClickableLabel::mouseReleaseEvent(QMouseEvent* event) {
    emit clicked();  // Émettre le signal quand on clique



    if (event->button() == Qt::LeftButton) {

        hover_background_color = HOVER_BACKGROUND_COLOR;
        updateStyleSheet();
    }
    QLabel::mouseReleaseEvent(event);  // Call the base class implementation
}

void ClickableLabel::updateStyleSheet() {
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