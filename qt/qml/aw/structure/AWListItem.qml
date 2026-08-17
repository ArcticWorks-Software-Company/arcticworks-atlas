import QtQuick
import QtQuick.Controls

// Standard list row: icon + text + subtitle + trailing slot, hover/selected states.
ItemDelegate {
    id: root

    property string iconName: ""
    property string subtitle: ""
    property bool selected: false
    property alias trailing: trailingSlot.data

    height: AWTheme.data["control.height"]
    hoverEnabled: true

    signal clicked()

    contentItem: Row {
        spacing: AWTheme.data["space.2"]
        leftPadding: AWTheme.data["control.padding-x"]
        rightPadding: AWTheme.data["control.padding-x"]

        AWIcon {
            visible: iconName !== ""
            name: iconName
            size: AWTheme.data["icon.size.default"]
            tint: AWTheme.data["text.secondary"]
            anchors.verticalCenter: parent.verticalCenter
        }

        Column {
            width: Math.max(0, parent.width - AWTheme.data["space.12"])
            anchors.verticalCenter: parent.verticalCenter
            spacing: AWTheme.data["space.0-5"]

            Text {
                width: parent.width
                text: root.text
                color: AWTheme.data["text.primary"]
                font.pixelSize: AWTheme.data["typography.size.sm"]
                elide: Text.ElideRight
            }

            Text {
                visible: subtitle !== ""
                width: parent.width
                text: root.subtitle
                color: AWTheme.data["text.secondary"]
                font.pixelSize: AWTheme.data["typography.size.xs"]
                elide: Text.ElideRight
            }
        }

        Item {
            id: trailingSlot
            width: 0
            height: parent.height
            visible: false
        }
    }

    background: Item {
        Rectangle {
            anchors.fill: parent
            color: root.selected ? AWTheme.data["interactive.subtle"]
                                 : root.hovered ? AWTheme.data["surface.hover"] : "transparent"
            Behavior on color { ColorAnimation { duration: AWTheme.data["motion.fast"]; easing.type: Easing.OutCubic } }
        }

        Rectangle {
            visible: root.selected
            width: 2
            height: parent.height - 2 * AWTheme.data["space.1"]
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            radius: 1
            color: AWTheme.data["interactive.default"]
        }
    }

    onClicked: root.clicked()
}
