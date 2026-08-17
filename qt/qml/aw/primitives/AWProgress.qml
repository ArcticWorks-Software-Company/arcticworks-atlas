import QtQuick

// Determinate/indeterminate progress bar.
Item {
    id: root

    property real from: 0
    property real to: 1
    property real value: 0
    property bool indeterminate: false

    implicitHeight: AWTheme.data["space.1"]
    implicitWidth: 240

    Rectangle {
        anchors.fill: parent
        radius: height / 2
        color: AWTheme.data["border.subtle"]
    }

    Rectangle {
        id: fill
        radius: height / 2
        height: parent.height
        color: AWTheme.data["interactive.default"]
        width: root.indeterminate ? parent.width * 0.3 : parent.width * progress()

        function progress() {
            if (root.to <= root.from)
                return 0
            return Math.max(0, Math.min(1, (root.value - root.from) / (root.to - root.from)))
        }

        Behavior on width { NumberAnimation { duration: AWTheme.data["motion.fast"]; easing.type: Easing.OutCubic } }

        SequentialAnimation on x {
            running: root.indeterminate && root.visible
            loops: Animation.Infinite
            NumberAnimation { from: 0; to: root.width * 0.7; duration: AWTheme.data["motion.normal"] * 3; easing.type: Easing.OutCubic }
            NumberAnimation { to: root.width * 0.7; duration: AWTheme.data["motion.normal"] * 3; easing.type: Easing.OutCubic }
        }
    }
}
