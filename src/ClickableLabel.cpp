#include "ClickableLabel.hpp"

ClickableLabel::ClickableLabel(Data* data, const QString& imagePath, QString toolTip, QWidget* parent, QSize size, bool setSize, int thumbnail, bool square, bool force)
    : QLabel(parent) {
    initial_border_color = border_color;
    initial_hover_border_color = hover_border_color;
    initial_background_color = background_color;
    initial_hover_background_color = hover_background_color;

    if (toolTip != "") {
        setToolTip(toolTip);
    }

    QImage qImage = data->loadImage(this, imagePath.toStdString(), size, setSize, thumbnail, true, square, true, force);

    if (!qImage.isNull()) {
        setPixmap(QPixmap::fromImage(qImage).scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        setCursor(Qt::PointingHandCursor);
    } else {
        setText("Error : null image");
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
        setHoverBackgroundColor(CLICK_BACKGROUND_COLOR);

        updateStyleSheet();
    }
    QLabel::mousePressEvent(event);
}

void ClickableLabel::mouseReleaseEvent(QMouseEvent* event) {
    emit clicked();
    if (event->button() == Qt::LeftButton) {
        if (event->modifiers() & Qt::ShiftModifier) {
            emit shiftLeftClicked();
        } else if (event->modifiers() & Qt::ControlModifier) {
            emit ctrlLeftClicked();

        } else {
            emit leftClicked();
            setHoverBackgroundColor(HOVER_BACKGROUND_COLOR);
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
        border: %1px solid %7;
            border-radius: %2px;
            background-color: %8; 

        }
    )")
                             .arg(border)
                             .arg(border_radius)
                             .arg(border_color.c_str())
                             .arg(hover_border_color.c_str())
                             .arg(background_color.c_str())
                             .arg(hover_background_color.c_str())
                             .arg(disabled_border_color.c_str())
                             .arg(disabled_background_color.c_str());
    setStyleSheet(styleSheet);
}

void ClickableLabel::setDisabledColor(std::string borderColor, std::string backgroundColor) {
    setDisabledBorderColor(borderColor);
    setDisabledBackgroundColor(backgroundColor);
    updateStyleSheet();
}

void ClickableLabel::setDisabledBorderColor(std::string borderColor) {
    disabled_border_color = borderColor;
}

void ClickableLabel::setDisabledBackgroundColor(std::string backgroundColor) {
    disabled_background_color = backgroundColor;
}

void ClickableLabel::setBorderColor(std::string borderColor) {
    border_color = borderColor;
}

void ClickableLabel::setHoverBorderColor(std::string hoverBorderColor) {
    hover_border_color = hoverBorderColor;
}

void ClickableLabel::setInitialBorderColor(std::string borderColor) {
    initial_border_color = borderColor;
}

void ClickableLabel::setInitialHoverBorderColor(std::string hoverBorderColor) {
    initial_hover_border_color = hoverBorderColor;
}

void ClickableLabel::setBackgroundColor(std::string backgroundColor) {
    background_color = backgroundColor;
}

void ClickableLabel::setHoverBackgroundColor(std::string hoverBackgroundColor) {
    hover_background_color = hoverBackgroundColor;
}

void ClickableLabel::setInitialBackgroundColor(std::string backgroundColor) {
    initial_background_color = backgroundColor;
}

void ClickableLabel::setInitialHoverBackgroundColor(std::string hoverBackgroundColor) {
    initial_hover_background_color = hoverBackgroundColor;
}

void ClickableLabel::setInitialBorder(std::string borderColor, std::string hoverBorderColor) {
    int border = (size().width() + size().height()) / 50;
    setBorder(border);
    // int border_radius = 2.5;
    setBorderRadius(border * 2);

    setBorderColor(borderColor);
    setHoverBorderColor(hoverBorderColor);
    setInitialBorderColor(borderColor);
    setInitialHoverBorderColor(hoverBorderColor);
    updateStyleSheet();
}

void ClickableLabel::setBorder(std::string borderColor, std::string hoverBorderColor) {
    setBorderRadius(border_radius);
    setBorderColor(borderColor);
    setHoverBorderColor(hoverBorderColor);
    updateStyleSheet();
}

void ClickableLabel::resetBorder() {
    setBorder(initial_border_color, initial_hover_border_color);
}

void ClickableLabel::setInitialBackground(std::string borderColor, std::string hoverBorderColor) {
    setBackgroundColor(borderColor);
    setHoverBackgroundColor(hoverBorderColor);
    setInitialBackgroundColor(borderColor);
    setInitialHoverBackgroundColor(hoverBorderColor);
    updateStyleSheet();
}

void ClickableLabel::setBackground(std::string borderColor, std::string hoverBorderColor) {
    setBackgroundColor(borderColor);
    setHoverBackgroundColor(hoverBorderColor);
    updateStyleSheet();
}

void ClickableLabel::resetBackground() {
    setBackground(initial_border_color, initial_hover_border_color);
}

void ClickableLabel::setBorderRadius(int border_radius) {
    this->border_radius = border_radius;
    updateStyleSheet();
}

void ClickableLabel::setBorder(int border) {
    this->border = border;
    updateStyleSheet();
}
#include <QPainter>

void ClickableLabel::paintEvent(QPaintEvent* event) {
    QLabel::paintEvent(event);
    if (logoVisible) {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(Qt::NoPen);  // Pas de bordure pour le cercle
        QFont font = painter.font();
        font.setPointSize(this->height() / 5);  // Ajuster la taille de la police en fonction de la hauteur de l'image
        painter.setFont(font);

        QRect rect = this->rect();
        QString text = QString::number(logoNumber);

        // Dessiner un rond rouge
        // int radius = font.pointSize()     * 2;
        int radius = font.pointSize();

        QRect circleRect = QRect(rect.right() - radius * 2, rect.bottom() - radius * 2, radius * 2, radius * 2);
        painter.setBrush(QBrush(logoColor));  // Couleur du rond
        painter.drawEllipse(circleRect);

        // Dessiner le texte au centre du rond
        painter.setPen(textColor);  // Couleur du texte
        painter.drawText(circleRect, Qt::AlignCenter, text);
    }
}