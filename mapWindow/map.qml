import QtQuick 2.12
import QtLocation 5.12
import QtPositioning 5.12

Rectangle {
    width: 800
    height: 600

    property double latitude: 48.858844
    property double longitude: 2.294351

    Plugin {
        id: mapPlugin
        name: "osm"
    }

    Map {
        anchors.fill: parent
        plugin: mapPlugin
        center: QtPositioning.coordinate(latitude, longitude)
        zoomLevel: 14
    }
}