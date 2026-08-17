import QtQuick

// Left navigation rail hosting AWNavSection children.
Item {
    id: root

    property bool collapsed: false
    default property alias content: column.data

    implicitWidth: collapsed ? AWTheme.data["nav.collapsed-width"] : AWTheme.data["nav.width"]

    clip: true

    Behavior on implicitWidth { NumberAnimation { duration: AWTheme.data["motion.normal"]; easing.type: Easing.OutCubic } }

    Rectangle {
        anchors.fill: parent
        color: AWTheme.data["surface.sidebar"]
    }

    Rectangle {
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        width: 1
        color: AWTheme.data["border.subtle"]
    }

    Column {
        id: column
        anchors.fill: parent
        anchors.margins: AWTheme.data["space.3"]
        spacing: AWTheme.data["space.1"]
    }
}
