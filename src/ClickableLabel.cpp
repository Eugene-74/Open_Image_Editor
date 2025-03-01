#include "ClickableLabel.hpp"

ClickableLabel::ClickableLabel(Data* data, const QString& imagePath, QString toolTip, QWidget* parent, QSize* sizePtr, bool setSize, int thumbnail, bool square, bool force)
    : QLabel(parent), sizePtr(sizePtr) {
    initial_border_color = border_color;
    initial_hover_border_color = hover_border_color;
    initial_background_color = background_color;
    initial_hover_background_color = hover_background_color;

    if (toolTip != "") {
        setToolTip(toolTip);
    }

    QImage qImage = data->loadImage(this, imagePath.toStdString(), *sizePtr, setSize, thumbnail, true, square, true, force);

    if (!qImage.isNull()) {
        setPixmap(QPixmap::fromImage(qImage).scaled(*sizePtr, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        setCursor(Qt::PointingHandCursor);
    } else {
        setText("Error : null image");
    }

    if (setSize)
        setFixedSize(*sizePtr);
    // resize(*sizePtr);
    else {
        QSize scaledSize = qImage.size();
        scaledSize.scale(*sizePtr, Qt::KeepAspectRatio);
        setFixedSize(scaledSize);
        // resize(scaledSize);
    }

    this->setAlignment(Qt::AlignCenter);

    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

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
    this->disabled_border_color = borderColor;
}

void ClickableLabel::setDisabledBackgroundColor(std::string backgroundColor) {
    this->disabled_background_color = backgroundColor;
}

void ClickableLabel::setBorderColor(std::string borderColor) {
    this->border_color = borderColor;
}

void ClickableLabel::setHoverBorderColor(std::string hoverBorderColor) {
    this->hover_border_color = hoverBorderColor;
}

void ClickableLabel::setInitialBorderColor(std::string borderColor) {
    this->initial_border_color = borderColor;
}

void ClickableLabel::setInitialHoverBorderColor(std::string hoverBorderColor) {
    this->initial_hover_border_color = hoverBorderColor;
}

void ClickableLabel::setBackgroundColor(std::string backgroundColor) {
    this->background_color = backgroundColor;
}

void ClickableLabel::setHoverBackgroundColor(std::string hoverBackgroundColor) {
    this->hover_background_color = hoverBackgroundColor;
}

void ClickableLabel::setInitialBackgroundColor(std::string backgroundColor) {
    this->initial_background_color = backgroundColor;
}

void ClickableLabel::setInitialHoverBackgroundColor(std::string hoverBackgroundColor) {
    this->initial_hover_background_color = hoverBackgroundColor;
}

void ClickableLabel::setInitialBorder(std::string borderColor, std::string hoverBorderColor) {
    int border = (size().width() + size().height()) / 50;
    setBorder(border);
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

void ClickableLabel::setBackground(std::string backgroundColor, std::string hoverBackgroundColor) {
    setBackgroundColor(backgroundColor);
    setHoverBackgroundColor(hoverBackgroundColor);
    updateStyleSheet();
}

void ClickableLabel::resetBackground() {
    setBackground(initial_background_color, initial_hover_background_color);
}

void ClickableLabel::setBorderRadius(int border_radius) {
    this->border_radius = border_radius;
    updateStyleSheet();
}

void ClickableLabel::setBorder(int border) {
    this->border = border;
    updateStyleSheet();
}

void ClickableLabel::paintEvent(QPaintEvent* event) {
    QLabel::paintEvent(event);

    if (logoVisible) {
        int radius = static_cast<int>(sqrt(this->height()) * 1.5f);
        int fontSize = radius / logoText.length() * 1.5f;

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(Qt::NoPen);
        QFont font = painter.font();
        font.setPointSize(fontSize);
        painter.setFont(font);

        QRect rect = this->rect();

        QRect circleRect = QRect(rect.right() - radius * 2, rect.bottom() - radius * 2, radius * 2, radius * 2);
        painter.setBrush(QBrush(logoColor));
        painter.drawEllipse(circleRect);

        painter.setPen(logoTextColor);
        painter.drawText(circleRect, Qt::AlignCenter, logoText);
    }
}

void ClickableLabel::addLogo(QColor logoColor, QColor logoTextColor) {
    setLogoColor(logoColor);
    setLogoTextColor(logoTextColor);
    setLogoVisible(true);
    update();
}

void ClickableLabel::addLogo(QColor logoColor, QColor logoTextColor, int logoNumber) {
    setLogoColor(logoColor);
    setLogoTextColor(logoTextColor);
    setLogoNumber(logoNumber);
    setLogoVisible(true);
    update();
}

void ClickableLabel::setLogoNumber(int logoNumber) {
    if (logoNumber < 0) {
        logoText = "?";
    } else if (logoNumber > 99) {
        logoText = "+99";
    } else {
        logoText = QString::number(logoNumber);
    }
}

void ClickableLabel::setLogoColor(QColor logoColor) {
    this->logoColor = logoColor;
}

void ClickableLabel::setLogoTextColor(QColor logoTextColor) {
    this->logoTextColor = logoTextColor;
}

void ClickableLabel::setLogoVisible(bool logoVisible) {
    this->logoVisible = logoVisible;
}

void ClickableLabel::setLogoEnabled() {
    setLogoVisible(true);
    update();
}
void ClickableLabel::setLogoDisabled() {
    setLogoVisible(false);
    update();
}

void ClickableLabel::resizeEvent(QResizeEvent* event) {
    QLabel::resizeEvent(event);
}