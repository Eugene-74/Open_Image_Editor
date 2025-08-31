
#include "MapWidget.hpp"

#include <QGeoCoordinate>
#include <QQuickItem>
#include <QQuickView>
#include <QUrl>
#include <QVBoxLayout>
#include <QWidget>
#include <cmath>
#include <vector>

#include "Const.hpp"

/**
 * @brief Calculate the distance between two geographic coordinates
 *
 * @param a First coordinate
 * @param b Second coordinate
 * @return Distance between the two coordinates
 */
static double distanceCoords(const QGeoCoordinate& a, const QGeoCoordinate& b) {
    double dLat = a.latitude() - b.latitude();
    double dLon = a.longitude() - b.longitude();
    return std::sqrt(dLat * dLat + dLon * dLon);
}

/**
 * @brief Set the list of coordinates to be displayed on the map
 *
 * @param coords List of geographic coordinates
 * @param clusterDistance Distance threshold for clustering points
 */
void MapWidget::setCoordinatesList(const std::vector<QGeoCoordinate>& selectedCoords, const std::vector<QGeoCoordinate>& notSelectedCoords, double clusterDistance) {
    if (!quickView) return;
    QQuickItem* rootItem = quickView->rootObject();
    if (!rootItem) return;
    QQuickItem* mapItem = rootItem->findChild<QQuickItem*>("mapView");
    if (!mapItem) return;

    QMetaObject::invokeMethod(mapItem, "removeAllPoints");

    struct Cluster {
        QGeoCoordinate center;
        int count;
    };
    std::vector<Cluster> clusters;

    // Add blue points (not selected)
    for (const auto& coord : notSelectedCoords) {
        bool found = false;
        for (auto& cluster : clusters) {
            if (distanceCoords(cluster.center, coord) < clusterDistance) {
                cluster.center.setLatitude((cluster.center.latitude() * cluster.count + coord.latitude()) / (cluster.count + 1));
                cluster.center.setLongitude((cluster.center.longitude() * cluster.count + coord.longitude()) / (cluster.count + 1));
                cluster.count++;
                found = true;
                break;
            }
        }
        if (!found) {
            clusters.push_back({coord, 1});
        }
    }

    if (clusters.size() > 100) {
        std::partial_sort(clusters.begin(), clusters.begin() + 100, clusters.end(), [](const Cluster& a, const Cluster& b) {
            return a.count > b.count;
        });
        clusters.resize(100);
    }

    for (const auto& cluster : clusters) {
        QVariant coordVar = QVariant::fromValue(cluster.center);
        QMetaObject::invokeMethod(mapItem, "addPointForOthers", Q_ARG(QVariant, coordVar), Q_ARG(QVariant, QVariant(QStringLiteral("otherPin"))));
    }

    clusters.clear();
    // Add red points (selected)
    for (const auto& coord : selectedCoords) {
        bool found = false;
        for (auto& cluster : clusters) {
            if (distanceCoords(cluster.center, coord) < clusterDistance) {
                cluster.center.setLatitude((cluster.center.latitude() * cluster.count + coord.latitude()) / (cluster.count + 1));
                cluster.center.setLongitude((cluster.center.longitude() * cluster.count + coord.longitude()) / (cluster.count + 1));
                cluster.count++;
                found = true;
                break;
            }
        }
        if (!found) {
            clusters.push_back({coord, 1});
        }
    }

    if (clusters.size() > 100) {
        std::partial_sort(clusters.begin(), clusters.begin() + 100, clusters.end(), [](const Cluster& a, const Cluster& b) {
            return a.count > b.count;
        });
        clusters.resize(100);
    }

    for (const auto& cluster : clusters) {
        QVariant coordVar = QVariant::fromValue(cluster.center);
        QMetaObject::invokeMethod(mapItem, "addPointForOthers", Q_ARG(QVariant, coordVar), Q_ARG(QVariant, QVariant(QStringLiteral("pin"))));
    }
}

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

/**
 * @brief Add a map point for other images
 *
 * @param latitude Latitude of the map point
 * @param longitude Longitude of the map point
 */
void MapWidget::addMapPointForOthers(double latitude, double longitude, const QString& pinFile) {
    if (!quickView)
        return;

    QQuickItem* rootItem = quickView->rootObject();
    if (!rootItem)
        return;

    QQuickItem* mapItem = rootItem->findChild<QQuickItem*>("mapView");
    if (!mapItem)
        return;

    QGeoCoordinate pointCoordinate(latitude, longitude);

    QMetaObject::invokeMethod(mapItem, "addPointForOthers",
                              Q_ARG(QVariant, QVariant::fromValue(pointCoordinate)),
                              Q_ARG(QVariant, QVariant(pinFile)));
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

/**
 * @brief Remove all points from the map
 */
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