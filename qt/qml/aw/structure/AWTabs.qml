import QtQuick
import QtQuick.Controls

// Horizontal tab bar with animated underline indicator.
TabBar {
    id: root

    implicitHeight: AWTheme.data["tabs.height"]
    spacing: AWTheme.data["tabs.gap"]

    background: Rectangle {
        color: "transparent"
    }

    delegate: TabButton {
        id: tabButton

        required property var modelData

        text: tabButton.modelData.text !== undefined ? tabButton.modelData.text : ""
        height: AWTheme.data["tabs.height"]
        hoverEnabled: true
        opacity: enabled ? 1.0 : AWTheme.data["opacity.disabled"]

        contentItem: Text {
            text: tabButton.text
            color: !tabButton.enabled ? AWTheme.data["tabs.disabled-text"]
                                      : tabButton.checked || tabButton.hovered ? AWTheme.data["tabs.text-active"]
                                      : AWTheme.data["tabs.text"]
            font.pixelSize: AWTheme.data["typography.size.sm"]
            font.weight: AWTheme.data["typography.weight.medium"]
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }

        background: Item {
            Rectangle {
                anchors.fill: parent
                radius: AWTheme.data["tabs.radius"]
                color: tabButton.hovered && !tabButton.checked ? AWTheme.data["tabs.hover-background"] : "transparent"
                Behavior on color { ColorAnimation { duration: AWTheme.data["motion.fast"]; easing.type: Easing.OutCubic } }
            }

            Rectangle {
                anchors.bottom: parent.bottom
                anchors.horizontalCenter: parent.horizontalCenter
                width: tabButton.contentItem.implicitWidth
                height: AWTheme.data["tabs.active-indicator-width"]
                radius: height / 2
                color: AWTheme.data["tabs.active-indicator"]
                visible: tabButton.checked
                Behavior on width { NumberAnimation { duration: AWTheme.data["motion.fast"]; easing.type: Easing.OutCubic } }
            }

            Rectangle {
                anchors.fill: parent
                anchors.margins: -2
                radius: AWTheme.data["tabs.radius"] + 2
                color: "transparent"
                border.width: 2
                border.color: AWTheme.data["focus.ring"]
                visible: tabButton.visualFocus
            }
        }
    }
}
