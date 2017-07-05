import QtQuick 2.3
import QtQuick.Window 2.2
//import QtLocation 5.2
import QtPositioning 5.2
import com.mapzen.tangram 1.0

Window {
    visible: true
    width: 800
    height: 600

    property var centerCoordinate: QtPositioning.coordinate(40.7053, -74.0098)

    TangramMap {
        id: map
        focus: true
        anchors.fill: parent;
        sceneConfiguration: "scenes/scene.yaml"
        zoomLevel: 16
        center: centerCoordinate
        tilt: 0

        TangramPolyline {
            id: line
            path: [
                centerCoordinate,
                QtPositioning.coordinate(centerCoordinate.latitude + 0.07, centerCoordinate.longitude)
            ]
            styling: {
                "color": "#50FF0000",
                width: 5,
                style: "heightglowline"
            }
        }

        TangramPoint {
            id: point
            coordinate: centerCoordinate
            styling: {
                "size": ["60px", "60px"],
                color: "#88ffffff",
                //style: "icons",
                collide: false
            }

            draggable: true
            clickable: true

            onClicked: {
                console.debug("icon was clicked.")
            }
        }

        TangramPoint {
            id: secondPoint
            coordinate: centerCoordinate
            styling: {
                "size": "20px",
                color: "#00ffff",
                //style: "icons",
                collide: false
            }

            draggable: true
            clickable: true

            onClicked: {
                console.debug("icon was clicked.")
            }
        }

        Keys.onPressed: {
            if (event.key === Qt.Key_P) {
                queueSceneUpdate("cameras", "{ main_camera: { type: perspective } }")
                applySceneUpdates()
            } else if (event.key === Qt.Key_I) {
                queueSceneUpdate("cameras", "{ main_camera: { type: isometric } }")
                applySceneUpdates()
            } else if (event.key === Qt.Key_J) {
                center = QtPositioning.coordinate(51.5099, -0.1171)
            } else if (event.key === Qt.Key_F) {
                if (line.line.width === 5)
                    line.line.width = 10
                else
                    line.line.width = 5
            }
        }
    }
}

