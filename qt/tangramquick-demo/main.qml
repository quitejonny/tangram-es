import QtQuick 2.3
import QtQuick.Window 2.2
import com.mapzen.tangram 1.0

Window {
    visible: true
    width: 800
    height: 600
    Tangram {
        anchors.fill: parent;
        sceneConfiguration:  "scene.yaml"
    }

    Text {
        anchors.centerIn: parent
        rotation: 45
        text: "Testing the scene graph"
        font.bold: true
        font.pixelSize: 30
        Timer {
            interval: 1000/60
            repeat: true
            ///running: true
            onTriggered: parent.rotation++
        }
    }
}

