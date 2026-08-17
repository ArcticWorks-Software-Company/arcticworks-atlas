import QtQuick
import QtQuick.Controls

// Checkbox: token-driven box, check glyph and label.
CheckBox {
    id: root

    property real boxSize: AWTheme.data["control.height"] * 0.55

    hoverEnabled: true
    opacity: enabled ? 1.0 : AWTheme.data["opacity.disabled"]

    indicator: Rectangle {
        implicitWidth: boxSize
        implicitHeight: boxSize
        anchors.verticalCenter: parent.verticalCenter
        radius: AWTheme.data["radius.sm"]
        color: root.checked ? AWTheme.data["interactive.primary"]
                            : root.hovered ? AWTheme.data["surface.hover"] : "transparent"
        border.width: 1
        border.color: root.checked ? AWTheme.data["interactive.primary"]
                                   : root.visualFocus ? AWTheme.data["focus.ring"]
                                   : root.hovered ? AWTheme.data["border.strong"]
                                   : AWTheme.data["border.strong"]

        AWIcon {
            visible: root.checked
            name: "check"
            size: boxSize * 0.7
            tint: AWTheme.data["text.on-accent"]
            anchors.centerIn: parent
        }

        Behavior on color { ColorAnimation { duration: AWTheme.data["motion.fast"]; easing.type: Easing.OutCubic } }
    }

    contentItem: Text {
        leftPadding: AWTheme.data["space.2"]
        rightPadding: AWTheme.data["space.2"]
        text: root.text
        color: root.enabled ? AWTheme.data["text.primary"] : AWTheme.data["text.disabled"]
        font.pixelSize: AWTheme.data["typography.size.sm"]
        verticalAlignment: Text.AlignVCenter
    }
}
