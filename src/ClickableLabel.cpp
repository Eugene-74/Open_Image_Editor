#include "ClickableLabel.hpp"

#include <QFileDialog>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <opencv2/opencv.hpp>

#include "Const.hpp"
#include "Data.hpp"
#include "Verification.hpp"

/**
 * @brief Constructor for ClickableLabel
 * @param data Pointer to Data
 * @param imagePath Path to the image
 * @param toolTip Tool tip text
 * @param parent Parent widget
 * @param sizePtr Pointer to the size of the label
 * @param setSize If true, set the size of the label to the sizePtr
 * @param thumbnail Thumbnail size (0 for full size)
 * @param square If true, the label will be square
 * @param force If true, force the loading of the image
 */
ClickableLabel::ClickableLabel(std::shared_ptr<Data> data, const QString& imagePath, QString toolTip, QWidget* parent, QSize* sizePtr, bool setSize, int thumbnail, bool square, bool force)
    : QLabel(parent), sizePtr(sizePtr) {
    initial_border_color = border_color;
    initial_hover_border_color = hover_border_color;
    initial_background_color = background_color;
    initial_hover_background_color = hover_background_color;

    if (toolTip != "") {
        setToolTip(toolTip);
    }

    QImage qImage;

    if (isImage(imagePath.toStdString()) || isVideo(imagePath.toStdString())) {
        qImage = data->loadImage(this, imagePath.toStdString(), *sizePtr, setSize, thumbnail, true, square, true, force);

    } else {
        qImage = QImage();
    }

    if (!qImage.isNull()) {
        setPixmap(QPixmap::fromImage(qImage).scaled(*sizePtr, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        setCursor(Qt::PointingHandCursor);
    } else {
        setText("Error : null image");
    }

    if (setSize)
        setFixedSize(*sizePtr);
    else {
        QSize scaledSize = qImage.size();
        scaledSize.scale(*sizePtr, Qt::KeepAspectRatio);
        setFixedSize(scaledSize);
    }

    this->setAlignment(Qt::AlignCenter);

    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    setMouseTracking(true);

    updateStyleSheet();
}

/**
 * @brief Emit the right signal when the label is clicked (left, right, shift left, ctrl left)
 * @param event Mouse event
 * @details This function also calls the base class mouseReleaseEvent function
 */
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

/**
 * @brief Update the style sheet of the label with the current colors and border of the object
 */
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

/**
 * @brief Set the color of the label when it is disabled
 * @param borderColor Color of the border when disabled
 * @param backgroundColor Color of the background when disabled
 * @details This function also updates the style sheet of the label
 */
void ClickableLabel::setDisabledColor(std::string borderColor, std::string backgroundColor) {
    setDisabledBorderColor(borderColor);
    setDisabledBackgroundColor(backgroundColor);
    updateStyleSheet();
}

/**
 * @brief Set the color of the border when the label is disabled
 * @param borderColor Color of the border when disabled
 * @details This function doesn't updates the style sheet of the label
 */
void ClickableLabel::setDisabledBorderColor(std::string borderColor) {
    this->disabled_border_color = borderColor;
}

/**
 * @brief Set the color of the background when the label is disabled
 * @param backgroundColor Color of the background when disabled
 * @details This function doesn't updates the style sheet of the label
 */
void ClickableLabel::setDisabledBackgroundColor(std::string backgroundColor) {
    this->disabled_background_color = backgroundColor;
}

/**
 * @brief Set the color of the border when the label is in normal state
 * @param borderColor Color of the border when in normal state
 * @details This function doesn't updates the style sheet of the label
 */
void ClickableLabel::setBorderColor(std::string borderColor) {
    this->border_color = borderColor;
}

/**
 * @brief Set the color of the border when the label is hovered
 * @param hoverBorderColor Color of the border when hovered
 * @details This function doesn't updates the style sheet of the label
 */
void ClickableLabel::setHoverBorderColor(std::string hoverBorderColor) {
    this->hover_border_color = hoverBorderColor;
}

/**
 * @brief Set the color of the initial background
 * @param borderColor Color of the initial background
 * @details This function doesn't updates the style sheet of the label
 */
void ClickableLabel::setInitialBorderColor(std::string borderColor) {
    this->initial_border_color = borderColor;
}

/**
 * @brief Set the color of the initial hover background
 * @param hoverBorderColor Color of the initial hover background
 * @details This function doesn't updates the style sheet of the label
 */
void ClickableLabel::setInitialHoverBorderColor(std::string hoverBorderColor) {
    this->initial_hover_border_color = hoverBorderColor;
}

/**
 * @brief Set the color of the background when the label is in normal state
 * @param backgroundColor Color of the background
 * @details This function doesn't updates the style sheet of the label
 */
void ClickableLabel::setBackgroundColor(std::string backgroundColor) {
    this->background_color = backgroundColor;
}

/**
 * @brief Set the color of the background when the label is hovered
 * @param hoverBackgroundColor Color of the background when hovered
 * @details This function doesn't updates the style sheet of the label
 */
void ClickableLabel::setHoverBackgroundColor(std::string hoverBackgroundColor) {
    this->hover_background_color = hoverBackgroundColor;
}

/**
 * @brief Set the color of the initial background
 * @param backgroundColor Color of the initial background
 * @details This function doesn't updates the style sheet of the label
 */
void ClickableLabel::setInitialBackgroundColor(std::string backgroundColor) {
    this->initial_background_color = backgroundColor;
}

/**
 * @brief Set the color of the initial hover background
 * @param hoverBackgroundColor Color of the initial hover background
 * @details This function doesn't updates the style sheet of the label
 */
void ClickableLabel::setInitialHoverBackgroundColor(std::string hoverBackgroundColor) {
    this->initial_hover_background_color = hoverBackgroundColor;
}

/**
 * @brief Set the initial border and hover border color of the label
 * @param borderColor Color of the border
 * @param hoverBorderColor Color of the hover border
 * @details This function also sets the border radius and border size of the label
 * @details This function also updates the style sheet of the label
 */
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

/**
 * @brief Set the border and hover border color of the label
 * @param borderColor Color of the border
 * @param hoverBorderColor Color of the hover border
 * @details This function also updates the style sheet of the label
 */
void ClickableLabel::setBorder(std::string borderColor, std::string hoverBorderColor) {
    setBorderColor(borderColor);
    setHoverBorderColor(hoverBorderColor);
    updateStyleSheet();
}

/**
 * @brief Set the border color of the label to the initial color
 */
void ClickableLabel::resetBorder() {
    setBorder(initial_border_color, initial_hover_border_color);
}

/**
 * @brief Set the initial background and hover background color of the label
 * @param backgroundColor Color of the background
 * @param hoverBackgroundColor Color of the hover background
 * @details This function also updates the style sheet of the label
 */
void ClickableLabel::setInitialBackground(std::string backgroundColor, std::string hoverBackgroundColor) {
    setBackgroundColor(backgroundColor);
    setHoverBackgroundColor(hoverBackgroundColor);
    setInitialBackgroundColor(backgroundColor);
    setInitialHoverBackgroundColor(hoverBackgroundColor);
    updateStyleSheet();
}

/**
 * @brief Set the background and hover background color of the label
 * @param backgroundColor Color of the background
 * @param hoverBackgroundColor Color of the hover background
 * @details This function also updates the style sheet of the label
 */
void ClickableLabel::setBackground(std::string backgroundColor, std::string hoverBackgroundColor) {
    setBackgroundColor(backgroundColor);
    setHoverBackgroundColor(hoverBackgroundColor);
    updateStyleSheet();
}

/**
 * @brief Set the background color of the label to the initial color
 */
void ClickableLabel::resetBackground() {
    setBackground(initial_background_color, initial_hover_background_color);
}

/**
 * @brief Set the border radius of the label
 * @param border_radius Border radius in pixels
 * @details This function also updates the style sheet of the label
 */
void ClickableLabel::setBorderRadius(int border_radius) {
    this->border_radius = border_radius;
    updateStyleSheet();
}

/**
 * @brief set the border of the label
 * @param border Color of the border
 * @details This function also updates the style sheet of the label
 */
void ClickableLabel::setBorder(int border) {
    this->border = border;
    updateStyleSheet();
}

/**
 * @brief Paint the label with a logo if it is visible
 * @param event Paint event
 * @details This function also calls the base class paintEvent function
 */
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

        QRect circleRect = QRect(rect.right() - (radius * 2), rect.bottom() - (radius * 2), radius * 2, radius * 2);
        painter.setBrush(QBrush(logoColor));
        painter.drawEllipse(circleRect);

        painter.setPen(logoTextColor);
        painter.drawText(circleRect, Qt::AlignCenter, logoText);
    }
}

/**
 * @brief Add a logo to the label
 * @param logoColor Color of the logo
 * @param logoTextColor Color of the logo text
 * @details This function also sets the logo visible and updates the label
 */
void ClickableLabel::addLogo(QColor logoColor, QColor logoTextColor) {
    setLogoColor(logoColor);
    setLogoTextColor(logoTextColor);
    setLogoVisible(true);
    update();
}

/**
 * @brief Add a logo to the label
 * @param logoColor Color of the logo
 * @param logoTextColor Color of the logo text
 * @param logoNumber Number to display in the logo
 * @details This function also sets the logo visible and updates the label
 */
void ClickableLabel::addLogo(QColor logoColor, QColor logoTextColor, int logoNumber) {
    setLogoColor(logoColor);
    setLogoTextColor(logoTextColor);
    setLogoNumber(logoNumber);
    setLogoVisible(true);
    update();
}

/**
 * @brief Set the logo number to display in the logo
 * @param logoNumber Number to display in the logo
 * @details If the number is less than 0, it will display "?".
 * @details If the number is greater than 99, it will display "99".
 */
void ClickableLabel::setLogoNumber(int logoNumber) {
    if (logoNumber < 0) {
        logoText = "?";
    } else if (logoNumber > 99) {
        logoText = "99";
    } else {
        logoText = QString::number(logoNumber);
    }
}

/**
 * @brief Set the color of the logo
 * @param logoColor Color of the logo
 * @details This function also updates the label
 */
void ClickableLabel::setLogoColor(QColor logoColor) {
    this->logoColor = logoColor;
}

/**
 * @brief Set the color of the logo text
 * @param logoTextColor Color of the logo text
 * @details This function also updates the label
 */
void ClickableLabel::setLogoTextColor(QColor logoTextColor) {
    this->logoTextColor = logoTextColor;
}

/**
 * @brief Set the visibility of the logo
 * @param logoVisible If true, the logo will be visible, else it will be hidden
 * @details This function also updates the label
 */
void ClickableLabel::setLogoVisible(bool logoVisible) {
    this->logoVisible = logoVisible;
}

/**
 * @brief Set the logo to be enabled (visible)
 * @details This function also updates the label
 */
void ClickableLabel::setLogoEnabled() {
    setLogoVisible(true);
    update();
}
/**
 * @brief Set the logo to be disabled (hidden)
 * @details This function also updates the label
 */
void ClickableLabel::setLogoDisabled() {
    setLogoVisible(false);
    update();
}
