import QtQuick

// Soft shadow approximation for Qt 6.4 (no QtQuick.Effects). Consumes the
// shadow.* tokens (a single {offsetX,offsetY,blur,spread,color} object, an
// array of such layers, or the string "none").
Item {
    id: root

    property var spec: null
    property real radius: 0

    function layers() {
        if (spec === null || spec === undefined || spec === "none")
            return []
        return Array.isArray(spec) ? spec : [spec]
    }

    function colorWithAlpha(color, alphaFactor) {
        var m = /^rgba\(\s*(\d+),\s*(\d+),\s*(\d+),\s*([\d.]+)\s*\)$/.exec(color)
        if (m)
            return "rgba(" + m[1] + "," + m[2] + "," + m[3] + "," +
                   (parseFloat(m[4]) * alphaFactor).toFixed(3) + ")"
        var h = /^#([0-9a-fA-F]{6})([0-9a-fA-F]{2})?$/.exec(color)
        if (h) {
            var r = parseInt(h[1].substring(0, 2), 16)
            var g = parseInt(h[1].substring(2, 4), 16)
            var b = parseInt(h[1].substring(4, 6), 16)
            var a = h[2] !== undefined ? parseInt(h[2], 16) / 255 : 1.0
            return "rgba(" + r + "," + g + "," + b + "," + (a * alphaFactor).toFixed(3) + ")"
        }
        return Qt.rgba(0, 0, 0, 0.1 * alphaFactor)
    }

    Repeater {
        model: root.layers()

        Item {
            required property var modelData
            property var layer: modelData
            property real expansion: Math.max(0, layer.blur || 0)
            width: root.width
            height: root.height
            x: layer.offsetX !== undefined ? layer.offsetX : 0
            y: layer.offsetY !== undefined ? layer.offsetY : 0

            // Three stacked translucent rectangles approximate the blur falloff.
            Repeater {
                model: 3
                Rectangle {
                    property int step: index
                    readonly property real k: [0.33, 0.66, 1.0][step]
                    anchors.fill: parent
                    anchors.margins: -parent.expansion * k
                    radius: root.radius + parent.expansion * k
                    color: root.colorWithAlpha(parent.layer.color, [0.35, 0.22, 0.12][step])
                    border.width: 0
                }
            }
        }
    }
}
