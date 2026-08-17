import QtQuick
import QtQuick.Controls

// Menu item: shared row style for dropdown/context menus.
MenuItem {
    id: root

    property string iconName: ""
    property string shortcutText: ""

    height: AWTheme.data["control.height"]
    hoverEnabled: true

    contentItem: Row {
        spacing: AWTheme.data["space.2"]
        leftPadding: AWTheme.data["space.3"]
        rightPadding: AWTheme.data["space.3"]

        AWIcon {
            visible: iconName !== ""
            name: iconName
            size: AWTheme.data["icon.size.default"]
            tint: AWTheme.data["text.secondary"]
            anchors.verticalCenter: parent.verticalCenter
        }

        Text {
            text: root.text
            color: root.enabled ? AWTheme.data["text.primary"] : AWTheme.data["text.disabled"]
            font.pixelSize: AWTheme.data["typography.size.sm"]
            anchors.verticalCenter: parent.verticalCenter
        }

        Item {
            width: parent.width - AWTheme.data["space.6"]
            height: 1
            anchors.verticalCenter: parent.verticalCenter
            visible: false
        }

        Text {
            visible: shortcutText !== ""
            text: root.shortcutText
            color: AWTheme.data["text.tertiary"]
            font.pixelSize: AWTheme.data["typography.size.xs"]
            anchors.verticalCenter: parent.verticalCenter
        }
    }

    background: Rectangle {
        radius: AWTheme.data["radius.sm"]
        color: root.highlighted ? AWTheme.data["surface.hover"] : "transparent"
        Behavior on color { ColorAnimation { duration: AWTheme.data["motion.fast"]; easing.type: Easing.OutCubic } }
    }
}
