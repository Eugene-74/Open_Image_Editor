#pragma once

#include <QLabel>
// #include <opencv2/opencv.hpp>
// #include <QFileDialog>
#include <QMouseEvent>
#include <QPainter>
#include <QImageIOHandler>

#include "../../Const.h"
#include "../../structure/data/Data.h"


class MainImage : public QLabel {
    Q_OBJECT

public:
    explicit MainImage(Data* data, const QString& i, QWidget* parent = nullptr, QSize size = QSize(0, 0), bool setSize = true, int thumbnail = 0, bool square = false);

signals:
    void clicked();
    void leftClicked();
    void ctrlLeftClicked();


protected:
    void enterEvent(QEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

    void mouseMoveEvent(QMouseEvent* event) override;
    void paintEvent(QPaintEvent* event) override;


private:
    Data* data;

    QPixmap defaultPixmap;

    int border = 0;
    int border_radius = 5;

    QImage qImage;
    bool cropping;
    bool drawingRectangle = false;

    QPoint cropStart = QPoint(-1, -1);
    QPoint cropEnd = QPoint(-1, -1);

    void cropImage();
    std::vector<QPoint> adjustPointsForOrientation(const std::vector<QPoint>& points, int orientation, QSize imageSize);



public:
    void updateStyleSheet();

    QString border_color = "transparent";
    QString hover_border_color = "transparent";


    QString background_color = "transparent";
    QString hover_background_color = "transparent";
};

