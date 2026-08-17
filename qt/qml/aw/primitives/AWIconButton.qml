import QtQuick
import QtQuick.Controls

// Square icon-only button. Uses control/radius tokens; hover shows surface.hover.
Button {
    id: root

    property string iconName: ""
    property string tooltipText: ""

    implicitWidth: AWTheme.data["control.height"]
    implicitHeight: AWTheme.data["control.height"]

    hoverEnabled: true
    opacity: enabled ? 1.0 : AWTheme.data["opacity.disabled"]

    contentItem: AWIcon {
        name: root.iconName
        size: AWTheme.data["icon.size.default"]
        tint: root.enabled ? AWTheme.data["text.primary"] : AWTheme.data["text.disabled"]
        anchors.centerIn: parent
    }

    background: Item {
        Rectangle {
            anchors.fill: parent
            anchors.margins: -2
            radius: AWTheme.data["radius.md"] + 2
            color: "transparent"
            border.width: 2
            border.color: AWTheme.data["focus.ring"]
            visible: root.visualFocus
        }

        Rectangle {
            anchors.fill: parent
            radius: AWTheme.data["radius.md"]
            color: (root.hovered || root.down) ? AWTheme.data["surface.hover"] : "transparent"
            Behavior on color { ColorAnimation { duration: AWTheme.data["motion.fast"]; easing.type: Easing.OutCubic } }
        }
    }

    onHoveredChanged: {
        if (hovered && tooltipText !== "")
            AWTooltip.show(tooltipText, root)
        else
            AWTooltip.hide()
    }
}
