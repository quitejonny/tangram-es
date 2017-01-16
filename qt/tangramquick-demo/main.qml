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
        sceneConfiguration: "scene.yaml"
        zoomLevel: 16
        center: centerCoordinate
        tilt: 0
        continuousRendering: true

        TangramPolyline {
            id: line
            path: [
                centerCoordinate,
                QtPositioning.coordinate(centerCoordinate.latitude + 0.07, centerCoordinate.longitude)
            ]
            line.color: "#50FF0000"
            line.width: 5
            line.style: "heightglowline"
        }

        TangramPoint {
            id: point
            coordinate: centerCoordinate
            visual.size: "32px"
            visual.style: "icons"
            draggable: true
            clickable: true
            Component.onCompleted: {
                visual.setStyling("sprite", "train")
            }
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

