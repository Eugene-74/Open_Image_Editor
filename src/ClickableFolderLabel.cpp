#include "ClickableFolderLabel.hpp"

#include <QPainter>

#include "Data.hpp"
/**
 * @brief Constructor for ClickableFolderLabel
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
ClickableFolderLabel::ClickableFolderLabel(std::shared_ptr<Data> dat, const QString& imagePath, QString toolTip, QWidget* parent, QSize* sizePtr, bool setSize, int thumbnail, bool square, bool force)
    : ClickableLabel(dat, imagePath, toolTip, parent, sizePtr, setSize, thumbnail, square, force) {
    this->data = dat;
}

/**
 * @brief Paint event for the ClickableFolderLabel
 * @param event Paint event
 */
void ClickableFolderLabel::paintEvent(QPaintEvent* event) {
    ClickableLabel::paintEvent(event);
    QPainter painter(this);
    if (this->data->getDarkMode()) {
        painter.setPen(Qt::white);
    } else {
        painter.setPen(Qt::black);
    }
    QFont font = painter.font();
    font.setBold(true);
    int widgetWidth = rect().width();
    int maxChar = 10;
    int minFontSize = 5;
    int maxFontSize = 18;
    int fontSize = maxFontSize;
    QString qText = QString::fromStdString(this->text);
    QFontMetrics fm(font);

    while (fontSize > minFontSize) {
        font.setPointSize(fontSize);
        painter.setFont(font);
        QFontMetrics fmTest(font);
        int width10 = fmTest.horizontalAdvance(QString("W").repeated(maxChar));
        if (width10 <= widgetWidth - 8)
            break;
        fontSize--;
    }
    font.setPointSize(fontSize);
    painter.setFont(font);
    QFontMetrics fmFinal(font);

    QRect textRect = rect();
    QString elided = fmFinal.elidedText(qText, Qt::ElideRight, widgetWidth - textRect.width() * 4 / 8);
    int textHeight = textRect.height() * 9 / 8;
    QRect bottomRect(textRect.left(), textRect.bottom() - textHeight, textRect.width(), textHeight);
    painter.drawText(bottomRect, Qt::AlignHCenter | Qt::AlignVCenter, elided);
}

/**
 * @brief Set the text of the label
 * @param newText New text to set
 */
void ClickableFolderLabel::setText(const std::string& text) {
    this->text = text;
    update();
}