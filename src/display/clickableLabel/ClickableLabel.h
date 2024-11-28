#pragma once

#include <QLabel>
#include <opencv2/opencv.hpp>
#include <QFileDialog>
#include <QMouseEvent>
#include "../../Const.h"
#include "../../structure/data/Data.h"
#include "../../functions/verification/Verification.h"




class ClickableLabel : public QLabel {
    Q_OBJECT

public:
    explicit ClickableLabel(Data* data, const QString& i, QWidget* parent = nullptr, QSize size = QSize(0, 0), bool setSize = true, int thumbnail = 0);

signals:
    void clicked();  // Signal émis lors du clic

protected:
    void enterEvent(QEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;   // Override mouse press event
    void mouseReleaseEvent(QMouseEvent* event) override;  // Override mouse release event





private:
    QPixmap defaultPixmap;


    int border = 0;
    int border_radius = 5;

public:
    void updateStyleSheet();

    QString border_color = "transparent"; //#b3b3b3
    QString hover_border_color = "#9c9c9c";


    QString background_color = "transparent";
    QString hover_background_color = "#b3b3b3";
};