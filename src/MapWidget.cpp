#include "MapWidget.hpp"

#include <QGeoCoordinate>
#include <QQuickItem>
#include <QQuickView>
#include <QUrl>
#include <QVBoxLayout>
#include <QWidget>

#include "Const.hpp"

/**
 * @brief Constructor for the MapWidget class
 * @param parent Pointer to the parent QWidget (usually the main window)
 * @param imageData Pointer to the ImageData object containing image metadata
 */
MapWidget::MapWidget(QWidget* parent, std::function<void(double, double)> validate) : QDialog(parent), validate(validate) {
    QVBoxLayout* layout = new QVBoxLayout(this);

    quickView = new QQuickView();
    quickView->setResizeMode(QQuickView::SizeRootObjectToView);
    quickView->setSource(QUrl(QStringLiteral("qrc:/MapView.qml")));
    QWidget* container = QWidget::createWindowContainer(quickView, this);
    container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    container->setMinimumSize(0, 0);

    layout->addWidget(container);

    QQuickItem* rootItem = quickView->rootObject();
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
void MapWidget::setMapCenter(double latitude, double longitude, double zoom) {
    if (!quickView)
        return;

    QQuickItem* rootItem = quickView->rootObject();
    if (!rootItem)
        return;

    QQuickItem* mapItem = rootItem->findChild<QQuickItem*>("mapView");
    if (!mapItem)
        return;

    QGeoCoordinate newCenter(latitude, longitude);
    mapItem->setProperty("center", QVariant::fromValue(newCenter));

    QMetaObject::invokeMethod(mapItem, "setZoomLevel", Q_ARG(QVariant, QVariant::fromValue(zoom)));
}

/**
 * @brief Move the map point to a new location
 * @param latitude Latitude of the new point
 * @param longitude Longitude of the new point
 */
void MapWidget::moveMapPoint(double latitude, double longitude) {
    if (!quickView)
        return;

    QQuickItem* rootItem = quickView->rootObject();
    if (!rootItem)
        return;

    QQuickItem* mapItem = rootItem->findChild<QQuickItem*>("mapView");
    if (!mapItem)
        return;

    QGeoCoordinate pointCoordinate(latitude, longitude);

    QMetaObject::invokeMethod(mapItem, "setInitialPoint", Q_ARG(QVariant, QVariant::fromValue(pointCoordinate)));
}

/**
 * @brief Remove the map point
 */
void MapWidget::removeMapPoint() {
    if (!quickView)
        return;

    QQuickItem* rootItem = quickView->rootObject();
    if (!rootItem)
        return;

    QQuickItem* mapItem = rootItem->findChild<QQuickItem*>("mapView");
    if (!mapItem)
        return;

    QMetaObject::invokeMethod(mapItem, "removePoint");
}

void MapWidget::addMapPointForOthers(double latitude, double longitude) {
    if (!quickView)
        return;

    QQuickItem* rootItem = quickView->rootObject();
    if (!rootItem)
        return;

    QQuickItem* mapItem = rootItem->findChild<QQuickItem*>("mapView");
    if (!mapItem)
        return;

    QGeoCoordinate pointCoordinate(latitude, longitude);

    QMetaObject::invokeMethod(mapItem, "addPointForOthers", Q_ARG(QVariant, QVariant::fromValue(pointCoordinate)));
}

/**
 * @brief Slot to handle coordinate validation
 * @param latitude Latitude of the validated point
 * @param longitude Longitude of the validated point
 */
void MapWidget::onCoordinateValidated(double latitude, double longitude) {
    validate(latitude, longitude);
}

/**
 * @brief Set the image data for the map widget
 * @param imageData Pointer to the ImageData object containing image metadata
 */
void MapWidget::setImageData(ImageData* imageData) {
    this->imageData = imageData;
}

void MapWidget::reject() {
    // Do nothing (normal)
}

void MapWidget::removeAllPoints() {
    if (!quickView)
        return;

    QQuickItem* rootItem = quickView->rootObject();
    if (!rootItem)
        return;

    QQuickItem* mapItem = rootItem->findChild<QQuickItem*>("mapView");
    if (!mapItem)
        return;

    QMetaObject::invokeMethod(mapItem, "removeAllPoints");
}