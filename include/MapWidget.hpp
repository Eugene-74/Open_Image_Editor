#pragma once

#include <QDialog>
#include <QGeoCoordinate>
#include <QLabel>
#include <QQmlComponent>
#include <QQuickItem>
#include <QQuickWidget>
#include <QVBoxLayout>
#include <QVariant>

#include "ImageData.hpp"

class MapWidget : public QDialog {
    Q_OBJECT
   public:
    MapWidget(QWidget* parent, ImageData* imageData);

    void setMapCenter(double latitude, double longitude);

    void moveMapPoint(double latitude, double longitude);
    void removeMapPoint();

   private slots:
    void onCoordinateValidated(double latitude, double longitude);

   private:
    QQuickWidget* quickWidget;
    ImageData* imageData;
};
