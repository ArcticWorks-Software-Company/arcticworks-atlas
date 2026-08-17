import QtQuick

// Circular spinner: infinite rotation, interactive.default arc.
Item {
    id: root

    property real size: AWTheme.data["control.height"] * 0.5

    implicitWidth: size
    implicitHeight: size

    Canvas {
        id: canvas
        anchors.fill: parent
        rotation: 0
        antialiasing: true

        RotationAnimation on rotation {
            from: 0
            to: 360
            duration: AWTheme.data["motion.normal"] * 5
            loops: Animation.Infinite
            running: root.visible
        }

        onPaint: {
            var ctx = getContext("2d")
            ctx.reset()
            ctx.lineWidth = 2
            ctx.strokeStyle = AWTheme.data["interactive.default"]
            ctx.lineCap = "round"
            ctx.beginPath()
            ctx.arc(width / 2, height / 2, width / 2 - ctx.lineWidth / 2 - 1, -Math.PI / 2, Math.PI * 0.75)
            ctx.stroke()
        }
    }
}
