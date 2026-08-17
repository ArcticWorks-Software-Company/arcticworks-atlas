import QtQuick
import QtQuick.Controls

// Sidebar navigation row: icon + label + optional badge, active indicator.
ItemDelegate {
    id: root

    property string iconName: ""
    property bool active: false
    property bool collapsed: false
    property string badgeText: ""
    default property alias badge: badgeSlot.data

    height: AWTheme.data["nav.item.height"]
    hoverEnabled: true

    signal clicked()

    contentItem: Item {
        AWIcon {
            id: icon
            name: root.iconName
            size: AWTheme.data["icon.size.navigation"]
            tint: root.active || root.hovered ? AWTheme.data["nav.item.hover-text"]
                                              : AWTheme.data["nav.item.text"]
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: root.collapsed ? parent.horizontalCenter : undefined
            anchors.left: root.collapsed ? undefined : parent.left
            anchors.leftMargin: root.collapsed ? 0 : AWTheme.data["nav.item.padding-x"]
        }

        Text {
            id: label
            visible: !root.collapsed
            text: root.text
            color: root.active ? AWTheme.data["nav.item.active-text"]
                               : root.hovered ? AWTheme.data["nav.item.hover-text"]
                               : AWTheme.data["nav.item.text"]
            font.pixelSize: AWTheme.data["nav.item.font-size"]
            font.weight: AWTheme.data["nav.item.font-weight"]
            anchors.left: icon.right
            anchors.leftMargin: AWTheme.data["nav.item.gap"]
            anchors.verticalCenter: parent.verticalCenter
            elide: Text.ElideRight
            width: Math.max(0, parent.width - icon.width - AWTheme.data["nav.item.gap"]
                            - AWTheme.data["nav.item.padding-x"] * 2 - (badgeSlot.visible ? badgeSlot.width + AWTheme.data["space.1"] : 0))
        }

        Item {
            id: badgeSlot
            visible: false
            anchors.right: parent.right
            anchors.rightMargin: AWTheme.data["nav.item.padding-x"]
            anchors.verticalCenter: parent.verticalCenter
        }
    }

    background: Item {
        Rectangle {
            anchors.fill: parent
            radius: AWTheme.data["nav.item.radius"]
            color: root.active ? AWTheme.data["nav.item.active-background"]
                               : root.hovered ? AWTheme.data["nav.item.hover-background"] : "transparent"
            Behavior on color { ColorAnimation { duration: AWTheme.data["motion.fast"]; easing.type: Easing.OutCubic } }
        }

        Rectangle {
            visible: root.active && !root.collapsed
            width: 2
            radius: 1
            height: parent.height - 2 * AWTheme.data["space.1"]
            anchors.left: parent.left
            anchors.leftMargin: -AWTheme.data["space.0-5"]
            anchors.verticalCenter: parent.verticalCenter
            color: AWTheme.data["nav.item.active-indicator"]
        }

        Rectangle {
            anchors.fill: parent
            anchors.margins: -2
            radius: AWTheme.data["nav.item.radius"] + 2
            color: "transparent"
            border.width: 2
            border.color: AWTheme.data["focus.ring"]
            visible: root.visualFocus
        }
    }

    onHoveredChanged: {
        if (hovered && collapsed && text !== "")
            AWTooltip.show(text, root)
        else
            AWTooltip.hide()
    }

    onClicked: root.clicked()
}
