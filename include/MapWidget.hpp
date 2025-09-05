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
    // Affiche une liste de coordonnées sur la carte, en regroupant les points proches et en limitant à 100 groupes
    Q_OBJECT
   public:
    MapWidget(QWidget* parent, std::function<void(double, double)> validate);

    void setMapCenter(double latitude, double longitude, double zoom = 14);

    void moveMapPoint(double latitude, double longitude);
    void removeMapPoint();

    void addMapPointForOthers(double latitude, double longitude, const QString& pinFile = "pin");

    void setImageData(ImageData* imageData);

    void removeAllPoints();

    void setCoordinatesList(const std::vector<QGeoCoordinate>& selectedCoords, const std::vector<QGeoCoordinate>& notSelectedCoords, double clusterDistance = 0.0005);

   private:
    std::unique_ptr<QQuickView> quickView;
    ImageData* imageData = nullptr;
    std::function<void(double, double)> validate;
   private slots:
    void onCoordinateValidated(double latitude, double longitude);

   protected:
    void reject() override;
};
