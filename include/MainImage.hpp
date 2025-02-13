#pragma once

#include <QImageIOHandler>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QResource>
#include <QTemporaryFile>
#include <opencv2/imgproc.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/opencv.hpp>

#include "Box.hpp"
#include "Const.hpp"
#include "Data.hpp"
#include "FaceRecognition.hpp"
#include "ImageEditor.hpp"

class ImageEditor;

class MainImage : public QLabel {
    Q_OBJECT

   public:
    void detectFaces();
    // std::vector<cv::Rect> faces;
    std::vector<Person> persons;

    explicit MainImage(Data* data, const QString& i, ImageEditor* parent = nullptr, QSize size = QSize(0, 0), bool setSize = true, int thumbnail = 0, bool square = false, bool force = false);

   signals:
    void clicked();
    void leftClicked();
    void ctrlLeftClicked();
    void imageCropted();

   protected:
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

    void mouseMoveEvent(QMouseEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

   private:
    ImageEditor* parent;

    QString imagePath;
    QSize mSize;
    bool setSize;
    int thumbnail;
    bool square;
    bool force;

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
