#pragma once

#include <QDialog>
#include <QGeoCoordinate>
#include <QLabel>
#include <QQmlComponent>
#include <QQuickItem>
#include <QQuickView>
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

    void addMapPointForOthers(double latitude, double longitude);

    void setImageData(ImageData* imageData);

   private:
    QQuickView* quickView;
    ImageData* imageData;

   private slots:
    void onCoordinateValidated(double latitude, double longitude);

   protected:
    void reject() override;
};
