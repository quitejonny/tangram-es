import QtQuick 2.3
import QtQuick.Window 2.2
import com.mapzen.tangram 1.0

Window {
    visible: true
    width: 800
    height: 600

    ShaderEffect {
        id: tileBackground
        anchors.fill: parent

        property real tileSize: 16
        property color color1: Qt.rgba(0.9, 0.9, 0.9, 1);
        property color color2: Qt.rgba(0.85, 0.85, 0.85, 1);

        property size pixelSize: Qt.size(width / tileSize, height / tileSize);

        fragmentShader:
            "
            uniform lowp vec4 color1;
            uniform lowp vec4 color2;
            uniform highp vec2 pixelSize;
            varying highp vec2 qt_TexCoord0;
            void main() {
                highp vec2 tc = sign(sin(3.14152 * qt_TexCoord0 * pixelSize));
                if (tc.x != tc.y)
                    gl_FragColor = color1;
                else
                    gl_FragColor = color2;
            }
            "
    }


    Tangram {
        id: map
        focus: true
        anchors.fill: parent;
        sceneConfiguration:  "scene.yaml"
        zoom: 16
        tilt: 0
        continuousRendering: true

        Behavior on heading {
            NumberAnimation { duration: 1000; easing.type: Easing.Linear }
        }
        Timer {
            interval: 1000
            repeat: true
            running: true
            onTriggered: parent.heading+=20
        }

        transform: [
            Rotation { id: rotation; axis.x: 0; axis.z: 0; axis.y: 1; angle: 0; origin.x: map.width / 2; origin.y: map.height / 2; },
            Translate { id: txOut; x: -map.width / 2; y: -map.height / 2 },
            Scale { id: scale; },
            Translate { id: txIn; x: map.width / 2; y: map.height / 2 }
        ]

        Keys.onPressed: {
            if (event.key === Qt.Key_P) {
                queueSceneUpdate("cameras", "{ main_camera: { type: perspective } }")
                applySceneUpdates()
            } else if (event.key === Qt.Key_I) {
                queueSceneUpdate("cameras", "{ main_camera: { type: isometric } }")
                applySceneUpdates()
            }
        }
    }

    // Just to show something interesting
    SequentialAnimation {
        id:ani
        PauseAnimation { duration: 2000 }
        ParallelAnimation {
            NumberAnimation { target: scale; property: "xScale"; to: 0.6; duration: 2000; easing.type: Easing.InOutBack }
            NumberAnimation { target: scale; property: "yScale"; to: 0.6; duration: 2000; easing.type: Easing.InOutBack }
        }
        NumberAnimation { target: rotation; property: "angle"; to: 80; duration: 1000; easing.type: Easing.InOutCubic }
        NumberAnimation { target: rotation; property: "angle"; to: -80; duration: 1000; easing.type: Easing.InOutCubic }
        NumberAnimation { target: rotation; property: "angle"; to: 0; duration: 1000; easing.type: Easing.InOutCubic }
        NumberAnimation { target: map; property: "opacity"; to: 0.3; duration: 1000; easing.type: Easing.InOutCubic }
        PauseAnimation { duration: 1000 }
        NumberAnimation { target: map; property: "opacity"; to: 1.0; duration: 1000; easing.type: Easing.InOutCubic }
        ParallelAnimation {
            NumberAnimation { target: scale; property: "xScale"; to: 1; duration: 1000; easing.type: Easing.InOutBack }
            NumberAnimation { target: scale; property: "yScale"; to: 1; duration: 1000; easing.type: Easing.InOutBack }
        }
        NumberAnimation { target: map; property: "zoom"; to: 14; duration: 2000; easing.type: Easing.InOutBack }
        NumberAnimation { target: map; property: "zoom"; to: 16; duration: 1000; easing.type: Easing.InOutBack }
        NumberAnimation { target: map; property: "tilt"; to: 45; duration: 2000; easing.type: Easing.InOutBack }
        NumberAnimation { target: map; property: "tilt"; to: 0; duration: 1000; easing.type: Easing.InOutBack }

        running: true
        loops: Animation.Infinite
    }
}

