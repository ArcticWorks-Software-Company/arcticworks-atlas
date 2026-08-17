import QtQuick
import "icons/IconSet.js" as IconSet

// ArcticWorks icon: 24px viewBox grid, stroke-only, 1.75px stroke, rounded
// caps/joins. Tinted at runtime from the token palette.
Image {
    id: root

    property string name: ""
    property color tint: AWTheme.data["text.primary"]
    property real size: AWTheme.data["icon.size.default"]

    width: size
    height: size
    smooth: true
    asynchronous: false

    sourceSize: Qt.size(Math.max(2, Math.round(size * Screen.devicePixelRatio)),
                        Math.max(2, Math.round(size * Screen.devicePixelRatio)))
    source: "data:image/svg+xml;base64," + IconSet.source(name, tint)
}
