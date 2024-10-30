#include "MapWindow.h"
#include <QQmlContext>

MapWindow::MapWindow(QWidget* parent) : QDialog(parent) {
    quickWidget = new QQuickWidget(this);
    quickWidget->setSource(QUrl(QStringLiteral("qrc:/map.qml")));
    setLayout(new QVBoxLayout);
    layout()->addWidget(quickWidget);
    setWindowTitle("Carte");
    resize(800, 600);
}

MapWindow::~MapWindow() {
    delete quickWidget;
}

void MapWindow::setCoordinates(double latitude, double longitude) {
    quickWidget->rootContext()->setContextProperty("latitude", latitude);
    quickWidget->rootContext()->setContextProperty("longitude", longitude);
}