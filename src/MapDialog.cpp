#include "MapDialog.hpp"

MapDialog::MapDialog(QWidget* parent, ImageData* imageData) : QDialog(parent), imageData(imageData) {
    setWindowTitle("Carte");
    resize(600, 400);

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

void MapDialog::setMapCenter(double latitude, double longitude) {
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

void MapDialog::moveMapPoint(double latitude, double longitude) {
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

void MapDialog::onCoordinateValidated(double latitude, double longitude) {
    qDebug() << "Coordonnées validées : Latitude =" << latitude << ", Longitude =" << longitude;
    imageData->setLatitude(latitude);
    imageData->setLongitude(longitude);
}