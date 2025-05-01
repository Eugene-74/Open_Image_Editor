#pragma once

#include <QDialog>
#include <QGeoCoordinate>
#include <QQmlComponent>
#include <QQuickItem>
#include <QQuickWidget>
#include <QVBoxLayout>
#include <QVariant>

#include "ImageData.hpp"

class MapDialog : public QDialog {
    Q_OBJECT
   public:
    MapDialog(QWidget* parent, ImageData* imageData);

    void setMapCenter(double latitude, double longitude);

    void moveMapPoint(double latitude, double longitude);

   private slots:
    void onCoordinateValidated(double latitude, double longitude);

   private:
    QQuickWidget* quickWidget;
    ImageData* imageData;
};
