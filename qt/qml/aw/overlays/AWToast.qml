import QtQuick

// Single toast row; instantiated by AWToaster.
Item {
    id: root

    property string text: ""
    property string kind: "info"
    property int timeout: 4000

    readonly property real toastWidth: Math.min(360, root.parent ? root.parent.width - 2 * AWTheme.data["space.4"] : 360)

    implicitWidth: toastWidth
    implicitHeight: row.implicitHeight + 2 * AWTheme.data["space.3"]
    opacity: 0
    x: AWTheme.data["space.8"]

    function showToast() {
        opacity = 1
        x = 0
        dismissTimer.interval = root.timeout
        dismissTimer.restart()
        countdown.requestPaint()
        countdownAnim.restart()
    }

    Behavior on opacity { NumberAnimation { duration: AWTheme.data["motion.normal"]; easing.type: Easing.OutCubic } }
    Behavior on x { NumberAnimation { duration: AWTheme.data["motion.normal"]; easing.type: Easing.OutCubic } }

    Item {
        anchors.fill: parent

        AWShadow {
            anchors.fill: parent
            spec: AWTheme.data["shadow.2"]
            radius: AWTheme.data["radius.md"]
        }

        Rectangle {
            anchors.fill: parent
            radius: AWTheme.data["radius.md"]
            color: AWTheme.data["surface.3"]
            border.width: 1
            border.color: AWTheme.data["border.strong"]
        }
    }

    Row {
        id: row
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        anchors.leftMargin: AWTheme.data["space.3"]
        anchors.rightMargin: AWTheme.data["space.2"]
        spacing: AWTheme.data["space.2"]

        AWIcon {
            name: iconForKind()
            size: AWTheme.data["icon.size.default"]
            tint: tintForKind()
            anchors.verticalCenter: parent.verticalCenter

            function iconForKind() {
                switch (root.kind) {
                case "success": return "check"
                case "warning":
                case "danger": return "warning"
                default: return "info"
                }
            }

            function tintForKind() {
                switch (root.kind) {
                case "success": return AWTheme.data["status.success"]
                case "warning": return AWTheme.data["status.warning"]
                case "danger": return AWTheme.data["status.danger"]
                default: return AWTheme.data["interactive.default"]
                }
            }
        }

        Text {
            width: parent.width - AWTheme.data["space.10"]
            text: root.text
            color: AWTheme.data["text.primary"]
            font.pixelSize: AWTheme.data["typography.size.sm"]
            wrapMode: Text.Wrap
            anchors.verticalCenter: parent.verticalCenter
        }

        AWIconButton {
            iconName: "close"
            anchors.verticalCenter: parent.verticalCenter
            onClicked: root.destroy()
        }
    }

    Canvas {
        id: countdown
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: 2
        visible: false

        property real progress: 1.0

        onPaint: {
            var ctx = getContext("2d")
            ctx.reset()
            ctx.fillStyle = AWTheme.data["interactive.default"]
            ctx.fillRect(0, 0, width * progress, height)
        }

        onProgressChanged: requestPaint()
    }

    NumberAnimation {
        id: countdownAnim
        target: countdown
        property: "progress"
        from: 1
        to: 0
        duration: root.timeout
        easing.type: Easing.Linear
        onRunningChanged: {
            if (!running && root.opacity > 0) {
                root.opacity = 0
                destroyTimer.start()
            }
        }
    }

    Timer {
        id: dismissTimer
        onTriggered: {
            root.opacity = 0
            destroyTimer.start()
        }
    }

    Timer {
        id: destroyTimer
        interval: AWTheme.data["motion.normal"] + 50
        onTriggered: root.destroy()
    }
}
