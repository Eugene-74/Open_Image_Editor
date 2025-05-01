import QtQuick
import QtQuick.Controls
import QtLocation
import QtPositioning

Item {
    width: 800
    height: 600

    signal coordinateValidated(double latitude, double longitude)

    Plugin {
        id: mapPlugin
        name: "osm" // Utilise OpenStreetMap
    }

    property var currentMarker: null
    property var currentCoordinate: null

    Map {
        id: map
        objectName: "mapView"
        anchors.fill: parent
        plugin: mapPlugin

        zoomLevel: 14
        center: QtPositioning.coordinate(48.8566, 2.3522) // Coordonnées de Paris

        MouseArea {
            anchors.fill: parent
            drag.target: null
            property bool isDragging: false
            property bool mouseMoved: false
            property var lastMousePosition: Qt.point(0, 0)

            onPressed: handleMousePressed(mouse)
            onReleased: handleMouseReleased(mouse)
            onPositionChanged: handleMousePositionChanged(mouse)

            function handleMousePressed(mouse) {
                isDragging = true;
                mouseMoved = false;
                lastMousePosition = Qt.point(mouse.x, mouse.y);
            }

            function handleMouseReleased(mouse) {
                isDragging = false;
                if (!mouseMoved) {
                    // Si la souris n'a pas bougé, c'est un clic simple
                    var coord = map.toCoordinate(Qt.point(mouse.x, mouse.y));
                    console.log("Mouse clicked at:", mouse.x, mouse.y, "Converted to coordinate:", coord);
                    map.addPoint(coord);
                }
            }

            function handleMousePositionChanged(mouse) {
                if (isDragging) {
                    var dx = mouse.x - lastMousePosition.x;
                    var dy = mouse.y - lastMousePosition.y;

                    // Si la souris bouge, marquez comme "mouseMoved"
                    if (Math.abs(dx) > 5 || Math.abs(dy) > 5) {
                        mouseMoved = true;
                    }

                    lastMousePosition = Qt.point(mouse.x, mouse.y);

                    // Déplace la carte en fonction du déplacement de la souris
                    var newCenter = map.toCoordinate(Qt.point(width / 2 - dx, height / 2 - dy));
                    map.center = newCenter;
                }
            }

            onWheel: handleMouseWheel(wheel)

            function handleMouseWheel(wheel) {
                if (wheel.angleDelta.y > 0) {
                    map.zoomLevel += 1;
                } else {
                    map.zoomLevel -= 1;
                }
            }
        }

        function addPoint(coordinate) {
            console.log("Adding point at:", coordinate.latitude, coordinate.longitude);

            if (currentMarker) {
                map.removeMapItem(currentMarker);
                currentMarker.destroy();
            }

            currentMarker = Qt.createQmlObject(
                'import QtQuick; import QtLocation ; import QtPositioning; MapQuickItem { coordinate: QtPositioning.coordinate(' + coordinate.latitude + ', ' + coordinate.longitude + '); anchorPoint.x: 24; anchorPoint.y: 48; sourceItem: Image { source: "qrc:/images/pin.png"; width: 48; height: 48; } }',
                map
            );
            map.addMapItem(currentMarker);

            currentCoordinate = coordinate;

            validateButton.visible = true;
            cancelButton.visible = true;
        }
    }

    Button {
        id: cancelButton
        text: "Annuler"
        visible: false
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.margins: 10
        onClicked: {
            if (currentMarker) {
                map.removeMapItem(currentMarker);
                currentMarker.destroy();
            }
            currentMarker = null;
            currentCoordinate = null;
            validateButton.visible = false;
            cancelButton.visible = false;
        }
    }

    Button {
        id: validateButton
        text: "OK"
        visible: false
        anchors.bottom: parent.bottom
        anchors.right: cancelButton.left
        anchors.margins: 10
        onClicked: {
            console.log("Point validé:", currentCoordinate.latitude, currentCoordinate.longitude);
            coordinateValidated(currentCoordinate.latitude, currentCoordinate.longitude);
            validateButton.visible = false;
            cancelButton.visible = false;
        }
    }
}