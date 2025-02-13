#pragma once

#include <QFileDialog>
#include <QLabel>
#include <QMouseEvent>
#include <opencv2/opencv.hpp>

#include "Const.hpp"
#include "Data.hpp"

class ClickableLabel : public QLabel {
    Q_OBJECT

   public:
    void select(std::string borderColor, std::string hoverBorderColor);
    void unSelect();

    explicit ClickableLabel(Data* data, const QString& i, QString toolTip = "", QWidget* parent = nullptr, QSize size = QSize(0, 0), bool setSize = true, int thumbnail = 0, bool square = false, bool force = false);

   signals:
    void clicked();
    void leftClicked();
    void rightClicked();
    void shiftLeftClicked();
    void ctrlLeftClicked();

   protected:
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;    // Override mouse press event
    void mouseReleaseEvent(QMouseEvent* event) override;  // Override mouse release event

   public:
    int border = 0;
    int border_radius = 5;
    void updateStyleSheet();

    QString border_color = "transparent";
    QString hover_border_color = "#9c9c9c";

    QString background_color = "transparent";
    QString hover_background_color = "#b3b3b3";
    QPixmap defaultPixmap;
    // private:
};