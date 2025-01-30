#include "ClickableLabel.h"
ClickableLabel::ClickableLabel(Data* data, const QString& imagePath, QWidget* parent, QSize size, bool setSize, int thumbnail, bool square, bool force)
    : QLabel(parent) {

    QImage qImage = data->loadImage(this, imagePath.toStdString(), size, setSize, thumbnail, true, square, true, force);

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

void ClickableLabel::enterEvent(QEnterEvent* event) {
    QLabel::enterEvent(event);
}

void ClickableLabel::leaveEvent(QEvent* event) {
    QLabel::leaveEvent(event);
}

void ClickableLabel::mousePressEvent(QMouseEvent* event) {

    if (event->button() == Qt::LeftButton) {

        hover_background_color = CLICK_BACKGROUND_COLOR;

        updateStyleSheet();

    }
    QLabel::mousePressEvent(event);

}

void ClickableLabel::mouseReleaseEvent(QMouseEvent* event) {
    emit clicked();
    if (event->button() == Qt::LeftButton) {
        if (event->modifiers() & Qt::ShiftModifier) {
            emit shiftLeftClicked();
        } else if (event->modifiers() & Qt::ControlModifier){
            emit ctrlLeftClicked();

        } else{
            emit leftClicked();
            hover_background_color = HOVER_BACKGROUND_COLOR;
            updateStyleSheet();
        }
    } else if (event->button() == Qt::RightButton) {
        emit rightClicked();
    }

    QLabel::mouseReleaseEvent(event);
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


void ClickableLabel::select(std::string backgroundColor, std::string hoverBackgroundColor){
    selected = true;
    background_color = QString::fromStdString(backgroundColor);
    hover_background_color = QString::fromStdString(hoverBackgroundColor);
    updateStyleSheet();
}

void ClickableLabel::unSelect(){
    selected = false;
    background_color = "transparent";
    hover_background_color = "#b3b3b3";
    updateStyleSheet();
}