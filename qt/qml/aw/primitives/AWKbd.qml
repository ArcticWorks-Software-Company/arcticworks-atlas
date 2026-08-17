import QtQuick

// Keycap label for shortcuts.
Item {
    id: root

    property string text: ""

    implicitWidth: label.implicitWidth + 2 * AWTheme.data["space.2"]
    implicitHeight: label.implicitHeight + AWTheme.data["space.1"]

    Rectangle {
        anchors.fill: parent
        radius: AWTheme.data["radius.sm"]
        color: AWTheme.data["surface.2"]
        border.width: 1
        border.color: AWTheme.data["border.default"]
    }

    Text {
        id: label
        anchors.centerIn: parent
        text: root.text
        color: AWTheme.data["text.secondary"]
        font.pixelSize: AWTheme.data["typography.size.xs"]
    }
}
