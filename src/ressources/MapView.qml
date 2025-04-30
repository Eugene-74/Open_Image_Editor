import QtQuick
import QtQuick.Controls
import QtLocation
import QtPositioning

Item {
    width: 800
    height: 600

    Plugin {
        id: mapPlugin
        name: "osm" // Utilise OpenStreetMap

    }

    Map {
        id: map
        objectName: "mapView"
        anchors.fill: parent
        plugin: mapPlugin

        zoomLevel: 14
        center: QtPositioning.coordinate(48.8566, 2.3522) // Coordonnées de Paris


        MouseArea {
            anchors.fill: parent
            drag.target: null // Désactive le comportement par défaut du drag
            property bool isDragging: false
            property var lastMousePosition: Qt.point(0, 0)

            onPressed: handleMousePressed(mouse)
            onReleased: handleMouseReleased()
            onPositionChanged: handleMousePositionChanged(mouse)
            onClicked: handleMouseClicked(mouse)
            onWheel: handleMouseWheel(wheel)

            function handleMousePressed(mouse) {
                isDragging = true;
                lastMousePosition = Qt.point(mouse.x, mouse.y);
            }

            function handleMouseReleased() {
                isDragging = false;
            }

            function handleMousePositionChanged(mouse) {
                if (isDragging) {
                    var dx = mouse.x - lastMousePosition.x;
                    var dy = mouse.y - lastMousePosition.y;
                    lastMousePosition = Qt.point(mouse.x, mouse.y);

                    // Déplace la carte en fonction du déplacement de la souris
                    var newCenter = map.toCoordinate(Qt.point(width / 2 - dx, height / 2 - dy));
                    map.center = newCenter;
                }
            }

            function handleMouseClicked(mouse) {
                if (!isDragging) {
                    var coord = map.toCoordinate(Qt.point(mouse.x, mouse.y));
                    map.addPoint(coord); // Ajoute un point si ce n'est pas un drag
                }
            }

            function handleMouseWheel(wheel) {
                if (wheel.angleDelta.y > 0) {
                    map.zoomLevel += 1;
                } else {
                    map.zoomLevel -= 1;
                }
            }
        }

        function addPoint(coordinate) {
            var marker = Qt.createQmlObject(
                'import QtQuick; import QtLocation; import QtPositioning; MapQuickItem { coordinate: QtPositioning.coordinate(' + coordinate[0] + ', ' + coordinate[1] + '); anchorPoint.x: 12; anchorPoint.y: 12; sourceItem: Rectangle { width: 24; height: 24; color: "red"; border.color: "black"; border.width: 2; radius: 12; } }',
                map
            );
            map.addMapItem(marker);
        }
    }
}