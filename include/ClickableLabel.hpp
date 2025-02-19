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
    void setInitialBorder(std::string borderColor, std::string hoverBorderColor);
    void setBorder(std::string borderColor, std::string hoverBorderColor);
    void resetBorder();

    void setInitialBackground(std::string backgroundColor, std::string hoverBackgroundColor);
    void setBackground(std::string backgroundColor, std::string hoverBackgroundColor);

    void resetBackground();

    int logoNumber = -1;
    bool logoVisible = false;
    QColor textColor = Qt::white;
    QColor logoColor = Qt::red;
    void updateStyleSheet();

    QPixmap defaultPixmap;

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
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

   private:
    int border = 0;
    int border_radius = 5;

    std::string disabled_border_color = "#b3b3b3";
    std::string disabled_background_color = "#b3b3b3";

    std::string border_color = "transparent";
    std::string hover_border_color = "transparent";

    std::string background_color = "transparent";
    std::string hover_background_color = "transparent";

    std::string initial_border_color = "transparent";
    std::string initial_hover_border_color = "transparent";

    std::string initial_background_color = "transparent";
    std::string initial_hover_background_color = "transparent";

    void setBorder(int border);
    void setBorderRadius(int border_radius);

    void setDisabledColor(std::string borderColor, std::string backgroundColor);
    void setDisabledBorderColor(std::string borderColor);
    void setDisabledBackgroundColor(std::string backgroundColor);

    void setInitialBorderColor(std::string borderColor);
    void setInitialHoverBorderColor(std::string hoverBorderColor);
    void setBorderColor(std::string borderColor);
    void setHoverBorderColor(std::string hoverBorderColor);

    void setInitialBackgroundColor(std::string backgroundColor);
    void setInitialHoverBackgroundColor(std::string hoverBackgroundColor);
    void setBackgroundColor(std::string backgroundColor);
    void setHoverBackgroundColor(std::string hoverBackgroundColor);
};