import QtQuick

import "../overlays/AWShadow.qml" as ShadowImport

// Container card with hover/selected borders and soft shadow.
Item {
    id: root

    property bool selected: false
    property bool hoverEnabled: true
    property alias contentItem: content.data
    default property alias data: content.data

    readonly property bool hovered: mouse.containsMouse

    implicitWidth: 240
    implicitHeight: 160

    Item {
        anchors.fill: parent

        ShadowImport.AWShadow {
            anchors.fill: parent
            spec: AWTheme.data["card.shadow"]
            radius: AWTheme.data["card.radius"]
        }

        Rectangle {
            id: bg
            anchors.fill: parent
            radius: AWTheme.data["card.radius"]
            color: AWTheme.data["card.background"]
            border.width: 1
            border.color: root.selected ? AWTheme.data["card.border-selected"]
                                        : root.hovered ? AWTheme.data["card.border-hover"]
                                        : AWTheme.data["card.border"]
            Behavior on border.color { ColorAnimation { duration: AWTheme.data["motion.fast"]; easing.type: Easing.OutCubic } }
        }

        Item {
            id: content
            anchors.fill: parent
            anchors.margins: AWTheme.data["card.padding"]
        }
    }

    MouseArea {
        id: mouse
        anchors.fill: parent
        hoverEnabled: root.hoverEnabled
        acceptedButtons: Qt.NoButton
        propagateComposedEvents: false
    }
}
