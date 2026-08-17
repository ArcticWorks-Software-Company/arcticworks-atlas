import QtQuick

// Thin separator inside menus.
Item {
    implicitHeight: AWTheme.data["space.3"]

    Rectangle {
        anchors.verticalCenter: parent.verticalCenter
        width: parent.width
        height: 1
        color: AWTheme.data["border.subtle"]
    }
}
