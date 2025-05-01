#include "MapWidget.hpp"

/**
 * @brief Constructor for the MapWidget class
 * @param parent Pointer to the parent QWidget (usually the main window)
 * @param imageData Pointer to the ImageData object containing image metadata
 */
MapWidget::MapWidget(QWidget* parent, ImageData* imageData) : QDialog(parent), imageData(imageData) {
    QVBoxLayout* layout = new QVBoxLayout(this);

    quickWidget = new QQuickWidget(this);
    quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    quickWidget->setSource(QUrl(QStringLiteral("qrc:/MapView.qml")));

    layout->addWidget(quickWidget);

    qRegisterMetaType<QGeoCoordinate>("QGeoCoordinate");

    QQuickItem* rootItem = quickWidget->rootObject();
    if (rootItem) {
        QObject::connect(rootItem, SIGNAL(coordinateValidated(double, double)),
                         this, SLOT(onCoordinateValidated(double, double)));
    }
}

/**
 * @brief Set the center of the map
 * @param latitude Latitude of the center point
 * @param longitude Longitude of the center point
 */
void MapWidget::setMapCenter(double latitude, double longitude) {
    if (!quickWidget)
        return;

    QQuickItem* rootItem = quickWidget->rootObject();
    if (!rootItem)
        return;

    QQuickItem* mapItem = rootItem->findChild<QQuickItem*>("mapView");
    if (!mapItem)
        return;

    QGeoCoordinate newCenter(latitude, longitude);
    mapItem->setProperty("center", QVariant::fromValue(newCenter));
}

/**
 * @brief Move the map point to a new location
 * @param latitude Latitude of the new point
 * @param longitude Longitude of the new point
 */
void MapWidget::moveMapPoint(double latitude, double longitude) {
    if (!quickWidget)
        return;

    QQuickItem* rootItem = quickWidget->rootObject();
    if (!rootItem)
        return;

    QQuickItem* mapItem = rootItem->findChild<QQuickItem*>("mapView");
    if (!mapItem)
        return;

    QVariantList point;
    point << latitude << longitude;

    QGeoCoordinate pointCoordinate(latitude, longitude);

    QMetaObject::invokeMethod(mapItem, "addPoint", Q_ARG(QVariant, QVariant::fromValue(pointCoordinate)));
}

void MapWidget::removeMapPoint() {
    if (!quickWidget)
        return;

    QQuickItem* rootItem = quickWidget->rootObject();
    if (!rootItem)
        return;

    QQuickItem* mapItem = rootItem->findChild<QQuickItem*>("mapView");
    if (!mapItem)
        return;

    QMetaObject::invokeMethod(mapItem, "removePoint");
}

/**
 * @brief Slot to handle coordinate validation
 * @param latitude Latitude of the validated point
 * @param longitude Longitude of the validated point
 */
void MapWidget::onCoordinateValidated(double latitude, double longitude) {
    imageData->getMetaData().setLatitude(latitude);
    imageData->getMetaData().setLongitude(longitude);
    imageData->saveMetaData();
}