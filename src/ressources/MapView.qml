import QtQuick
import QtQuick.Controls
import QtLocation
import QtPositioning



Item {
    signal coordinateValidated(double latitude, double longitude)

    Plugin {
        id: mapPlugin
        name: "osm" // OpenStreetMap
    }

    property var currentMarker: null
    property var currentCoordinate: null

    property var lastCoordinate: null


    Map {
        id: map
        objectName: "mapView"
        anchors.fill: parent
        plugin: mapPlugin

        zoomLevel: 14
        center: QtPositioning.coordinate(48.8566, 2.3522) //    Paris

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
                    var coord = map.toCoordinate(Qt.point(mouse.x, mouse.y));
        
                    validateButton.visible = true;
                    cancelButton.visible = true;
                    map.addPoint(coord);
                }
            }

            function handleMousePositionChanged(mouse) {
                if (isDragging) {
                    var dx = mouse.x - lastMousePosition.x;
                    var dy = mouse.y - lastMousePosition.y;

                    if (Math.abs(dx) > 5 || Math.abs(dy) > 5) {
                        mouseMoved = true;
                    }

                    lastMousePosition = Qt.point(mouse.x, mouse.y);

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
        }

        function removeAllPoints() {
            var itemsToRemove = [];
            for (var i = 0; i < map.mapItems.length; ++i) {
                var item = map.mapItems[i];
                if (item && item.destroy) {
                    itemsToRemove.push(item);
                }
            }
            for (var j = 0; j < itemsToRemove.length; ++j) {
                map.removeMapItem(itemsToRemove[j]);
                itemsToRemove[j].destroy();
            }
            currentMarker = null;
            currentCoordinate = null;
        }

        function setInitialPoint(coordinate) {
            lastCoordinate = coordinate;
            addPoint(coordinate)

        }

        function addPointForOthers(coordinate) {
            var marker = Qt.createQmlObject(
                'import QtQuick; import QtLocation ; import QtPositioning; MapQuickItem { coordinate: QtPositioning.coordinate(' + coordinate.latitude + ', ' + coordinate.longitude + '); anchorPoint.x: 24; anchorPoint.y: 48; sourceItem: Image { source: "qrc:/images/otherPin.png"; width: 48; height: 48; } }',
                map
            );
            map.addMapItem(marker);
        }

        function removePoint() {
            console.log("Removing point");

            if (currentMarker) {
                map.removeMapItem(currentMarker);
                currentMarker.destroy();
            }
            validateButton.visible = false;
            cancelButton.visible = false;
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
            if(lastCoordinate){
                map.addPoint(lastCoordinate);
                map.center(lastCoordinate);
            }


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
            coordinateValidated(currentCoordinate.latitude, currentCoordinate.longitude);
            validateButton.visible = false;
            cancelButton.visible = false;
        }
    }
}