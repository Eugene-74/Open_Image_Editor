#pragma once

#include <QLabel>
#include <QSize>

#include "Const.hpp"
#include "ImageLabel.hpp"

// Forward declaration
class QMouseEvent;
class Data;

class MainImage : public ImageLabel {
    Q_OBJECT
    using ImageLabel::ImageLabel;

   public:
    explicit MainImage(std::shared_ptr<Data> data, const QString& imagePath, QSize size, int thumbnail = Const::Thumbnail::NORMAL_QUALITY, bool personsEditor = false);

    bool getPersonsEditorConst() const;
    void setPersonsEditor(bool personsEditor);

   signals:
    void clicked();
    void leftClicked();
    void ctrlLeftClicked();
    void imageCropted();

   protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

    void mouseMoveEvent(QMouseEvent* event) override;

    void paintEvent(QPaintEvent* event) override;

   private:
    bool inFace(QPoint point);
    QImage qImage;

    bool personsEditor = false;
    QString imagePath;
    int thumbnail;
    std::shared_ptr<Data> data;

    bool cropping;
    bool drawingRectangle = false;

    QPoint cropStart = QPoint(-1, -1);
    QPoint cropEnd = QPoint(-1, -1);

    void cropImage();
    std::vector<QPoint> adjustPointsForOrientation(const std::vector<QPoint>& points, int orientation, QSize imageSize);

    int lastSelectedFaceIndex = -1;
};
