#include "ClickableLabel.h"
ClickableLabel::ClickableLabel(Data data, const QString& imagePath, QWidget* parent, QSize size, bool setSize, int thumbnail)
    : QLabel(parent) {

    QImage qImage = data.loadImage(this, imagePath.toStdString(), size, setSize, thumbnail);

    // QImage qImage;

    if (!qImage.isNull()) {
        this->setPixmap(QPixmap::fromImage(qImage).scaled(size - QSize(5, 5), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    else {
        // Handle the case where the image is not valid (optional)
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