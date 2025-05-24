#pragma once

#include <QLabel>
#include <QSize>

#include "Const.hpp"

// Forward declaration
class QMouseEvent;
class Data;

class MainImage : public QLabel {
    Q_OBJECT

   public:
    explicit MainImage(std::shared_ptr<Data> data, const QString& i, QSize size = QSize(0, 0), bool setSize = true, bool personsEditor = false, bool square = false, bool force = false);

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
    void mouseDoubleClickEvent(QMouseEvent* event) override;

    void paintEvent(QPaintEvent* event) override;

   private:
    bool inFace(QPoint point);

    bool personsEditor = false;

    QString imagePath;
    QSize mSize;
    bool setSize;
    int thumbnail;
    bool square;
    bool force;

    std::shared_ptr<Data> data;

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
        //    TODO make private
        void updateStyleSheet();

        QString border_color = QString::fromStdString(Const::Color::TRANSPARENT1);
        QString hover_border_color = QString::fromStdString(Const::Color::TRANSPARENT1);

        QString background_color = QString::fromStdString(Const::Color::TRANSPARENT1);
        QString hover_background_color = QString::fromStdString(Const::Color::TRANSPARENT1);
};
